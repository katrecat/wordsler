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
    /* 
     * Converts two bytes long message to unsigned int number.
     */
    unsigned int number = (((unsigned int) ((unsigned char) message[1])) & 0xFF) << 8;
    number += (((unsigned char) message[0]) & 0xFF);
    return number;
}

void numToBytes(unsigned int num, char *message)
{
    /*
     * Converts integer to two bytes long usigned character message.
     */
    message[0] = (num) & 0xFF;
    message[1] = (num >> 8) & 0xFF;
    return;
}

void readSid(int fd, char *user)
{
    /*
     * Reads the user SID from <fd> and saves it to <user> array.
     */
    if ((recv(fd, user, SID_LEN, 0)) != SID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    user[SID_LEN] = '\0';
    return;
}

void addPoint(int serverid, char *user)
{
    /*
     * Adds a point to a user with corresponding <serverid> and <sid>.
     */
    #ifdef DEBUG
    printf("[CALLBACK] [ADD POINT] looking for user %s from %d server\n", user, serverid);
    #endif
    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid &&
            (strcmp(users[i].sid, user)) == 0)
        {
            #ifdef DEBUG
            printf("[CALLBACK] [ADD POINT] adding point to usr %s\n", users[i].sid);
            #endif
            users[i].score = users[i].score + 1;
            break;
        }
    }
    return;
}

void updateWords(int index)
{
    /*
     * Replaces the <index> word from the word list with a new one.
     */
    #ifdef DEBUG
    printf("[CALLBACK] [UPDATE WORDS] updating words\n");
    #endif
    words.erase(words.begin() + index);
    int max = dict.words.size();
    int randint = std::rand() % max;
    words.push_back(dict.words[randint]);
    return;
}

int processData(int serverid, char *user, char *data)
{
    /*
     * Verifies if <data> is in the current word list.
     */
    int status = -1;
    #ifdef DEBUG
    printf("[CALLBACK] [PROCESS DATA] compairing %s with word list\n", data);
    #endif
    for (int i=0; i<WORDS; i++)
    {
        if ((strcmp(data, words[i].c_str())) == 0)
        {
            #ifdef DEBUG
            printf("[CALLBACK] [PROCESS DATA] found match\n");
            #endif
            addPoint(serverid, user);
            updateWords(i);
            status = 0;
            break;
        }
    }
    return status;
}

int processUsername(int serverid, char *user, char *data)
{
    /*
     * Sets the <data> username to user with corresponding <serverid> and <sid>.
     */
    int status = -1;
    #ifdef DEBUG
    printf("[CALLBACK] [PROCESS USERNAME] looking for match\n");
    #endif
    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid &&
            (strcmp(users[i].sid, user)) == 0)
        {
            #ifdef DEBUG
            printf("[CALLBACK] [PROCESS USERNAME] found a match\n");
            #endif
            std::copy(data, data+strlen(data), users[i].username);
            users[i].username[strlen(data)] = '\0';
            status = 0;
            break;
        }
    }
    return status;
}

void addUser(int fd, int serverid)
{
    /*
     * Adds the user connected to <serverid> to the users list.
     */
    user_info tmpuser;
    readSid(fd, tmpuser.sid);
    #ifdef DEBUG
    printf("[CALLBACK] [ADD USER] user: %s\n", tmpuser.sid);
    #endif
    tmpuser.serverid = serverid;
    tmpuser.score = 0;
    std::copy(tmpuser.sid, tmpuser.sid+SID_LEN+1, tmpuser.username);
    users.push_back(tmpuser);
    return;
}

void removeUser(int fd, int serverid)
{
    /*
     * Reads the user <sid> from <serverid> and removes from the users list.
     */
    char *user = new char[SID_LEN+1];
    readSid(fd, user);
    #ifdef DEBUG
    printf("[CALLBACK] [REMOVE USER] user: %s\n", user);
    #endif
    for (unsigned int i=0; i<users.size(); i++)
    {
        if (users[i].serverid == serverid &&
            (strcmp(users[i].sid, user)) == 0)
        {
            users.erase(users.begin() + i);
            #ifdef DEBUG
            printf("[CALLBACK] [REMOVE USER] removed\n");
            #endif
        }
    }
    delete[] user;
    return;
}

void sendWords(void)
{
    /*
     * Sends the current list of words in the game to connected servers.
     */
    for (unsigned int i=0; i<servers.size(); i++)
    {
        int fd = servers[i].fd;
        char *len = new char[MSGID_LEN];
        numToBytes(WORDS, len);
        send(fd, len, MSGID_LEN, 0);
        for (int i=0; i<WORDS; i++)
        {
            numToBytes(strlen(words[i].c_str()), len);
            send(fd, len, MSGID_LEN, 0);
            send(fd, words[i].c_str(), strlen(words[i].c_str()), 0);
        }
        delete[] len;
    }
    return;
}

