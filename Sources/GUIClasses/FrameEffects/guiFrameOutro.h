#ifndef _GUI_FRAME_OUTRO_H
#define _GUI_FRAME_OUTRO_H

#include "guiFrameEffect.h"

class guiFrameOutro : public guiFrameEffect
{
public:
  guiFrameOutro(u16 uEffectId, float fOutroTime, u8 onFinished = EFFECT_HIDE_ON_FINISHED);
  ~guiFrameOutro();

  virtual void onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor);
  virtual void onEndDisplay();
  virtual void onUpdate(double delta);
  virtual void reset();
  virtual guiFrameOutro * clone();

protected:
  float m_fTimer, m_fTotalTime;
};

#endif
