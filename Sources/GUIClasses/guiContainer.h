#ifndef _GUI_CONTAINER_H
#define _GUI_CONTAINER_H

#include "guiComponent.h"
#include "guiDocument.h"
#include "../Geometries/GeometryQuads.h"
#include "../Geometries/StencilGeometry.h"

enum FrameFitBehavior
{
  FB_NoFit = 0,
  FB_FitDocumentToFrame,
  FB_FitDocumentToFrameWhenSmaller,
  FB_FitFrameToDocument,
  FB_FitFrameToDocumentWhenSmaller
};

class guiContainer : public guiComponent
{
public:
  // Constructor / destructor
  guiContainer();
  ~guiContainer();

  // Inherited functions
  virtual u32 getType() { return guiComponent::getType() | GOTYPE_CONTAINER; };
  virtual void setVisible(bool bVisible);

  // Update / display
  virtual void update(double delta);
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);
  virtual void updateSizeFit();

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);
  virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);

  // Size and position
  int getInnerXPos() { return m_iInnerXPxl; };
  int getInnerYPos() { return m_iInnerYPxl; };
  int getInnerWidth() { return m_iInnerWidth; };
  int getInnerHeight() { return m_iInnerHeight; };
  bool isDocumentAt(int xPxl, int yPxl);
  virtual void moveTo(int xPxl, int yPxl);
  virtual void moveBy(int xPxl, int yPxl);
  virtual void onResize(int iOldWidth, int iOldHeight);

  // Scroll functions
  void scrollToTop();
  void scrollToBottom();
  void scrollToLeft();
  void scrollToRight();

  // Document management
  virtual guiDocument * getDocument() { return m_pDoc; };
  virtual void setDocument(guiDocument * pDoc);
  virtual guiDocument * unsetDocument();
  void checkDocumentPosition();

  // Member access
  FrameFitBehavior getWidthFitBehavior() { return m_WidthFit; };
  FrameFitBehavior getHeightFitBehavior() { return m_HeightFit; };
  void setWidthFitBehavior(FrameFitBehavior widthFit) { m_WidthFit = widthFit; };
  void setHeightFitBehavior(FrameFitBehavior heightFit) { m_HeightFit = heightFit; };
  int getMaxWidth() { return m_iMaxWidth; };
  int getMaxHeight() { return m_iMaxHeight; };
  void setMaxWidth(int width) { m_iMaxWidth = width; };
  void setMaxHeight(int height) { m_iMaxHeight = height; };
  virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO);
  virtual void setTargetValid(bool bValid);

  // Clone / init
  virtual void init(FrameFitBehavior widthFit, FrameFitBehavior heightFit, int iXOffset, int iYOffset, int iMaxWidth, int iMaxHeight, int * iMainTexs, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Static default constructors
  static guiContainer * createDefaultPanel(int width, int height, const wchar_t * sId, DisplayEngine * pDisplay);

  // Static data
  static void initStatic();
  static void deleteStatic();

protected:
  int m_iMaxWidth;
  int m_iMaxHeight;
  int m_iInnerXPxl;
  int m_iInnerYPxl;
  int m_iInnerWidth;
  int m_iInnerHeight;
  guiDocument * m_pDoc;
  StencilGeometry * m_pStencilGeometry;
  static GeometryQuads * m_pScrollButtons[4];   // top, bottom, left, right
  bool m_bShowScrollButtons[4];                 // top, bottom, left, right
  CoordsScreen m_ScrollButtonsCoords[4];        // top, bottom, left, right
  int m_iClickedScroll;
  float m_fScrollDelta;
  static int m_iScrollButtonWidth;
  static int m_iScrollButtonHeight;
  FrameFitBehavior m_WidthFit;
  FrameFitBehavior m_HeightFit;
  int m_iXOffset;
  int m_iYOffset;

private:
  int computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay);
  void stepScroll(int iDir);  // 0=top, 1=bottom, 2=left, 3=right
};

#endif
