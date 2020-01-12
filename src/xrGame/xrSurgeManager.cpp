#include "stdafx.h"
#include "xrSurgeManager.h"
#include "Level.h"
#include "game_base.h"
#include "xr_time.h"
#include "space_restrictor.h"
#include "actor_mp_server.h"
#include "game_sv_mp.h"

extern xrTime get_time_struct();
extern u32 get_time();

s32 sv_MIN_SURGE_TIME = 12 * 60 * 60;
s32 sv_MAX_SURGE_TIME = 24 * 60 * 60;
u32 SURGE_TIME = 190 * 60;
u32 prev_sec = 0;

xrSurgeManager::xrSurgeManager(game_sv_mp* owner)
{
    m_game = owner;
    m_started = false;
    m_surge_time = 190;
    m_killed_all_unhided = false;
    m_need_reload_covers = false;
}

xrSurgeManager::~xrSurgeManager() {}

void xrSurgeManager::Update()
{
    if (m_need_reload_covers)
    {
        for (auto id : m_surge_covers_temp)
        {
            CSpaceRestrictor* temp = smart_cast<CSpaceRestrictor*>(Level().Objects.net_Find(id));
            R_ASSERT(temp);
            m_surge_covers.emplace_back(temp);
        }
        m_need_reload_covers = false;
    }
    if (m_started)
    {
        if (!m_killed_all_unhided && get_time_struct() > m_surge_end_time - 10)
        {
            CheckPlayers_InCover();
            m_killed_all_unhided = true;
        }
        else if (get_time_struct() > m_surge_end_time)
        {
            m_started = false;
            EndSurge();
            PrepareForNextSurge();
        }
    }
    else
    {
        if (get_time_struct() > m_next_surge_time)
        {
            m_started = true;
            StartSurge();
        }
    }
}

void xrSurgeManager::ReStart()
{
    m_surge_covers.clear();
    m_surge_covers_temp.clear();
    if (m_started)
    {
        NET_Packet P;
        m_game->GenerateGameMessage(P);
        P.w_u32(GAME_EVENT_SURGE_END);
        m_game->u_EventSend(P);
    }
    PrepareForNextSurge();
}

void xrSurgeManager::AddCover(u16 ID)
{
    m_need_reload_covers = true;
    m_surge_covers_temp.push_back(ID);
}

void xrSurgeManager::StartSurge()
{
    Msg("Surge Start");
    if (Covers().size())
    {
        // Signal
        NET_Packet P;
        m_game->GenerateGameMessage(P);
        P.w_u32(GAME_EVENT_SURGE_START);
        m_game->u_EventSend(P);
    }
    else
    {
        Msg("Surge:: Covers not found!!!");
    }
}

void xrSurgeManager::EndSurge()
{
    Msg("Surge End");

    m_killed_all_unhided = false;
    // Signal
    NET_Packet P;
    m_game->GenerateGameMessage(P);
    P.w_u32(GAME_EVENT_SURGE_END);
    m_game->u_EventSend(P);
}

void xrSurgeManager::CheckPlayers_InCover()
{
    struct ActorInCoverChecker
    {
        xr_vector<CSpaceRestrictor*>& m_surge_covers;
        game_sv_mp* m_sv_game;
        ActorInCoverChecker(xr_vector<CSpaceRestrictor*>& surge_covers, game_sv_mp* sv_game)
            : m_surge_covers(surge_covers), m_sv_game(sv_game)
        {
        }
        void operator()(IClient* client)
        {
            xrClientData* l_pC = static_cast<xrClientData*>(client);
            game_PlayerState* ps = l_pC->ps;
            CSE_ActorMP* pActor = smart_cast<CSE_ActorMP*>(l_pC->owner);
            if (!ps || !pActor)
                return;

            for (auto cover : m_surge_covers)
            {
                Fsphere test;
                test.P = pActor->Position();
                test.R = EPS_L;
                if (!cover->inside(test))
                {
                    m_sv_game->KillPlayer(client->ID, pActor->ID);
                }
            }
        }
    };
    ActorInCoverChecker team_calculator(m_surge_covers, m_game);
    Level().Server->ForEachClientDo(team_calculator);
}

void xrSurgeManager::PrepareForNextSurge()
{
    m_last_surge_time = get_time_struct();

    m__delta = ::Random.randI(sv_MIN_SURGE_TIME, sv_MAX_SURGE_TIME) * 3 * 3600 *
        m_game->GetGameTimeFactor(); // --global minutes, врем¤ между выбросами
    m_next_surge_time = m_last_surge_time + m__delta;
    m_surge_end_time = m_next_surge_time + SURGE_TIME;
}
