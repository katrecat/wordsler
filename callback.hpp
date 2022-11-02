#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <cstdint>
#include <vector>

#define USERNAME_LEN 32

namespace Callback
{
    int connectionCallback(uint16_t fd);
    void disconnectCallback(uint16_t fd);
    void inputCallback(uint16_t fd, char *word);
}

struct user_info {
    char username[USERNAME_LEN];
    int fd;
    int score;
};
#endif
