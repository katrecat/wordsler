#include "callback.hpp"
#include <stdio.h>
#include <sys/socket.h>

std::vector<user_info> users;

int Callback::connectionCallback(uint16_t fd)
{
    user_info tempuser;
    if (recv(fd, tempuser.username, USERNAME_LEN, 0) <= 0)
        return -1;
    tempuser.fd = fd;
    tempuser.score = 0;
    users.push_back(tempuser);
    return 0;
}

void Callback::disconnectCallback(uint16_t fd)
{
    for (unsigned int i=0; i<users.size(); i++)
        if (users[i].fd == fd)
        {
            users.erase(users.begin() + i);
            break;
        }
}

void Callback::inputCallback(uint16_t fd, char *word)
{
    for (unsigned int i=0; i<users.size(); i++)
        if (users[i].fd == fd)
        {
            printf("User #%d: %s\n", i, word);
            break;
        }
}
