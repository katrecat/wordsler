#!/usr/bin/python

import socket
import sys
import helpers
import select
from queue import Queue
from threading import Lock
from flask import Flask, render_template, request
from flask_socketio import SocketIO

async_mode = None

app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

HOST = "127.0.0.1"
PORT = 1234
MSGID_LEN = 2
WORDS = []
PLAYERS = []
queue = Queue()


def receive_words(socket):
    """
    Fetches the word list from cpp server
    """
    global WORDS
    WORDS = []
    amount = socket.recv(MSGID_LEN)
    amount = int.from_bytes(amount, 'little', signed=False)
    for i in range(amount):
        wordlen = socket.recv(MSGID_LEN)
        wordlen = int.from_bytes(wordlen, 'little', signed=False)
        word = socket.recv(wordlen)
        word = helpers.wordFromBytes(word)
        WORDS.append(word)
    print(WORDS)
    return


def receive_players(socket):
    """
    Fetches the players list from cpp server
    """
    global PLAYERS
    PLAYERS = []
    amount = socket.recv(MSGID_LEN)
    amount = int.from_bytes(amount, 'little', signed=False)
    for i in range(amount):
        usernamelen = socket.recv(MSGID_LEN)
        usernamelen = int.from_bytes(usernamelen, 'little', signed=False)
        username = socket.recv(usernamelen)
        username = helpers.wordFromBytes(username)
        scorelen = socket.recv(MSGID_LEN)
        scorelen = int.from_bytes(scorelen, 'little', signed=False)
        score = socket.recv(scorelen)
        score = int.from_bytes(score, 'little', signed=False)
        PLAYERS.append((username, score))
    print(PLAYERS)
    return


def connect_to_server():
    """
    Handles communication with cppserver.
    Sends all the messages presented in queue to the cpp server.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        while True:
            socketio.sleep(1)
            read_sockets, _, _ = select.select([s], [], [], 0)
            for sock in read_sockets:
                receive_words(sock)
                receive_players(sock)
            if not(queue.empty()):
                msg = queue.get()
                s.sendall(msg)
    return


@socketio.on('rcv_msg')
def rcv_msg(data):
    """
    An event to recive message from client.
    """

    data = str(data)[:255]
    msg_len = len(data).to_bytes(2, 'little', signed=True)
    msg_typ = helpers.MessageType.DATA.to_bytes() + bytes(request.sid, 'utf-8')
    msg = msg_typ + msg_len + bytes(data, 'utf-8')
    queue.put(msg)
    return


@app.route("/")
def index():
    """
    Renders the index page.
    """
    return render_template('index.html', async_mode=socketio.async_mode)


@socketio.event
def connect():
    """
    Handles new user connections.

    Starts background task when first user is connected.
    On following users only executes business logic.
    """

    msg = helpers.MessageType.CONNECT.to_bytes() + bytes(request.sid, 'utf-8')
    queue.put(msg)
    return


@socketio.event
def disconnect():
    """
    Handles client disconnection.
    """

    msg = helpers.MessageType.DISCONNECT.to_bytes() + bytes(request.sid,
                                                            'utf-8')
    queue.put(msg)
    return


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Too few arguments. Use {sys.argv[0]} <port> instead")
        raise(TypeError)

    with thread_lock:
        thread = socketio.start_background_task(connect_to_server)
    socketio.run(app, port=int(sys.argv[1]))
