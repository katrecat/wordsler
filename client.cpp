#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in con_info;
    con_info.sin_port = htons(12345);
    con_info.sin_family = AF_INET;
    con_info.sin_addr.s_addr = inet_addr("127.0.0.1");
    connect(client, (sockaddr *)&con_info, sizeof(con_info));

    char buf[128];
    read(client, buf, 128 * sizeof(char));
    printf("%s\n", buf);

    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}
