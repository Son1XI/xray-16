#pragma once
#include "xr_time.h"
class CSpaceRestrictor;
class game_sv_mp;
class xrSurgeManager
{
public:
    xrSurgeManager(game_sv_mp* owner);
    ~xrSurgeManager();
    void Update();

    void ReStart();
    bool IsStarted() { return m_started; };
    xr_vector<CSpaceRestrictor*>& Covers() { return m_surge_covers; };

    void AddCover(u16 ID);

private:
    void StartSurge();
    void EndSurge();
    void CheckPlayers_InCover();
    void PrepareForNextSurge();
    xrTime m_next_surge_time;
    xrTime m_last_surge_time;
    u32 m_surge_time;
    u32 m__delta;
    bool m_started;
    xrTime m_surge_end_time;

    game_sv_mp* m_game;

    bool m_need_reload_covers;
    xr_vector<u16> m_surge_covers_temp;
    xr_vector<CSpaceRestrictor*> m_surge_covers;
    bool m_killed_all_unhided;
};
