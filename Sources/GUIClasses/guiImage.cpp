#include "guiImage.h"
#include "../Geometries/GeometryQuads.h"
#include "ComponentOwnerInterface.h"

// -----------------------------------------------------------------
// Name : guiImage
//  Constructor
// -----------------------------------------------------------------
guiImage::guiImage() : guiComponent()
{
  m_iWidth = m_iHeight = -1;
  m_bCatchClicks = false;
}

// -----------------------------------------------------------------
// Name : ~guiImage
//  Destructor
// -----------------------------------------------------------------
guiImage::~guiImage()
{
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiImage::init(int iTexId, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(iTexId);
  if (m_iWidth < 0)
  {
    if (pTex->m_iMasterTexture < 0)
      m_iWidth = pTex->m_iWidth * (pTex->m_fU1 - pTex->m_fU0);
    else
      m_iWidth = pTex->m_iWidth;
  }
  if (m_iHeight < 0)
  {
    if (pTex->m_iMasterTexture < 0)
      m_iHeight = pTex->m_iHeight * (pTex->m_fV1 - pTex->m_fV0);
    else
      m_iHeight = pTex->m_iHeight;
  }
  QuadData quad(0, m_iWidth, 0, m_iHeight, iTexId, pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiImage::clone()
{
  guiImage * pObj = new guiImage();
  pObj->init(((GeometryQuads*)m_pGeometry)->getTexture(), m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiImage::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (!m_bVisible)
    return;
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pGeometry->display(coords, F_RGBA_MULTIPLY(cpntColor, m_DiffuseColor));
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiImage::onResize(int iOldWidth, int iOldHeight)
{
  guiComponent::onResize(iOldWidth, iOldHeight);
  if (iOldWidth == m_iWidth && iOldHeight == m_iHeight)
    return;
  if (m_pGeometry != NULL)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, ((GeometryQuads*)m_pGeometry)->getTexture(), getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(&quad);
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * guiImage::onButtonEvent(ButtonAction * pEvent)
{
  if (pEvent->eButton != Button1 || /*pEvent->eEvent != Event_Down || */!m_bEnabled || !m_bVisible || !m_bCatchClicks)
    return NULL;
  m_pOwner->onButtonEvent(pEvent, this);
  return this;
}

// -----------------------------------------------------------------
// Name : getImageTexture
// -----------------------------------------------------------------
int guiImage::getImageTexture()
{
 if (m_pGeometry)
   return ((GeometryQuads*)m_pGeometry)->getTexture();
 return -1;
}

// -----------------------------------------------------------------
// Name : setImageTexture
// -----------------------------------------------------------------
void guiImage::setImageTexture(int iTexId)
{
 if (m_pGeometry)
   ((GeometryQuads*)m_pGeometry)->setTexture(iTexId);
}
