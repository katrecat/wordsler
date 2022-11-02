#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <cstdint>
#include <vector>

#define USERNAME_LEN 32

namespace Callback
{
    int connectionCallback(uint16_t fd);
}

struct user_info {
    char username[USERNAME_LEN];
    int fd;
    int score;
};
#endif
