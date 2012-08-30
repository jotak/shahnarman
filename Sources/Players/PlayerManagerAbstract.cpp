// -----------------------------------------------------------------
// Name : PlayerManagerAbstract
// -----------------------------------------------------------------
#include "PlayerManagerAbstract.h"
#include "Player.h"
#include "Spell.h"
#include "../LocalClient.h"
#include "../Data/DataFactory.h"
#include "../Common/ObjectList.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/Town.h"
#include "../Data/LuaContext.h"

// -----------------------------------------------------------------
// Name : PlayerManagerAbstract
// -----------------------------------------------------------------
PlayerManagerAbstract::PlayerManagerAbstract()
{
  m_pPlayersList = new ObjectList(true);
  m_pDeadPlayers = new ObjectList(true);
  m_uFirstResolutionListIdx = 0;
  m_pGlobalSpells = new ObjectList(false);
  m_pNeutralPlayer = NULL;
}

// -----------------------------------------------------------------
// Name : ~PlayerManagerAbstract
// -----------------------------------------------------------------
PlayerManagerAbstract::~PlayerManagerAbstract()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy PlayerManagerAbstract\n");
#endif
  FREE(m_pPlayersList);
  FREE(m_pDeadPlayers);
  delete m_pGlobalSpells;
  FREE(m_pNeutralPlayer);
#ifdef DBG_VERBOSE1
  printf("End destroy PlayerManagerAbstract\n");
#endif
}

// -----------------------------------------------------------------
// Name : deserializePlayersData
// -----------------------------------------------------------------
void PlayerManagerAbstract::deserializePlayersData(NetworkData * pData, LocalClient * pLocalClient, Map * pMap)
{
  std::queue<RELINK_PTR_DATA> relinkPtrData;
  m_pNeutralPlayer = new Player(0, 0, getGlobalSpellsPtr());
  m_pNeutralPlayer->deserialize(pData, false, pLocalClient);
  long nbPlayers = pData->readLong();
  while (nbPlayers-- > 0)
  {
    // Player general data
    Player * pPlayer = new Player(0, 0, getGlobalSpellsPtr());
    pPlayer->deserialize(pData, false, pLocalClient);
    pPlayer->deserializeSpells(pData, pLocalClient);
    m_pPlayersList->addLast(pPlayer);
  }
  m_pNeutralPlayer->deserializeUnits(pData, pMap, pLocalClient, &relinkPtrData);
  pLocalClient->getDataFactory()->resetGameAvatarData();
  Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
  while (pPlayer != NULL)
  {
    pPlayer->deserializeUnits(pData, pMap, pLocalClient, &relinkPtrData);
    pLocalClient->getDataFactory()->addGameAvatarData(pPlayer->m_pAvatarData);
    pPlayer = (Player*) m_pPlayersList->getNext(0);
  }

  // Rebuild pointers links after deserialization (attack targets, etc.)
  while (!relinkPtrData.empty())
  {
    RELINK_PTR_DATA data = relinkPtrData.front();
    relinkPtrData.pop();
    switch (data.type)
    {
    case RELINK_TYPE_ATTACKING_UNIT:
      {
        Unit * pUnit1 = (Unit*) data.data1;
        Player * pPlayer = findPlayer((u8)data.data2);
        if (pPlayer != NULL)
        {
          pUnit1->m_pAttackTarget = pPlayer->findUnit(data.data3);
          if (pUnit1->m_pAttackTarget->getStatus() == US_Dead)
            pUnit1->m_pAttackTarget = NULL;
        }
        pUnit1->recomputePath();
        break;
      }
    //case RELINK_TYPE_SPELL_TARGET_UNIT:
    //  {
    //    Unit * pUnit = (Unit*) data.data1;
    //    Player * pPlayer = findPlayer(data.data2);
    //    assert(pPlayer != NULL);
    //    Spell * pSpell = pPlayer->findSpell(0, data.data3, pPlayer->m_pActiveSpells);
    //    assert(pSpell != NULL);
    //    pSpell->addTarget(pUnit, LUATARGET_UNIT);
    //    break;
    //  }
    //case RELINK_TYPE_SPELL_TARGET_PLAYER:
    //  {
    //    Player * pTarget = (Player*) data.data1;
    //    Player * pPlayer = findPlayer(data.data2);
    //    assert(pPlayer != NULL);
    //    Spell * pSpell = pPlayer->findSpell(0, data.data3, pPlayer->m_pActiveSpells);
    //    assert(pSpell != NULL);
    //    pSpell->addTarget(pTarget, LUATARGET_PLAYER);
    //    break;
    //  }
    }
  }
}

