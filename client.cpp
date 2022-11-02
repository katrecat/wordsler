#include "server.hpp"
#include <unistd.h>

int main(int argc, char *argv[]) {
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

    while(1)
    {
        send(client, argv[1], 32*sizeof(char), 0);
        sleep(3);
    }
    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}
