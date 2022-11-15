#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <cstdint>
#include <vector>

#define USERNAME_LEN 32
#define SID_LEN 20
#define MSGID_LEN 2

namespace Callback
{
    int connectionCallback(uint16_t fd);
    void disconnectCallback(uint16_t fd);
    void inputCallback(uint16_t fd, char *word, int received);
}

struct user_info {
    char username[USERNAME_LEN];
    char sid[SID_LEN];
    int score;
    int serverid;
};

struct server_info {
    int id;
    int fd;
};
#endif
