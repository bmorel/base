#ifndef SERVERINFO_HPP
#define SERVERINFO_HPP

#include "defs.hpp"

struct serverinfo;
extern int totalmillis;
extern bool sortedservers;
extern vector<serverinfo*> servers;
extern int serverdecay;
extern int lastreset;

extern bool filterstring(char *dst, const char *src, bool newline, bool colour, bool whitespace, bool wsstrip, size_t len);
namespace client
{
    extern int serverstat( serverinfo* );
}

struct serverinfo
{
    enum
    {
        MAXPINGS = 3,
        WAITING = INT_MAX
    };
    enum { UNRESOLVED = 0, RESOLVING, RESOLVED };

private:
    string sdesc;
    string flags;
    string branch;
    string authhandle;
    vector<char *> players;
    vector<char *> handles;
    int lastping;
    int nextping;
    int pings[MAXPINGS];
    int lastinfo;
public:
    ENetAddress address;
    string name;
    string map;
    vector<int> attr;
    int ping;
    int resolved;
    int numplayers;
    int priority;
    int port;

public:
    serverinfo(uint ip, int port, int priority = 0);
    ~serverinfo();

    void clearpings();

    void cleanup();

    void reset();

    void checkdecay(int decay);

    void calcping();

    void addping(int rtt, int millis);

    const char* description( void ) const;//only used at game/client.cpp:L: 38 57 3440

    static serverinfo *newserver(const char *name, int port = SERVER_PORT, int priority = 0, const char *desc = nullptr, const char *handle = nullptr, const char *flags = nullptr, const char *branch = nullptr, uint ip = ENET_HOST_ANY);
    void writecfg( stream& file ) const;
    void update( size_t len, void const* data );
    void cube_get_property( int property, int index );
};

#endif
