#ifndef _GUI_SMART_SLIDER_H
#define _GUI_SMART_SLIDER_H

#include "guiLabel.h"
#include "../Geometries/GeometryQuads.h"

#define SLIDER_ITEM_MAX_CHARS   256

class guiSliderItem : public BaseObject
{
public:
  virtual char * getInfo(char * str, int strsize)
  {
    wsafecpy(str, strsize, m_sName);
    return str;
  };
  int m_iTexId;
  char m_sName[SLIDER_ITEM_MAX_CHARS];
  bool m_bEnabled;
  char m_sDisabledReason[SLIDER_ITEM_MAX_CHARS];
};

class guiSmartSlider : public guiComponent
{
public:
  // Constructor / destructor
  guiSmartSlider();
  ~guiSmartSlider();

  // Inherited functions
  virtual void init(int iItemSize, int iSpacing, FontId fontId, F_RGBA textColor, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();
  virtual u32 getType() { return guiComponent::getType() | GOTYPE_SMARTSLIDER; };
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);

  // Member access
  int getSpacing() { return m_iSpacing; };
  void setSpacing(int iSpacing) { m_iSpacing = iSpacing; };
  guiSliderItem * getSelectedItem() { return m_pSelectedItem; };

  // Specific functions
  void addItem(guiSliderItem * pItem, bool bFirst = false);
  void loadGeometry(DisplayEngine * pDisplay = NULL);
  void deleteItems();

protected:
  ObjectList * m_pItems;
  int m_iSpacing;
  int m_iSliderPos;
  int m_iTheoricSize;
  guiSliderItem * m_pSelectedItem;
  GeometryQuads * m_pDisabledGeometry;
  GeometryQuads * m_pSelectorGeometry;
  int m_iSelectorPos;
  guiLabel * m_pLabel;
  guiLabel * m_pDisableReasonLabel;
  int m_iItemSize;
};

#endif
