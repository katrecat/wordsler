#include "callback.hpp"
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>

std::vector<user_info> users;
std::vector<server_info> servers;
std::vector<std::string> words;
struct Dictionary dict = Dictionary::load("./callback/words.txt");

int getMessageID(char *message)
{
    int id = message[0] | message[1] << 8;
    return id;
}

int getMessageLength(char *message)
{
    int init = MSGID_LEN + SID_LEN;
    int length = message[init] | message[init + 1] << 8;
    return length;
}

void addUser(int serverid, char *message, int length)
{
    if (length != MSGID_LEN + SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [ADD USER]: ERROR: Length doesn't correspond. Aborting user adding\n");
        return;
    }

    user_info tmpuser;
    tmpuser.serverid = serverid;
    tmpuser.score = 0;
    std::copy(message+MSGID_LEN, message+length, tmpuser.sid);
    std::copy(message+MSGID_LEN, message+length, tmpuser.username);
    users.push_back(tmpuser);
    return;
}

void removeUser(int serverid, char *message, int length)
{
    if (length != MSGID_LEN + SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [REMOVE USER]: ERROR: Length doesn't correspond. Aborting user removing\n");
        return;
    }

    char *user = new char[SID_LEN];
    std::copy(message+MSGID_LEN, message+length, user);

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

void processData(int serverid, char *user, char *data)
{
    std::string sentData = std::string(data);
    for (int i=0; i<WORDS; i++)
    {
        if ((strcmp(sentData.c_str(), words[i].c_str())) == 0)
        {
            printf("%s and %s are equal!\n", sentData.c_str(), words[i].c_str());
            break;
        }
    }
    return;
}


void handleData(int serverid, char *message, int length)
{
    int mlength = getMessageLength(message);
    char *data = new char[mlength];
    char *user = new char[SID_LEN];

    std::copy(message+MSGID_LEN, message+MSGID_LEN+SID_LEN, user);
    std::copy(message+MSGID_LEN+MSGID_LEN+SID_LEN, message+length, data);

    processData(serverid, user, data);
    delete[] user;
    delete[] data;
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

    int id = getMessageID(message);
    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (servers[i].fd == fd)
        {
            switch(id) {
                case 2:
                    addUser(servers[i].id, message, received);
                    break;
                case 3:
                    removeUser(servers[i].id, message, received);
                    break;
                case 4:
                    handleData(servers[i].id, message, received);
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
