#include "callback.hpp"
#include <stdio.h>
#include <sys/socket.h>

std::vector<user_info> users;
std::vector<server_info> servers;

int Callback::connectionCallback(uint16_t fd)
{
    server_info tempserver;
    tempserver.fd = fd;
    int maxid = 1;
    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (maxid < servers[i].id)
            maxid = servers[i].id;
    }
    tempserver.id = maxid + 1;
    servers.push_back(tempserver);

    // FIXME
    for (unsigned int i=0; i<servers.size(); i++)
    {
        printf("Server #%d is still connected\n", servers[i].id);
    }
    return 0;
}

void Callback::disconnectCallback(uint16_t fd)
{
    int serverid = 0;
    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (servers[i].fd == fd)
        {
            serverid = servers[i].id;
            servers.erase(servers.begin() + i);
            break;
        }
    }

    // FIXME: Raise error if serverid is still zero
    printf("Server #%d disconnected\n", serverid);

    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid)
        {
            users.erase(users.begin() + i);
        }
    }
}

void Callback::inputCallback(uint16_t fd, char *word)
{
    for (unsigned int i=0; i<servers.size(); i++)
        if (servers[i].fd == fd)
        {
            printf("Server #%d: %s\n", servers[i].id, word);
            break;
        }
}
