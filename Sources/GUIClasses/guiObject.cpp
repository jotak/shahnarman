#include "guiObject.h"
#include "../GameRoot.h"
#include "../LocalClient.h"
#include "../Interface/InterfaceManager.h"

int guiObject::m_aiAllFonts[];

FontId H1_FONT = Arabolical_wh_32;
F_RGBA H1_COLOR = rgb(1, 1, 1);
FontId H2_FONT = Arabolical_wh_16;
F_RGBA H2_COLOR = rgb(1.0f, 1.0f, 0.6f);
FontId TEXT_FONT = Bookantiqua_wh_16;
F_RGBA TEXT_COLOR = rgb(0.7f, 1.0f, 0.7f);
F_RGBA TEXT_COLOR_DARK = rgb(0,0,0);

// -----------------------------------------------------------------
// Name : guiObject
//  Constructor
// -----------------------------------------------------------------
guiObject::guiObject() : GraphicObject()
{
  m_iXPxl = 0;
  m_iYPxl = 0;
  m_iWidth = 0;
  m_iHeight = 0;
  m_DiffuseColor = F_RGBA_NULL;
  wsafecpy(m_sTooltip, 128, L"");
}

// -----------------------------------------------------------------
// Name : ~guiObject
//  Destructor
// -----------------------------------------------------------------
guiObject::~guiObject()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy guiObject\n");
#endif
  extern GameRoot * g_pMainGameRoot;
  g_pMainGameRoot->m_pLocalClient->getInterface()->resetSharedPointers(this);
#ifdef DBG_VERBOSE1
  printf("End destroy guiObject\n");
#endif
}

// -----------------------------------------------------------------
// Name : isAt
// -----------------------------------------------------------------
bool guiObject::isAt(int xPxl, int yPxl)
{
  return (xPxl >= m_iXPxl && yPxl >= m_iYPxl && xPxl <= m_iXPxl + m_iWidth && yPxl <= m_iYPxl + m_iHeight);
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void guiObject::moveTo(int xPxl, int yPxl)
{
  m_iXPxl = xPxl;
  m_iYPxl = yPxl;
}

// -----------------------------------------------------------------
// Name : moveBy
// -----------------------------------------------------------------
void guiObject::moveBy(int xPxl, int yPxl)
{
  m_iXPxl += xPxl;
  m_iYPxl += yPxl;
}

// -----------------------------------------------------------------
// Name : setWidth
// -----------------------------------------------------------------
void guiObject::setWidth(int iWidth)
{
  int oldw = m_iWidth;
  m_iWidth = iWidth;
  onResize(oldw, m_iHeight);
}

// -----------------------------------------------------------------
// Name : setHeight
// -----------------------------------------------------------------
void guiObject::setHeight(int iHeight)
{
  int oldh = m_iHeight;
  m_iHeight = iHeight;
  onResize(m_iWidth, oldh);
}

// -----------------------------------------------------------------
// Name : setDimensions
// -----------------------------------------------------------------
void guiObject::setDimensions(int iWidth, int iHeight)
{
  int oldw = m_iWidth;
  m_iWidth = iWidth;
  int oldh = m_iHeight;
  m_iHeight = iHeight;
  onResize(oldw, oldh);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void guiObject::init(int xPxl, int yPxl, int wPxl, int hPxl)
{
  m_iXPxl = xPxl;
  m_iYPxl = yPxl;
  m_iWidth = wPxl;
  m_iHeight = hPxl;
}

//// -----------------------------------------------------------------
//// Name : clone
//// -----------------------------------------------------------------
//guiObject * guiObject::clone()
//{
//  guiObject * obj = new guiObject();
//  obj->init(m_iXPxl, m_iYPxl, m_iWidth, m_iHeight);
//  return obj;
//}

// -----------------------------------------------------------------
// Name : registerTextures
//  Static function
// -----------------------------------------------------------------
void guiObject::registerTextures(TextureEngine * pTexEngine, FontEngine * pFontEngine)
{
  m_aiAllFonts[(int)Arabolical_wh_16] = pFontEngine->registerFont(L"Arabolical_16", pTexEngine);
  m_aiAllFonts[(int)Arabolical_wh_32] = pFontEngine->registerFont(L"Arabolical_32", pTexEngine);
  m_aiAllFonts[(int)Argos_wh_16] = pFontEngine->registerFont(L"Argos_16", pTexEngine);
  m_aiAllFonts[(int)Blackchancery_wh_16] = pFontEngine->registerFont(L"BlackChancery_16", pTexEngine);
  m_aiAllFonts[(int)Bookantiqua_wh_16] = pFontEngine->registerFont(L"BookAntiqua_16", pTexEngine);
}
