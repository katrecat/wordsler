#ifndef SERVER_HPP
#define SERVER_HPP 

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
using namespace std;

#define INPUT_BUFFER_SIZE 128
#define DEFAULT_PORT 1234

class Server {
    public:
        Server();
        Server(int port);
        Server(const Server& orig);
        virtual ~Server();

        void init();
        void loop();

        void setConnectCallback(int (*newConnectCallback)(uint16_t fd));
        void setDisconnectCallback(void (*newDisconnectCallback)(uint16_t fd));
        void setInputCallback(void (*newInputCallback)(uint16_t fd, char *word));
         
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

        int (*connectionCallback)(uint16_t fd);
        void (*disconnectCallback)(uint16_t fd);
        void (*inputCallback)(uint16_t fd, char *word);

        //function prototypes
        void setup(int port);
        void initializeSocket();
        void bindSocket();
        void startListen();
        void handleNewConnection();
        void recvInputFromExisting(int fd);
};
#endif
