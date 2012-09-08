#ifndef _EFFECT_COMEIN_H
#define _EFFECT_COMEIN_H

#include "guiFrameEffect.h"

class EffectComeIn : public guiFrameEffect
{
public:
    EffectComeIn(u16 uEffectId, float fDuration);
    ~EffectComeIn();

    virtual void onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor);
    virtual void onEndDisplay();
    virtual void onUpdate(double delta);
    virtual void reset();
    virtual EffectComeIn * clone();

protected:
    float m_fTimer, m_fTotalTime;
};

#endif
