#ifndef _GUI_COMBOBOX_H
#define _GUI_COMBOBOX_H

#include "guiContainer.h"
#include "guiButton.h"

class InterfaceManager;

class guiComboBox : public guiComponent, public ComponentOwnerInterface
{
public:
  // Constructor / destructor
  guiComboBox(InterfaceManager * pInterface);
  ~guiComboBox();

  // Inherited functions
  virtual void update(double delta);
  virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

  // Input functions
  virtual guiObject * onButtonEvent(ButtonAction * pEvent);

  // Events
  virtual void onFocusLost();
  virtual bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

  // Size and position
  virtual bool isAt(int xPxl, int yPxl);
  virtual void onResize(int iOldWidth, int iOldHeight);
  void moveTo(int xPxl, int yPxl) { guiComponent::moveTo(xPxl, yPxl); m_pList->moveTo(xPxl, getHeight() + yPxl); centerLabel(); };
  void moveBy(int xPxl, int yPxl) { guiComponent::moveBy(xPxl, yPxl); m_pList->moveBy(xPxl, yPxl); centerLabel(); };
  virtual void setWidth(int iWidth);
  virtual void setDimensions(int iWidth, int iHeight);

  // List elements
  void clearList() { m_pList->getDocument()->deleteAllComponents(); m_pLabel->setText(L""); };
  guiButton * addString(const wchar_t * sText, const wchar_t * sId);
  wchar_t * getText() { return m_pLabel->getText(); };
  void setItem(int id);
  guiButton * getItem(const wchar_t * sId);
  guiButton * getItem(u16 uId);
  guiButton * getSelectedItem();
  int getSelectedItemId();
  u16 getItemsCount();

  // Clone / init
  virtual void init(int * iMainTex, int iDocTex, F_RGBA textColor, FontId fontId, FrameFitBehavior wfit, FrameFitBehavior hfit, int iMaxWidth, int iMaxHeight, int btnTex1, int btnTex2, BtnClickOptions btnClickOpt, int btnHeight, int * frameTexs, const wchar_t * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
  virtual guiObject * clone();

  // Static default constructors
  static guiComboBox * createDefaultComboBox(const wchar_t * sId, InterfaceManager * pInterface, DisplayEngine * pDisplay);

protected:
  void centerLabel();

  guiLabel * m_pLabel;
  guiContainer * m_pList;
  guiButton * m_pListButtonTemplate;
  InterfaceManager * m_pInterface;
  double m_dListPos;

private:
  int computeQuadsList(QuadData *** pQuads, int * iTextures, DisplayEngine * pDisplay);
};

#endif
