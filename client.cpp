#include "server.hpp"
#include <unistd.h>

int main(int argc, char *argv[]) {
    fd_set serverfds;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 100;
    FD_ZERO(&serverfds);

    int client = socket(AF_INET, SOCK_STREAM, 0);
    
    if (argc < 2){
        fprintf(stderr, "Use %s <message> instead.\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    sockaddr_in con_info;
    con_info.sin_port = htons(DEFAULT_PORT);
    con_info.sin_family = AF_INET;
    con_info.sin_addr.s_addr = INADDR_ANY;
    connect(client, (sockaddr *)&con_info, sizeof(con_info));

    FD_SET(client, &serverfds);
    int maxfd = client;
    char buf[32];
    send(client, argv[1], 32*sizeof(char), 0);
    while(1)
    {
        send(client, argv[1], 32*sizeof(char), 0);
        sleep(3);
        select(maxfd+1, &serverfds, NULL, NULL, &timeout);
        for (int i=0; i <= maxfd; i++)
            if (FD_ISSET(i, &serverfds))
            {
                recv(client, buf, INPUT_BUFFER_SIZE, 0);
                printf("%s\n", buf);
            }
    }
    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}
