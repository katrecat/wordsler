#!/usr/bin/python

from threading import Lock
from flask import Flask, render_template
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

    # Do business logic on new connection
    emit('global_counter', {'counter': 0})
    return


if __name__ == '__main__':
    socketio.run(app)
