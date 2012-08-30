#ifndef _GUI_IMAGE_H
#define _GUI_IMAGE_H

#include "guiComponent.h"

class guiImage : public guiComponent
{
public:
  // Constructor / destructor
  guiImage();
  ~guiImage();

  // Inherited functions
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Size and position
  virtual void onResize(int iOldWidth, int iOldHeight);

  // Clone / init
  virtual void init(int iTex, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Misc.
  void setCatchClicks(bool b) { m_bCatchClicks = b; };
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);
  int getImageTexture();
  void setImageTexture(int iTexId);

protected:
  bool m_bCatchClicks;
};

#endif