void sendPlayers(void)
{
    /*
     * Sends the current list of players in the game to connected servers.
     */
    for (unsigned int i=0; i<servers.size(); i++)
    {
        int fd = servers[i].fd;
        char *len = new char[MSGID_LEN];
        numToBytes(users.size(), len);
        send(fd, len, MSGID_LEN, 0);
        for (unsigned int i=0; i<users.size(); i++)
        {
            numToBytes(strlen(users[i].username), len);
            send(fd, len, MSGID_LEN, 0);
            send(fd, users[i].username, strlen(users[i].username), 0);
            numToBytes(MSGID_LEN, len);
            send(fd, len, MSGID_LEN, 0);
            numToBytes(users[i].score, len);
            send(fd, len, MSGID_LEN, 0);
        }
        delete[] len;
    }
    return;
}

void handleUsername(int fd, int serverid)
{
    /*
     * Handles the message with USERNAME message id.
     * Reads the message from server with <serverid> and sends it for further processing.
     */
    char *user = new char[SID_LEN+1];
    readSid(fd, user);
    #ifdef DEBUG
    printf("[CALLBACK] [USERNAME] user: %s\n", user);
    #endif

    char *length = new char[MSGID_LEN];
    if ((recv(fd, length, MSGID_LEN, 0)) != MSGID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    int mlength = numFromBytes(length);
    #ifdef DEBUG
    printf("[CALLBACK] [USERNAME] length: %d\n", mlength);
    #endif
    delete[] length;

    char *username = new char[mlength+1];
    username[mlength] = '\0';
    if ((recv(fd, username, mlength, 0)) != mlength)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    #ifdef DEBUG
    printf("[CALLBACK] [USERNAME] username: %s\n", username);
    #endif

    int status = processUsername(serverid, user, username);
    if (status == 0)
    {
        #ifdef DEBUG
        printf("[CALLBACK] [USERNAME] updating user list\n");
        #endif
        sendWords();
        sendPlayers();
    }
    else
    {
        printf("[CALLBACK] [USERNAME] [ERROR] didn't update a username for %s\n", user);
    }

    delete[] user;
    delete[] username;
}

void handleData(int fd, int serverid)
{
    /*
     * Handles the message with DATA message id.
     * Reads the message from server with <serverid> and sends it for further processing.
     */
    char *user = new char[SID_LEN+1];
    readSid(fd, user);
    #ifdef DEBUG
    printf("[CALLBACK] [DATA] user: %s\n", user);
    #endif

    char *length = new char[MSGID_LEN];
    if ((recv(fd, length, MSGID_LEN, 0)) != MSGID_LEN)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    int mlength = numFromBytes(length);
    #ifdef DEBUG
    printf("[CALLBACK] [DATA] msglen: %d\n", mlength);
    #endif
    delete[] length;

    char *message = new char[mlength+1];
    message[mlength] = '\0';
    if ((recv(fd, message, mlength, 0)) != mlength)
    {
        fprintf(stderr, "[CALLBACK]: [HANDLE DATA]: ERROR: Length doesn't correspond. Exiting\n");
        exit(-1);
        return;
    }
    #ifdef DEBUG
    printf("[CALLBACK] [DATA] message: %s\n", message);
    #endif

    int status = processData(serverid, user, message);
    if (status == 0)
    {
        #ifdef DEBUG
        printf("[CALLBACK] [DATA] updating score list\n");
        #endif
        sendWords();
        sendPlayers();
    }
    delete[] user;
    delete[] message;
    return;
}

int Callback::connectionCallback(uint16_t fd)
{
    /*
     * Processes incoming connection;
     */
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
    sendWords();
    sendPlayers();
    #ifdef DEBUG
    printf("[CALLBACK] [CONNECT] Server #%d connected\n", fd);
    #endif
    return 0;
}

void Callback::inputCallback(uint16_t fd, char *message, int received)
{
    /*
     * Handles the <message> of <received> length and sends it
     * for further processing.
     */
    if (received < 2)
    {
        printf("[INPUT CALLBACK]: Error: Received too few bytes");
        return;
    }

    int id = numFromBytes(message);
    #ifdef DEBUG
    printf("[CALLBACK] [INPUT] Server #%d: msgid: %d\n", fd, id);
    #endif
    for (unsigned int i=0; i<servers.size(); i++)
    {
        if (servers[i].fd == fd)
        {
            switch(id) {
                case 2:
                    addUser(fd, servers[i].id);
                    sendWords();
                    sendPlayers();
                    break;
                case 3:
                    removeUser(fd, servers[i].id);
                    sendWords();
                    sendPlayers();
                    break;
                case 4:
                    handleData(fd, servers[i].id);
                    break;
                case 5:
                    handleUsername(fd, servers[i].id);
                    break;
            }
            break;
        }
    }
}

void Callback::disconnectCallback(uint16_t fd)
{
    /*
     * Handles the server disconnect.
     * Removes the server with <fd> from servers list and
     * erases all the users that were connected through it.
     */
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

    for (unsigned int i=users.size(); i>0; i--)
    {
        if (users[i-1].serverid == serverid)
        {
            users.erase(users.begin() + i-1);
        }
    }
    #ifdef DEBUG
    printf("[CALLBACK] [DISCONNECT] Server #%d disconnected\n", fd);
    #endif
}

void Callback::initCallback(void)
{
    /*
     * Initializes the seed and words for the game.
     */
    int max = dict.words.size();
    std::srand((unsigned) time(NULL));
    int randint;
    for (int i=0; i<WORDS; i++)
    {
        randint = std::rand() % max;
        words.push_back(dict.words[randint]);
    }
    return;
}
