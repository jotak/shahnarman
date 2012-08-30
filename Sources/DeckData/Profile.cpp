#include "Profile.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "Edition.h"
#include "ShopItem.h"
#include "AvatarData.h"
#include "../Data/DataFactory.h"
#include "../Data/FileSerializer.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../Interface/InterfaceManager.h"
#include "../Interface/LevelUpDlg.h"
#include "../Interface/BuildDeckDlg.h"
#include "../Interface/StartMenuDlg.h"
#include "../Interface/ShopDlg.h"

// -----------------------------------------------------------------
// Name : Profile
//  Constructor
// -----------------------------------------------------------------
Profile::Profile(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pAvatars = new ObjectList(true);
  m_pSpells = new ObjectList(true);
  m_pArtifacts = new ObjectList(true);
  m_iCash = 0;
  m_iWonGames = 0;
  m_iLostGames = 0;
}

// -----------------------------------------------------------------
// Name : ~Profile
//  Destructor
// -----------------------------------------------------------------
Profile::~Profile()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Profile\n");
#endif
  delete m_pAvatars;
  delete m_pSpells;
  delete m_pArtifacts;
#ifdef DBG_VERBOSE1
  printf("End destroy Profile\n");
#endif
}

// -----------------------------------------------------------------
// Name : create
// -----------------------------------------------------------------
bool Profile::create(wchar_t * sName)
{
  wsafecpy(m_sName, NAME_MAX_CHARS, sName);
  m_pAvatars->deleteAll();
  m_pSpells->deleteAll();
  m_pArtifacts->deleteAll();
  m_iCash = 100;
  m_iWonGames = 0;
  m_iLostGames = 0;
  return save();
}

// -----------------------------------------------------------------
// Name : load
// -----------------------------------------------------------------
bool Profile::load(wchar_t * sName)
{
  int iStrSize;
  wsafecpy(m_sName, NAME_MAX_CHARS, sName);
  wchar_t sFilePath[MAX_PATH];
  swprintf_s(sFilePath, MAX_PATH, L"%s%s.dat", PROFILES_PATH, m_sName);

  FILE * f = NULL;
  if (0 != wfopen(&f, sFilePath, L"rb"))
  {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Error: cannot open file %s for reading. Operation cancelled.", sFilePath);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }

  m_pAvatars->deleteAll();
  m_pSpells->deleteAll();

  fread(&m_iCash, sizeof(int), 1, f);
  fread(&m_iWonGames, sizeof(int), 1, f);
  fread(&m_iLostGames, sizeof(int), 1, f);
  int nbAvatars;
  fread(&nbAvatars, sizeof(int), 1, f);
  for (int i = 0; i < nbAvatars; i++)
  {
    AvatarData * pAvatar = new AvatarData();
    pAvatar->deserialize(FileSerializer::getInstance(f), m_pLocalClient->getDebug());
    m_pAvatars->addLast(pAvatar);
  }
  int nbSpells;
  wchar_t sOwnerName[NAME_MAX_CHARS];
  wchar_t sOwnerEdition[NAME_MAX_CHARS];
  fread(&nbSpells, sizeof(int), 1, f);
  for (int i = 0; i < nbSpells; i++)
  {
    SpellData * p = new SpellData();
    fread(&iStrSize, sizeof(int), 1, f);
    fread(p->m_sEdition, sizeof(wchar_t), iStrSize, f);
    fread(&iStrSize, sizeof(int), 1, f);
    fread(p->m_sName, sizeof(wchar_t), iStrSize, f);
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sOwnerEdition, sizeof(wchar_t), iStrSize, f);
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sOwnerName, sizeof(wchar_t), iStrSize, f);
    p->m_pOwner = findAvatar(sOwnerEdition, sOwnerName);
    m_pSpells->addLast(p);
  }
  wchar_t sEdition[NAME_MAX_CHARS];
  wchar_t sObjectId[NAME_MAX_CHARS];
  int nbArtifacts;
  fread(&nbArtifacts, sizeof(int), 1, f);
  for (int i = 0; i < nbArtifacts; i++)
  {
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sEdition, sizeof(wchar_t), iStrSize, f);
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sObjectId, sizeof(wchar_t), iStrSize, f);
    Artifact * pArtifact = NULL;
    Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(sEdition);
    if (pEdition != NULL)
      pArtifact = pEdition->findArtifact(sObjectId);
    // If artifact can't be loaded, we'll create a dummy one anyway, because maybe it was just not found beacause the edition was deactivated... but it should be saved anyway when calling Profile::save!
    if (pArtifact == NULL)
      pArtifact = new Artifact(sEdition, sObjectId, L"", 0, false); // dummy one
    else
      pArtifact = pArtifact->clone();
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sOwnerEdition, sizeof(wchar_t), iStrSize, f);
    fread(&iStrSize, sizeof(int), 1, f);
    fread(sOwnerName, sizeof(wchar_t), iStrSize, f);
    pArtifact->m_pOwner = findAvatar(sOwnerEdition, sOwnerName);
    m_pArtifacts->addLast(pArtifact);
  }
  fclose(f);

  return true;
}

