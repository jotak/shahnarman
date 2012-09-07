#ifndef _DATA_FACTORY_H
#define _DATA_FACTORY_H

#include "../Common/ObjectList.h"

class Edition;
class LocalClient;
class UnitData;
class AvatarData;
class Spell;
class Profile;

class DataFactory
{
public:
  // Constructor / destructor
  DataFactory();
  ~DataFactory();

  void Init(LocalClient * pLocalClient);
  UnitData * getUnitData(const char * sEdition, const char * strId);
  Spell * findSpell(const char * sEdition, const char * sName);
  Edition * findEdition(const char * sId);
  Edition * getFirstEdition() { return (Edition*) m_pEditions->getFirst(0); };
  Edition * getNextEdition() { return (Edition*) m_pEditions->getNext(0); };
  Profile * findProfile(const char * sName);
  Profile * getFirstProfile() { return (Profile*) m_pAllProfiles->getFirst(0); };
  Profile * getNextProfile() { return (Profile*) m_pAllProfiles->getNext(0); };
  void onProfileAdded(Profile * pProfile);
  void onProfileDeleted(Profile * pProfile);
  void resetGameAvatarData();
  void addGameAvatarData(AvatarData * pAvatar);

private:
  ObjectList * m_pEditions;
  ObjectList * m_pAllProfiles;
  ObjectList * m_pGameAvatarsData;
};

#endif
