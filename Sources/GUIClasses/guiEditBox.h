#ifndef _GUI_EDITBOX_H
#define _GUI_EDITBOX_H

#include "../Input/KeyboardListener.h"
#include "guiLabel.h"

class GeometryQuads;
class StencilGeometry;
class GeometryText;
class QuadData;
class KeyboardInputEngine;

class guiEditBox : public guiComponent, public KeyboardListener
{
public:
  // Constructor / destructor
  guiEditBox(KeyboardInputEngine * pInputs);
  ~guiEditBox();

  // Inherited functions
  virtual u32 getType() { return guiComponent::getType() | GOTYPE_EDITBOX; };
  virtual bool onKeyDown(unsigned char c);
  virtual bool onSpecialKeyDown(int key);

  // Update / display
  virtual void update(double delta);
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);

  // Events
  virtual void onFocusLost();
  virtual void onResize(int iOldWidth, int iOldHeight);

  // Member access
  FontId getFontId() { return m_FontId; };
  void setFontId(FontId id) { m_FontId = id; };
  void setTextColor(F_RGBA textColor) { m_TextColor = textColor; };
  F_RGBA getTextColor() { return m_TextColor; };
  void setText(const char * sText);
  char * getText() { return m_sText; };
  void setNbLines(int iNbLines) { m_iNbLines = iNbLines; };
  int getNbLines() { return m_iNbLines; };

  // Clone / init
  virtual void init(int iCaretTex, const char * sText, FontId fontId, F_RGBA textColor, int iNbLines, bool bMultiLines, int * iMainTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Other
  void setFocus();

  // Static default constructors
  static guiEditBox * createDefaultEditBox(int iNbLines, bool bMultiLines, int wPxl, const char * sId, KeyboardInputEngine * pInputs, DisplayEngine * pDisplay);

protected:
  void updateSelectionGeometry();
  int getStartOfWord(int caretpos);
  int getEndOfWord(int caretpos);
  int getStartOfLine(int caretpos);
  int getEndOfLine(int caretpos);
  void onButton1Down(int xPxl, int yPxl);
  void onButton1Drag(int xPxl, int yPxl);
  void onButton1DoubleClick(int xPxl, int yPxl);

  StencilGeometry * m_pStencilGeometry;
  GeometryText * m_pTextGeometry;
  GeometryQuads * m_pCaretGeometry;
  GeometryQuads * m_pSelectionGeometry;
  int m_iNbLines;
  bool m_bMultiLines;
  char m_sText[LABEL_MAX_CHARS];
  FontId m_FontId;
  F_RGBA m_TextColor;
  int m_iCaretPos;
  bool m_bHasFocus;
  double m_fBlinkTimer;
  KeyboardInputEngine * m_pInputs;
  int m_iXScrollPos;
  int m_iYScrollPos;
  int m_iSelectionStart;
  int m_iSelectionEnd;

private:
  int computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay);
};

#endif
