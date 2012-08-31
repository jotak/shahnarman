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

  wchar_t m_sEdition[NAME_MAX_CHARS];
  wchar_t m_sAvatarId[NAME_MAX_CHARS];
  ObjectList * m_pSpellsPacks;
};

#endif
