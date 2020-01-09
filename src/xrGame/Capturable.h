#pragma once
#include "HangingLamp.h"
#include "../xrEngine/feel_touch.h"
class CCapturable : public CHangingLamp, public Feel::Touch
{
    typedef CHangingLamp inherited;

public:
    CCapturable();
    virtual ~CCapturable();

    virtual void Load(LPCSTR section);
    virtual BOOL net_Spawn(CSE_Abstract* DC);
    virtual void net_Destroy();

    virtual void Center(Fvector& C) const;
    virtual float Radius() const;

    virtual void shedule_Update(u32 dt);
    virtual void feel_touch_new(IGameObject* O);
    virtual void feel_touch_delete(IGameObject* O);
    virtual bool feel_touch_contact(IGameObject* O);

    bool m_bActorIN;

private:
    float m_fRadius;
#ifdef DEBUG
    virtual void OnRender();
#endif
};
