#ifndef _GUI_OBJECT_H
#define _GUI_OBJECT_H

#include "../Common/GraphicObject.h"
#include "../Input/InputEngine.h"

#define NB_FONTS    5
enum FontId
{
  Arabolical_wh_16 = 0,
  Arabolical_wh_32,
  Argos_wh_16,
  Blackchancery_wh_16,
  Bookantiqua_wh_16
};

extern FontId H1_FONT;
extern F_RGBA H1_COLOR;
extern FontId H2_FONT;
extern F_RGBA H2_COLOR;
extern FontId TEXT_FONT;
extern F_RGBA TEXT_COLOR;
extern F_RGBA TEXT_COLOR_DARK;

class guiObject : public GraphicObject
{
public:
  guiObject();
  ~guiObject();

  // Texture(s) registration
  static void registerTextures(TextureEngine * pTexEngine, FontEngine * pFontEngine);

  // Update / display
  virtual void update(double delta) {};
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL) = 0;

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent) { return NULL; };
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl) { return this; };
  virtual void onCursorMoveOutEvent() {};

  // Size and position
  int getXPos() { return m_iXPxl; };
  int getYPos() { return m_iYPxl; };
  int getWidth() { return m_iWidth; };
  int getHeight() { return m_iHeight; };
  virtual void setXPos(int iXPxl) { m_iXPxl = iXPxl; };
  virtual void setYPos(int iYPxl) { m_iYPxl = iYPxl; };
  virtual void setWidth(int iWidth);
  virtual void setHeight(int iHeight);
  virtual void setDimensions(int iWidth, int iHeight);
  virtual bool isAt(int xPxl, int yPxl);
  virtual void moveTo(int xPxl, int yPxl);
  virtual void moveBy(int xPxl, int yPxl);
  virtual void onResize(int iOldWidth, int iOldHeight) {};

  // Clone and init
  virtual void init(int xPxl, int yPxl, int wPxl, int hPxl);

  // Tooltip text
  char * getTooltipText() { return m_sTooltip; };
  void setTooltipText(const char * sTooltip) { wsafecpy(m_sTooltip, 256, sTooltip); };

  // Other
  F_RGBA  getDiffuseColor() { return m_DiffuseColor; };
  void setDiffuseColor(F_RGBA color) { m_DiffuseColor = color; };

protected:
  int m_iXPxl;
  int m_iYPxl;
  int m_iWidth;
  int m_iHeight;
  char m_sTooltip[256];
  F_RGBA m_DiffuseColor;

  static int m_aiAllFonts[NB_FONTS];
};

#endif
