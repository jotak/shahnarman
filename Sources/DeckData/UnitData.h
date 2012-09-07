#ifndef _UNITDATA_H
#define _UNITDATA_H

#include "../Data/XMLObject.h"
#include "../Gameboard/Skill.h"

#define STRING_MELEE        "melee"
#define STRING_RANGE        "range"
#define STRING_ARMOR        "armor"
#define STRING_ENDURANCE    "endurance"
#define STRING_SPEED        "speed"
#define STRING_LIFE         "life"
#define STRING_ALIGNMENT    "alignment"
#define STRING_MCPLAIN      "movecost_plain"
#define STRING_MCFOREST     "movecost_forest"
#define STRING_MCMOUNTAIN   "movecost_mountain"
#define STRING_MCTOUNDRA    "movecost_toundra"
#define STRING_MCDESERT     "movecost_desert"
#define STRING_MCSEA        "movecost_sea"

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

  char * getInfos(char * sBuf, int iSize, const char * sSeparator, bool bName = true, const char * sEthn = NULL, bool bAlign = true, bool bCharacs = true, bool bSkills = true, bool bDesc = true);
  static char * getAlignmentInfos(int iValue, char * sBuf, int iSize);
  void serialize(Serializer * pSerializer);
  void deserialize(Serializer * pSerializer, DebugManager * pDebug);

  // WARNING: WHEN ADDING DATA HERE, DON'T FORGET TO UPDATE AvatarData::cloneStaticData()
  char m_sEdition[NAME_MAX_CHARS];
  char m_sEthnicityId[NAME_MAX_CHARS];
  long_hash m_lValues;
  char m_sTextureFilename[MAX_PATH];
  ObjectList * m_pSkills;
};

#endif
