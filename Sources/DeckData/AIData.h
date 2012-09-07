#ifndef _AIDATA_H
#define _AIDATA_H

#include "../Data/XMLObject.h"

class LocalClient;
class AvatarData;

class AIData : public XMLObject
{
public:
  AIData();
  ~AIData();

  AvatarData * createAvatar(LocalClient * pLocalClient);
  void fillSpellsList(ObjectList * pList, LocalClient * pLocalClient);

  char m_sEdition[NAME_MAX_CHARS];
  char m_sAvatarId[NAME_MAX_CHARS];
  ObjectList * m_pSpellsPacks;
};

#endif
