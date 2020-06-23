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
    string m_name;
    vector<char *> players;
    vector<char *> handles;
    int lastping;
    int nextping;
    int pings[MAXPINGS];
    int lastinfo;
    int port;
public:
    ENetAddress address;
public:
    string map;
    vector<int> attr;
    int ping;
    int resolved;
    int numplayers;
    int priority;

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
    bool same_name( char const* other ) const;
    bool is_same( char const* oname, int oport ) const;
    char const* name( void ) const; //only used by resolverquery in serverbrowser.cpp and servercompare()
    static bool server_compatible( serverinfo* );
};

#endif
