#ifndef _GUI_FRAME_FLASH_H
#define _GUI_FRAME_FLASH_H

#include "guiFrameEffect.h"

class guiFrameFlash : public guiFrameEffect
{
public:
    guiFrameFlash(u16 uEffectId, float fFlashTime);
    ~guiFrameFlash();

    virtual void onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor);
    virtual void onEndDisplay();
    virtual void onUpdate(double delta);
    virtual void reset();
    virtual guiFrameFlash * clone();

protected:
    float m_fTimer, m_fTotalTime;
};

#endif
