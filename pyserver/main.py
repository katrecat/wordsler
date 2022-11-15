#!/usr/bin/python

import socket
import sys
import helpers
from queue import Queue
from threading import Lock
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit

async_mode = None

app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()

HOST = "127.0.0.1"
PORT = 1234
queue = Queue()


def connect_to_server():
    """
    Handles communication with cppserver.
    Sends all the messages presented in queue to the cpp server.
    """
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((HOST, PORT))
        while True:
            socketio.sleep(1)
            while not(queue.empty()):
                msg = queue.get()
                s.sendall(msg)
    return


@socketio.on('rcv_message')
def rcv_message(data):
    """
    An event to recive message from client.
    """

    # FIXME: We should also include message length so cpp server could
    # verify if message was sent completely
    msg = helpers.MessageType.DATA.to_bytes() + bytes(request.sid, 'utf-8')
    msg += bytes(str(data), 'utf-8')
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
