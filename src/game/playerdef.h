#ifndef PLAYERDEF_H
#define PLAYERDEF_H
#ifdef GAMESERVER
    #define APVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GVAR(flags, player##name, mn, w00, mx); \
        GVAR(flags, bot##name, mn, w01, mx); \
        GVAR(flags, turret##name, mn, w02, mx); \
        GVAR(flags, grunt##name, mn, w03, mx); \
        GVAR(flags, drone##name, mn, w04, mx); \
        int *sv_actor_stat_##name[] = { \
            &sv_player##name, \
            &sv_bot##name, \
            &sv_turret##name, \
            &sv_grunt##name, \
            &sv_drone##name, \
        };

    #define APFVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GFVAR(flags, player##name, mn, w00, mx); \
        GFVAR(flags, bot##name, mn, w01, mx); \
        GFVAR(flags, turret##name, mn, w02, mx); \
        GFVAR(flags, grunt##name, mn, w03, mx); \
        GFVAR(flags, drone##name, mn, w04, mx); \
        float *sv_actor_stat_##name[] = { \
            &sv_player##name, \
            &sv_bot##name, \
            &sv_turret##name, \
            &sv_grunt##name, \
            &sv_drone##name, \
        };

    #define APSVAR(flags, name, w00, w01, w02, w03, w04) \
        GSVAR(flags, player##name, w00); \
        GSVAR(flags, bot##name, w01); \
        GSVAR(flags, turret##name, w02); \
        GSVAR(flags, grunt##name, w03); \
        GSVAR(flags, drone##name, w04); \
        char **sv_actor_stat_##name[] = { \
            &sv_player##name, \
            &sv_bot##name, \
            &sv_turret##name, \
            &sv_grunt##name, \
            &sv_drone##name, \
        };

    #define AA(t,name)         (*sv_actor_stat_##name[t])
#else
#ifdef GAMEWORLD
    #define APVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GVAR(flags, player##name, mn, w00, mx); \
        GVAR(flags, bot##name, mn, w01, mx); \
        GVAR(flags, turret##name, mn, w02, mx); \
        GVAR(flags, grunt##name, mn, w03, mx); \
        GVAR(flags, drone##name, mn, w04, mx); \
        int *actor_stat_##name[] = { \
            &player##name, \
            &bot##name, \
            &turret##name, \
            &grunt##name, \
            &drone##name, \
        };

    #define APFVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GFVAR(flags, player##name, mn, w00, mx); \
        GFVAR(flags, bot##name, mn, w01, mx); \
        GFVAR(flags, turret##name, mn, w02, mx); \
        GFVAR(flags, grunt##name, mn, w03, mx); \
        GFVAR(flags, drone##name, mn, w04, mx); \
        float *actor_stat_##name[] = { \
            &player##name, \
            &bot##name, \
            &turret##name, \
            &grunt##name, \
            &drone##name, \
        };

    #define APSVAR(flags, name, w00, w01, w02, w03, w04) \
        GSVAR(flags, player##name, w00); \
        GSVAR(flags, bot##name, w01); \
        GSVAR(flags, turret##name, w02); \
        GSVAR(flags, grunt##name, w03); \
        GSVAR(flags, drone##name, w04); \
        char **actor_stat_##name[] = { \
            &player##name, \
            &bot##name, \
            &turret##name, \
            &grunt##name, \
            &drone##name, \
        };
#else
    #define APVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GVAR(flags, player##name, mn, w00, mx); \
        GVAR(flags, bot##name, mn, w01, mx); \
        GVAR(flags, turret##name, mn, w02, mx); \
        GVAR(flags, grunt##name, mn, w03, mx); \
        GVAR(flags, drone##name, mn, w04, mx); \
        extern int *actor_stat_##name[];
    #define APFVAR(flags, name, mn, mx, w00, w01, w02, w03, w04) \
        GFVAR(flags, player##name, mn, w00, mx); \
        GFVAR(flags, bot##name, mn, w01, mx); \
        GFVAR(flags, turret##name, mn, w02, mx); \
        GFVAR(flags, grunt##name, mn, w03, mx); \
        GFVAR(flags, drone##name, mn, w04, mx); \
        extern float *actor_stat_##name[];
    #define APSVAR(flags, name, w00, w01, w02, w03, w04) \
        GSVAR(flags, player##name, w00); \
        GSVAR(flags, bot##name, w01); \
        GSVAR(flags, turret##name, w02); \
        GSVAR(flags, grunt##name, w03); \
        GSVAR(flags, drone##name, w04); \
        extern char **actor_stat_##name[];
#endif
    #define AA(t,name)         (*actor_stat_##name[t])
#endif
#endif
