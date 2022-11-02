#include "server.hpp"
#include "callback.hpp"

int main(int argc, char **argv)
{
    Server server;
    server.setConnectCallback(&Callback::connectionCallback);
    server.init();
    while(true)
        server.loop();
}
