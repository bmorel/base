#ifndef DEFS_HPP
#define DEFS_HPP

#define SERVER_PORT 28801
#define MAXSDESCLEN 80
#define MAXBRANCHLEN 16
#define MAXTRANS 5000   // max amount of data to swallow in 1 go
#define VERSION_GAME 230

//TODO: move to serverinfo.hpp
enum server_status{ SSTAT_OPEN = 0, SSTAT_LOCKED, SSTAT_PRIVATE, SSTAT_FULL, SSTAT_UNKNOWN, SSTAT_MAX };
enum { MM_OPEN = 0, MM_VETO, MM_LOCKED, MM_PRIVATE, MM_PASSWORD };
#endif
