#!/usr/bin/python

import socket
import sys
import helpers
import select
from queue import Queue
from threading import Lock
from flask import Flask, render_template, request
from flask_socketio import SocketIO
from random import randint

async_mode = None

app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

HOST = "127.0.0.1"
PORT = 1234
MSGID_LEN = 2
WORDS = []
SEND = []
PLAYERS = []
queue = Queue()
SIZE = 512


def dead():
    print("[SERVER]: [ERROR]: [FATAL] Couldn't connect to CPP server!")
    while True:
        socketio.emit('kill-game-event', {'data': "True"})
        socketio.sleep(0.0001)
    return


def update_canvas():
    socketio.emit('canvas_event', {'data': SEND})
    return


def update_players():
    global PLAYERS
    PLAYERS = sorted(PLAYERS, key=lambda x: x[1], reverse=True)
    socketio.emit('leaderboard-event', {'data': PLAYERS})
    return


def receive_words(socket, amount):
    """
    Fetches the word list from cpp server
    """
    global WORDS, SEND
    WORDS = []
    amount = int.from_bytes(amount, 'little', signed=False)
    for i in range(amount):
        wordlen = socket.recv(MSGID_LEN)
        wordlen = int.from_bytes(wordlen, 'little', signed=False)
        word = socket.recv(wordlen)
        word = helpers.wordFromBytes(word)
        WORDS.append(word)

    if not(SEND):
        SEND = [[x, randint(0, SIZE - 8 * len(x)), randint(12, SIZE)]
                for x in WORDS]
    else:
        for i in range(len(WORDS)):
            if SEND[i][0] != WORDS[i]:
                if i != (len(SEND)-1):
                    SEND = SEND[:i] + SEND[i+1:]
                else:
                    SEND = SEND[:i]
                x = randint(0, SIZE - 8 * len(WORDS[-1]))
                y = randint(12, SIZE)
                to_add = [WORDS[-1], x, y]
                SEND.append(to_add)
                break
    return


def receive_players(socket, amount):
    """
    Fetches the players list from cpp server
    """
    global PLAYERS
    PLAYERS = []
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
        try:
            s.connect((HOST, PORT))
        except ConnectionRefusedError:
            dead()
        while True:
            socketio.sleep(0.0001)
            read_sockets, _, _ = select.select([s], [], [], 0)
            for sock in read_sockets:
                amount = sock.recv(MSGID_LEN)
                if len(amount) == 0:
                    dead()
                receive_words(sock, amount)
                amount = sock.recv(MSGID_LEN)
                if len(amount) == 0:
                    dead()
                receive_players(sock, amount)
                update_canvas()
                update_players()
            if not(queue.empty()):
                msg = queue.get()
                s.sendall(msg)
    return


@socketio.on('get-name')
def rcv_name(data):
    """
    An event to recive username from client.
    """

    username = str(data)[:20]
    for user, _ in PLAYERS:
        if username == user:
            socketio.emit('username-event', {'data': 'ERROR'}, to=request.sid)
            return
    msg = helpers.MessageType.USERNAME.to_bytes() + bytes(request.sid, 'utf-8')
    msg_len = len(username).to_bytes(2, 'little', signed=False)
    msg += msg_len + bytes(username, 'utf-8')
    queue.put(msg)
    socketio.emit('username-event', {'data': 'OK'}, to=request.sid)
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
    try:
        socketio.run(app, port=int(sys.argv[1]))
    except OSError:
        print("[SERVER]: [ERROR]: [FATAL] Address already in use")
