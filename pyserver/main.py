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


def background_thread():
    """
    Sends counter value to 'global_counter' event every three seconds.
    """
    count = 0
    while True:
        socketio.emit('global_counter', {'counter': count})
        socketio.sleep(3)
        count += 1
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
            while not(queue.empty()):
                msg = queue.get()
                print(msg)
                s.sendall(msg)
    return


@socketio.on('rcv_message')
def rcv_message(data):
    """
    An event to recive message from client.
    """

    # FIXME: We should verify message, format it
    # and eventually forward formatted message to the cpp server.
    # All the further business logic should be holded by cpp server.
    queue.put(f"Client {request.sid} says: {str(data)}")
    print(f"[SERVER]: Client {request.sid} sent: {str(data)}")
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

    # FIXME: on client connection we want to pass the data to cpp server
    # instead of storing it on python one
    print(f"[SERVER]: Client {request.sid} connected")
    msg = helpers.MessageType.CONNECT.to_bytes() + bytes(request.sid, 'utf-8')
    queue.put(msg)

    # Do business logic on new connection
    return


@socketio.event
def disconnect():
    """
    Handles client disconnection.
    """

    print(f"[SERVER]: Client {request.sid} disconnected.")
    msg = helpers.MessageType.DISCONNECT.to_bytes() + bytes(request.sid,
                                                            'utf-8')
    queue.put(msg)
    # Do business logic on client disconnection
    # FIXME: On client disconnect we want to inform cpp server about it
    return


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(f"Too few arguments. Use {sys.argv[0]} <port> instead")
        raise(TypeError)

    with thread_lock:
        thread = socketio.start_background_task(connect_to_server)
    socketio.run(app, port=int(sys.argv[1]))
