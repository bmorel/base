#include <limits.h>
#include <zlib.h>
#include <SDL2/SDL.h>
#include <enet/enet.h>
#include <GL/gl.h>

#include "tools.h"
#include "command.h"
#include "serverinfo.hpp"

serverinfo::serverinfo(uint ip, int port, int priority)
 : port(port), priority(priority), numplayers(0), resolved(ip==ENET_HOST_ANY ? UNRESOLVED : RESOLVED)
{
    m_name[0] = map[0] = sdesc[0] = authhandle[0] = flags[0] = branch[0] = '\0';
    m_address.host = ip;
    m_address.port = port+1;
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
    attr.clear();
    players.clear();
    handles.clear();
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

void serverinfo::addping(int rtt, int millis)
{
    if(millis >= lastping) lastping = -1;
    pings[nextping] = rtt;
    nextping = (nextping+1)%MAXPINGS;
    int numpings = 0, totalpings = 0;
    loopk(MAXPINGS) if(pings[k] != WAITING) { totalpings += pings[k]; numpings++; }
    ping = numpings ? totalpings/numpings : WAITING;
}

const char* serverinfo::description( void ) const
{
    return sdesc[0] ? sdesc : m_name;
}

void serverinfo::writecfg( stream& file ) const
{
    file.printf("addserver %s %d %d %s %s %s %s\n", m_name, port, priority, escapestring(description()), escapestring(authhandle), escapestring(flags), escapestring(branch));
}

serverinfo *serverinfo::newserver(const char *name, int port, int priority, const char *desc, const char *handle, const char *flags, const char *branch, uint ip)
{
    serverinfo *si = new serverinfo(ip, port, priority);

    if(name) copystring(si->m_name, name);
    else if(ip == ENET_HOST_ANY || enet_address_get_host_ip(&si->m_address, si->m_name, sizeof(si->m_name)) < 0)
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

void serverinfo::update( size_t len, void const* data )
{
    char text[MAXTRANS];
    ucharbuf p(static_cast<unsigned char*>( const_cast<void*>( data ) ), len);
    int millis = getint(p), rtt = clamp(totalmillis - millis, 0, min(serverdecay*1000, totalmillis));
    if(millis >= lastreset && rtt < serverdecay*1000) addping(rtt, millis);
    lastinfo = totalmillis;
    numplayers = getint(p);
    int numattr = getint(p);
    attr.clear();
    loopj(numattr) attr.emplace_back(getint(p));
    int gver = attr.empty() ? 0 : attr[0];
    getstring(text, p);
    filterstring(map, text, false, true, true, false, sizeof( map ));
    getstring(text, p);
    filterstring(sdesc, text, true, true, true, false, MAXSDESCLEN+1);
    players.clear();
    handles.clear();
    if(gver >= 227)
    {
        getstring(text, p);
        filterstring(branch, text, true, true, true, false, MAXBRANCHLEN+1);
    }
    loopi(numplayers)
    {
        if(p.overread()) break;
        getstring(text, p);
        players.emplace_back(newstring(text));
    }
    if(gver >= 225) loopi(numplayers)
    {
        if(p.overread()) break;
        getstring(text, p);
        handles.emplace_back(newstring(text));
    }
    sortedservers = false;
}

void serverinfo::cube_get_property( int prop, int idx )
{
    if(prop < 0) intret(4);
    else switch(prop)
    {
        case 0:
            if(idx < 0) intret(11);
            else switch(idx)
            {
                case 0: intret(client::serverstat(this)); break;
                case 1: result(m_name); break;
                case 2: intret(port); break;
                case 3: result(description()); break;
                case 4: result(map); break;
                case 5: intret(numplayers); break;
                case 6: intret(ping); break;
                case 7: intret(lastinfo); break;
                case 8: result(authhandle); break;
                case 9: result(flags); break;
                case 10: result(branch); break;
                case 11: intret(priority); break;
            }
            break;
        case 1:
            if(idx < 0) intret(attr.size());
            else if( idx < attr.size()) intret(attr[idx]);
            break;
        case 2:
            if(idx < 0) intret(players.size());
            else if( idx < players.size()) result(players[idx].data());
            break;
        case 3:
            if(idx < 0) intret(handles.size());
            else if( idx < handles.size()) result(handles[idx].data());
            break;
    }
}

bool serverinfo::is_same( ENetAddress const& addr ) const
{
    return m_address.host == addr.host && m_address.port == addr.port;
}

bool serverinfo::is_same( char const* oname, int oport ) const
{
    return 0 == strcmp( m_name, oname ) && port == oport;
}

char const* serverinfo::name( void ) const
{
    return m_name;
}

int serverinfo::compare( serverinfo const& other, int style, bool reverse ) const
{
    int ac = 0, bc = 0;
    int index = 0;
    int comp  = 0;
    switch(style)
    {
        case SINFO_DESC:
            if( ( comp = strcmp(sdesc, other.sdesc) ) ) { return reverse ? 0 - comp: comp ; }
            return 0;
        case SINFO_MAP:
            if( ( comp = strcmp(map, other.map) ) ) { return reverse ? 0 - comp : comp; }
            return 0;
        case SINFO_MODE:
            index = 1;
        case SINFO_MUTS:
            index = 2;
            break;
        case SINFO_TIME:
            index = 3;
            break;
        case SINFO_MAXPLRS:
            index = 4;
            break;
        case SINFO_STATUS:
            ac = client::serverstat( const_cast<serverinfo*>( this ) );
            bc = client::serverstat( const_cast<serverinfo*>( &other ) );
            break;
        case SINFO_NUMPLRS:
            ac = numplayers;
            bc = other.numplayers;
            break;
        case SINFO_PING:
            ac = ping;
            bc = other.ping;
            break;
        case SINFO_PRIO:
            ac = priority;
            bc = other.priority;
            break;
        default:
            return 0;
    }
    if( index != 0 )
    {
        ac =       attr.size() > index ?       attr[index] : 0;
        bc = other.attr.size() > index ? other.attr[index] : 0;
    }

    if( ac != bc )
    {
        bool higher_is_better = style != SINFO_NUMPLRS && style != SINFO_PRIO;
        if( higher_is_better != reverse ? ac < bc : ac > bc ) return -1;
        if( higher_is_better != reverse ? ac > bc : ac < bc ) return  1;
    }
    return 0;
}

int serverinfo::version_compare( serverinfo const& other ) const
{
    int ac = 0, bc = 0;
    ac = m_address.host == ENET_HOST_ANY || ping >= serverinfo::WAITING || attr.empty() ? -1 : attr[0] == VERSION_GAME ? 0x7FFF : clamp(attr[0], 0, 0x7FFF-1);

    bc = other.m_address.host == ENET_HOST_ANY || other.ping >= serverinfo::WAITING || other.attr.empty() ? -1 : other.attr[0] == VERSION_GAME ? 0x7FFF : clamp(other.attr[0], 0, 0x7FFF-1);

    if(ac > bc) return -1;
    if(ac < bc) return  1;
    return 0;
}

bool serverinfo::server_full( void ) const
{
    return attr.size() > 4 && numplayers >= attr[4];
}

server_status serverinfo::server_status( void ) const
{
    if(attr.size() <= 5)
    {
        return SSTAT_UNKNOWN;
    }
    switch(attr[5])
    {
        case MM_LOCKED:
        {
            return SSTAT_LOCKED;
        }
        case MM_PRIVATE:
        case MM_PASSWORD:
        {
            return SSTAT_PRIVATE;
        }
        default:
        {
            return SSTAT_OPEN;
        }
    }
}

bool serverinfo::server_compatible( serverinfo const* si )
{
    return si->attr.empty() || si->attr[0] == server::getver(1);
}

bool serverinfo::validate_resolve( char const* name, ENetAddress const& addr )
{
    if( m_name != name )
    {
        return false;
    }
    resolved = serverinfo::RESOLVED;
    m_address.host = addr.host;
    return true;
}

bool serverinfo::need_resolve( int& resolving )
{
    if( resolved == RESOLVED || m_address.host != ENET_HOST_ANY )
    {
        return false;
    }
    int ret = resolved == UNRESOLVED;
    ++resolving;
    resolved = RESOLVING;
    return ret;
}

ENetAddress const* serverinfo::address( void ) const
{
    return &m_address;
}
