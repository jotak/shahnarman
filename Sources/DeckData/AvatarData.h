#ifndef _AVATARDATA_H
#define _AVATARDATA_H

#include "UnitData.h"
#include "ProgressionTree.h"

#define CUSTOMDESC_MAX_CHARS    2048
#define NB_PROGRESSION_TREES    4   // 0=ethnicity, 1=magic, 2=trait1, 3=trait2
#define MAX_LEVELS              15
#define NB_BANNERS              12

class DebugManager;
class LocalClient;
class Profile;
class Artifact;

class ProgressionData
{
public:
  char sTreeName[NAME_MAX_CHARS];
  char sElements[NB_PROGRESSION_LEVELS][NAME_MAX_CHARS];
};

class AvatarData : public UnitData
{
public:
  class EthnicityAndFame : public BaseObject
  {
  public:
    s8 iFame;
    char sEthnicity[NAME_MAX_CHARS];
  };
  AvatarData();
  ~AvatarData();

  AvatarData * clone(LocalClient * pLocalClient);
  AvatarData * cloneStaticData(Profile * pOwner, DebugManager * pDebug); // "static" data stands for data read from XML: UnitData heritage and m_sEthnicityId;
  void serialize(Serializer * pSerializer);
  void deserialize(Serializer * pSerializer, DebugManager * pDebug);
  bool isLevelUp();
  u16 getPotentialLevel();
  u16 getRealLevel();
  u16 getNextLevelXP();
  static void getBanner(u8 uBanner, char * sBuf, int iBufSize);
  void getBanner(char * sBuf, int iBufSize);

  u16 m_uXP;
  ProgressionData m_pProgression[NB_PROGRESSION_TREES];
  ObjectList * m_pAllFames;
  char m_sCustomName[NAME_MAX_CHARS];
  char m_sCustomDescription[CUSTOMDESC_MAX_CHARS];
  bool m_bLoaded;
  Profile * m_pOwner;
  u8 m_uBanner;
  Artifact * m_pEquippedArtifacts[5];
};

#endif
