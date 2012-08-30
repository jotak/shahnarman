#include "ModProgressiveBlending.h"
#include "../SystemHeaders.h"

// -----------------------------------------------------------------
// Name : ModProgressiveBlending
//  Constructor
// -----------------------------------------------------------------
ModProgressiveBlending::ModProgressiveBlending(u16 uModId, F_RGBA startColor, F_RGBA endColor, float fPeriod) : GeometryModifier(uModId)
{
  m_StartColor = startColor;
  m_EndColor = endColor;
  m_fPeriod = fPeriod;
  m_fTimer = 0;
}

// -----------------------------------------------------------------
// Name : ~ModProgressiveBlending
//  Destructor
// -----------------------------------------------------------------
ModProgressiveBlending::~ModProgressiveBlending()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy ModProgressiveBlending\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy ModProgressiveBlending\n");
#endif
}

// -----------------------------------------------------------------
// Name : doTransforms
// -----------------------------------------------------------------
void ModProgressiveBlending::doTransforms(F_RGBA * pColor)
{
  F_RGBA blend = rgba(1, 1, 1, 1);
  float fHalfPeriod = m_fPeriod / 2;
  float mod = (m_fTimer / fHalfPeriod); // [0, 2]
  if (mod < 1) // from start color to end color
    mod = 1 - mod;
  else  // from end color to start color
    mod = mod - 1;
  blend = (m_StartColor * mod + m_EndColor * (1-mod)) / 2;
  *pColor = F_RGBA_MULTIPLY((*pColor), blend);
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void ModProgressiveBlending::update(double delta)
{
  m_fTimer += (float) delta;
  if (m_fTimer >= m_fPeriod)
    m_fTimer = 0;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
GeometryModifier * ModProgressiveBlending::clone(u16 uModId)
{
  ModProgressiveBlending * pClone = new ModProgressiveBlending(uModId, m_StartColor, m_EndColor, m_fPeriod);
  return pClone;
}
