#include "guiContainer.h"

#define SCROLLSTEP          30    // in pixels

// -----------------------------------------------------------------
// Name : guiContainer
//  Fixed-sized constructor
// -----------------------------------------------------------------
guiContainer::guiContainer() : guiComponent()
{
  m_iMaxWidth = m_iMaxHeight = m_iInnerXPxl = m_iInnerYPxl = m_iInnerWidth = m_iInnerHeight = 0;
  m_pDoc = NULL;
  m_pStencilGeometry = NULL;
  m_WidthFit = m_HeightFit = FB_NoFit;
  m_iXOffset = m_iYOffset = 0;
}

// -----------------------------------------------------------------
// Name : ~guiContainer
//  Destructor
// -----------------------------------------------------------------
guiContainer::~guiContainer()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy guiContainer\n");
#endif
  FREE(m_pDoc);
  FREE(m_pStencilGeometry);
#ifdef DBG_VERBOSE1
  printf("End destroy guiContainer\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiContainer::init(FrameFitBehavior widthFit, FrameFitBehavior heightFit, int iXOffset, int iYOffset, int iMaxWidth, int iMaxHeight, int * iMainTexs, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_iMaxWidth = iMaxWidth;
  m_iMaxHeight = iMaxHeight;
  m_WidthFit = widthFit;
  m_HeightFit = heightFit;
  m_iXOffset = iXOffset;
  m_iYOffset = iYOffset;

  Texture * pTLTex = pDisplay->getTextureEngine()->getTexture(iMainTexs[0]);
  Texture * pBRTex = pDisplay->getTextureEngine()->getTexture(iMainTexs[7]);
  m_iInnerXPxl = m_iXPxl + m_iXOffset + pTLTex->m_iWidth;
  m_iInnerYPxl = m_iYPxl + m_iYOffset + pTLTex->m_iHeight;
  m_iInnerWidth = m_iWidth - pTLTex->m_iWidth - pBRTex->m_iWidth - m_iXOffset;
  m_iInnerHeight = m_iHeight - pTLTex->m_iHeight - pBRTex->m_iHeight - m_iYOffset;

  QuadData ** pQuads;
  int nQuads = computeQuadsList(&pQuads, iMainTexs, pDisplay);
  m_pGeometry = new GeometryQuads(nQuads, pQuads, VB_Static);
  QuadData::releaseQuads(nQuads, pQuads);
  m_pStencilGeometry = new StencilGeometry(m_iInnerWidth, m_iInnerHeight, VB_Static, pDisplay);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiContainer::clone()
{
  int texlist[8] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7) };
  guiContainer * pObj = new guiContainer();
  pObj->init(m_WidthFit, m_HeightFit, m_iXOffset, m_iYOffset, m_iMaxWidth, m_iMaxHeight, texlist, m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : computeQuadsList
// -----------------------------------------------------------------
int guiContainer::computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay)
{
  // 8 Quads
  *pQuads = new QuadData*[8];
  int xPxlMiddleStart = pDisplay->getTextureEngine()->getTexture(iTextures[0])->m_iWidth;
  int xPxlMiddleEnd = m_iWidth - m_iXOffset - pDisplay->getTextureEngine()->getTexture(iTextures[2])->m_iWidth;
  int yPxlMiddleStart = pDisplay->getTextureEngine()->getTexture(iTextures[0])->m_iHeight;
  int yPxlMiddleEnd = m_iHeight - m_iYOffset - pDisplay->getTextureEngine()->getTexture(iTextures[5])->m_iHeight;
  (*pQuads)[0] = new QuadData(0,               xPxlMiddleStart,       0,                yPxlMiddleStart,        iTextures[0], pDisplay);
  (*pQuads)[1] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd,         0,                yPxlMiddleStart,        iTextures[1], pDisplay);
  (*pQuads)[2] = new QuadData(xPxlMiddleEnd,   m_iWidth - m_iXOffset, 0,                yPxlMiddleStart,        iTextures[2], pDisplay);
  (*pQuads)[3] = new QuadData(0,               xPxlMiddleStart,       yPxlMiddleStart,  yPxlMiddleEnd,          iTextures[3], pDisplay);
  (*pQuads)[4] = new QuadData(xPxlMiddleEnd,   m_iWidth - m_iXOffset, yPxlMiddleStart,  yPxlMiddleEnd,          iTextures[4], pDisplay);
  (*pQuads)[5] = new QuadData(0,               xPxlMiddleStart,       yPxlMiddleEnd,    m_iHeight - m_iYOffset, iTextures[5], pDisplay);
  (*pQuads)[6] = new QuadData(xPxlMiddleStart, xPxlMiddleEnd,         yPxlMiddleEnd,    m_iHeight - m_iYOffset, iTextures[6], pDisplay);
  (*pQuads)[7] = new QuadData(xPxlMiddleEnd,   m_iWidth - m_iXOffset, yPxlMiddleEnd,    m_iHeight - m_iYOffset, iTextures[7], pDisplay);
  return 8;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void guiContainer::update(double delta)
{
  if (!m_bVisible)
    return;
  if (m_pDoc != NULL)
  {
    if (m_pDoc->doesNeedDestroy())
    {
      delete m_pDoc;
      m_pDoc = NULL;
    }
    else
    {
      if (m_bEnabled != m_pDoc->isEnabled())
        setEnabled(m_pDoc->isEnabled());
      updateSizeFit();
      m_pDoc->update(delta);
    }
  }
  guiComponent::update(delta);
}

