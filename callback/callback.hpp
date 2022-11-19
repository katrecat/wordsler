#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>

#define USERNAME_LEN 32
#define SID_LEN 20
#define MSGID_LEN 2
#define WORDS 10

namespace Callback
{
    int connectionCallback(uint16_t fd);
    void disconnectCallback(uint16_t fd);
    void inputCallback(uint16_t fd, char *word, int received);
    void initCallback(void);
}

struct Dictionary {
    std::vector<std::string> words;

    static Dictionary load(const std::string & path) {
        Dictionary result;

        std::ifstream fin(path);
        for (std::string line; getline(fin, line);) {
            result.words.emplace_back(std::move(line));
        }
        fin.close();

        return result;
    }
};

struct user_info {
    char username[USERNAME_LEN];
    char sid[SID_LEN+1];
    int score;
    int serverid;
};

struct server_info {
    int id;
    int fd;
};
#endif
