#ifndef _GUI_FRAME_INTRO_H
#define _GUI_FRAME_INTRO_H

#include "guiFrameEffect.h"

class guiFrameIntro : public guiFrameEffect
{
public:
    guiFrameIntro(u16 uEffectId, float fIntroTime);
    ~guiFrameIntro();

    virtual void onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor);
    virtual void onEndDisplay();
    virtual void onUpdate(double delta);
    virtual void reset();
    virtual guiFrameIntro * clone();

protected:
    float m_fTimer, m_fTotalTime;
};

#endif