// -----------------------------------------------------------------
// Name : updateSizeFit
// -----------------------------------------------------------------
void guiContainer::updateSizeFit()
{
  Texture * pTLTex = getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(0));
  Texture * pBRTex = getDisplay()->getTextureEngine()->getTexture(((GeometryQuads*)m_pGeometry)->getTexture(7));

  if (m_pDoc->getWidth() != m_iInnerWidth)
  {
    switch (m_WidthFit)
    {
    case FB_FitDocumentToFrame:
      m_pDoc->setWidth(m_iInnerWidth);
      break;
    case FB_FitDocumentToFrameWhenSmaller:
      {
        if (m_pDoc->getWidth() < m_iInnerWidth)
          m_pDoc->setWidth(m_iInnerWidth);
        break;
      }
    case FB_FitFrameToDocument:
      m_iInnerWidth = m_pDoc->getWidth();
      setWidth(m_iInnerWidth + pTLTex->m_iWidth + pBRTex->m_iWidth + m_iXOffset);
      break;
    case FB_FitFrameToDocumentWhenSmaller:
      {
        int maxInnerW = m_iMaxWidth - pTLTex->m_iWidth - pBRTex->m_iWidth - m_iXOffset;
        if (m_pDoc->getWidth() < maxInnerW)
        {
          m_iInnerWidth = m_pDoc->getWidth();
          setWidth(m_iInnerWidth + pTLTex->m_iWidth + pBRTex->m_iWidth + m_iXOffset);
        }
        else if (maxInnerW != m_iInnerWidth)
        {
          m_iInnerWidth = maxInnerW;
          setWidth(m_iInnerWidth + pTLTex->m_iWidth + pBRTex->m_iWidth + m_iXOffset);
        }
        break;
      }
    default:
      break;
    }
  }
  if (m_pDoc->getHeight() != m_iInnerHeight)
  {
    switch (m_HeightFit)
    {
    case FB_FitDocumentToFrame:
      m_pDoc->setHeight(m_iInnerHeight);
      break;
    case FB_FitDocumentToFrameWhenSmaller:
      {
        if (m_pDoc->getHeight() < m_iInnerHeight)
          m_pDoc->setHeight(m_iInnerHeight);
        break;
      }
    case FB_FitFrameToDocument:
      m_iInnerHeight = m_pDoc->getHeight();
      setHeight(m_iInnerHeight + pTLTex->m_iHeight + pBRTex->m_iHeight + m_iYOffset);
      break;
    case FB_FitFrameToDocumentWhenSmaller:
      {
        int maxInnerH = m_iMaxHeight - pTLTex->m_iHeight - pBRTex->m_iHeight - m_iYOffset;
        if (m_pDoc->getHeight() < maxInnerH)
        {
          m_iInnerHeight = m_pDoc->getHeight();
          setHeight(m_iInnerHeight + pTLTex->m_iHeight + pBRTex->m_iHeight + m_iYOffset);
        }
        else if (maxInnerH != m_iInnerHeight)
        {
          m_iInnerHeight = maxInnerH;
          setHeight(m_iInnerHeight + pTLTex->m_iHeight + pBRTex->m_iHeight + m_iYOffset);
        }
        break;
      }
    default:
      break;
    }
  }
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiContainer::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  CoordsScreen coords;
  if (!m_bVisible)
    return;
  if (m_pDoc != NULL)
  {
    coords = CoordsScreen(m_iInnerXPxl + iXOffset, m_iInnerYPxl + iYOffset, GUIPLANE);
    m_pStencilGeometry->fillStencil(coords, true);
    int iPreviousState = getDisplay()->setStencilState(2);
    docColor = m_pDoc->isEnabled() ? docColor : rgba(0.5f, 0.5f, 0.5f, 0.5f);
    cpntColor = m_pDoc->isEnabled() ? cpntColor : rgba(0.5f, 0.5f, 0.5f, 0.5f);
    m_pDoc->displayAt(m_iInnerXPxl + iXOffset, m_iInnerYPxl + iYOffset, cpntColor, docColor);
    m_pStencilGeometry->fillStencil(coords, false);
    getDisplay()->setStencilState(iPreviousState);
  }
  coords = CoordsScreen(m_iXPxl + iXOffset + m_iXOffset, m_iYPxl + iYOffset + m_iYOffset, GUIPLANE);
  m_pGeometry->display(coords, docColor);
}

