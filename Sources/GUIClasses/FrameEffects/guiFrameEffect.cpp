#include "guiFrameEffect.h"
#include "../guiFrame.h"

// -----------------------------------------------------------------
// Name : guiFrameEffect
//  Constructor
// -----------------------------------------------------------------
guiFrameEffect::guiFrameEffect(u16 uEffectId, u8 onFinished)
{
    m_uEffectId = uEffectId;
    m_bActive = false;
    m_pFrame = NULL;
    m_bFinished = false;
    m_uActionOnFinished = onFinished;
}

// -----------------------------------------------------------------
// Name : ~guiFrameEffect
//  Destructor
// -----------------------------------------------------------------
guiFrameEffect::~guiFrameEffect()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy guiFrameEffect\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy guiFrameEffect\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiFrameEffect::update(double delta)
{
    if (m_bActive && !m_bFinished)
    {
        onUpdate(delta);
        if (m_bFinished)
        {
            switch (m_uActionOnFinished)
            {
            case EFFECT_ACTIVATE_ON_FINISHED:
                m_pFrame->setEnabled(true);
                m_pFrame->getDocument()->setEnabled(true);
                break;
            case EFFECT_HIDE_ON_FINISHED:
                m_pFrame->setVisible(false);
                break;
            case EFFECT_DELFRM_ON_FINISHED:
                m_pFrame->getDocument()->close();
                break;
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : setActive
// -----------------------------------------------------------------
void guiFrameEffect::setActive(bool bActive)
{
    m_bActive = bActive;
}

// -----------------------------------------------------------------
// Name : reset
// -----------------------------------------------------------------
void guiFrameEffect::reset()
{
    if (m_uActionOnFinished == EFFECT_ACTIVATE_ON_FINISHED)
    {
        m_pFrame->setEnabled(false);
        m_pFrame->getDocument()->setEnabled(false);
    }
    m_bActive = true;
    m_bFinished = false;
}
