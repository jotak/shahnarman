#include "guiToggleButton.h"
#include "ComponentOwnerInterface.h"

// -----------------------------------------------------------------
// Name : guiToggleButton
//  Constructor
// -----------------------------------------------------------------
guiToggleButton::guiToggleButton() : guiButton()
{
}

// -----------------------------------------------------------------
// Name : ~guiToggleButton
//  Destructor
// -----------------------------------------------------------------
guiToggleButton::~guiToggleButton()
{
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiToggleButton::clone()
{
  guiToggleButton * pObj = new guiToggleButton();
  pObj->init(m_pLabel->getText(), m_pLabel->getFontId(), m_pLabel->getDiffuseColor(), (m_pGeometryClicked == NULL) ? -1 : m_pGeometryClicked->getTexture(), m_ClickOption, (m_pGeometryOver == NULL) ? -1 : m_pGeometryOver->getTexture(), m_OverOption, m_pGeometryNormal->getTexture(), m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiToggleButton::onButtonEvent(ButtonAction * pEvent)
{
  if ((pEvent->eButton != Button1 && (pEvent->eButton != Button2 || !m_bCatchButton2Events)) || !m_bEnabled || !m_bVisible)
    return NULL;
  bool bClickState = m_bClickState;
  bool bMouseOver = m_bMouseOver;
  guiObject * pObj = (pEvent->eEvent != Event_Up) ? guiButton::onButtonEvent(pEvent) : this;
  if (pEvent->eEvent == Event_Down)
  {
    m_bClickState = !bClickState;
    return this;
  }
  else if (pEvent->eEvent == Event_Drag)
  {
    m_bClickState = m_bMouseOver == bMouseOver ? bClickState : !bClickState;
    return this;
  }
  else if (pEvent->eEvent == Event_Up)
  {
    m_bMouseDown = false;
    if (m_bMouseOver && m_pOwner != NULL)
      m_pOwner->onButtonEvent(pEvent, this);
    return this;
  }
  return pObj;
}

// -----------------------------------------------------------------
// Name : createDefaultTexturedToggleButton
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiToggleButton * guiToggleButton::createDefaultTexturedToggleButton(int iTex, int iSize, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiToggleButton * pBtn = new guiToggleButton();
  pBtn->init(
    L"", (FontId)0, rgb(0,0,0),
    pDisplay->getTextureEngine()->findTexture(L"interface:Selector"),
    BCO_AddTex, -1, BCO_None, iTex, sId, 0, 0, iSize, iSize, pDisplay);
  return pBtn;
}

// -----------------------------------------------------------------
// Name : createDefaultCheckBox
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiToggleButton * guiToggleButton::createDefaultCheckBox(const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiToggleButton * pBtn = new guiToggleButton();
  pBtn->init(
    L"", (FontId)0, rgb(0,0,0),
    pDisplay->getTextureEngine()->findTexture(L"interface:CheckBoxTick"),
    BCO_AddTex, -1, BCO_None,
    pDisplay->getTextureEngine()->findTexture(L"interface:Transparent"),
    sId, 0, 0, 23, 23, pDisplay);
//  pBtn->m_pLabel->setCatchClicks(true);
//  pBtn->m_pLabel->setComponentOwner(pBtn);
  return pBtn;
}
