// -----------------------------------------------------------------
// AVATAR DATA
// -----------------------------------------------------------------
#include "AvatarData.h"
#include "Edition.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../Display/TextureEngine.h"
#include "../Data/DataFactory.h"
#include "../Data/LocalisationTool.h"
#include "../Data/NetworkSerializer.h"

#define NB_LEVELS   15
//      LEVELS        : 1    2    3,   4,    5,    6,    7,    8,    9,   10,    11,    12,    13,    14,    15
#define XP_LEVELS     { 0, 100, 300, 600, 1000, 1600, 2500, 3900, 6000, 9100, 14000, 22000, 34000, 52000, 80000 }
#define BANNERS       { L"blason1", L"blason2", L"blason3", L"blason4", L"blason5", L"blason6", L"blason7", L"blason8", L"blason9", L"blason10", L"blason11", L"blason12" }

// -----------------------------------------------------------------
// Name : AvatarData
//  Constructor
// -----------------------------------------------------------------
AvatarData::AvatarData()
{
  for (int i = 0; i < NB_PROGRESSION_TREES; i++)
  {
    wsafecpy(m_pProgression[i].sTreeName, NAME_MAX_CHARS, L"");
    for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
      wsafecpy(m_pProgression[i].sElements[j], NAME_MAX_CHARS, L"");
  }
  for (int i = 0; i < 5; i++)
    m_pEquippedArtifacts[i] = NULL;
  m_uXP = 1000;
  m_pAllFames = new ObjectList(true);
  wsafecpy(m_sCustomDescription, CUSTOMDESC_MAX_CHARS, L"");
  wsafecpy(m_sCustomName, NAME_MAX_CHARS, L"");
  m_bLoaded = false;
  m_pOwner = NULL;
  m_uBanner = 0;
}

