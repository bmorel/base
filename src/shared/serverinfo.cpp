#include <limits.h>
#include <zlib.h>
#include <SDL2/SDL.h>
#include <enet/enet.h>
#include <GL/gl.h>

#include "tools.h"
#include "serverinfo.hpp"

serverinfo::serverinfo(uint ip, int port, int priority)
 : numplayers(0), resolved(ip==ENET_HOST_ANY ? UNRESOLVED : RESOLVED), port(port), priority(priority)
{
    name[0] = map[0] = sdesc[0] = authhandle[0] = flags[0] = branch[0] = '\0';
    address.host = ip;
    address.port = port+1;
    clearpings();
}
serverinfo::~serverinfo() { cleanup(); }

void serverinfo::clearpings()
{
    ping = WAITING;
    loopk(MAXPINGS) pings[k] = WAITING;
    nextping = 0;
    lastping = lastinfo = -1;
}

void serverinfo::cleanup()
{
    clearpings();
    attr.setsize(0);
    players.deletearrays();
    handles.deletearrays();
    numplayers = 0;
}

void serverinfo::reset()
{
    lastping = lastinfo = -1;
}

void serverinfo::checkdecay(int decay)
{
    if(lastping >= 0 && totalmillis - lastping >= decay)
        cleanup();
    if(lastping < 0) lastping = totalmillis ? totalmillis : 1;
}

void serverinfo::calcping()
{
    int numpings = 0, totalpings = 0;
    loopk(MAXPINGS) if(pings[k] != WAITING) { totalpings += pings[k]; numpings++; }
    ping = numpings ? totalpings/numpings : WAITING;
}

void serverinfo::addping(int rtt, int millis)
{
    if(millis >= lastping) lastping = -1;
    pings[nextping] = rtt;
    nextping = (nextping+1)%MAXPINGS;
    calcping();
}

const char* serverinfo::description( void ) const
{
    return sdesc[0] ? sdesc : name;
}

char* serverinfo::description( void )
{
    return sdesc;
}

serverinfo *serverinfo::newserver(const char *name, int port, int priority, const char *desc, const char *handle, const char *flags, const char *branch, uint ip)
{
    serverinfo *si = new serverinfo(ip, port, priority);

    if(name) copystring(si->name, name);
    else if(ip == ENET_HOST_ANY || enet_address_get_host_ip(&si->address, si->name, sizeof(si->name)) < 0)
    {
        delete si;
        return NULL;
    }
    if(desc && *desc) copystring(si->sdesc, desc, MAXSDESCLEN+1);
    if(handle && *handle) copystring(si->authhandle, handle);
    if(flags && *flags) copystring(si->flags, flags);
    if(branch && *branch) copystring(si->branch, branch, MAXBRANCHLEN+1);

    servers.emplace_back( si );
    sortedservers = false;

    return si;
}

