TARGET = client server
CC = g++  
CFLAGS_OBJ =  -Wall -Wextra -pedantic -g 

normal: $(TARGET)

client: client.cpp 
	$(CC) $(CFLAGS_OBJ) client.cpp -o client
server: main.cpp server.cpp
	$(CC) $(CFLAGS_OBJ) server.cpp main.cpp -o server

clean:
	$(RM) $(TARGET)

