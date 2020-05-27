#ifndef DUELMUT_H
#define DUELMUT_H

#include <vector>

#ifdef GAMESERVER
struct duelservmode : servmode
{
    int duelround, dueltime, duelcheck, dueldeath, duelwinner, duelwins, duelaffin;
    bool waitforhumans;
    std::vector<clientinfo *> restricted;
    std::vector<clientinfo *> duelqueue;
    std::vector<clientinfo *> allowed;
    std::vector<clientinfo *> playing;

    duelservmode() {}

    #define DSGS(x) DSG(gamemode, mutators, x)

    void shrink()
    {
        allowed.clear();
        playing.clear();
    }

    void doreset(bool start)
    {
        dueltime = start ? 1 : DSGS(cooloff);
        duelcheck = dueldeath = -1;
        duelaffin = 0;
    }

    void reset()
    {
        waitforhumans = true;
        shrink();
        duelqueue.clear();
        restricted.clear();
        doreset(true);
        duelround = duelwins = 0;
        duelwinner = -1;
    }

    void position()
    {
        loopv(clients)
        {
            clientinfo *ci = clients[i];
            if(m_survivor(gamemode, mutators)) ci->queuepos = -1;
            else
            {
                int n = find( duelqueue, ci );
                ci->queuepos = n > 0 && dueltime >= 0 ? n-1 : n;
            }
            sendf(-1, 1, "ri3", N_QUEUEPOS, ci->clientnum, ci->queuepos);
        }
    }

    bool remqueue(clientinfo *ci, bool pos = true)
    {
        auto it = std::find( duelqueue.begin(), duelqueue.end(), ci );
        if( duelqueue.end() != it )
        {
            duelqueue.erase( it );
            if(pos) position();
            return true;
        }
        return false;
    }

    bool queue(clientinfo *ci, bool pos = true, bool top = false, bool wait = true)
    {
        if(ci->actortype >= A_ENEMY || ci->state == CS_SPECTATOR) return remqueue(ci, pos);
        if(gamestate == G_S_OVERTIME && !restricted.empty() && find( restricted, ci ) < 0) return remqueue(ci, pos);
        if(DSGS(maxqueued) && find( duelqueue, ci ) < 0 && find( playing, ci ) < 0)
        {
            int count = 0;
            for( auto& player : duelqueue ) if(player->actortype == A_PLAYER) count++;
            for( size_t i = 0; i < playing.size(); ++i )
            {
                if(playing[i]->actortype == A_PLAYER) count++;
            }
            if(count >= DSGS(maxqueued))
            {
                spectator(ci);
                srvmsgft(ci->clientnum, CON_EVENT, "\fysorry, the \fs\fcqueue\fS is \fs\fzgcFULL\fS (max: \fs\fc%d\fS %s)", DSGS(maxqueued), DSGS(maxqueued) != 1 ? "players" : "player");
                return remqueue(ci, pos);
            }
        }
        if(ci->actortype == A_PLAYER && waitforhumans) waitforhumans = false;
        int n = find( duelqueue, ci );
        if(top)
        {
            if(n >= 0) duelqueue.erase( duelqueue.begin() + n );
            duelqueue.insert(duelqueue.begin(), ci);
            n = 0;
        }
        else if(n < 0)
        {
            n = duelqueue.size();
            duelqueue.emplace_back( ci );
        }
        if(wait && ci->state != CS_WAITING) waiting(ci, DROP_RESET);
        auto it = std::find( allowed.begin(), allowed.end(), ci );
        if( allowed.end() != it )
        {
            allowed.erase( it );
        }
        if(pos) position();
        return true;
    }

    void entergame(clientinfo *ci)
    {
        queue(ci);
        if(dueltime < 0 && dueldeath < 0 && m_affinity(gamemode) && (DSGS(affinity) || numclients() <= 1)) allowed.emplace_back( ci );
    }

    void leavegame(clientinfo *ci, bool disconnecting = false)
    {
        if(duelwinner == ci->clientnum)
        {
            duelwinner = -1;
            duelwins = 0;
        }
        auto it = std::find( allowed.begin(), allowed.end(), ci );
        if( allowed.end() != it )
        {
            allowed.erase( it );
        }
        auto jt = std::find( playing.begin(), playing.end(), ci );
        if( playing.end() != jt )
        {
            playing.erase( jt );
        }
        remqueue(ci);
    }

    bool damage(clientinfo *m, clientinfo *v, int damage, int weap, int flags, int material, const ivec &hitpush, const ivec &hitvel, float dist)
    {
        if(dueltime >= 0) return false;
        return true;
    }