// -----------------------------------------------------------------
// Name : checkDocumentPosition
// -----------------------------------------------------------------
void guiContainer::checkDocumentPosition()
{
  int iDocX = m_pDoc->getXPos();
  int iDocY = m_pDoc->getYPos();
  if (iDocX < m_iInnerWidth - m_pDoc->getWidth())
  {
    iDocX = m_iInnerWidth - m_pDoc->getWidth();
    m_pDoc->moveTo(iDocX, iDocY);
  }
  if (iDocY < m_iInnerHeight - m_pDoc->getHeight())
  {
    iDocY = m_iInnerHeight - m_pDoc->getHeight();
    m_pDoc->moveTo(iDocX, iDocY);
  }
  if (iDocX > 0)
  {
    iDocX = 0;
    m_pDoc->moveTo(0, iDocY);
  }
  if (iDocY > 0)
  {
    iDocY = 0;
    m_pDoc->moveTo(iDocX, 0);
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiContainer::onButtonEvent(ButtonAction * pEvent)
{
  if (!m_bEnabled || !m_bVisible || m_pDoc == NULL)
    return NULL;

  if (pEvent->eEvent == Event_Down && isDocumentAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset))
  {
    pEvent->xOffset += m_iInnerXPxl + m_pDoc->getXPos();
    pEvent->yOffset += m_iInnerYPxl + m_pDoc->getYPos();
    guiObject * pObj = m_pDoc->onButtonEvent(pEvent);
    if (pObj != NULL)
      return pObj;
  }
  if (pEvent->eButton == ButtonX)
  {
    m_pDoc->moveBy(0, -SCROLLSTEP);
    checkDocumentPosition();
    return this;
  }
  if (pEvent->eButton == ButtonZ)
  {
    m_pDoc->moveBy(0, SCROLLSTEP);
    checkDocumentPosition();
    return this;
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * guiContainer::onCursorMoveEvent(int xPxl, int yPxl)
{
  if (!m_bVisible)
    return NULL;
  if (m_pDoc != NULL)
  {
    if (m_pDoc->isEnabled() && isDocumentAt(xPxl, yPxl))
      return m_pDoc->onCursorMoveEvent(xPxl - m_iInnerXPxl - m_pDoc->getXPos(), yPxl - m_iInnerYPxl - m_pDoc->getYPos());
  }
  return this;
}

// -----------------------------------------------------------------
// Name : isDocumentAt
// -----------------------------------------------------------------
bool guiContainer::isDocumentAt(int xPxl, int yPxl)
{
  return (m_bVisible && xPxl >= m_iInnerXPxl && yPxl >= m_iInnerYPxl && xPxl <= m_iInnerXPxl + m_iInnerWidth && yPxl <= m_iInnerYPxl + m_iInnerHeight);
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void guiContainer::moveTo(int xPxl, int yPxl)
{
  m_iInnerXPxl += xPxl - m_iXPxl;
  m_iInnerYPxl += yPxl - m_iYPxl;
  guiComponent::moveTo(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : moveBy
// -----------------------------------------------------------------
void guiContainer::moveBy(int xPxl, int yPxl)
{
  guiComponent::moveBy(xPxl, yPxl);
  m_iInnerXPxl += xPxl;
  m_iInnerYPxl += yPxl;
}

// -----------------------------------------------------------------
// Name : scrollToTop
// -----------------------------------------------------------------
void guiContainer::scrollToTop()
{
  m_pDoc->moveTo(m_pDoc->getXPos(), 0);
}

// -----------------------------------------------------------------
// Name : scrollToBottom
// -----------------------------------------------------------------
void guiContainer::scrollToBottom()
{
  m_pDoc->moveTo(m_pDoc->getXPos(), m_iInnerHeight - m_pDoc->getHeight());
}

// -----------------------------------------------------------------
// Name : scrollToLeft
// -----------------------------------------------------------------
void guiContainer::scrollToLeft()
{
  m_pDoc->moveTo(0, m_pDoc->getYPos());
}

// -----------------------------------------------------------------
// Name : scrollToRight
// -----------------------------------------------------------------
void guiContainer::scrollToRight()
{
  m_pDoc->moveTo(m_iInnerWidth - m_pDoc->getWidth(), m_pDoc->getYPos());
}

// -----------------------------------------------------------------
// Name : setDocument
// -----------------------------------------------------------------
void guiContainer::setDocument(guiDocument * pDoc)
{
  m_pDoc = pDoc;
  pDoc->setOwner(this);
  pDoc->moveTo(0, 0);
  if (m_pGeometry != NULL)
    updateSizeFit();
}

// -----------------------------------------------------------------
// Name : unsetDocument
// -----------------------------------------------------------------
guiDocument * guiContainer::unsetDocument()
{
  if (m_pDoc == NULL)
    return NULL;
  guiDocument * pDoc = m_pDoc;
  m_pDoc = NULL;
  return pDoc;
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiContainer::onResize(int iOldWidth, int iOldHeight)
{
  guiComponent::onResize(iOldWidth, iOldHeight);
  if (m_iWidth == iOldWidth && m_iHeight == iOldHeight)
    return;
  if (m_pGeometry != NULL)
  {
    int texlist[8] = { ((GeometryQuads*)m_pGeometry)->getTexture(0), ((GeometryQuads*)m_pGeometry)->getTexture(1), ((GeometryQuads*)m_pGeometry)->getTexture(2), ((GeometryQuads*)m_pGeometry)->getTexture(3), ((GeometryQuads*)m_pGeometry)->getTexture(4), ((GeometryQuads*)m_pGeometry)->getTexture(5), ((GeometryQuads*)m_pGeometry)->getTexture(6), ((GeometryQuads*)m_pGeometry)->getTexture(7) };
    m_iInnerWidth = m_iWidth - m_iXOffset - getDisplay()->getTextureEngine()->getTexture(texlist[0])->m_iWidth - getDisplay()->getTextureEngine()->getTexture(texlist[7])->m_iWidth;
    m_iInnerHeight = m_iHeight - m_iYOffset - getDisplay()->getTextureEngine()->getTexture(texlist[0])->m_iHeight - getDisplay()->getTextureEngine()->getTexture(texlist[7])->m_iHeight;

    QuadData ** pQuads;
    int nQuads = computeQuadsList(&pQuads, texlist, getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(nQuads, pQuads);
    QuadData::releaseQuads(nQuads, pQuads);
    m_pStencilGeometry->resize(m_iInnerWidth, m_iInnerHeight);
  }
}

// -----------------------------------------------------------------
// Name : getTargetedObject
// -----------------------------------------------------------------
BaseObject * guiContainer::getTargetedObject(u8 * isLuaPlayerGO)
{
  if (m_pDoc == NULL)
    return NULL;
  else
    return m_pDoc->getTargetedObject(isLuaPlayerGO);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void guiContainer::setTargetValid(bool bValid)
{
  if (m_pDoc != NULL)
    m_pDoc->setTargetValid(bValid);
}

// -----------------------------------------------------------------
// Name : setVisible
// -----------------------------------------------------------------
void guiContainer::setVisible(bool bVisible)
{
  bool bWasVisible = isVisible();
  guiComponent::setVisible(bVisible);
  if (m_pDoc != NULL && bVisible && !bWasVisible)
    m_pDoc->onShow();
  else if (m_pDoc != NULL && !bVisible && bWasVisible)
    m_pDoc->onHide();
}

// -----------------------------------------------------------------
// Name : createDefaultPanel
//  Static default constructor
//  Use it to avoid passing always the same 3591218 arguments to "init"
// -----------------------------------------------------------------
guiContainer * guiContainer::createDefaultPanel(int width, int height, wchar_t * sId, DisplayEngine * pDisplay)
{
  guiContainer * pPanel = new guiContainer();
  int frmtex[8];
  frmtex[0] = pDisplay->getTextureEngine()->findTexture(L"interface:LstTL");
  frmtex[1] = pDisplay->getTextureEngine()->findTexture(L"interface:LstTC");
  frmtex[2] = pDisplay->getTextureEngine()->findTexture(L"interface:LstTR");
  frmtex[3] = pDisplay->getTextureEngine()->findTexture(L"interface:LstCL");
  frmtex[4] = pDisplay->getTextureEngine()->findTexture(L"interface:LstCR");
  frmtex[5] = pDisplay->getTextureEngine()->findTexture(L"interface:LstBL");
  frmtex[6] = pDisplay->getTextureEngine()->findTexture(L"interface:LstBC");
  frmtex[7] = pDisplay->getTextureEngine()->findTexture(L"interface:LstBR");
  pPanel->init(
    FB_FitDocumentToFrameWhenSmaller,
    FB_FitDocumentToFrameWhenSmaller,
    0, 0, 0, 0, frmtex, sId, 0, 0, width, height, pDisplay);

  // Attach document
  guiDocument * pDoc = new guiDocument();
  pDoc->init(
    L"",
    pDisplay->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pDisplay);
  pPanel->setDocument(pDoc);

  return pPanel;
}
