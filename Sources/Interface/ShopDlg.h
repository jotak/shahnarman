#ifndef _SHOP_DLG_H
#define _SHOP_DLG_H

#include "../GUIClasses/guiDocument.h"

class ShopItem;
class LocalClient;
class guiComponent;
class guiContainer;
class guiPopup;

class ShopDlg : public guiDocument
{
public:
  ShopDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
  ~ShopDlg();

  virtual void update(double delta);

  // Handlers
  bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
  void onShow();
  bool onClickStart();

protected:
  void showShopDialog(ShopItem * pItem);
  void reloadContent();

  LocalClient * m_pLocalClient;
  guiPopup * m_pShopItemDlg;
  guiContainer * m_pPlayerPanel;
  guiContainer * m_pShopPanel;
  ShopItem * m_pBuyingShopItem;
};

#endif
