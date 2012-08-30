#include "guiFrameIntro.h"
#include "../guiFrame.h"
#include "../../SystemHeaders.h"

// -----------------------------------------------------------------
// Name : guiFrameIntro
//  Constructor
// -----------------------------------------------------------------
guiFrameIntro::guiFrameIntro(u16 uEffectId, float fIntroTime) : guiFrameEffect(uEffectId, EFFECT_ACTIVATE_ON_FINISHED)
{
  m_fTimer = m_fTotalTime = fIntroTime;
}

// -----------------------------------------------------------------
// Name : ~guiFrameIntro
//  Destructor
// -----------------------------------------------------------------
guiFrameIntro::~guiFrameIntro()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy guiFrameIntro\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy guiFrameIntro\n");
#endif
}

// -----------------------------------------------------------------
// Name : onBeginDisplay
// -----------------------------------------------------------------
void guiFrameIntro::onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor)
{
  float coef = max(1 - m_fTimer / m_fTotalTime, 0.001f); // must not be 0

  // Scaling
  Coords3D fCenter = m_pFrame->getDisplay()->get3DCoords(CoordsScreen(
        iXOffset + m_pFrame->getXPos() + m_pFrame->getWidth() / 2, 
        iYOffset + m_pFrame->getYPos() + m_pFrame->getHeight() / 2),
        DMS_2D);
  glPushMatrix();
  glTranslatef(fCenter.x * (1 - coef), fCenter.y * (1 - coef), 0.0f);
  glScalef(coef, coef, 1.0f);
}

// -----------------------------------------------------------------
// Name : onEndDisplay
// -----------------------------------------------------------------
void guiFrameIntro::onEndDisplay()
{
  glPopMatrix();
}

// -----------------------------------------------------------------
// Name : onUpdate
// -----------------------------------------------------------------
void guiFrameIntro::onUpdate(double delta)
{
  m_fTimer -= delta;
  if (m_fTimer <= 0)
  {
    m_fTimer = 0;
    m_bActive = false;
    m_bFinished = true;
  }
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiFrameIntro * guiFrameIntro::clone()
{
  guiFrameIntro * pClone = new guiFrameIntro(m_uEffectId, m_fTotalTime);
  return pClone;
}

// -----------------------------------------------------------------
// Name : reset
// -----------------------------------------------------------------
void guiFrameIntro::reset()
{
  guiFrameEffect::reset();
  m_fTimer = m_fTotalTime;
}
