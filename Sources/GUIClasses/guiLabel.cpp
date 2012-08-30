#include "guiLabel.h"
#include "../Geometries/GeometryText.h"
#include "ComponentOwnerInterface.h"

// -----------------------------------------------------------------
// Name : guiLabel
//  Constructor
// -----------------------------------------------------------------
guiLabel::guiLabel() : guiComponent()
{
  wsafecpy(m_sText, LABEL_MAX_CHARS, L"");
  m_FontId = (FontId)0;
  m_iBoxWidth = 0;
  m_bCatchClicks = false;
  m_pComponentOwner = NULL;
}

// -----------------------------------------------------------------
// Name : ~guiLabel
//  Destructor
// -----------------------------------------------------------------
guiLabel::~guiLabel()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy guiLabel\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy guiLabel\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiLabel::init(const wchar_t * sText, FontId fontId, F_RGBA textColor, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_FontId = fontId;
  setDiffuseColor(textColor);
  m_iBoxWidth = wPxl;
  wsafecpy(m_sText, LABEL_MAX_CHARS, sText);
  computeGeometry(pDisplay);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiLabel::clone()
{
  guiLabel * pObj = new guiLabel();
  pObj->init(m_sText, m_FontId, getDiffuseColor(), m_sCpntId, m_iXPxl, m_iYPxl, m_iBoxWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiLabel::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (!m_bVisible)
    return;
  cpntColor = F_RGBA_MULTIPLY(cpntColor, m_DiffuseColor);
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pGeometry->display(coords, cpntColor);
}

// -----------------------------------------------------------------
// Name : setText
// -----------------------------------------------------------------
void guiLabel::setText(const wchar_t * sText)
{
  if (wcscmp(sText, m_sText) == 0)
    return;
  wsafecpy(m_sText, LABEL_MAX_CHARS, sText);
  computeGeometry(getDisplay());
}

// -----------------------------------------------------------------
// Name : setBoxWidth
// -----------------------------------------------------------------
void guiLabel::setBoxWidth(int iWidth)
{
  m_iBoxWidth = iWidth;
  computeGeometry(getDisplay());
}

// -----------------------------------------------------------------
// Name : computeGeometry
// -----------------------------------------------------------------
void guiLabel::computeGeometry(DisplayEngine * pDisplay)
{
  if (pDisplay == NULL)
    return;
  if (m_iBoxWidth > 0)
  {
    setHeight(pDisplay->getFontEngine()->putStringInBox(m_sText, m_iBoxWidth, m_aiAllFonts[(int)m_FontId]));
    setWidth(pDisplay->getFontEngine()->getStringLength(m_sText, m_aiAllFonts[(int)m_FontId]));
  }
  else
  {
    setWidth(pDisplay->getFontEngine()->getStringLength(m_sText, m_aiAllFonts[(int)m_FontId]));
    setHeight(pDisplay->getFontEngine()->getStringHeight(m_sText, m_aiAllFonts[(int)m_FontId]));
  }
  if (m_pGeometry != NULL)
    ((GeometryText*)m_pGeometry)->setText(m_sText, m_aiAllFonts[(int)m_FontId]);
  else
    m_pGeometry = new GeometryText(m_sText, m_aiAllFonts[(int)m_FontId], VB_Static, pDisplay);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiLabel::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled || !m_bVisible || !m_bCatchClicks)
    return NULL;
  if (m_pComponentOwner != NULL)
  {
    m_pComponentOwner->onButtonEvent(pEvent);
    return this;
  }
  if (pEvent->eButton != Button1 || pEvent->eEvent != Event_Down || !m_bEnabled || !m_bVisible || !m_bCatchClicks)
    return NULL;
  m_pOwner->onButtonEvent(pEvent, this);
  return this;
}
