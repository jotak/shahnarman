#ifndef _GUI_BUTTON_H
#define _GUI_BUTTON_H

#include "guiImage.h"
#include "guiLabel.h"
#include "../Geometries/GeometryQuads.h"

#define BUTTON_MAX_CHARS            64

enum BtnClickOptions
{
  BCO_None = 0,
  BCO_ReplaceTex,
  BCO_AddTex,
  BCO_Decal,
  BCO_Scale,
  BCO_Enlight
};

class guiButton : public guiImage
{
public:
  // Constructor / destructor
  guiButton();
  ~guiButton();

  // Inherited functions
  virtual u32 getType() { return guiComponent::getType() | GOTYPE_BUTTON; };
  virtual void update(double delta);
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);
  virtual void moveTo(int xPxl, int yPxl);
  virtual void moveBy(int xPxl, int yPxl);
  virtual void setEnabled(bool bEnabled);

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);
  virtual void onCursorMoveOutEvent();

  // Member access functions
  char * getText() { return m_pLabel->getText(); };
  void setText(const char * sText);
  void setFontId(FontId id) { m_pLabel->setFontId(id); };
  void setTextColor(F_RGBA textColor) { m_pLabel->setDiffuseColor(textColor); };
  BtnClickOptions getClickOption() { return m_ClickOption; };
  BtnClickOptions getOverOption() { return m_OverOption; };
  void setClickOption(BtnClickOptions clickOption) { m_ClickOption = clickOption; };
  void setOverOption(BtnClickOptions overOption) { m_OverOption = overOption; };
  GeometryQuads * getClickedGeometry() { return m_pGeometryClicked; };
  GeometryQuads * getOverGeometry() { return m_pGeometryOver; };
  GeometryQuads * getNormalGeometry() { return m_pGeometryNormal; };
  void setCatchButton2Events(bool bCatch) { m_bCatchButton2Events = bCatch; };
  void setCatchDoubleClicks(bool bCatch) { m_bCatchDoubleClicks = bCatch; };
  void setMultiClicks(bool bMulti) { m_bMultiClicks = bMulti; };
  void setNormalTexture(int iTexId);

  // Clone / init
  virtual void init(const char * sText, FontId fontId, F_RGBA textColor, int iClickedTex, BtnClickOptions clickOption, int iOverTex, BtnClickOptions overOption, int iTex, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Size & position
  virtual void onResize(int iOldWidth, int iOldHeight);
  void autoPad(int margin);
  void autoPadWidth(int margin, int minWidth = 0);

  // Other
  void attachImage(int iTex);

  // Static default constructors
  static guiButton * createDefaultNormalButton(const char * sText, const char * sId, DisplayEngine * pDisplay);
  static guiButton * createDefaultSmallButton(const char * sText, int width, const char * sId, DisplayEngine * pDisplay);
  static guiButton * createDefaultWhiteButton(const char * sText, int width, int height, const char * sId, DisplayEngine * pDisplay);
  static guiButton * createDefaultImageButton(int iTex, const char * sId, DisplayEngine * pDisplay);

protected:
  bool m_bMouseDown;
  bool m_bMouseOver;
  guiLabel * m_pLabel;
  BtnClickOptions m_ClickOption;
  BtnClickOptions m_OverOption;
  GeometryQuads * m_pGeometryClicked;
  GeometryQuads * m_pGeometryOver;
  GeometryQuads * m_pGeometryNormal;
  GeometryQuads * m_pGeometryAttachedImage;
  bool m_bClickState;
  bool m_bCatchButton2Events;
  bool m_bCatchDoubleClicks;
  bool m_bMultiClicks;
  float m_fMultiClicksTimer;
  ButtonAction m_MultiClicksEvent;
};

#endif