// -----------------------------------------------------------------
// Name : save
// -----------------------------------------------------------------
bool Profile::save()
{
  int iStrSize;
  wchar_t sFilePath[MAX_PATH];
  swprintf_s(sFilePath, MAX_PATH, L"%s%s.dat", PROFILES_PATH, m_sName);

  FILE * f = NULL;
  if (0 != wfopen(&f, sFilePath, L"wb"))
  {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Error: cannot open file %s for writing. Operation cancelled.", sFilePath);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }

  fwrite(&m_iCash, sizeof(int), 1, f);
  fwrite(&m_iWonGames, sizeof(int), 1, f);
  fwrite(&m_iLostGames, sizeof(int), 1, f);
  fwrite(&(m_pAvatars->size), sizeof(int), 1, f);
  AvatarData * pAvatar = (AvatarData*) m_pAvatars->getFirst(0);
  while (pAvatar != NULL)
  {
    // Write avatar data : edition, name, xp, progression, fame, custom description
    pAvatar->serialize(FileSerializer::getInstance(f));
    pAvatar = (AvatarData*) m_pAvatars->getNext(0);
  }
  fwrite(&m_pSpells->size, sizeof(int), 1, f);
  SpellData * pSpell = (SpellData*) m_pSpells->getFirst(0);
  while (pSpell != NULL)
  {
    iStrSize = 1 + wcslen(pSpell->m_sEdition);
    fwrite(&iStrSize, sizeof(int), 1, f);
    fwrite(pSpell->m_sEdition, sizeof(wchar_t), iStrSize, f);
    iStrSize = 1 + wcslen(pSpell->m_sName);
    fwrite(&iStrSize, sizeof(int), 1, f);
    fwrite(pSpell->m_sName, sizeof(wchar_t), iStrSize, f);
    if (pSpell->m_pOwner == NULL)
    {
      iStrSize = 1;
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(L"", sizeof(wchar_t), iStrSize, f);
      iStrSize = 1;
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(L"", sizeof(wchar_t), iStrSize, f);
    }
    else
    {
      iStrSize = 1 + wcslen(pSpell->m_pOwner->m_sEdition);
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(pSpell->m_pOwner->m_sEdition, sizeof(wchar_t), iStrSize, f);
      iStrSize = 1 + wcslen(pSpell->m_pOwner->m_sObjectId);
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(pSpell->m_pOwner->m_sObjectId, sizeof(wchar_t), iStrSize, f);
    }
    pSpell = (SpellData*) m_pSpells->getNext(0);
  }
  fwrite(&m_pArtifacts->size, sizeof(int), 1, f);
  Artifact * pArtifact = (Artifact*) m_pArtifacts->getFirst(0);
  while (pArtifact != NULL)
  {
    iStrSize = 1 + wcslen(pArtifact->getEdition());
    fwrite(&iStrSize, sizeof(int), 1, f);
    fwrite(pArtifact->getEdition(), sizeof(wchar_t), iStrSize, f);
    iStrSize = 1 + wcslen(pArtifact->m_sObjectId);
    fwrite(&iStrSize, sizeof(int), 1, f);
    fwrite(pArtifact->m_sObjectId, sizeof(wchar_t), iStrSize, f);
    if (pArtifact->m_pOwner == NULL)
    {
      iStrSize = 1;
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(L"", sizeof(wchar_t), iStrSize, f);
      iStrSize = 1;
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(L"", sizeof(wchar_t), iStrSize, f);
    }
    else
    {
      iStrSize = 1 + wcslen(pArtifact->m_pOwner->m_sEdition);
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(pArtifact->m_pOwner->m_sEdition, sizeof(wchar_t), iStrSize, f);
      iStrSize = 1 + wcslen(pArtifact->m_pOwner->m_sObjectId);
      fwrite(&iStrSize, sizeof(int), 1, f);
      fwrite(pArtifact->m_pOwner->m_sObjectId, sizeof(wchar_t), iStrSize, f);
    }
    pArtifact = (Artifact*) m_pArtifacts->getNext(0);
  }
  fclose(f);

  return true;
}

// -----------------------------------------------------------------
// Name : deleteProfile
// -----------------------------------------------------------------
void Profile::deleteProfile()
{
  wchar_t sFilePath[MAX_PATH];
  swprintf_s(sFilePath, MAX_PATH, L"%s%s.dat", PROFILES_PATH, m_sName);
  _wremove(sFilePath);
}

// -----------------------------------------------------------------
// Name : addSpell
// -----------------------------------------------------------------
void Profile::addSpell(wchar_t * sEdition, wchar_t * sName)
{
  SpellData * p = new SpellData();
  wsafecpy(p->m_sEdition, NAME_MAX_CHARS, sEdition);
  wsafecpy(p->m_sName, NAME_MAX_CHARS, sName);
  p->m_pOwner = NULL;
  m_pSpells->addLast(p);
}

// -----------------------------------------------------------------
// Name : addArtifact
// -----------------------------------------------------------------
void Profile::addArtifact(wchar_t * sEdition, wchar_t * sName)
{
  Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(sEdition);
  Artifact * pArtifact = pEdition->findArtifact(sName);
  m_pArtifacts->addLast(pArtifact->clone());
}

// -----------------------------------------------------------------
// Name : addAvatar
// -----------------------------------------------------------------
void Profile::addAvatar(wchar_t * sEdition, wchar_t * sName)
{
  AvatarData * pAvatar = (AvatarData*) m_pLocalClient->getDataFactory()->getUnitData(sEdition, sName);
  assert(pAvatar != NULL);
  m_pAvatars->addLast(pAvatar->cloneStaticData(this, m_pLocalClient->getDebug()));
}

// -----------------------------------------------------------------
// Name : addAvatar
// -----------------------------------------------------------------
void Profile::addAvatar(AvatarData * pAvatar, int iCost)
{
  if (wcscmp(pAvatar->m_sObjectId, L"") == 0)
  {
    // Generate random object id
    while (true)
    {
      swprintf_s(pAvatar->m_sObjectId, NAME_MAX_CHARS, L"*gen*%ld%ld%ld", (long) getRandom(RAND_MAX), (long) getRandom(RAND_MAX), (long) getRandom(RAND_MAX));
      // Check that it doesn't already exist in DataFactory
      UnitData * pUnitData = m_pLocalClient->getDataFactory()->getUnitData(pAvatar->m_sEdition, pAvatar->m_sObjectId);
      if (pUnitData == NULL)
      {
        // Check that it doesn't already exist in Avatars
        bool bFound = false;
        AvatarData * pOther = (AvatarData*) m_pAvatars->getFirst(0);
        while (pOther != NULL)
        {
          if (wcscmp(pAvatar->m_sEdition, pOther->m_sEdition) == 0 && wcscmp(pAvatar->m_sObjectId, pOther->m_sObjectId) == 0)
          {
            bFound = true;
            break;
          }
          pOther = (AvatarData*) m_pAvatars->getNext(0);
        }
        if (!bFound)
          break;
      }
    }
  }
  m_pAvatars->addLast(pAvatar);
  m_iCash -= iCost;
  if (pAvatar->isLevelUp()) // should always be true
    m_pLocalClient->getInterface()->getLevelUpDialog()->doAvatarLevelUp(pAvatar, m_pLocalClient->getInterface()->getStartMenuDialog(), true);
  save();
}

// -----------------------------------------------------------------
// Name : buyItem
// -----------------------------------------------------------------
void Profile::buyItem(ShopItem * pItem)
{
  Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(pItem->m_sEdition);
  if (pEdition == NULL)
  {
    m_pLocalClient->getDebug()->notifyErrorMessage(L"Error: edition is probably disabled.");
    return;
  }

  if (pItem->m_iType == 0)  // pack
  {
    PackShopItem::PackShopItem_content * pContent = (PackShopItem::PackShopItem_content*) ((PackShopItem*)pItem)->m_pContent->getFirst(0);
    while (pContent != NULL)
    {
      for (int i = 0; i < pContent->m_iNbSpells; i++)
      {
        Spell * pSpell = pEdition->buySpell(pContent->m_iMode);
        addSpell(pItem->m_sEdition, pSpell->getObjectName());
      }
      pContent = (PackShopItem::PackShopItem_content*) ((PackShopItem*)pItem)->m_pContent->getNext(0);
    }
  }
  else if (pItem->m_iType == 1)  // avatar
  {
    AvatarData * pAvatar = ((AvatarData*) m_pLocalClient->getDataFactory()->getUnitData(pItem->m_sEdition, ((AvatarShopItem*)pItem)->m_sAvatarId))->cloneStaticData(this, m_pLocalClient->getDebug());
    m_pAvatars->addLast(pAvatar);
    if (pAvatar->isLevelUp()) // should always be true
      m_pLocalClient->getInterface()->getLevelUpDialog()->doAvatarLevelUp(pAvatar, m_pLocalClient->getInterface()->getShopDialog(), true);
  }
  else if (pItem->m_iType == 2)  // artifact
  {
    Artifact * pArtifact = pEdition->findArtifact(((ArtifactShopItem*)pItem)->m_sArtifactId);
    m_pArtifacts->addLast(pArtifact->clone());
  }
  m_iCash -= pItem->m_iCost;
  save();
}

// -----------------------------------------------------------------
// Name : findAvatar
// -----------------------------------------------------------------
AvatarData * Profile::findAvatar(wchar_t * sEdition, wchar_t * sName)
{
  AvatarData * pAvatar = (AvatarData*) m_pAvatars->getFirst(0);
  while (pAvatar != NULL)
  {
    if (wcscmp(pAvatar->m_sEdition, sEdition) == 0
        && wcscmp(pAvatar->m_sObjectId, sName) == 0)
      return pAvatar;
    pAvatar = (AvatarData*) m_pAvatars->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : replaceAvatar
// -----------------------------------------------------------------
bool Profile::replaceAvatar(AvatarData * pNewAvatar)
{
  AvatarData * pOldAvatar = (AvatarData*) m_pAvatars->getFirst(0);
  while (pOldAvatar != NULL)
  {
    if (wcscmp(pOldAvatar->m_sEdition, pNewAvatar->m_sEdition) == 0
        && wcscmp(pOldAvatar->m_sObjectId, pNewAvatar->m_sObjectId) == 0)
    {
      // Replace avatar ptr in spells data
      SpellData * pSpell = (SpellData*) m_pSpells->getFirst(0);
      while (pSpell != NULL)
      {
        if (pSpell->m_pOwner == pOldAvatar)
          pSpell->m_pOwner = pNewAvatar;
        pSpell = (SpellData*) m_pSpells->getNext(0);
      }
      // Replace avatar ptr in artifacts data
      Artifact * pArtifact = (Artifact*) m_pArtifacts->getFirst(0);
      while (pArtifact != NULL)
      {
        if (pArtifact->m_pOwner == pOldAvatar)
          pArtifact->m_pOwner = pNewAvatar;
        pArtifact = (Artifact*) m_pArtifacts->getNext(0);
      }
      m_pAvatars->deleteCurrent(0, true);
      pNewAvatar->m_pOwner = this;
      m_pAvatars->addLast(pNewAvatar);
      return true;
    }
    pOldAvatar = (AvatarData*) m_pAvatars->getNext(0);
  }
  return false;
}

// -----------------------------------------------------------------
// Name : deleteAvatar
// -----------------------------------------------------------------
void Profile::deleteAvatar(AvatarData * pAvatar)
{
  if (m_pAvatars->deleteObject(pAvatar, true) > 0)
    save();
}

// -----------------------------------------------------------------
// Name : _applyAvatarProgressionEffects
// -----------------------------------------------------------------
void Profile::_applyAvatarProgressionEffects(AvatarData * pAvatar, ObjectList * pEffects)
{
  ProgressionEffect * pEffect = (ProgressionEffect*) pEffects->getFirst(0);
  while (pEffect != NULL)
  {
    // PROGRESSION_EFFECT_AVATAR, PROGRESSION_EFFECT_SPELL and PROGRESSION_EFFECT_ARTIFACT require imediate effect
    //  whereas PROGRESSION_EFFECT_CHARAC and PROGRESSION_EFFECT_SKILL are applied on avatar loading
    switch (pEffect->getType())
    {
    case PROGRESSION_EFFECT_AVATAR:
      addAvatar(((ProgressionEffect_Avatar*)pEffect)->m_sAvatarEdition, ((ProgressionEffect_Avatar*)pEffect)->m_sAvatarName);
      break;
    case PROGRESSION_EFFECT_SPELL:
      addSpell(((ProgressionEffect_Spell*)pEffect)->m_sSpellEdition, ((ProgressionEffect_Spell*)pEffect)->m_sSpellName);
      break;
    case PROGRESSION_EFFECT_ARTIFACT:
      addArtifact(((ProgressionEffect_Artifact*)pEffect)->m_sArtifactEdition, ((ProgressionEffect_Artifact*)pEffect)->m_sArtifactName);
      break;
    case PROGRESSION_EFFECT_CHARAC:
      {
        long_hash::iterator it = pAvatar->m_lValues.find(((ProgressionEffect_Charac*)pEffect)->m_sKey);
        if (it != pAvatar->m_lValues.end())
          it->second += ((ProgressionEffect_Charac*)pEffect)->m_iModifier;
        break;
      }
    case PROGRESSION_EFFECT_SKILL:
      Skill * pSkill = new Skill(((ProgressionEffect_Skill*)pEffect)->m_sSkillEdition, ((ProgressionEffect_Skill*)pEffect)->m_sSkillName, ((ProgressionEffect_Skill*)pEffect)->m_sSkillParameters, m_pLocalClient->getDebug());
      pAvatar->m_pSkills->addFirst(pSkill);
      break;
    }
    pEffect = (ProgressionEffect*) pEffects->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : applyAvatarProgression
// -----------------------------------------------------------------
void Profile::applyAvatarProgression(AvatarData * pAvatar, u8 uTree, ProgressionElement * pElt)
{
  assert(uTree >= 0 && uTree < NB_PROGRESSION_TREES);
  wsafecpy(pAvatar->m_pProgression[uTree].sElements[pElt->m_uLevel], NAME_MAX_CHARS, pElt->m_sObjectId);
  _applyAvatarProgressionEffects(pAvatar, pElt->m_pEffects);
  save();
}

// -----------------------------------------------------------------
// Name : openAvatarProgressionTree
// -----------------------------------------------------------------
void Profile::openAvatarProgressionTree(AvatarData * pAvatar, u8 uTree, ProgressionTree * pTree)
{
  assert(uTree >= 0 && uTree < NB_PROGRESSION_TREES);
  assert(pTree != NULL);
  wsafecpy(pAvatar->m_pProgression[uTree].sTreeName, NAME_MAX_CHARS, pTree->m_sObjectId);
  _applyAvatarProgressionEffects(pAvatar, pTree->m_pBaseEffects);
  save();
}
