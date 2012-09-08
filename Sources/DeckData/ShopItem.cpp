#include "ShopItem.h"
#include "../Data/XMLObject.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiPopup.h"

// -----------------------------------------------------------------
// Name : ShopItem
//  ctor
// -----------------------------------------------------------------
ShopItem::ShopItem()
{
  m_pXml = new XMLObject();
}

// -----------------------------------------------------------------
// Name : ~ShopItem
//  destructor
// -----------------------------------------------------------------
ShopItem::~ShopItem()
{
  delete m_pXml;
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
char * ShopItem::getInfo(char * str, int strsize)
{
  char sbuf1[64];
  char sbuf2[64];
  i18n->getText1stUp("PRICE", sbuf1, 64);
  i18n->getText("2P", sbuf2, 64);
  snprintf(str, strsize, "%s. %s%s%d", m_sName, sbuf1, sbuf2, m_iCost);
  return str;
}

// -----------------------------------------------------------------
// Name : createPopup
// -----------------------------------------------------------------
guiPopup * ShopItem::createPopup(int iCash, DisplayEngine * pDisplay)
{
  char sText[512];
  char sbuf1[64];
  char sbuf2[64];
  char sbuf3[64];
  i18n->getText1stUp("PRICE", sbuf1, 64);
  i18n->getText("2P", sbuf2, 64);
  i18n->getText("YOUR_CASH", sbuf3, 64);
  snprintf(sText, 512, "%s\n%s%s%d (%s%d)", m_sName, sbuf1, sbuf2, m_iCost, sbuf3, iCash);
  int iDocWidth = 350;
  int iImageSize = 64;
  guiPopup * pPopup = guiPopup::createTextAndMultiButtonsPopup(sText, 2, iDocWidth, pDisplay);

  // Re-organize popup (add image, move text, add full description)
  // Add image
  guiImage * pImg = new guiImage();
  pImg->init(m_iTexId, "Image", 5, 5, iImageSize, iImageSize, pDisplay);
  pPopup->getDocument()->addComponent(pImg);

  // Move top label
  guiLabel * pLbl = (guiLabel*) pPopup->getDocument()->getComponent("TopLabe");
  pLbl->moveTo(15 + iImageSize, 5);
  pLbl->setBoxWidth(iDocWidth - 20 - iImageSize);

  // Add full description
  m_pXml->findLocalizedElement(sText, 512, i18n->getCurrentLanguageName(), "description");
  int yPxl = 10 + max(pLbl->getHeight() + 5, 5 + iImageSize);
  guiLabel * pLbl2 = new guiLabel();
  pLbl2->init(sText, pLbl->getFontId(), pLbl->getDiffuseColor(), "FullDescription", 5, yPxl, iDocWidth - 10, 0, pDisplay);
  pPopup->getDocument()->addComponent(pLbl2);

  // Update buttons data
  yPxl += pLbl2->getHeight() + 10;
  guiButton * pBtn = (guiButton*) pPopup->getDocument()->getComponent("0");
  pBtn->setId("CancelButton");
  char str[64] = "";
  i18n->getText1stUp("CANCEL", str, 64);
  pBtn->setText(str);
  int width = pBtn->getWidth();
  pBtn->autoPadWidth(6, 64);
  int oldy = pBtn->getYPos();
  pBtn->moveBy((width-pBtn->getWidth()) / 2, yPxl - oldy);
  pBtn = (guiButton*) pPopup->getDocument()->getComponent("1");
  pBtn->setId("BuyButton");
  i18n->getText1stUp("BUY", str, 64);
  pBtn->setText(str);
  width = pBtn->getWidth();
  pBtn->autoPadWidth(6, 64);
  pBtn->moveBy((width-pBtn->getWidth()) / 2, yPxl - oldy);

  pPopup->getDocument()->setHeight(yPxl + pBtn->getHeight() + 5);

  return pPopup;
}
