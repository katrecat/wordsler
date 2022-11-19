#include "cppserver/server.hpp"
#include "callback/callback.hpp"

int main()
{
    Server server;
    server.setConnectCallback(&Callback::connectionCallback);
    server.setDisconnectCallback(&Callback::disconnectCallback);
    server.setInputCallback(&Callback::inputCallback);
    server.setInitCallback(&Callback::initCallback);
    server.init();
    while(true)
        server.loop();
}
