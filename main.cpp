#include "cppserver/server.hpp"
#include "callback/callback.hpp"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Too few arguments. Use %s <port> instead.\n", argv[0]);
        exit(-1);
    }
    Server server = Server(atoi(argv[1]));
    server.setConnectCallback(&Callback::connectionCallback);
    server.setDisconnectCallback(&Callback::disconnectCallback);
    server.setInputCallback(&Callback::inputCallback);
    server.setInitCallback(&Callback::initCallback);
    server.init();
    while(true)
        server.loop();
}
