#include "guiSmartSlider.h"
#include "ComponentOwnerInterface.h"

// -----------------------------------------------------------------
// Name : guiSmartSlider
//  Constructor
// -----------------------------------------------------------------
guiSmartSlider::guiSmartSlider() : guiComponent()
{
  m_pItems = new ObjectList(true);
  m_iSliderPos = 0;
  m_iItemSize = 0;
  m_pDisabledGeometry = NULL;
  m_pSelectorGeometry = NULL;
  m_pSelectedItem = NULL;
  m_pLabel = new guiLabel();
  m_pDisableReasonLabel = new guiLabel();
  m_iSpacing = m_iTheoricSize = m_iSelectorPos = 0;
}

// -----------------------------------------------------------------
// Name : ~guiSmartSlider
//  Destructor
// -----------------------------------------------------------------
guiSmartSlider::~guiSmartSlider()
{
  delete m_pItems;
  FREE(m_pDisabledGeometry);
  FREE(m_pSelectorGeometry);
  delete m_pLabel;
  delete m_pDisableReasonLabel;
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiSmartSlider::init(int iItemSize, int iSpacing, FontId fontId, F_RGBA textColor, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_pLabel->init(L"dummy", fontId, textColor, L"", 0, 0, 0, 0, pDisplay);
  m_pDisableReasonLabel->init(L"dummy", fontId, rgb(1,0,0), L"", 0, 0, 0, 0, pDisplay);
  m_iSpacing = iSpacing;
  m_iItemSize = iItemSize;

  QuadData quad1(0, m_iItemSize, 0, m_iItemSize, L"EmptyWhiteSquare", pDisplay);
  m_pSelectorGeometry = new GeometryQuads(&quad1, VB_Static);
  QuadData quad2(0, m_iItemSize, 0, m_iItemSize, L"Disabled", pDisplay);
  m_pDisabledGeometry = new GeometryQuads(&quad2, VB_Static);

  loadGeometry(pDisplay);
  setHeight(m_iItemSize + m_pLabel->getHeight() + 3);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiSmartSlider::clone()
{
  guiSmartSlider * pObj = new guiSmartSlider();
  pObj->init(m_iItemSize, m_iSpacing, m_pLabel->getFontId(), m_pLabel->getDiffuseColor(), m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiSmartSlider::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (m_bVisible)
  {
    if (!m_bEnabled)
      cpntColor = F_RGBA_MULTIPLY(cpntColor, rgb(0.3f, 0.3f, 0.3f));
    CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset + m_iSliderPos, m_iYPxl + iYOffset, GUIPLANE);
    ((GeometryQuads*)m_pGeometry)->display(coords, cpntColor);

    // Find all disabled items
    CoordsScreen darkCoords = coords;
    guiSliderItem * pItem = (guiSliderItem*) m_pItems->getFirst(0);
    while (pItem != NULL)
    {
      if (!pItem->m_bEnabled)
        m_pDisabledGeometry->display(darkCoords, cpntColor);
      darkCoords.x += m_iItemSize + m_iSpacing;
      pItem = (guiSliderItem*) m_pItems->getNext(0);
    }

    // Draw selection-related (selector, label...)
    if (m_pSelectedItem != NULL)
    {
      // just ignore stencil for text
      m_pLabel->displayAt(iXOffset, iYOffset, cpntColor);
      if (!m_pSelectedItem->m_bEnabled)
        m_pDisableReasonLabel->displayAt(iXOffset, iYOffset, cpntColor);
//      bool bWasBlending = (blendColor.a >= 0 && blendColor.a < 1);
//      if (!bWasBlending)
        getDisplay()->enableBlending();
      cpntColor = F_RGBA_MULTIPLY(cpntColor, rgba(1, 1, 1, 0.3f));
      coords.x += m_iSelectorPos;
      m_pSelectorGeometry->display(coords, cpntColor);
//      if (!bWasBlending)
//        getDisplay()->disableBlending();
    }
  }
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * guiSmartSlider::onCursorMoveEvent(int xPxl, int yPxl)
{
  if (!m_bEnabled || !m_bVisible)
    return NULL;
  guiComponent::onCursorMoveEvent(xPxl, yPxl);

  xPxl -= m_iXPxl;
  yPxl -= m_iYPxl;
  if (yPxl >= m_iItemSize)
  {
    m_pSelectedItem = NULL;
    return this;
  }

  m_iSliderPos = xPxl - (m_iTheoricSize * xPxl / m_iWidth);
  m_iSelectorPos = 0;
  guiSliderItem * pItem = (guiSliderItem*) m_pItems->getFirst(0);
  while (pItem != NULL)
  {
    if (xPxl < m_iSelectorPos + m_iItemSize + m_iSliderPos)
    {
      if (m_pSelectedItem != pItem)
      {
        m_pSelectedItem = pItem;
        wchar_t str[SLIDER_ITEM_MAX_CHARS];
        m_pLabel->setText(pItem->getInfo(str, SLIDER_ITEM_MAX_CHARS));
        int textX = (getWidth() - m_pLabel->getWidth()) / 2;
        int textY = getYPos() + getHeight() - m_pLabel->getHeight();
        if (!m_pSelectedItem->m_bEnabled)
        {
          m_pDisableReasonLabel->setText(pItem->m_sDisabledReason);
          textX -= m_pDisableReasonLabel->getWidth() / 2 - 5;
          m_pDisableReasonLabel->moveTo(textX + m_pLabel->getWidth() + 10, textY);
        }
        m_pLabel->moveTo(textX, textY);
      }
      return this;
    }
    m_iSelectorPos += m_iItemSize + m_iSpacing;
    pItem = (guiSliderItem*) m_pItems->getNext(0);
  }
  m_pSelectedItem = NULL;
  return this;
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiSmartSlider::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled || !m_bVisible)
    return NULL;
  if (m_pSelectedItem != NULL && m_pSelectedItem->m_bEnabled && pEvent->eButton == Button1 && pEvent->eEvent == Event_Down)
  {
    m_pOwner->onButtonEvent(pEvent, this);
    return NULL;
  }
  else
    return NULL;
}

// -----------------------------------------------------------------
// Name : addItem
// -----------------------------------------------------------------
void guiSmartSlider::addItem(guiSliderItem * pItem, bool bFirst)
{
  if (bFirst)
    m_pItems->addFirst(pItem);
  else
    m_pItems->addLast(pItem);
}

// -----------------------------------------------------------------
// Name : deleteItems
// -----------------------------------------------------------------
void guiSmartSlider::deleteItems()
{
  m_pItems->deleteAll();
  m_iSliderPos = 0;
  m_pSelectedItem = NULL;
  m_pLabel->setText(L"");
  m_pDisableReasonLabel->setText(L"");
}

// -----------------------------------------------------------------
// Name : loadGeometry
// -----------------------------------------------------------------
void guiSmartSlider::loadGeometry(DisplayEngine * pDisplay)
{
  if (pDisplay == NULL)
  {
    if (m_pGeometry == NULL)
      return;
    else
      pDisplay = m_pGeometry->getDisplay();
  }

  m_iTheoricSize = 0;
  if (m_pItems->size == 0)
  {
    if (m_pGeometry == NULL)
      m_pGeometry = new GeometryQuads(VB_Static, pDisplay);
    else
      ((GeometryQuads*)m_pGeometry)->modify(0, NULL);
  }
  else
  {
    QuadData ** pQuads = new QuadData*[m_pItems->size];
    int i = 0;
    guiSliderItem * pItem = (guiSliderItem*) m_pItems->getFirst(0);
    while (pItem != NULL)
    {
      pQuads[i] = new QuadData(m_iTheoricSize, m_iTheoricSize + m_iItemSize, 0, m_iItemSize, pItem->m_iTexId, pDisplay);
      m_iTheoricSize += m_iItemSize + m_iSpacing;
      i++;
      pItem = (guiSliderItem*) m_pItems->getNext(0);
    }
    if (m_pGeometry == NULL)
      m_pGeometry = new GeometryQuads(m_pItems->size, pQuads, VB_Static);
    else
      ((GeometryQuads*)m_pGeometry)->modify(m_pItems->size, pQuads);
    QuadData::releaseQuads(m_pItems->size, pQuads);
  }
  if (m_iTheoricSize < m_iWidth)
	  m_iTheoricSize = m_iWidth;
}
