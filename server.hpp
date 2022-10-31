#ifndef SERVER_HPP
#define SERVER_HPP 

#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<fcntl.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<iostream> 
#include<arpa/inet.h>
using namespace std;

#define INPUT_BUFFER_SIZE 100 
#define DEFAULT_PORT 1234

class Server {
    public:
        void init();
        void loop();
         
    private:
        // fd_set file descriptor sets for use with FD_ macros        
        fd_set masterfds;
        fd_set tempfds;

        //maximum fd value, required for select()
        uint16_t maxfd;

        //socket file descriptors
        int mastersocket; //master socket which receives new connections
        int tempsocket; //temporary socket file descriptor which holds new clients
        
        //server socket details
        struct sockaddr_in con_info;
        //client connection data
        struct sockaddr_storage client_addr;
        
        //input buffer 
        char input_buffer[INPUT_BUFFER_SIZE];

        void (*newConnectionCallback) (uint16_t fd);
        void (*receiveCallback) (uint16_t fd, char *buffer);
        void (*disconnectCallback) (uint16_t fd);


        //function prototypes
        void setup(int port);
        void initializeSocket();
        void bindSocket();
        void startListen();
        void shutdown();
        void handleNewConnection();
        void recvInputFromExisting(int fd);
    
};

#endif
