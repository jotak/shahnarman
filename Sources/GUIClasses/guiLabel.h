#ifndef _GUI_LABEL_H
#define _GUI_LABEL_H

#include "guiComponent.h"

#define LABEL_MAX_CHARS    2048

class guiLabel : public guiComponent
{
public:
  guiLabel();
  ~guiLabel();

  // Inherited functions
  virtual u32 getType() { return guiComponent::getType() | GOTYPE_LABEL; };
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);

  // Other functions
  FontId getFontId() { return m_FontId; };
  void setFontId(FontId id) { m_FontId = id; };
  int getBoxWidth() { return m_iBoxWidth; };
  void setBoxWidth(int iWidth);
  void setText(const char * sText);
  char * getText() { return m_sText; };
  void setCatchClicks(bool b) { m_bCatchClicks = b; };
  void setComponentOwner(guiComponent * pOwner) { m_pComponentOwner = pOwner; };

  // Clone / init
  virtual void init(const char * sText, FontId fontId, F_RGBA textColor, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

protected:
  void computeGeometry(DisplayEngine * pDisplay);

  char m_sText[LABEL_MAX_CHARS];
  FontId m_FontId;
  int m_iBoxWidth;
  bool m_bCatchClicks;
  guiComponent * m_pComponentOwner;
};

#endif
