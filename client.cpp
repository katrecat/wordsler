#include "server.hpp"


int main(int argc, char *argv[]) {
    int client = socket(AF_INET, SOCK_STREAM, 0);
    
    sockaddr_in con_info;
    con_info.sin_port = htons(DEFAULT_PORT);
    con_info.sin_family = AF_INET;
    con_info.sin_addr.s_addr = INADDR_ANY;
    connect(client, (sockaddr *)&con_info, sizeof(con_info));

    char buf[128]= "Test it works";
    send(client, buf, 128 * sizeof(char),0);

    shutdown(client, SHUT_RDWR);
    close(client);
    return 0;
}