// -----------------------------------------------------------------
// Name : deserializeLuaTargets
// -----------------------------------------------------------------
void PlayerManagerAbstract::deserializeLuaTargets(NetworkData * pData, LocalClient * pLocalClient, Map * pMap)
{
  LuaContext context;
  int nbObjects = pData->readLong();
  for (int i = 0; i < nbObjects; i++)
    context.deserializeTargets(pData, this, pMap);
}

// -----------------------------------------------------------------
// Name : findPlayer
// -----------------------------------------------------------------
Player * PlayerManagerAbstract::findPlayer(u8 uPlayerId)
{
  if (uPlayerId == 0)
    return m_pNeutralPlayer;
  Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
  while (pPlayer != NULL)
  {
    if (pPlayer->m_uPlayerId == uPlayerId)
      return pPlayer;
    pPlayer = (Player*) m_pPlayersList->getNext(0);
  }
  pPlayer = (Player*) m_pDeadPlayers->getFirst(0);
  while (pPlayer != NULL)
  {
    if (pPlayer->m_uPlayerId == uPlayerId)
      return pPlayer;
    pPlayer = (Player*) m_pDeadPlayers->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getPlayersCount
// -----------------------------------------------------------------
u8 PlayerManagerAbstract::getPlayersCount()
{
  return m_pPlayersList->size;
}

// -----------------------------------------------------------------
// Name : getFirstPlayerAndNeutral
// -----------------------------------------------------------------
Player * PlayerManagerAbstract::getFirstPlayerAndNeutral(int _it)
{
  m_pPlayersList->getFirst(_it);
  return m_pNeutralPlayer;
}

// -----------------------------------------------------------------
// Name : getNextPlayerAndNeutral
// -----------------------------------------------------------------
Player * PlayerManagerAbstract::getNextPlayerAndNeutral(int _it)
{
  Player * pPlayer = (Player*) m_pPlayersList->getCurrent(_it);
  if (pPlayer != NULL)
    m_pPlayersList->getNext(_it);
  return pPlayer;
}

// -----------------------------------------------------------------
// Name : findTargetFromIdentifiers
// -----------------------------------------------------------------
LuaTargetable * PlayerManagerAbstract::findTargetFromIdentifiers(long iType, wchar_t * sIds, Map * pMap)
{
  wchar_t sType[64];
  if (iType == LUATARGET_PLAYER)
  {
    int id;
    swscanf_s(sIds, L"%s %d", sType, 64, &id);
    return findPlayer(id);
  }
  else if (iType == LUATARGET_TEMPLE)
  {
    long id, owner;
    swscanf_s(sIds, L"%s %ld %ld", sType, 64, &owner, &id);
    return pMap->findTemple(id);
  }
  else if (iType == LUATARGET_TOWN)
  {
    long id, owner;
    swscanf_s(sIds, L"%s %ld %ld", sType, 64, &owner, &id);
    return pMap->findTown(id);
  }
  else if (iType == LUATARGET_TILE)
  {
    long x, y;
    swscanf_s(sIds, L"%s %ld %ld", sType, 64, &x, &y);
    return pMap->getTileAt(CoordsMap(x, y));
  }
  else if (iType == LUATARGET_UNIT)
  {
    long id, owner;
    swscanf_s(sIds, L"%s %ld %ld", sType, 64, &owner, &id);
    Player * pPlayer = findPlayer(owner);
    assert(pPlayer != NULL);
    return pPlayer->findUnit(id);
  }
  return NULL;
}
