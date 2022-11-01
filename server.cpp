#include "server.hpp"

Server::Server()
{
    setup(DEFAULT_PORT);
}

Server::Server(int port)
{
    setup(port);
}

Server::Server(const Server& orig)
{
}

Server::~Server()
{
    close(mastersocket);
}


void Server::setup(int port)
{
    if ((mastersocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)) <0){
       perror("[SERVER] [ERROR] can't create a socket");
       exit(EXIT_FAILURE);
    }
        
    // Clear an masterfds and tempfds
    FD_ZERO(&masterfds);
    FD_ZERO(&tempfds);

    // Sets the buffer to the specified character.
    memset(&con_info, 0 , sizeof(con_info)); 

    // Socket properties
    uint16_t hostPort = htons(port);
    con_info.sin_port = hostPort;
    con_info.sin_family = AF_INET;
    con_info.sin_addr.s_addr = htons(INADDR_ANY);
    // zero the input buffer before use to avoid random data appearing in first receives
    bzero(input_buffer,INPUT_BUFFER_SIZE);

          
}

void Server::initializeSocket()
{   
    /* setsockopt - sets flags on the socket
     *
     * mastersocket - socket
     * SOL_SOCKET - manipulating flags at the socket level
     * SO_REUSEADDR - allows a socket to forcibly bind to a port used by another socket
     * optval - access to flag values(set to TRUE)
     */
    int optval = 1;
    if((setsockopt(mastersocket, SOL_SOCKET, SO_REUSEADDR,&optval, sizeof(optval))) < 0)
        {
            perror("[SERVER] [ERROR] setsockopt() failed");
            close(mastersocket);
        }
}

void Server::bindSocket()
{   
    // Binding a previously created socket to a local address
    if ((bind(mastersocket, (struct sockaddr*) &con_info, sizeof (struct sockaddr))) < 0){
        perror("[SERVER] [ERROR] bind() failed");
        exit(EXIT_FAILURE);
    }
    // insert the master socket file-descriptor into the master fd-set
    FD_SET(mastersocket, &masterfds);
    // set the current known maximum file descriptor count;
    maxfd = mastersocket;
}

void Server::startListen()
{
    /* Prepare a socket for peeling requests 
     * and allows you to determine the size 
     * of the queue of requests, 
     * waiting for the execution of the accept() function.
     */

    int listen_ret = listen(mastersocket, 3);
    if (listen_ret < 0) {
        perror("[SERVER] [ERROR] listen() failed");
    }

}

void Server::handleNewConnection()
{   
    socklen_t addrlen = sizeof(client_addr);
    tempsocket = accept(mastersocket, (struct sockaddr*) &client_addr, &addrlen);
        
    if (tempsocket < 0) {
            perror("[SERVER] [ERROR] accept() failed");
            exit(EXIT_FAILURE);

    } else {
            // Add a descriptor to an masterfds
            FD_SET(tempsocket, &masterfds);
            // increment the maximum known file descriptor (select() needs it)
            if (tempsocket > maxfd) {
                    maxfd = tempsocket;
            }
    }
}

void Server::recvInputFromExisting(int fd)
{   
    int nbytesrecv = recv(fd, input_buffer, INPUT_BUFFER_SIZE, 0);
    
    if (nbytesrecv <= 0)
    {
        /* If the client does not send any messages,
         * it means that it has ended its work
         * and we close the connection 
         */
        if (nbytesrecv == 0)
        {
            // close connection to client
            close(fd);
            // clear the client fd from fd set
            FD_CLR(fd, &masterfds);
            return;
        } 
        else 
        {
            perror("[SERVER] [ERROR] recv() failed");
        }
        close(fd); // close connection to client
        FD_CLR(fd, &masterfds); // clear the client fd from fd set
        return;
    }
    bzero(&input_buffer,INPUT_BUFFER_SIZE); // clear input buffer
}


void Server::loop(){
    // copy fd_set for select()
    tempfds = masterfds;

    /* select call monitors activity on a set of sockets
     * looking for sockets ready for reading, writing and exepting
     *
     * maxfd+1 -  number of socket descriptors to be checked.
     * This value should be one greater than the greatest 
     * number of sockets to be checked.
     *
     * &tempfds - Points to a bit set of descriptors to check for reading.
     */

    if(select(maxfd+1,&tempfds,NULL,NULL,NULL)<0)
    {
        perror("[SERVER] [ERROR] select() failed");
        close(mastersocket);
    }

    // loop the fd_set and check which socket has interactions available
    for (int i = 0; i <= maxfd; i++){
    // If something happened on the master socket , then its an incoming connection  
        if(FD_ISSET(i,&tempfds)){
            if(mastersocket == i){
                // new connection on master socket
                handleNewConnection();
            }
            else
            {
                // exisiting connection has new data
                recvInputFromExisting(i);

            }
        }
    }
}

void Server::init()
{
    initializeSocket();
    bindSocket();
    startListen();
}