    bool canspawn(clientinfo *ci, bool tryspawn = false)
    {
        if( find( allowed, ci ) >= 0 || ci->actortype >= A_ENEMY) return true;
        if(gamestate == G_S_OVERTIME && !restricted.empty() && find( restricted, ci ) < 0) return false;
        else if(tryspawn)
        {
            queue(ci);
            return true;
        }
        return false; // you spawn when we want you to buddy
    }

    void spawned(clientinfo *ci)
    {
        auto it = std::find( allowed.begin(), allowed.end(), ci );
        if( allowed.end() != it )
        {
            allowed.erase( it );
        }
    }

    void layout()
    {
        loopvj(clients)
        {
            std::vector<int> shots;
            loop(a, W_MAX) loop(b, 2)
            {
                loopv(clients[j]->weapshots[a][b].projs)
                    shots.emplace_back(clients[j]->weapshots[a][b].projs[i].id);
                clients[j]->weapshots[a][b].projs.shrink(0);
            }
            if(!shots.empty()) sendf(-1, 1, "ri2iv", N_DESTROY, clients[j]->clientnum, shots.size(), shots.size(), shots.data());

        }
        if(DSGS(clear))
            loopv(sents) if(enttype[sents[i].type].usetype == EU_ITEM) setspawn(i, hasitem(i), true, true);
    }

    void scoreaffinity(clientinfo *ci, bool win)
    {
        if(!m_affinity(gamemode) || dueltime >= 0 || duelround <= 0) return;
        #define scoredteam(x,y) (win ? x != y : x == y)
        int queued = 0;
        for( size_t i = 0; i < clients.size(); ++i )
        if(clients[i]->actortype < A_ENEMY) switch(clients[i]->state)
        {
            case CS_ALIVE:
                if( find( playing, clients[i] ) < 0 || scoredteam(clients[i]->team, ci->team)) if(queue(clients[i], false)) queued++;
                break;
            case CS_DEAD:
                if( find( playing, clients[i] ) < 0 || scoredteam(clients[i]->team, ci->team))
                {
                    if(queue(clients[i], false)) queued++;
                    break;
                }
                if( find( allowed, clients[i] ) < 0) allowed.emplace_back( clients[i] );
                waiting(clients[i], DROP_RESET);
                break;
            case CS_WAITING:
                if( find( playing, clients[i] ) < 0 || scoredteam(clients[i]->team, ci->team))
                {
                    if(queue(clients[i], false)) queued++;
                    break;
                }
                if( find( allowed, clients[i] ) < 0) allowed.emplace_back( clients[i] );
                break;
            default: break;
        }
        duelaffin = win ? ci->team : -ci->team;
        duelcheck = gamemillis+DSGS(delay);
        if(queued) position();
    }

    void clear()
    {
        doreset(false);
        bool reset = false;
        if(m_duel(gamemode, mutators) && G(duelcycle)&(m_team(gamemode, mutators) ? 2 : 1) && duelwinner >= 0 && duelwins > 0)
        {
            clientinfo *ci = (clientinfo *)getinfo(duelwinner);
            if(ci)
            {
                int numwins = G(duelcycles), numplrs = 0;
                loopv(clients)
                    if(clients[i]->actortype < A_ENEMY && clients[i]->state != CS_SPECTATOR && clients[i]->team == ci->team)
                        numplrs++;
                if(numplrs > (m_team(gamemode, mutators) ? 1 : 2))
                {
                    if(!numwins) numwins = numplrs;
                    if(duelwins >= numwins) reset = true;
                }
            }
            else
            {
                duelwinner = -1;
                duelwins = 0;
            }
        }
        int queued = 0;
        loopv(clients) if(queue(clients[i], false, !reset && clients[i]->state == CS_ALIVE, reset || DSGS(reset) || clients[i]->state != CS_ALIVE)) queued++;
        shrink();
        if(queued) position();
    }

    void endffaround(std::vector<clientinfo *> alive)
    {
        loopv(clients) if( find( playing, clients[i] ) >= 0)
        {
            ffaroundstats rs;
            rs.round = duelround;
            rs.winner = !alive.empty() && clients[i] == alive[0];
            clients[i]->ffarounds.add(rs);
        }
    }

