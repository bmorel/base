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

enum { SINFO_NONE = 0, SINFO_STATUS, SINFO_NAME, SINFO_PORT, SINFO_QPORT, SINFO_DESC, SINFO_MODE, SINFO_MUTS, SINFO_MAP, SINFO_TIME, SINFO_NUMPLRS, SINFO_MAXPLRS, SINFO_PING, SINFO_PRIO, SINFO_MAX };
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
    int priority;
    string map;
    int ping;
    int numplayers;
public:
    ENetAddress address;
    vector<int> attr;
    int resolved;

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
    int compare( serverinfo const& other, int style, bool reverse ) const;
    int version_compare( serverinfo const& other ) const;
    bool server_full( void ) const;
};

#endif
