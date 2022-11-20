#include "callback.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

std::vector<user_info> users;
std::vector<server_info> servers;
std::vector<std::string> words;
struct Dictionary dict = Dictionary::load("./callback/words.txt");

unsigned int numFromBytes(char *message)
{
    unsigned int number = (((unsigned int) ((unsigned char) message[1])) & 0xFF) << 8;
    number += (((unsigned char) message[0]) & 0xFF);
    return number;
}

void numToBytes(int num, char *message)
{
    message[0] = (num) & 0xFF;
    message[1] = (num >> 8) & 0xFF;
    return;
}

void addPoint(int serverid, char *user)
{
    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid &&
            (strcmp(users[i].sid, user)) == 0)
        {
            users[i].score = users[i].score + 1;
            break;
        }
    }
    return;
}

void updateWords(int index)
{
    words.erase(words.begin() + index);
    int max = dict.words.size();
    int randint = std::rand() % max;
    words.push_back(dict.words[randint]);
    return;
}

int processData(int serverid, char *user, char *data)
{
    int status = 5;
    for (int i=0; i<WORDS; i++)
    {
        if ((strcmp(data, words[i].c_str())) == 0)
        {
            addPoint(serverid, user);
            updateWords(i);
            status = 0;;
            break;
        }
    }

    // FIXME
    for (int i=0; i<WORDS; i++)
        printf("%s\n", words[i].c_str());
    return status;
}

void addUser(int fd, int serverid)
{
    user_info tmpuser;
    if ((recv(fd, tmpuser.sid, SID_LEN, 0)) != SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [ADD USER]: ERROR: Length doesn't correspond. Aborting user adding\n");
        return;
    }
    tmpuser.sid[SID_LEN] = '\0';
    tmpuser.serverid = serverid;
    tmpuser.score = 0;
    std::copy(tmpuser.sid, tmpuser.sid+SID_LEN+1, tmpuser.username);
    users.push_back(tmpuser);
    return;
}

void removeUser(int fd, int serverid)
{
    char *user = new char[SID_LEN];
    if ((recv(fd, user, SID_LEN, 0)) != SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [REMOVE USER]: ERROR: Length doesn't correspond. Aborting user removing\n");
        return;
    }
    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid &&
            (strcmp(users[i].sid, user)) == 0)
        {
            users.erase(users.begin() + i);
        }
    }
    delete[] user;
    return;
}

void handleData(int fd, int serverid)
{
    // FIXME Move to separate function
    char *user = new char[SID_LEN];
    if ((recv(fd, user, SID_LEN, 0)) != SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }

    char *length = new char[MSGID_LEN];
    if ((recv(fd, length, MSGID_LEN, 0)) != MSGID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    int mlength = numFromBytes(length);
    delete[] length;

    char *message = new char[mlength+1];
    message[mlength] = '\0';
    if ((recv(fd, message, mlength, 0)) != mlength)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    processData(serverid, user, message);
    delete[] user;
    delete[] message;
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
    return 0;
}

void Callback::inputCallback(uint16_t fd, char *message, int received)
{
    if (received < 2)
    {
        printf("[INPUT CALLBACK]: Error: Received too few bytes");
        return;
    }

    int id = numFromBytes(message);
    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (servers[i].fd == fd)
        {
            switch(id) {
                case 2:
                    addUser(fd, servers[i].id);
                    break;
                case 3:
                    removeUser(fd, servers[i].id);
                    break;
                case 4:
                    handleData(fd, servers[i].id);
                    break;
            }
            break;
        }
    }
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

void Callback::initCallback(void)
{
    int max = dict.words.size();
    std::srand((unsigned) time(NULL));
    int randint;
    for (int i=0; i<WORDS; i++)
    {
        randint = std::rand() % max;
        words.push_back(dict.words[randint]);
    }

    for (int i=0; i<WORDS; i++)
        printf("%s\n", words[i].c_str());
    return;
}
