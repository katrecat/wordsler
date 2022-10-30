#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);    
    sockaddr_in con_info;

    uint16_t hostPort = htons(12345);
    con_info.sin_port = hostPort;
    con_info.sin_family = AF_INET;
    con_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    bind(server_socket, (sockaddr*)&con_info, sizeof(con_info));

    listen(server_socket, 10);
    while(1) {
        sockaddr_in client = {};
        socklen_t lClient = sizeof(client);
        int connection = accept(server_socket, (sockaddr*)&client, &lClient);
        char buf[128] = "Test if works\n";
        write(connection, buf, strlen(buf));
        shutdown(connection, SHUT_RDWR);
        close(connection);
    }
    return 0;
}
