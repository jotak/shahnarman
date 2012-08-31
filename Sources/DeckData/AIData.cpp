// -----------------------------------------------------------------
// AI DATA
// -----------------------------------------------------------------
#include "AIData.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../Data/DataFactory.h"
#include "../Players/Spell.h"
#include "ShopItem.h"
#include "Edition.h"
#include "AvatarData.h"

// -----------------------------------------------------------------
// Name : AIData
//  ctor
// -----------------------------------------------------------------
AIData::AIData()
{
  wsafecpy(m_sEdition, NAME_MAX_CHARS, L"");
  wsafecpy(m_sAvatarId, NAME_MAX_CHARS, L"");
  m_pSpellsPacks = new ObjectList(true);
}

// -----------------------------------------------------------------
// Name : ~AIData
//  destructor
// -----------------------------------------------------------------
AIData::~AIData()
{
  delete m_pSpellsPacks;
}

// -----------------------------------------------------------------
// Name : createAvatar
// -----------------------------------------------------------------
AvatarData * AIData::createAvatar(LocalClient * pLocalClient)
{
  Edition * pEdition = pLocalClient->getDataFactory()->findEdition(m_sEdition);
  if (pEdition == NULL) {
    pLocalClient->getDebug()->notifyErrorMessage(L"Error: edition not found for AIData::createAvatar.");
    return NULL;
  }
  AvatarData * pAvatar = (AvatarData*) pEdition->findUnitData(m_sAvatarId);
  pAvatar = pAvatar->cloneStaticData(NULL, pLocalClient->getDebug());
  return pAvatar;
}

// -----------------------------------------------------------------
// Name : fillSpellsList
// -----------------------------------------------------------------
void AIData::fillSpellsList(ObjectList * pList, LocalClient * pLocalClient)
{
  Edition * pEdition = pLocalClient->getDataFactory()->findEdition(m_sEdition);
  if (pEdition == NULL) {
    pLocalClient->getDebug()->notifyErrorMessage(L"Error: edition not found for AIData::fillSpellsList.");
    return;
  }

  SpellsPackContent * pPack = (SpellsPackContent*) m_pSpellsPacks->getFirst(0);
  while (pPack != NULL) {
    for (int i = 0; i < pPack->m_iNbSpells; i++)
    {
      Spell * pSpell = NULL;
      if (pPack->m_iMode == PACK_MODE_FIXED)
        pSpell = pEdition->findSpell(pPack->m_sSpellId);
      else
        pSpell = pEdition->selectRandomSpell(pPack->m_iMode);
      if (pSpell == NULL)
        pLocalClient->getDebug()->notifyErrorMessage(L"NULL spell in AIData::fillSpellsList - spell selection failed.");
      else
        pList->addLast(pSpell);
    }
    pPack = (SpellsPackContent*) m_pSpellsPacks->getNext(0);
  }
}
