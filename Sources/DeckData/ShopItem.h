#ifndef _SHOP_ITEM_H
#define _SHOP_ITEM_H

#include "../GUIClasses/guiSmartSlider.h"

class guiPopup;
class XMLObject;

#define PACK_MODE_FIXED         0
#define PACK_MODE_RANDOM        1
#define PACK_MODE_RANDOM_RARE   2
#define PACK_MODE_RANDOM_LIFE   3
#define PACK_MODE_RANDOM_LAW    4
#define PACK_MODE_RANDOM_DEATH  5
#define PACK_MODE_RANDOM_CHAOS  6

//#define SHOP_ITEM_FULL_DESCRIPTION_MAX_CHARS    512

class ShopItem : public guiSliderItem
{
public:
  ShopItem();
  virtual ~ShopItem();
  virtual char * getInfo(char * str, int strsize);
  guiPopup * createPopup(int iCash, DisplayEngine * pDisplay);

  XMLObject * m_pXml;
  int m_iCost;
  char m_sEdition[NAME_MAX_CHARS];
//  char m_sFullText[SHOP_ITEM_FULL_DESCRIPTION_MAX_CHARS];
  int m_iType;
};

class AvatarShopItem : public ShopItem
{
public:
  AvatarShopItem() { m_iType = 1; };
  char m_sAvatarId[NAME_MAX_CHARS];
};

class ArtifactShopItem : public ShopItem
{
public:
  ArtifactShopItem() { m_iType = 2; };
  char m_sArtifactId[NAME_MAX_CHARS];
};

class PackShopItem : public ShopItem
{
public:
  PackShopItem() { m_pContent = new ObjectList(true); m_iType = 0; };
  ~PackShopItem() { delete m_pContent; };
  ObjectList * m_pContent;
};

class SpellsPackContent : public BaseObject
{
public:
  SpellsPackContent() { m_iMode = -1; m_iNbSpells = 0; wsafecpy(m_sSpellId, NAME_MAX_CHARS, ""); };
  int m_iMode;
  int m_iNbSpells;
  char m_sSpellId[NAME_MAX_CHARS];
};

#endif
