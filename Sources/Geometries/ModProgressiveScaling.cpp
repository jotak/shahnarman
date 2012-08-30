#include "ModProgressiveScaling.h"
#include "../SystemHeaders.h"

// -----------------------------------------------------------------
// Name : ModProgressiveScaling
//  Constructor
// -----------------------------------------------------------------
ModProgressiveScaling::ModProgressiveScaling(u16 uModId, float fScaleMin, float fScaleMax, float fScaleInit, float fCoef, float fScaleCenterX, float fScaleCenterY, ProgressiveScalingBehavior behavior)
      : GeometryModifier(uModId)
{
  m_fScaleMin = fScaleMin;
  m_fScaleMax = fScaleMax;
  m_fCurrentScale = fScaleInit;
  m_fCoef = fCoef;
  m_fScaleCenterX = fScaleCenterX;
  m_fScaleCenterY = fScaleCenterY;
  m_Behavior = behavior;
}

// -----------------------------------------------------------------
// Name : ~ModProgressiveScaling
//  Destructor
// -----------------------------------------------------------------
ModProgressiveScaling::~ModProgressiveScaling()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy ModProgressiveScaling\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy ModProgressiveScaling\n");
#endif
}

// -----------------------------------------------------------------
// Name : doTransforms
// -----------------------------------------------------------------
void ModProgressiveScaling::doTransforms(F_RGBA * pColor)
{
  glTranslatef(m_fScaleCenterX * (1 - m_fCurrentScale), m_fScaleCenterY * (1 - m_fCurrentScale), 0.0f);
  glScalef(m_fCurrentScale, m_fCurrentScale, 1.0f);
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void ModProgressiveScaling::update(double delta)
{
  m_fCurrentScale += (float) (m_fCoef * delta);
  if (m_fCurrentScale < m_fScaleMin)
  {
    switch (m_Behavior)
    {
    case PSB_Stop:
      m_fCurrentScale = m_fScaleMin;
      setRunning(false);
      break;
    case PSB_ForthAndBack:
      m_fCurrentScale = m_fScaleMin;
      m_fCoef *= -1;
      break;
    case PSB_Repeat:
      m_fCurrentScale = m_fScaleMax;
      break;
    }
  }
  else if (m_fCurrentScale > m_fScaleMax)
  {
    switch (m_Behavior)
    {
    case PSB_Stop:
      m_fCurrentScale = m_fScaleMax;
      setRunning(false);
      break;
    case PSB_ForthAndBack:
      m_fCurrentScale = m_fScaleMax;
      m_fCoef *= -1;
      break;
    case PSB_Repeat:
      m_fCurrentScale = m_fScaleMin;
      break;
    }
  }
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
GeometryModifier * ModProgressiveScaling::clone(u16 uMoveId)
{
  ModProgressiveScaling * pClone = new ModProgressiveScaling(uMoveId, m_fScaleMin, m_fScaleMax, m_fCurrentScale, m_fCoef, m_fScaleCenterX, m_fScaleCenterY, m_Behavior);
  return pClone;
}
