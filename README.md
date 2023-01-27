# Wordsler

## Overview
Wordsler - is an online service to improve your typing memory in Italian language by competiting with other players.
Before the game you should log in using name, after you will see the given words. Your goal is to write any word from given to the textbox as faster as you can. If you can do it before other users, you will have 1 point. After writing correct word, program update list of words and add one more.

## How to build
The project consists of two servers: one is written in `C++` and computes the game, the other is written in `Python` and responsible for transfering data between `user` and `C++` server.
In order to build the `C++` run the `make` command:
```
git clone https://github.com/katrecat/wordsler.git
cd wordsler
make
```

The resulting `server` binary will be placed in primary project folder.

## How to run
Once the `C++` binary was built, the server can be started to listen.
Run the following command:
```
./server <port>
```

In order to start python server you need to use following command:
```
cd pyserver
python main.py <ip-to-serve> <port> <cpp-server-ip> <cpp-server-port>
```

As python server is running, you can obtain connection to it by openning the `htpp://<ip-to-serve>:<port>` in your browser.

## Dependencies
List:
* make
* python3
* g++/gcc

The python dependencies are present under the `requirements.txt` file.

## The communication protocol
This projekt represents an implementation of a server in C++ language, which uses the TCP protocol for communication with another server written in Python. The Server class contains methods for creating a socket, setting its options, binding, listening, and handling connections. The setup() method is called by the constructors to set the port and create the socket, and the initializeSocket() method sets the SO_REUSEADDR flag, which allows for rebinding the port. The bindSocket() method binds the socket to the local address, and the startListen() method prepares the socket for receiving requests. The handleNewConnection() method accepts new connections and calls the connectionCallback function for the new connection. The recvInputFromExisting() method receives data from existing connections and closes the connection if the client is not sending more messages.

The Python server is based on the Flask and Flask-SocketIO framework, which allows for creating web applications and communicating with clients through WebSockets. The C++ server is responsible for game logic, and the Python server handles communication with clients and updating information on the screen. Python server code contains several functions that serve to retrieve and update information between servers and also between the server and clients. The "dead" function sends a signal to all users that the server is dead. The "update_canvas" function sends data on words to clients for drawing on the screen. The "update_players" function sends a list of players and their scores to clients. The "receive_words" and "receive_players" functions retrieve words and a list of players from the C++ server.