    void update()
    {
        if(!canplay() || waitforhumans) return;
        if(dueltime >= 0)
        {
            if(dueltime && ((dueltime -= curtime) <= 0)) dueltime = 0;
            if(!dueltime)
            {
                shrink();
                int wants = max(numteams(gamemode, mutators), 2);
                for( size_t i = 0; i < clients.size(); ++i ) if( !is_spectator( clients[i] ) && is_dead( clients[i] ) ) queue(clients[i], false);
                if(gamestate == G_S_OVERTIME && DSGS(overtime) && m_ffa(gamemode, mutators) && restricted.empty())
                {
                    for( size_t i = 0; i < clients.size(); ++i )
                    {
                        clientinfo *cs = clients[i];
                        if( is_dead( cs ) && !is_waiting( cs ) )
                        {
                            remqueue(cs, false);
                            continue;
                        }
                        if(restricted.empty() || cs->points > restricted[0]->points)
                        {
                            restricted.clear();
                            restricted.emplace_back( cs );
                        }
                        else if(cs->points == restricted[0]->points) restricted.emplace_back( cs );
                    }
                    for( size_t i = 0; i < clients.size(); ++i )
                    {
                        clientinfo *cs = clients[i];
                        if(find( duelqueue, cs ) >= 0 && find( restricted, cs ) < 0)
                        {
                            remqueue(cs, false);
                        }
                    }
                }
                for( size_t i = 0; i < duelqueue.size(); ++i )
                {
                    if(m_duel(gamemode, mutators) && playing.size() >= wants) break;
                    if( is_dead( duelqueue[i] ) )
                    {
                        if( !is_waiting( duelqueue[i] ) ) waiting(duelqueue[i], DROP_RESET);
                        if(m_duel(gamemode, mutators) && m_team(gamemode, mutators))
                        {
                            bool skip = false;
                            for( size_t j = 0; j < playing.size(); ++j ) if(duelqueue[i]->team == playing[j]->team) { skip = true; break; }
                            if(skip) continue;
                        }
                    }
                    playing.emplace_back( duelqueue[i] );
                }
                if(playing.size() >= wants)
                {
                    if(smode) smode->layout();
                    mutate(smuts, mut->layout());
                    duelround++;
                    string fight = "";
                    if(m_duel(gamemode, mutators))
                    {
                        string names = "";
                        for( size_t i = 0; i < playing.size(); ++i )
                        {
                            concatstring(names, colourname(playing[i]));
                            if(i == wants-1) break;
                            else if(i == wants-2) concatstring(names, " and ");
                            else concatstring(names, ", ");
                        }
                        formatstring(fight, "duel between %s, round \fs\fc#%d\fS", names, duelround);
                    }
                    else if(m_survivor(gamemode, mutators)) formatstring(fight, "survivor, round \fs\fc#%d\fS", duelround);
                    for( size_t i = 0; i < playing.size(); ++i )
                    {
                        if( is_alive( playing[i] ) )
                        {
                            playing[i]->lastregen = gamemillis;
                            playing[i]->lastregenamt = 0; // amt = 0 regens impulse
                            playing[i]->resetresidual();
                            playing[i]->health = m_health(gamemode, mutators, playing[i]->actortype);
                            sendf(-1, 1, "ri4", N_REGEN, playing[i]->clientnum, playing[i]->health, playing[i]->lastregenamt);
                        }
                        else if(find( allowed, playing[i] ) < 0) allowed.emplace_back( playing[i] );
                        auto it1 = std::find( duelqueue.begin(), duelqueue.end(), playing[i] );
                        if( it1 != duelqueue.end() )
                        {
                            duelqueue.erase( it1 );
                        }
                    }
                    if(gamestate == G_S_OVERTIME && !restricted.empty())
                        ancmsgft(-1, S_V_FIGHT, CON_SELF, "\fy\fs\fzcgsudden death\fS %s", fight);
                    else ancmsgft(-1, S_V_FIGHT, CON_SELF, "\fy%s", fight);
                    dueltime = dueldeath = -1;
                    duelcheck = gamemillis+5000;
                }
                else shrink();
                position();
            }
        }
        else if(duelround > 0)
        {
            bool cleanup = false;
            std::vector<clientinfo *> alive;
            int queued = 0;
            loopv(clients) if(clients[i]->actortype < A_ENEMY && clients[i]->state == CS_ALIVE)
            {
                auto it = std::find( playing.begin(), playing.end(), clients[i] );
                if( playing.end() == it )
                {
                    if(queue(clients[i], false)) queued++;
                }
                else alive.emplace_back( clients[i] );
            }
            if(queued) position();
            if(!allowed.empty() && duelcheck >= 0 && gamemillis >= duelcheck)
            for( ssize_t i = allowed.size() - 1; i >= 0; --i )
            {
                if( find( alive, allowed[i] ) < 0) spectator(allowed[i]);
                allowed.erase( allowed.begin() + i );
                cleanup = true;
            }
            if(allowed.empty())
            {
                if(m_survivor(gamemode, mutators) && m_team(gamemode, mutators) && !alive.empty())
                {
                    bool found = false;
                    for( size_t i = 0; i < alive.size(); ++i ) if(i && alive[i]->team != alive[i-1]->team) { found = true; break; }
                    if(!found)
                    {
                        if(dueldeath < 0) dueldeath = gamemillis+DSGS(delay);
                        else if(gamemillis >= dueldeath)
                        {
                            if(!cleanup)
                            {
                                defformatstring(end, "\fyteam %s are the winners", colourteam(alive[0]->team));
                                bool teampoints = true;
                                for( size_t i = 0; i < clients.size(); ++i ) if(find( playing, clients[i] ) >= 0)
                                {
                                    if(clients[i]->team == alive[0]->team)
                                    {
                                        ancmsgft(clients[i]->clientnum, S_V_YOUWIN, CON_SELF, "%s", end);
                                        if(find( alive, clients[i] ) >= 0)
                                        {
                                            if(!m_affinity(gamemode))
                                            {
                                                givepoints(clients[i], 1, !m_dm_oldschool(gamemode, mutators), teampoints);
                                                teampoints = false;
                                            }
                                            else if(!duelaffin && teampoints && !m_dm_oldschool(gamemode, mutators))
                                            {
                                                score &ts = teamscore(clients[i]->team);
                                                ts.total++;
                                                sendf(-1, 1, "ri3", N_SCORE, ts.team, ts.total);
                                                teampoints = false;
                                            }
                                        }
                                    }
                                    else ancmsgft(clients[i]->clientnum, S_V_YOULOSE, CON_SELF, "%s", end);
                                }
                                else ancmsgft(clients[i]->clientnum, S_V_BOMBSCORE, CON_SELF, "%s", end);
                            }
                            clear();
                        }
                    }
                }
                else switch(alive.size())
                {
                    case 0:
                    {
                        if(m_affinity(gamemode) && duelaffin) // reverse it!
                        {
                            score &ts = teamscore(abs(duelaffin));
                            if(duelaffin > 0) ts.total--;
                            else ts.total++;
                            sendf(-1, 1, "ri3", N_SCORE, ts.team, ts.total);
                        }
                        if(!cleanup)
                        {
                            endffaround(alive);
                            ancmsgft(-1, S_V_DRAW, CON_SELF, "\fyeveryone died, \fzoyepic fail");
                            duelwinner = -1;
                            duelwins = 0;
                        }
                        clear();
                        break;
                    }
                    case 1:
                    {
                        if(dueldeath < 0)
                        {
                            dueldeath = gamemillis+DSGS(delay);
                            break;
                        }
                        else if(gamemillis < dueldeath) break;
                        if(!cleanup)
                        {
                            endffaround(alive);
                            string end = "", hp = "";
                            if(!m_insta(gamemode, mutators))
                            {
                                if(alive[0]->health >= m_health(gamemode, mutators, alive[0]->actortype))
                                    formatstring(hp, " with a \fs\fcflawless victory\fS");
                                else formatstring(hp, " with \fs\fc%d\fS health left", alive[0]->health);
                            }
                            if(duelwinner != alive[0]->clientnum)
                            {
                                duelwinner = alive[0]->clientnum;
                                duelwins = 1;
                                formatstring(end, "\fy%s was the winner%s", colourname(alive[0]), hp);
                            }
                            else
                            {
                                duelwins++;
                                formatstring(end, "\fy%s was the winner%s (\fs\fc%d\fS in a row)", colourname(alive[0]), hp, duelwins);
                            }
                            for( size_t i = 0; i < clients.size(); ++i )
                            {
                                if(find( playing, clients[i] ) >= 0)
                                {
                                    if(clients[i] == alive[0])
                                    {
                                        ancmsgft(clients[i]->clientnum, S_V_YOUWIN, CON_SELF, "%s", end);
                                        if(!m_dm_oldschool(gamemode, mutators))
                                        {
                                            if(!m_affinity(gamemode)) givepoints(clients[i], 1, true, true);
                                            else if(!duelaffin)
                                            {
                                                score &ts = teamscore(clients[i]->team);
                                                ts.total++;
                                                sendf(-1, 1, "ri3", N_SCORE, ts.team, ts.total);
                                            }
                                        }
                                    }
                                    else ancmsgft(clients[i]->clientnum, S_V_YOULOSE, CON_SELF, "%s", end);
                                }
                                else ancmsgft(clients[i]->clientnum, S_V_BOMBSCORE, CON_SELF, "%s", end);
                            }
                        }
                        clear();
                        break;
                    }
                    default: break;
                }
            }
        }
    }

    bool wantsovertime()
    {
        if(dueltime < 0 && duelround > 0) return true;
        return false;
    }

    bool canbalance()
    {
        if(dueltime < 0 && duelround > 0) return false;
        return true;
    }

    void balance(int oldbalance)
    {
        doreset(true);
    }
} duelmutator;
#endif
#endif
