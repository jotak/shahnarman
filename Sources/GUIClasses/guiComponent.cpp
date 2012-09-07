#include "guiComponent.h"

// -----------------------------------------------------------------
// Name : guiComponent
//  Constructor
// -----------------------------------------------------------------
guiComponent::guiComponent() : guiObject()
{
  wsafecpy(m_sCpntId, 32, L"");
  m_bEnabled = m_bVisible = true;
  m_uHighlight = HIGHLIGHT_TYPE_NONE;
  m_pOwner = NULL;
}

// -----------------------------------------------------------------
// Name : ~guiComponent
//  Destructor
// -----------------------------------------------------------------
guiComponent::~guiComponent()
{
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiComponent::init(const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl)
{
  guiObject::init(xPxl, yPxl, wPxl, hPxl);
  wsafecpy(m_sCpntId, CPNT_ID_MAX_CHARS, sCpntId);
}

//// -----------------------------------------------------------------
//// Name : clone
//// -----------------------------------------------------------------
//guiObject * guiComponent::clone()
//{
//  guiComponent * pObj = new guiComponent();
//  pObj->init(m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight);
//  return pObj;
//}

// -----------------------------------------------------------------
// Name : highlight
// -----------------------------------------------------------------
void guiComponent::highlight(u8 type)
{
  m_uHighlight = type;
}

// -----------------------------------------------------------------
// Name : centerOnComponent
// -----------------------------------------------------------------
void guiComponent::centerOnComponent(guiComponent * pOther)
{
  moveTo(pOther->getXPos() + (pOther->getWidth() - getWidth()) / 2, pOther->getYPos() + (pOther->getHeight() - getHeight()) / 2);
}

// -----------------------------------------------------------------
// Name : setVisible
// -----------------------------------------------------------------
void guiComponent::setVisible(bool bVisible)
{
  m_bVisible = bVisible;
}
