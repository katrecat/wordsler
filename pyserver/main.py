#!/usr/bin/python

from threading import Lock
from flask import Flask, render_template, request
from flask_socketio import SocketIO, emit

async_mode = None

app = Flask(__name__)
socketio = SocketIO(app, async_mode=async_mode)
thread = None
thread_lock = Lock()


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


@socketio.on('rcv_message')
def rcv_message(data):
    """
    An event to recive message from client.
    """

    # FIXME: We should verify message, format it
    # and eventually forward formatted message to the cpp server.
    # All the further business logic should be holded by cpp server.
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

    global thread
    with thread_lock:
        if thread is None:
            thread = socketio.start_background_task(background_thread)

    # FIXME: on client connection we want to pass the data to cpp server
    # instead of storing it on python one
    print(f"[SERVER]: Client {request.sid} connected")

    # Do business logic on new connection
    emit('global_counter', {'counter': 0})
    return


@socketio.event
def disconnect():
    """
    Handles client disconnection.
    """

    print(f"[SERVER]: Client {request.sid} disconnected.")
    # Do business logic on client disconnection
    # FIXME: On client disconnect we want to inform cpp server about it
    return


if __name__ == '__main__':
    socketio.run(app)
