#ifndef _UNITDATA_H
#define _UNITDATA_H

#include "../Data/XMLObject.h"
#include "../Gameboard/Skill.h"

#define STRING_MELEE        L"melee"
#define STRING_RANGE        L"range"
#define STRING_ARMOR        L"armor"
#define STRING_ENDURANCE    L"endurance"
#define STRING_SPEED        L"speed"
#define STRING_LIFE         L"life"
#define STRING_ALIGNMENT    L"alignment"
#define STRING_MCPLAIN      L"movecost_plain"
#define STRING_MCFOREST     L"movecost_forest"
#define STRING_MCMOUNTAIN   L"movecost_mountain"
#define STRING_MCTOUNDRA    L"movecost_toundra"
#define STRING_MCDESERT     L"movecost_desert"
#define STRING_MCSEA        L"movecost_sea"

#define ALIGNMENT_LIFE      1
#define ALIGNMENT_LAW       2
#define ALIGNMENT_DEATH     4
#define ALIGNMENT_CHAOS     8

class DebugManager;
class Serializer;

class UnitData : public XMLObject
{
public:
  UnitData();
  ~UnitData();

  wchar_t * getInfos(wchar_t * sBuf, int iSize, const wchar_t * sSeparator, bool bName = true, const wchar_t * sEthn = NULL, bool bAlign = true, bool bCharacs = true, bool bSkills = true, bool bDesc = true);
  static wchar_t * getAlignmentInfos(int iValue, wchar_t * sBuf, int iSize);
  void serialize(Serializer * pSerializer);
  void deserialize(Serializer * pSerializer, DebugManager * pDebug);

  // WARNING: WHEN ADDING DATA HERE, DON'T FORGET TO UPDATE AvatarData::cloneStaticData()
  wchar_t m_sEdition[NAME_MAX_CHARS];
  wchar_t m_sEthnicityId[NAME_MAX_CHARS];
  long_hash m_lValues;
  wchar_t m_sTextureFilename[MAX_PATH];
  ObjectList * m_pSkills;
};

#endif
