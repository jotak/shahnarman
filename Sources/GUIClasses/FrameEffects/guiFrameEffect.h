#ifndef _GUI_FRAME_EFFECT_H
#define _GUI_FRAME_EFFECT_H

#include "../../Common/BaseObject.h"

#define EFFECT_NOTHING_ON_FINISHED    0
#define EFFECT_ACTIVATE_ON_FINISHED   1
#define EFFECT_HIDE_ON_FINISHED       2
#define EFFECT_REMOVE_ON_FINISHED     3
#define EFFECT_DELFRM_ON_FINISHED     4

class guiFrame;

class guiFrameEffect : public BaseObject
{
public:
  guiFrameEffect(u16 uEffectId, u8 onFinished = EFFECT_NOTHING_ON_FINISHED);
  ~guiFrameEffect();

  void update(double delta);
  virtual void onUpdate(double delta) {};
  virtual void onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor) {};
  virtual void onEndDisplay() {};
  virtual guiFrameEffect * clone() = 0;
  bool isFinished() { return m_bFinished; };
  bool isActive() { return m_bActive; };
  virtual void setActive(bool bActive);
  virtual void reset();
  u16 getId() { return m_uEffectId; };
  void setFrame(guiFrame * pFrame) { m_pFrame = pFrame; };
  u8 getOnFinished() { return m_uActionOnFinished; };
  void setOnFinished(u8 val) { m_uActionOnFinished = val; };

protected:
  u16 m_uEffectId;
  bool m_bActive;
  bool m_bFinished;
  guiFrame * m_pFrame;
  u8 m_uActionOnFinished;
};

#endif
