#ifndef _EDITION_H
#define _EDITION_H

#include "../Data/XMLObject.h"

#define STRING_AVATAR_XML   "shahmah"
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
class SpellsPackContent;

class Edition : public XMLObject
{
public:
  Edition(const char * sName, LocalClient * pLocalClient);
  ~Edition();

  // Member access
  bool isActive() { return m_bActive; };

  // Other
  bool activate(DebugManager * pDebug);
  void deactivate();
  void addShopItems(Profile * pPlayer, guiSmartSlider * pShopSlider, DebugManager * pDebug);
  Ethnicity * findEthnicity(const char * strId, bool bLookDependencies = true);
  ObjectList * getEthnicities() { return m_pEthnicities; };
  ProgressionTree * findProgressionTree(const char * strId, bool bLookDependencies = true);
  UnitData * findUnitData(const char * strId, bool bLookDependencies = true);
  Spell * findSpell(const char * sName, bool bLookDependencies = true);
  SpecialTile * findSpecialTile(const char * sName, bool bLookDependencies = true);
  Artifact * findArtifact(const char * sName, bool bLookDependencies = true);
  Edition * findSkillEdition(const char * sName);
  Spell * selectRandomSpell(int iSelectMode);
  ObjectList * getSpecialTiles() { return m_pSpecTiles; };
  ObjectList * getAIs() { return m_pAIs; };
  void getAllTreesByType(ObjectList * pList, u8 uType);
  ShahmahCreation * getShahmahCreationData() { return m_pShahmahCreation; };
  char * getChecksum() { return m_sChecksum; };

protected:
  XMLLiteElement * loadXMLFile(XMLLiteReader * pReader, const char * fileName, DebugManager * pDebug);
  void parseXMLObjectData(XMLLiteElement * pRootNode, DebugManager * pDebug);
  void checkShopItemValidity(Profile * pPlayer, ShopItem * pItem);
  void computeChecksum(DebugManager * pDebug);
  SpellsPackContent * readSpellsPackContent(XMLLiteElement * pSpellsNode, DebugManager * pDebug, const char * sFileName);

  bool m_bActive;
  int m_iTotalFreq;
  char m_sLocale[32];
  char m_sChecksum[NAME_MAX_CHARS];
  char m_sVersion[16];
  ObjectList * m_pAIs;
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
