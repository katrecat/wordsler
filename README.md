# Wordsler

## Overview
Wordsler - is an online service to improve your typing memory in Italian language by competiting with other players.

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
TODO

## Docker image
TODO
