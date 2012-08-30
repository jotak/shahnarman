#include "ShopItem.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiPopup.h"

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
wchar_t * ShopItem::getInfo(wchar_t * str, int strsize)
{
  wchar_t sbuf1[64];
  wchar_t sbuf2[64];
  i18n->getText1stUp(L"PRICE", sbuf1, 64);
  i18n->getText(L"2P", sbuf2, 64);
  swprintf_s(str, strsize, L"%s. %s%s%d", m_sName, sbuf1, sbuf2, m_iCost);
  return str;
}

// -----------------------------------------------------------------
// Name : createPopup
// -----------------------------------------------------------------
guiPopup * ShopItem::createPopup(int iCash, DisplayEngine * pDisplay)
{
  wchar_t sText[512];
  wchar_t sbuf1[64];
  wchar_t sbuf2[64];
  wchar_t sbuf3[64];
  i18n->getText1stUp(L"PRICE", sbuf1, 64);
  i18n->getText(L"2P", sbuf2, 64);
  i18n->getText(L"YOUR_CASH", sbuf3, 64);
  swprintf_s(sText, 512, L"%s\n%s%s%d (%s%d)", m_sName, sbuf1, sbuf2, m_iCost, sbuf3, iCash);
  int iDocWidth = 350;
  int iImageSize = 64;
  guiPopup * pPopup = guiPopup::createTextAndMultiButtonsPopup(sText, 2, iDocWidth, pDisplay);

  // Re-organize popup (add image, move text, add full description)
  // Add image
  guiImage * pImg = new guiImage();
  pImg->init(m_iTexId, L"Image", 5, 5, iImageSize, iImageSize, pDisplay);
  pPopup->getDocument()->addComponent(pImg);

  // Move top label
  guiLabel * pLbl = (guiLabel*) pPopup->getDocument()->getComponent(L"TopLabel");
  pLbl->moveTo(15 + iImageSize, 5);
  pLbl->setBoxWidth(iDocWidth - 20 - iImageSize);

  // Add full description
  int yPxl = 10 + max(pLbl->getHeight() + 5, 5 + iImageSize);
  guiLabel * pLbl2 = new guiLabel();
  pLbl2->init(m_sFullText, pLbl->getFontId(), pLbl->getDiffuseColor(), L"FullDescription", 5, yPxl, iDocWidth - 10, 0, pDisplay);
  pPopup->getDocument()->addComponent(pLbl2);

  // Update buttons data
  yPxl += pLbl2->getHeight() + 10;
  guiButton * pBtn = (guiButton*) pPopup->getDocument()->getComponent(L"0");
  pBtn->setId(L"CancelButton");
  wchar_t str[64] = L"";
  i18n->getText1stUp(L"CANCEL", str, 64);
  pBtn->setText(str);
  int width = pBtn->getWidth();
  pBtn->autoPadWidth(6, 64);
  int oldy = pBtn->getYPos();
  pBtn->moveBy((width-pBtn->getWidth()) / 2, yPxl - oldy);
  pBtn = (guiButton*) pPopup->getDocument()->getComponent(L"1");
  pBtn->setId(L"BuyButton");
  i18n->getText1stUp(L"BUY", str, 64);
  pBtn->setText(str);
  width = pBtn->getWidth();
  pBtn->autoPadWidth(6, 64);
  pBtn->moveBy((width-pBtn->getWidth()) / 2, yPxl - oldy);

  pPopup->getDocument()->setHeight(yPxl + pBtn->getHeight() + 5);

  return pPopup;
}
