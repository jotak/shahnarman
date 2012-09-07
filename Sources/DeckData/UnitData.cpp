// -----------------------------------------------------------------
// UNIT DATA
// -----------------------------------------------------------------
#include "UnitData.h"
#include "../Data/LocalisationTool.h"
#include "../Debug/DebugManager.h"
#include "../Data/Serializer.h"

// -----------------------------------------------------------------
// Name : UnitData
//  Constructor
// -----------------------------------------------------------------
UnitData::UnitData()
{
  m_pSkills = new ObjectList(true);
  wsafecpy(m_sTextureFilename, MAX_PATH, L"");
  wsafecpy(m_sEdition, NAME_MAX_CHARS, L"");
  wsafecpy(m_sEthnicityId, NAME_MAX_CHARS, L"");
}

// -----------------------------------------------------------------
// Name : ~UnitData
//  Destructor
// -----------------------------------------------------------------
UnitData::~UnitData()
{
  delete m_pSkills;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void UnitData::serialize(Serializer * pSerializer)
{
  pSerializer->writeString(m_sEdition);
  pSerializer->writeString(m_sObjectId);
  pSerializer->writeString(m_sEthnicityId);
  pSerializer->writeString(m_sTextureFilename);
  // hashmap values
  pSerializer->writeLong((long) m_lValues.size());
  long_hash::iterator it;
  for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
  {
    pSerializer->writeString(it->first.c_str());
    pSerializer->writeLong((long) it->second);
  }
  // skills ref
  pSerializer->writeLong((long) m_pSkills->size);
  Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
  while (pSkill != NULL)
  {
    pSerializer->writeString(pSkill->getObjectEdition());
    pSerializer->writeString(pSkill->getObjectName());
    pSerializer->writeString(pSkill->getParameters());
    pSkill = (Skill*) m_pSkills->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void UnitData::deserialize(Serializer * pSerializer, DebugManager * pDebug)
{
  pSerializer->readString(m_sEdition);
  pSerializer->readString(m_sObjectId);
  pSerializer->readString(m_sEthnicityId);
  pSerializer->readString(m_sTextureFilename);
  // hashmap values
  int len = pSerializer->readLong();
  for (int i = 0; i < len; i++)
  {
    wchar_t str[1024];
    long val = 0;
    pSerializer->readString(str);
    val = pSerializer->readLong();
    m_lValues.insert(long_hash::value_type(str, val));
  }
  // skills ref
  len = pSerializer->readLong();
  for (int i = 0; i < len; i++)
  {
    wchar_t sEdition[NAME_MAX_CHARS];
    wchar_t sName[NAME_MAX_CHARS];
    wchar_t sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
    pSerializer->readString(sEdition);
    pSerializer->readString(sName);
    pSerializer->readString(sParameters);
    Skill * pSkill = new Skill(sEdition, sName, sParameters, pDebug);
    m_pSkills->addLast(pSkill);
  }
}

// -----------------------------------------------------------------
// Name : getInfos
// -----------------------------------------------------------------
wchar_t * UnitData::getInfos(wchar_t * sBuf, int iSize, const wchar_t * sSeparator, bool bName, const wchar_t * sEthn, bool bAlign, bool bCharacs, bool bSkills, bool bDesc)
{
  // Item full description
  wsafecpy(sBuf, iSize, L"");
  wchar_t sTemp[512];
  wchar_t s2P[8];
  wchar_t sSep[8] = L"";
  i18n->getText(L"2P", s2P, 8);

  // Name
  if (bName)
  {
    findLocalizedElement(sTemp, 512, i18n->getCurrentLanguageName(), L"name");
    wsafecat(sBuf, iSize, sTemp);
    wsafecpy(sSep, 8, sSeparator);
  }

  // Ethnicity
  if (sEthn != NULL)
  {
    wsafecat(sBuf, iSize, sSep);
    i18n->getText1stUp(L"ETHNICITY", sTemp, 512);
    wsafecat(sBuf, iSize, sTemp);
    wsafecat(sBuf, iSize, s2P);
    wsafecat(sBuf, iSize, sEthn);
    wsafecpy(sSep, 8, sSeparator);
  }

  // Alignment
  if (bAlign)
  {
    wsafecat(sBuf, iSize, sSep);
    i18n->getText1stUp(STRING_ALIGNMENT, sTemp, 512);
    wsafecat(sBuf, iSize, sTemp);
    wsafecat(sBuf, iSize, s2P);
    int iAlign = 0;
    long_hash::iterator it = m_lValues.find(STRING_ALIGNMENT);
    if (it != m_lValues.end())
      iAlign = it->second;
    getAlignmentInfos(iAlign, sTemp, 512);
    wsafecat(sBuf, iSize, sTemp);
    wsafecpy(sSep, 8, sSeparator);
  }

  // Characs
  if (bCharacs)
  {
    wsafecat(sBuf, iSize, sSep);
    i18n->long_hashToString(sTemp,
      512, sSeparator,
      &m_lValues,
      5,
      STRING_MELEE,
      STRING_RANGE,
      STRING_ARMOR,
      STRING_ENDURANCE,
      STRING_SPEED);
    wsafecat(sBuf, iSize, sTemp);
    wsafecpy(sSep, 8, sSeparator);
  }

  // Skills
  if (bSkills)
  {
    wsafecat(sBuf, iSize, sSep);
    i18n->getText1stUp(L"SKILLS", sTemp, 512);
    wsafecat(sBuf, iSize, sTemp);
    wsafecat(sBuf, iSize, s2P);
    wchar_t sSep2[4] = L"";
    Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
    if (pSkill == NULL)
    {
      wsafecat(sBuf, iSize, i18n->getText(L"NONE", sTemp, 512));
    }
    while (pSkill != NULL)
    {
      wsafecat(sBuf, iSize, sSep2);
      wsafecpy(sSep2, 4, L", ");
      wsafecat(sBuf, iSize, pSkill->getLocalizedName());
      pSkill = (Skill*) m_pSkills->getNext(0);
    }
    wsafecpy(sSep, 8, sSeparator);
  }

  // Description
  if (bDesc)
  {
    wsafecat(sBuf, iSize, sSep);
    findLocalizedElement(sTemp, 512, i18n->getCurrentLanguageName(), L"description");
    wsafecat(sBuf, iSize, sTemp);
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getAlignmentInfos
// -----------------------------------------------------------------
wchar_t * UnitData::getAlignmentInfos(int iValue, wchar_t * sBuf, int iSize)
{
  wchar_t sTemp[64];
  wchar_t sSeparator[8] = L"";
  wsafecpy(sBuf, iSize, L"");
  if (iValue == 0)
    wsafecat(sBuf, iSize, i18n->getText(L"NONE", sTemp, 64));
  else
  {
    if (iValue & ALIGNMENT_LIFE)
    {
      wsafecat(sBuf, iSize, i18n->getText(L"LIFE", sTemp, 64));
      wsafecpy(sSeparator, 8, L", ");
    }
    if (iValue & ALIGNMENT_LAW)
    {
      wsafecat(sBuf, iSize, sSeparator);
      wsafecat(sBuf, iSize, i18n->getText(L"LAW", sTemp, 64));
      wsafecpy(sSeparator, 8, L", ");
    }
    if (iValue & ALIGNMENT_DEATH)
    {
      wsafecat(sBuf, iSize, sSeparator);
      wsafecat(sBuf, iSize, i18n->getText(L"DEATH", sTemp, 64));
      wsafecpy(sSeparator, 8, L", ");
    }
    if (iValue & ALIGNMENT_CHAOS)
    {
      wsafecat(sBuf, iSize, sSeparator);
      wsafecat(sBuf, iSize, i18n->getText(L"CHAOS", sTemp, 64));
    }
  }
  return sBuf;
}
