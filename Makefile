main: server client

server:
	g++ server.cpp -Wall -Wextra -pedantic -o server
client:
	g++ client.cpp -Wall -Wextra -pedantic -o client
clean:
	rm server client
