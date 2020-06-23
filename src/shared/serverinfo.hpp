#ifndef SERVERINFO_HPP
#define SERVERINFO_HPP

extern int totalmillis;
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
public:
    ENetAddress address;
    string sdesc;
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

    serverinfo(uint ip, int port, int priority = 0);
    ~serverinfo();

    void clearpings();

    void cleanup();

    void reset();

    void checkdecay(int decay);

    void calcping();

    void addping(int rtt, int millis);
};

#endif
