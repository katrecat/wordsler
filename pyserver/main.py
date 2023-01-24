#!/usr/bin/python

import sys
import socket
import select
import helpers
from threading import Lock
from random import randint
from queue import Queue
from flask_socketio import SocketIO
from flask import Flask, render_template, request

async_mode = None

app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode, async_handlers=True)
thread = None
thread_lock = Lock()

DEBUG = True
HOST = "127.0.0.1"
PORT = 12345
CPPHOST = "127.0.0.1"
CPPPORT = 1234
MSGID_LEN = 2
PLAYERS = []
SEND = []
SIZE = 512
WORDS = []
queue = Queue()


def dead():
    """
    Sends signal to all the users that server is dead.
    """
    print("[SERVER]: [ERROR]: [FATAL] Couldn't connect to CPP server!")
    while True:
        socketio.emit('kill-game-event', {'data': "True"})
        socketio.sleep()
    return


def update_canvas():
    """
    Sends the words data to canvas to be drawn.
    """
    print(f'[SERVER]: [SEND WORDS]: {len(SEND)} words: {SEND}')
    socketio.emit('canvas_event', {'data': SEND})
    return


def update_players():
    """
    Sends the players list along with their scores to the clients.
    """
    global PLAYERS
    PLAYERS = sorted(PLAYERS, key=lambda x: x[1], reverse=True)
    socketio.emit('leaderboard-event', {'data': PLAYERS})
    return


def receive_words(socket, amount):
    """
    Fetches the word list from cpp server.
    """
    global WORDS, SEND
    WORDS = []

    # Get the total amount of words to be received
    amount = int.from_bytes(amount, 'little', signed=False)

    for i in range(amount):
        # Get the i-th word length
        wordlen = socket.recv(MSGID_LEN)
        wordlen = int.from_bytes(wordlen, 'little', signed=False)

        # Receive the word
        word = socket.recv(wordlen)
        word = helpers.wordFromBytes(word)
        WORDS.append(word)

    if not(SEND):
        SEND = []
        for x in WORDS:
            fontSize = 13 + randint(0, 17)
            cordX = randint(0, SIZE - round(len(x) * fontSize * 0.57))
            cordY = randint(fontSize * 2, SIZE - fontSize)
            SEND.append([x, cordX, cordY, fontSize])
    else:
        # Find the word got eleminated and get rid of it
        for i in range(len(WORDS)):
            if SEND[i][0] != WORDS[i]:
                if i != (len(SEND)-1):
                    SEND = SEND[:i] + SEND[i+1:]
                else:
                    SEND = SEND[:i]

                # Replace it with new word
                x = WORDS[-1]
                fontSize = 13 + randint(0, 17)
                cordX = randint(0, SIZE - round(len(x) * fontSize * 0.57))
                cordY = randint(fontSize * 2, SIZE - fontSize)
                to_add = [x, cordX, cordY, fontSize]
                SEND.append(to_add)
                break
    if DEBUG:
        print(f'[SERVER]: [UPDATE WORDS]: {len(WORDS)} words: {WORDS}')
    return


def receive_players(socket, amount):
    """
    Fetches the players list from cpp server.
    """
    global PLAYERS
    PLAYERS = []

    # Get amount of players
    amount = int.from_bytes(amount, 'little', signed=False)
    for i in range(amount):

        # Get the username length
        usernamelen = socket.recv(MSGID_LEN)
        usernamelen = int.from_bytes(usernamelen, 'little', signed=False)

        # Fetch the username
        username = socket.recv(usernamelen)
        username = helpers.wordFromBytes(username)

        # Fetch user score length
        scorelen = socket.recv(MSGID_LEN)
        scorelen = int.from_bytes(scorelen, 'little', signed=False)

        # Fetch the user's score
        score = socket.recv(scorelen)
        score = int.from_bytes(score, 'little', signed=False)
        PLAYERS.append((username, score))
    if DEBUG:
        print(f'[SERVER]: [UPDATE PLAYERS]: {len(PLAYERS)} players: {PLAYERS}')
    return


def connect_to_server():
    """
    Handles communication with cppserver.
    Sends all the messages presented in queue to the cpp server.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        try:
            s.connect((CPPHOST, CPPPORT))
        except ConnectionRefusedError:
            dead()
        if DEBUG:
            print('[SERVER]: [CONNECT TO CPP]: Successfully connected')
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
    An event that recives username from client and passes it to cpp server.
    """

    if DEBUG:
        print(f'[SERVER]: [USERNAME]: {request.sid} sent username: {data}')
    username = str(data)[:20]
    for user, _ in PLAYERS:
        if username == user:
            socketio.emit('username-event', {'data': 'ERROR'}, to=request.sid)
            return
    msg = helpers.MessageType.CONNECT.to_bytes() + bytes(request.sid, 'utf-8')
    queue.put(msg)
    msg_len = len(username).to_bytes(2, 'little', signed=False)
    msg = helpers.MessageType.USERNAME.to_bytes() + bytes(request.sid, 'utf-8')
    msg += msg_len + bytes(username, 'utf-8')
    queue.put(msg)
    socketio.emit('username-event', {'data': 'OK'}, to=request.sid)
    return


@socketio.on('rcv_msg')
def rcv_msg(data):
    """
    An event that recives word from client and passes it to cpp server.
    """

    if DEBUG:
        print(f'[SERVER]: [DATA]: {request.sid} sent \'{str(data)}\'')
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
    """

    # It was decided to display user's sid only when they set the proper name
    # That's why we only log the connection event here.
    # The "get-name" event is responsible for sending 'CONNECTED'
    # message to cpp server.
    if DEBUG:
        print(f'[SERVER]: [CONNECTED]: {request.sid} connected')
    return


@socketio.event
def disconnect():
    """
    Handles client disconnection.
    """

    if DEBUG:
        print(f'[SERVER]: [DISCONNECT]: {request.sid} disconnected')

    msg = helpers.MessageType.DISCONNECT.to_bytes() + bytes(request.sid,
                                                            'utf-8')
    queue.put(msg)
    return


def main():
    global HOST, PORT, CPPHOST, CPPPORT
    if len(sys.argv) == 3:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
    elif len(sys.argv) == 5:
        HOST = sys.argv[1]
        PORT = int(sys.argv[2])
        CPPHOST = sys.argv[3]
        CPPPORT = int(sys.argv[4])
    elif len(sys.argv) != 1:
        print(f"ERROR: Use {sys.argv[0]} <server-host> <server-port> <cpp-host> <cpp-port> instead")
        return

    global thread, thread_lock
    with thread_lock:
        thread = socketio.start_background_task(connect_to_server)
    try:
        socketio.run(app, host=HOST, port=PORT)
    except OSError:
        print("[SERVER]: [ERROR]: [FATAL] Address already in use")
    return


if __name__ == '__main__':
    main()
