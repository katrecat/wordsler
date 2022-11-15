TARGET = server
CC = g++  
CFLAGS_OBJ = -Wall -Wextra -pedantic -g 

normal: $(TARGET)

server: main.cpp cppserver/server.cpp
	$(CC) $(CFLAGS_OBJ) callback/callback.cpp cppserver/server.cpp main.cpp -o server

clean:
	$(RM) $(TARGET)
