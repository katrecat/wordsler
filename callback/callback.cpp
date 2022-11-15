#include "callback.hpp"
#include <stdio.h>
#include <sys/socket.h>

std::vector<user_info> users;
std::vector<server_info> servers;

int getMessageID(char *message)
{
    int id = message[0] | message[1] << 8;
    return id;
}

void addUser(int serverid, char *user)
{
    return;
}

void removeUser(int serverid, char *user)
{
    return;
}

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

void Callback::inputCallback(uint16_t fd, char *word, int received)
{
    if (received < 2)
    {
        printf("[INPUT CALLBACK]: Error: Too few received bytes");
        return;
    }

    // FIXME: Refactor
    int id = getMessageID(word);
    int length;
    char *message;
    if (id == 4)
    {
        length = received - MSGID_LEN - SID_LEN;
        message = new char[length];
        std::copy(word+MSGID_LEN+SID_LEN, word+received, message);
    }
    char *user = new char[SID_LEN];
    std::copy(word+MSGID_LEN, word+MSGID_LEN+SID_LEN, user);

    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (servers[i].fd == fd)
        {
            switch(id) {
                case 2:
                    addUser(servers[i].id, user);
                    break;
                case 3:
                    removeUser(servers[i].id, user);
                    break;
                case 4:
                    printf("User %s sent: %s\n", user, message);
                    break;
            }
            break;
        }
    }
    delete[] user;
    if (id == 4)
        delete[] message;
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
