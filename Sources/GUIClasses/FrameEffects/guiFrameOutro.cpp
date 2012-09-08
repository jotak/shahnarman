#include "guiFrameOutro.h"
#include "../guiFrame.h"
#include "../../SystemHeaders.h"

// -----------------------------------------------------------------
// Name : guiFrameOutro
//  Constructor
// -----------------------------------------------------------------
guiFrameOutro::guiFrameOutro(u16 uEffectId, float fOutroTime, u8 onFinished) : guiFrameEffect(uEffectId, onFinished)
{
    m_fTimer = m_fTotalTime = fOutroTime;
}

// -----------------------------------------------------------------
// Name : ~guiFrameOutro
//  Destructor
// -----------------------------------------------------------------
guiFrameOutro::~guiFrameOutro()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy guiFrameOutro\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy guiFrameOutro\n");
#endif
}

// -----------------------------------------------------------------
// Name : onBeginDisplay
// -----------------------------------------------------------------
void guiFrameOutro::onBeginDisplay(int iXOffset, int iYOffset, F_RGBA * cpntColor, F_RGBA * docColor)
{
    float coef = max(m_fTimer / m_fTotalTime, 0.001f); // must not be 0

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
void guiFrameOutro::onEndDisplay()
{
    glPopMatrix();
}

// -----------------------------------------------------------------
// Name : onUpdate
// -----------------------------------------------------------------
void guiFrameOutro::onUpdate(double delta)
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
guiFrameOutro * guiFrameOutro::clone()
{
    guiFrameOutro * pClone = new guiFrameOutro(m_uEffectId, m_fTotalTime);
    return pClone;
}

// -----------------------------------------------------------------
// Name : reset
// -----------------------------------------------------------------
void guiFrameOutro::reset()
{
    guiFrameEffect::reset();
    m_fTimer = m_fTotalTime;
    m_pFrame->setEnabled(false);
    m_pFrame->getDocument()->setEnabled(false);
}
