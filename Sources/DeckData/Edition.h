#ifndef _EDITION_H
#define _EDITION_H

#include "../Data/XMLObject.h"

#define STRING_AVATAR_XML   L"shahmah"
#define MAX_SKILLS          1024

class DebugManager;
class LocalClient;
class guiSmartSlider;
class DisplayEngine;
class UnitData;
class Spell;
class SpecialTile;
class XMLLiteElement;
class XMLLiteReader;
class Profile;
class ShopItem;
class ObjectList;
class Ethnicity;
class ProgressionTree;
class Artifact;
class ShahmahCreation;

class Edition : public XMLObject
{
public:
  Edition(wchar_t * sChecksum, LocalClient * pLocalClient);
  ~Edition();

  // Member access
  bool isActive() { return m_bActive; };

  // Other
  bool activate(DebugManager * pDebug);
  void deactivate();
  void addShopItems(Profile * pPlayer, guiSmartSlider * pShopSlider, DebugManager * pDebug);
  Ethnicity * findEthnicity(wchar_t * strId, bool bLookDependencies = true);
  ObjectList * getEthnicities() { return m_pEthnicities; };
  ProgressionTree * findProgressionTree(wchar_t * strId, bool bLookDependencies = true);
  UnitData * findUnitData(wchar_t * strId, bool bLookDependencies = true);
  Spell * findSpell(wchar_t * sName, bool bLookDependencies = true);
  SpecialTile * findSpecialTile(wchar_t * sName, bool bLookDependencies = true);
  Artifact * findArtifact(wchar_t * sName, bool bLookDependencies = true);
  Edition * findSkillEdition(wchar_t * sName);
  Spell * buySpell(int iRandMode);
  ObjectList * getSpecialTiles() { return m_pSpecTiles; };
  void getAllTreesByType(ObjectList * pList, u8 uType);
  ShahmahCreation * getShahmahCreationData() { return m_pShahmahCreation; };

protected:
  XMLLiteElement * loadXMLFile(XMLLiteReader * pReader, wchar_t * fileName, DebugManager * pDebug);
  void parseXMLObjectData(XMLLiteElement * pRootNode, DebugManager * pDebug);
  void checkShopItemValidity(Profile * pPlayer, ShopItem * pItem);
  void checksum(DebugManager * pDebug);

  bool m_bActive;
  int m_iTotalFreq;
  wchar_t m_sLocale[32];
  wchar_t m_sChecksum[NAME_MAX_CHARS];
  wchar_t m_sVersion[16];
  ObjectList * m_pUnits;
  ObjectList * m_pSpells;
  ObjectList * m_pSpecTiles;
  ObjectList * m_pArtifacts;
  ObjectList * m_pSkillNames;
  ObjectList * m_pEthnicities;
  ObjectList * m_pDependencies;
  ObjectList * m_pProgressionTrees;
  ShahmahCreation * m_pShahmahCreation;
};

#endif
