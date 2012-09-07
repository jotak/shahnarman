#include "guiGauge.h"
#include "../Geometries/GeometryQuads.h"

// -----------------------------------------------------------------
// Name : guiGauge
//  Constructor
// -----------------------------------------------------------------
guiGauge::guiGauge() : guiComponent()
{
  m_iWidth = m_iHeight = -1;
  m_pForegroundGeometry = NULL;
  m_iRefValue = 1;
  m_iCurValue = 0;
  m_Color = F_RGBA_NULL;
}

// -----------------------------------------------------------------
// Name : ~guiGauge
//  Destructor
// -----------------------------------------------------------------
guiGauge::~guiGauge()
{
  FREE(m_pForegroundGeometry);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiGauge::init(int iRef, int iVal, F_RGBA color, int iFgTex, int iBgTex, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay)
{
  guiComponent::init(sCpntId, xPxl, yPxl, wPxl, hPxl);
  m_iRefValue = iRef;
  m_iCurValue = iVal;
  m_Color = color;
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(iBgTex);
  if (m_iWidth < 0)
    m_iWidth = pTex->m_iWidth;
  if (m_iHeight < 0)
    m_iHeight = pTex->m_iHeight;
  QuadData quad(0, m_iWidth, 0, m_iHeight, iBgTex, pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
  int fgw = max(1, m_iWidth * m_iCurValue / m_iRefValue);
  QuadData quad2(0, fgw, 0, m_iHeight, iFgTex, pDisplay);
  m_pForegroundGeometry = new GeometryQuads(&quad2, VB_Static);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
guiObject * guiGauge::clone()
{
  guiGauge * pObj = new guiGauge();
  pObj->init(m_iRefValue, m_iCurValue, m_Color, m_pForegroundGeometry->getTexture(), ((GeometryQuads*)m_pGeometry)->getTexture(), m_sCpntId, m_iXPxl, m_iYPxl, m_iWidth, m_iHeight, getDisplay());
  return pObj;
}

// -----------------------------------------------------------------
// Name : displayAt
// -----------------------------------------------------------------
void guiGauge::displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor, F_RGBA docColor)
{
  if (!m_bVisible)
    return;
  CoordsScreen coords = CoordsScreen(m_iXPxl + iXOffset, m_iYPxl + iYOffset, GUIPLANE);
  m_pForegroundGeometry->display(coords, F_RGBA_MULTIPLY(m_Color, cpntColor)); // actually display foreground before background
  m_pGeometry->display(coords, cpntColor);
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void guiGauge::onResize(int iOldWidth, int iOldHeight)
{
  guiComponent::onResize(iOldWidth, iOldHeight);
  if (iOldWidth == m_iWidth && iOldHeight == m_iHeight)
    return;
  if (m_pGeometry != NULL)
  {
    QuadData quad(0, m_iWidth, 0, m_iHeight, ((GeometryQuads*)m_pGeometry)->getTexture(), getDisplay());
    ((GeometryQuads*)m_pGeometry)->modify(&quad);
    int fgw = max(1, m_iWidth * m_iCurValue / m_iRefValue);
    QuadData quad2(0, fgw, 0, m_iHeight, m_pForegroundGeometry->getTexture(), getDisplay());
    m_pForegroundGeometry->modify(&quad2);
  }
}

// -----------------------------------------------------------------
// Name : setMax
// -----------------------------------------------------------------
void guiGauge::setMax(int iVal)
{
  m_iRefValue = iVal;
  int fgw = max(1, m_iWidth * m_iCurValue / m_iRefValue);
  QuadData quad2(0, fgw, 0, m_iHeight, m_pForegroundGeometry->getTexture(), getDisplay());
  m_pForegroundGeometry->modify(&quad2);
}

// -----------------------------------------------------------------
// Name : setValue
// -----------------------------------------------------------------
void guiGauge::setValue(int iVal)
{
  m_iCurValue = iVal;
  int fgw = max(1, m_iWidth * m_iCurValue / m_iRefValue);
  QuadData quad2(0, fgw, 0, m_iHeight, m_pForegroundGeometry->getTexture(), getDisplay());
  m_pForegroundGeometry->modify(&quad2);
}

// -----------------------------------------------------------------
// Name : createDefaultGauge
//  Static default constructor
// -----------------------------------------------------------------
guiGauge * guiGauge::createDefaultGauge(int iRef, F_RGBA color, int iWidth, int iHeight, const wchar_t * sId, DisplayEngine * pDisplay)
{
  guiGauge * pGauge = new guiGauge();
  pGauge->init(
    iRef, 0, color,
    pDisplay->getTextureEngine()->findTexture(L"interface:GaugeFg"),
    pDisplay->getTextureEngine()->findTexture(L"interface:GaugeBg"),
    sId, 0, 0, iWidth, iHeight, pDisplay);
  return pGauge;
}
