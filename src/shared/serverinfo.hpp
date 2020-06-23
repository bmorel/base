#ifndef SERVERINFO_HPP
#define SERVERINFO_HPP

#include "defs.hpp"

struct serverinfo;
extern int totalmillis;
extern bool sortedservers;
extern vector<serverinfo*> servers;

struct serverinfo
{
    enum
    {
        MAXPINGS = 3,
        WAITING = INT_MAX
    };
    enum { UNRESOLVED = 0, RESOLVING, RESOLVED };

private:
    int lastping;
    int nextping;
    int pings[MAXPINGS];
    string sdesc;
public:
    ENetAddress address;
    string flags;
    string branch;
    string authhandle;
    string name;
    string map;
    vector<int> attr;
    vector<char *> players;
    vector<char *> handles;
    int ping;
    int resolved;
    int numplayers;
    int priority;
    int port;
    int lastinfo;

public:
    serverinfo(uint ip, int port, int priority = 0);
    ~serverinfo();

    void clearpings();

    void cleanup();

    void reset();

    void checkdecay(int decay);

    void calcping();

    void addping(int rtt, int millis);

    const char* description( void ) const;
    char* description( void ); //TODO: remove me (used only by filterstring in serverbrowser.cpp)

    static serverinfo *newserver(const char *name, int port = SERVER_PORT, int priority = 0, const char *desc = nullptr, const char *handle = nullptr, const char *flags = nullptr, const char *branch = nullptr, uint ip = ENET_HOST_ANY);
};

#endif
