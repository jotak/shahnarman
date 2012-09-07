#ifndef _GUI_GAUGE_H
#define _GUI_GAUGE_H

#include "guiComponent.h"

class GeometryQuads;

class guiGauge : public guiComponent
{
public:
  // Constructor / destructor
  guiGauge();
  ~guiGauge();

  // Inherited functions
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Size and position
  virtual void onResize(int iOldWidth, int iOldHeight);

  // Clone / init
  virtual void init(int iRef, int iVal, F_RGBA color, int iFgTex, int iBgTex, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Static default constructors
  static guiGauge * createDefaultGauge(int iRef, F_RGBA color, int iWidth, int iHeight, const char * sId, DisplayEngine * pDisplay);

  // Other
  void setMax(int iVal);
  void setValue(int iVal);

protected:
  GeometryQuads * m_pForegroundGeometry;
  int m_iRefValue;
  int m_iCurValue;
  F_RGBA m_Color;
};

#endif
