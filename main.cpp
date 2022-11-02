#include "server.hpp"

Server server;
int main(int argc, char **argv)
{
    server.init();
    while(true)
        server.loop();
}