// -----------------------------------------------------------------
// Name : ~AvatarData
//  Destructor
// -----------------------------------------------------------------
AvatarData::~AvatarData()
{
  delete m_pAllFames;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
//void AvatarData::serialize(NetworkData * pData)
//{
//  pData->addString(m_sObjectId);
//  pData->addString(m_sEdition);
//  long_hash::iterator it;
//  for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
//    pData->addLong(it->second);
//  pData->addLong(m_pSkills->size);
//  Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
//  while (pSkill != NULL)
//  {
//    pSkill->serialize(pData);
//    pSkill = (Skill*) m_pSkills->getNext(0);
//  }
//  pData->addLong(m_uBanner);
//  pData->addLong(m_uXP);
//  for (int i = 0; i < NB_PROGRESSION_TREES; i++)
//  {
//    pData->addString(m_pProgression[i].sTreeName);
//    for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
//      pData->addString(m_pProgression[i].sElements[j]);
//  }
//  pData->addLong(m_pAllFames->size);
//  AvatarData::EthnicityAndFame * pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getFirst(0);
//  while (pFameObj != NULL)
//  {
//    pData->addString(pFameObj->sEthnicity);
//    pData->addLong(pFameObj->iFame);
//    pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getNext(0);
//  }
//  pData->addString(m_sCustomName);
//  pData->addString(m_sCustomDescription);
//}
//
//// -----------------------------------------------------------------
//// Name : deserialize
//// -----------------------------------------------------------------
//AvatarData * AvatarData::deserialize(NetworkData * pData, LocalClient * pLocalClient)
//{
//  wchar_t sId[NAME_MAX_CHARS];
//  wchar_t sEdition[NAME_MAX_CHARS];
//  pData->readString(sId);
//  pData->readString(sEdition);
//
//  AvatarData * pBaseData = (AvatarData*) pLocalClient->getDataFactory()->getUnitData(sEdition, sId);
//  assert(pBaseData != NULL);
//  AvatarData * pAvatar = pBaseData->cloneStaticData(NULL, pLocalClient->getDebug());
//
//  long_hash::iterator it;
//  for (it = pAvatar->m_lValues.begin(); it != pAvatar->m_lValues.end(); ++it)
//    it->second = pData->readLong();
//
//  pAvatar->m_pSkills->deleteAll();
//  int i = 0;
//  int nbSkills = (int) pData->readLong();
//  for (i = 0; i < nbSkills; i++)
//  {
//    Skill * pSkill = Skill::deserialize(i, pData, pLocalClient->getDebug());
//    pAvatar->m_pSkills->addLast(pSkill);
//  }
//  pAvatar->m_uBanner = pData->readLong();
//  pAvatar->m_uXP = pData->readLong();
//  for (i = 0; i < NB_PROGRESSION_TREES; i++)
//  {
//    pData->readString(pAvatar->m_pProgression[i].sTreeName);
//    for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
//      pData->readString(pAvatar->m_pProgression[i].sElements[j]);
//  }
//  int size = pData->readLong();
//  for (i = 0; i < size; i++)
//  {
//    AvatarData::EthnicityAndFame * pFameObj = new AvatarData::EthnicityAndFame();
//    pData->readString(pFameObj->sEthnicity);
//    pFameObj->iFame = pData->readLong();
//    pAvatar->m_pAllFames->addLast(pFameObj);
//  }
//  pData->readString(pAvatar->m_sCustomName);
//  pData->readString(pAvatar->m_sCustomDescription);
//  return pAvatar;
//}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void AvatarData::serialize(Serializer * pSerializer)
{
  UnitData::serialize(pSerializer);
  pSerializer->writeChar((char) m_uBanner);
  pSerializer->writeShort((short) m_uXP);
  for (int j = 0; j < NB_PROGRESSION_TREES; j++)
  {
    pSerializer->writeString(m_pProgression[j].sTreeName);
    for (int k = 0; k < NB_PROGRESSION_LEVELS; k++)
    {
      pSerializer->writeString(m_pProgression[j].sElements[k]);
    }
  }
  pSerializer->writeLong((long) m_pAllFames->size);
  AvatarData::EthnicityAndFame * pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getFirst(0);
  while (pFameObj != NULL)
  {
    pSerializer->writeString(pFameObj->sEthnicity);
    pSerializer->writeLong((long) pFameObj->iFame);
    pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getNext(0);
  }
  pSerializer->writeString(m_sCustomName);
  pSerializer->writeString(m_sCustomDescription);
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void AvatarData::deserialize(Serializer * pSerializer, DebugManager * pDebug)
{
  UnitData::deserialize(pSerializer, pDebug);
  m_uBanner = pSerializer->readChar();
  m_uXP = pSerializer->readShort();
  for (int j = 0; j < NB_PROGRESSION_TREES; j++)
  {
    pSerializer->readString(m_pProgression[j].sTreeName);
    for (int k = 0; k < NB_PROGRESSION_LEVELS; k++)
    {
      pSerializer->readString(m_pProgression[j].sElements[k]);
    }
  }
  int len = pSerializer->readLong();
  for (int i = 0; i < len; i++)
  {
    AvatarData::EthnicityAndFame * pFameObj = new AvatarData::EthnicityAndFame();
    pSerializer->readString(pFameObj->sEthnicity);
    pFameObj->iFame = pSerializer->readLong();
    m_pAllFames->addLast(pFameObj);
  }
  pSerializer->readString(m_sCustomName);
  pSerializer->readString(m_sCustomDescription);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
AvatarData * AvatarData::clone(LocalClient * pLocalClient)
{
  NetworkData dummy(1);
  serialize(NetworkSerializer::getInstance(&dummy));
  dummy.readLong();
  AvatarData * pClone = new AvatarData();
  pClone->deserialize(NetworkSerializer::getInstance(&dummy), pLocalClient->getDebug());
  return pClone;
}

// -----------------------------------------------------------------
// Name : cloneStaticData
//  This function is used to copy data from XML model to avatar instance.
// -----------------------------------------------------------------
AvatarData * AvatarData::cloneStaticData(Profile * pOwner, DebugManager * pDebug)
{
  AvatarData * pClone = new AvatarData();
  // Copy XMLObject data
  wsafecpy(pClone->m_sObjectId, NAME_MAX_CHARS, m_sObjectId);
  LocalizedElement * pLoc = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
  while (pLoc != NULL)
  {
    pClone->addLocalizedElement(pLoc->m_sKey, pLoc->m_sValue, pLoc->m_sLanguage);
    pLoc = (LocalizedElement*) m_pLocalizedElements->getNext(0);
  }
  // Copy UnitData data
  wsafecpy(pClone->m_sEdition, NAME_MAX_CHARS, m_sEdition);
  wsafecpy(pClone->m_sTextureFilename, MAX_PATH, m_sTextureFilename);

  long_hash::iterator it;
  for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
    pClone->m_lValues.insert(long_hash_pair(it->first, it->second));
  Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
  while (pSkill != NULL)
  {
    Skill * pClonedSkill = pSkill->clone(true, pDebug);
    pClone->m_pSkills->addFirst(pClonedSkill);
    pSkill = (Skill*) m_pSkills->getNext(0);
  }
  // Copy AvatarData data
  pClone->m_pOwner = pOwner;
  wsafecpy(pClone->m_sEthnicityId, NAME_MAX_CHARS, m_sEthnicityId);
  wsafecpy(pClone->m_pProgression[0].sTreeName, NAME_MAX_CHARS, m_sEthnicityId);
  pClone->m_bLoaded = true;
  // Set custom name & description
  pClone->findLocalizedElement(pClone->m_sCustomName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
  pClone->findLocalizedElement(pClone->m_sCustomDescription, CUSTOMDESC_MAX_CHARS, i18n->getCurrentLanguageName(), L"description");

  return pClone;
}

// -----------------------------------------------------------------
// Name : isLevelUp
// -----------------------------------------------------------------
bool AvatarData::isLevelUp()
{
  u16 uLevel = getRealLevel();
  return (getPotentialLevel() > uLevel && uLevel < MAX_LEVELS);
}

// -----------------------------------------------------------------
// Name : getRealLevel
// -----------------------------------------------------------------
u16 AvatarData::getRealLevel()
{
  u16 currentlevel = 0;
  // Find current level
  for (int i = 0; i < NB_PROGRESSION_TREES; i++)
  {
    if (wcscmp(m_pProgression[i].sTreeName, L"") != 0)
    {
      currentlevel++;
      for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
      {
        if (wcscmp(m_pProgression[i].sElements[j], L"") != 0)
          currentlevel++;
      }
    }
  }
  return currentlevel;
}

// -----------------------------------------------------------------
// Name : getPotentialLevel
// -----------------------------------------------------------------
u16 AvatarData::getPotentialLevel()
{
  int iLevelXP[] = XP_LEVELS;
  for (int i = NB_LEVELS-1; i >= 0; i--)
  {
    if (m_uXP >= iLevelXP[i])
      return (u16)(i + 1);
  }
  assert(false);  // should not be there
  return 0;
}

// -----------------------------------------------------------------
// Name : getNextLevelXP
// -----------------------------------------------------------------
u16 AvatarData::getNextLevelXP()
{
  int iLevelXP[] = XP_LEVELS;
  for (int i = NB_LEVELS-1; i >= 0; i--)
  {
    if (m_uXP >= iLevelXP[i])
      return (u16) iLevelXP[i+1];
  }
  assert(false);  // should not be there
  return 0;
}

// -----------------------------------------------------------------
// Name : getBanner
//  Static
// -----------------------------------------------------------------
void AvatarData::getBanner(u8 uBanner, wchar_t * sBuf, int iBufSize)
{
  wchar_t * sBanners[] = BANNERS;
  wsafecpy(sBuf, iBufSize, sBanners[uBanner]);
}

// -----------------------------------------------------------------
// Name : getBanner
// -----------------------------------------------------------------
void AvatarData::getBanner(wchar_t * sBuf, int iBufSize)
{
  wchar_t * sBanners[] = BANNERS;
  wsafecpy(sBuf, iBufSize, sBanners[m_uBanner]);
}
