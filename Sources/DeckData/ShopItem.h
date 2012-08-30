#ifndef _SHOP_ITEM_H
#define _SHOP_ITEM_H

#include "../GUIClasses/guiSmartSlider.h"

class guiPopup;

#define SHOP_ITEM_FULL_DESCRIPTION_MAX_CHARS    512

class ShopItem : public guiSliderItem
{
public:
  virtual wchar_t * getInfo(wchar_t * str, int strsize);
  guiPopup * createPopup(int iCash, DisplayEngine * pDisplay);

  int m_iCost;
  wchar_t m_sEdition[NAME_MAX_CHARS];
  wchar_t m_sFullText[SHOP_ITEM_FULL_DESCRIPTION_MAX_CHARS];
  int m_iType;
};

class AvatarShopItem : public ShopItem
{
public:
  AvatarShopItem() { m_iType = 1; };
  wchar_t m_sAvatarId[NAME_MAX_CHARS];
};

class ArtifactShopItem : public ShopItem
{
public:
  ArtifactShopItem() { m_iType = 2; };
  wchar_t m_sArtifactId[NAME_MAX_CHARS];
};

class PackShopItem : public ShopItem
{
public:
  class PackShopItem_content : public BaseObject
  {
  public:
    int m_iMode;
    int m_iNbSpells;
  };
  PackShopItem() { m_pContent = new ObjectList(true); m_iType = 0; };
  ~PackShopItem() { delete m_pContent; };
  ObjectList * m_pContent;
};

#endif
