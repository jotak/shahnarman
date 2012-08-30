#ifndef _PROFILE_H
#define _PROFILE_H

#include "../Common/BaseObject.h"

class LocalClient;
class ShopItem;
class ObjectList;
class AvatarData;
class ProgressionElement;
class ProgressionTree;

class Profile : public BaseObject
{
public:
  class SpellData : public BaseObject
  {
  public:
    wchar_t m_sEdition[NAME_MAX_CHARS];
    wchar_t m_sName[NAME_MAX_CHARS];
    AvatarData * m_pOwner;
  };

  Profile(LocalClient * pLocalClient);
  ~Profile();

  bool create(wchar_t * sName);
  bool load(wchar_t * sName);
  bool save();
  void deleteProfile();

  // Member access
  wchar_t * getName() { return m_sName; };
  int getCash() { return m_iCash; };
  int getNumberOfWonGames() { return m_iWonGames; };
  int getNumberOfLostGames() { return m_iLostGames; };
  ObjectList * getAvatarsList() { return m_pAvatars; };
  ObjectList * getSpellsList() { return m_pSpells; };
  ObjectList * getArtifactsList() { return m_pArtifacts; };

  void buyItem(ShopItem * pItem);
  AvatarData * findAvatar(wchar_t * sEdition, wchar_t * sName);
  bool replaceAvatar(AvatarData * pNewAvatar);
  void addCash(int cash) { m_iCash += cash; };
  void addSpell(wchar_t * sEdition, wchar_t * sName);
  void addArtifact(wchar_t * sEdition, wchar_t * sName);
  void addAvatar(wchar_t * sEdition, wchar_t * sName);
  void addAvatar(AvatarData * pAvatar, int iCost);
  void deleteAvatar(AvatarData * pAvatar);
  void applyAvatarProgression(AvatarData * pAvatar, u8 uTree, ProgressionElement * pElt);
  void openAvatarProgressionTree(AvatarData * pAvatar, u8 uTree, ProgressionTree * pTree);

protected:
  void _applyAvatarProgressionEffects(AvatarData * pAvatar, ObjectList * pEffects);

  wchar_t m_sName[NAME_MAX_CHARS];
  int m_iCash;
  int m_iWonGames;
  int m_iLostGames;
  ObjectList * m_pAvatars;
  ObjectList * m_pSpells;
  LocalClient * m_pLocalClient;
  ObjectList * m_pArtifacts;
};

#endif
