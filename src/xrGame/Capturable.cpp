#include "stdafx.h"
#include "Capturable.h"
#include "..\xrEngine\IGame_Persistent.h"
#include "Level.h"
#include "map_manager.h"
#include "..\xrEngine\xr_collide_form.h"
#include "map_location.h"

#ifdef DEBUG
#include "xrServer.h"
#include "debug_renderer.h"
#endif
#include "Actor.h"

static CCapturable* pCurrentCapturable = nullptr;
IGameObject* LastActor_pObject = nullptr;

CCapturable::CCapturable()
{
    m_fRadius = 30.f;
    m_bActorIN = false;
}

CCapturable::~CCapturable() {}

void CCapturable::Load(LPCSTR section)
{
    inherited::Load(section);
    m_fRadius = READ_IF_EXISTS(pSettings, r_float, section, "radius", m_fRadius);
}

void CCapturable::Center(Fvector& C) const { inherited::Center(C); }

float CCapturable::Radius() const { return inherited::Radius(); }

BOOL CCapturable::net_Spawn(CSE_Abstract* DC)
{
    BOOL bOk = inherited::net_Spawn(DC);
    feel_touch.clear();

    setEnabled(TRUE);

    if (GameID() != eGameIDSingle && !GEnv.isDedicatedServer)
    {
        (Level().MapManager().AddMapLocation("mp_capturable", ID()))->EnablePointer();
    };

    pCurrentCapturable = this;
    return (bOk);
}

void CCapturable::net_Destroy()
{
    if (!GEnv.isDedicatedServer)
        Level().MapManager().OnObjectDestroyNotify(ID());

    pCurrentCapturable = nullptr;
    inherited::net_Destroy();
};

void CCapturable::shedule_Update(u32 dt)
{
    inherited::shedule_Update(dt);

    feel_touch_update(XFORM().c, m_fRadius);
}

void CCapturable::feel_touch_new(IGameObject* tpObject)
{
    if (OnClient())
    {
        if (tpObject == Level().CurrentControlEntity())
        {
            m_bActorIN = true;
            LastActor_pObject = tpObject;
        }
    }
}

void CCapturable::feel_touch_delete(IGameObject* tpObject)
{
    if (OnClient())
    {
        if (tpObject == Level().CurrentControlEntity())
        {
            m_bActorIN = false;
        }
        else if (LastActor_pObject == tpObject)
        {
            m_bActorIN = false;
        }
    }
}

bool CCapturable::feel_touch_contact(IGameObject* O)
{
    CActor* pActor = smart_cast<CActor*>(O);
    if (!pActor || pActor->GetfHealth() <= 0)
        return false;
    return true;
}

#ifdef DEBUG
extern Flags32 dbg_net_Draw_Flags;
void CCapturable::OnRender()
{

}
#endif
