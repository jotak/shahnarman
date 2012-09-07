// -----------------------------------------------------------------
// Name : TURN SOLVER
// -----------------------------------------------------------------
#include "TurnSolver.h"
#include "Server.h"
#include "NetworkData.h"
#include "../Debug/DebugManager.h"
#include "../Players/Player.h"
#include "SpellsSolver.h"
#include "AISolver.h"
#include "../Data/DataFactory.h"
#include "../Players/Spell.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/SpecialTile.h"

// -----------------------------------------------------------------
// Name : TurnSolver
// -----------------------------------------------------------------
TurnSolver::TurnSolver(Server * pServer)
{
  m_pServer = pServer;
  m_iPlayerIt = m_pPlayersList->getIterator();
  m_pSpellsSolver = new SpellsSolver(pServer, this);
  m_pAISolver = new AISolver(pServer);
  m_uUnitsCount = 0;
  m_pCurrentPlayer = NULL;
  m_pCurrentUnit = NULL;
  m_ResolveState = RS_NotResolving;
  m_uNbHumanPlayers = 0;
  m_pDefendingUnit = NULL;
  m_bIsRangeAttack = false;
  m_bUnitHasAttacked = false;
  m_iAttackerDamages = m_iDefenderDamages = m_iAttackerArmor = m_iDefenderArmor = m_iAttackerLife = m_iDefenderLife = 0;
}

// -----------------------------------------------------------------
// Name : ~TurnSolver
// -----------------------------------------------------------------
TurnSolver::~TurnSolver()
{
  FREE(m_pSpellsSolver);
  FREE(m_pAISolver);
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void TurnSolver::Init()
{
  m_ResolveState = RS_NotResolving;
  m_pSpellsSolver->Init();
  m_uNbHumanPlayers = 0;
}

// -----------------------------------------------------------------
// Name : onInitFinished
// -----------------------------------------------------------------
void TurnSolver::onInitFinished()
{
  m_ResolveState = RS_EndResolve;
  // Call special tiles "onCreate"
  SpecialTile * pSpec = m_pServer->getMap()->getFirstSpecialTile();
  while (pSpec != NULL)
  {
    pSpec->callLuaFunction(L"onCreate", 0, L"l", (long) pSpec->getInstanceId());
    pSpec = m_pServer->getMap()->getNextSpecialTile();
  }
  resetDataForNextTurn(true);
  NetworkData data(NETWORKMSG_RESOLVE_PHASE_ENDS);
  data.addLong((long)m_uFirstResolutionListIdx);
  m_pServer->sendMessageToAllClients(&data);
  m_ResolveState = RS_NotResolving;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void TurnSolver::serialize(NetworkData * pData)
{
  pData->addLong(m_uNbHumanPlayers);
  pData->addLong(m_uFirstResolutionListIdx);
  pData->addLong(m_uUnitsCount);
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void TurnSolver::deserialize(NetworkData * pData)
{
  m_uNbHumanPlayers = (u8) pData->readLong();
  m_uFirstResolutionListIdx = (u8) pData->readLong();
  m_uUnitsCount = (unsigned long) pData->readLong();
}

// -----------------------------------------------------------------
// Name : setInitialAvatar
// -----------------------------------------------------------------
void TurnSolver::setInitialAvatar(UnitData * pAvatar, Player * pPlayer, CoordsMap pos)
{
  assert(pPlayer != NULL);
  assert(pAvatar != NULL);
  Unit * unit = new Unit(pos, m_pServer->getMap(), getGlobalSpellsPtr());
  pPlayer->m_pUnits->addFirst(unit);
  unit->init(++m_uUnitsCount, pPlayer->m_uPlayerId, pAvatar, m_pServer->getDebug());
  pPlayer->setAvatar((AvatarData*) pAvatar, unit);
  m_uNbHumanPlayers++;
}

// -----------------------------------------------------------------
// Name : addInitialPlayerSpell
// -----------------------------------------------------------------
void TurnSolver::addInitialPlayerSpell(Player * pPlayer, wchar_t * sEdition, wchar_t * sName)
{
  Spell * pSpell = new Spell(pPlayer->m_uPlayerId, sEdition, 1, sName, m_pServer->getDebug());
  pPlayer->m_pDeck->addLast(pSpell);
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void TurnSolver::Update(double delta)
{
  switch (m_ResolveState)
  {
  case RS_NotResolving:
    {
      bool bAllFinished = true;
      Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
      while (pPlayer != NULL)
      {
        bAllFinished &= ((pPlayer->getState() == finished) || (pPlayer->m_bIsAI));
        pPlayer = (Player*) m_pPlayersList->getNext(0);
      }
      if (bAllFinished)
        nextPhase();
      break;
    }
  case RS_ResolveAI:
    {
      bool bAllFinished = true;
      Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
      while (pPlayer != NULL)
      {
        if (pPlayer->m_bIsAI && pPlayer->getState() != finished) {
          m_pAISolver->resolveAI(pPlayer);
          bAllFinished = false;
          break;
        }
        pPlayer = (Player*) m_pPlayersList->getNext(0);
      }
      if (bAllFinished)
        nextPhase();
      break;
    }
  case RS_ResolveNeutral:
    {
      m_pCurrentUnit = (Unit*) m_pNeutralPlayer->m_pUnits->getCurrent(0);
      if (m_pCurrentUnit == NULL)
        nextPhase();
      else
      {
        m_pAISolver->resolveNeutralAI(m_pCurrentUnit);
        m_pNeutralPlayer->m_pUnits->getNext(0);
      }
      break;
    }
  case RS_ResolveSpells:
    {
      ResolveSpellsState oldstate = m_pSpellsSolver->getState();
      m_pSpellsSolver->Update(delta);
      if (m_pSpellsSolver->getState() == RSS_NotResolving)
        nextPhase(oldstate);
      break;
    }
  case RS_ResolveMoves:
    {
      if (!resolvePlayerMoves())
        nextPhase();
      break;
    }
  case RS_FindBattles:
    {
      if (!findPlayerBattles())
        nextPhase();
      else if (m_pCurrentPlayer != m_pNeutralPlayer)  // If it's neutral player, then we skip "RS_ResolveBattle" step (battles are quickly resolved)
        m_ResolveState = RS_ResolveBattle;
      break;
    }
  case RS_UpdateTowns:
    {
      updateTowns();
      updateTilesInfluence();
      nextPhase();
      break;
    }
  case RS_EndResolve:
    nextPhase();
    break;
  default:
    break;
  }
}

// -----------------------------------------------------------------
// Name : nextPhase
// -----------------------------------------------------------------
void TurnSolver::nextPhase(ResolveSpellsState oldstate)
{
  switch (m_ResolveState)
  {
  case RS_NotResolving:
    {
      // Let the clients know that resolve phase begins
      NetworkData data(NETWORKMSG_RESOLVE_PHASE_BEGINS);
      m_pServer->sendMessageToAllClients(&data);
      NetworkData data2(NETWORKMSG_PROCESS_AI);
      m_pServer->sendMessageToAllClients(&data2);
      m_ResolveState = RS_ResolveAI;
      break;
    }
  case RS_ResolveAI:
    {
      // Let the clients know that resolve phase begins
      NetworkData data(NETWORKMSG_RESOLVE_NEUTRAL_AI);
      m_pServer->sendMessageToAllClients(&data);
      m_pNeutralPlayer->m_pUnits->getFirst(0);
      m_ResolveState = RS_ResolveNeutral;
      break;
    }
  case RS_ResolveNeutral:
    {
      m_ResolveState = RS_ResolveEOT;
      callNewTurnHandlers(1);
      // Start spells solver
      m_pSpellsSolver->startResolveSpells(RSS_TurnSpells);
      m_ResolveState = RS_ResolveSpells;
      break;
    }
  case RS_ResolveSpells:
    {
      switch (oldstate)
      {
      case RSS_TurnSpells:
        {
          callNewTurnHandlers(2);
          m_pCurrentPlayer = m_pNeutralPlayer;
          m_pCurrentPlayer->m_pUnits->getFirst(0);
          NetworkData data(NETWORKMSG_RESOLVING_MOVE_ORDERS);
          data.addLong((long)m_pCurrentPlayer->m_uPlayerId);
          m_pServer->sendMessageToAllClients(&data);
          m_ResolveState = RS_ResolveMoves;
          break;
        }
      case RSS_BattleSpells:
        {
          resolveBattle();
          m_ResolveState = RS_FindBattles;
          break;
        }
      case RSS_PostBattleSpells:
        {
          m_ResolveState = RS_UpdateTowns;
          break;
        }
      case RSS_ChildEffect:
        {
          m_ResolveState = RS_ResolveMoves;
          break;
        }
        default:
        break;
      }
      break;
    }
  case RS_ResolveMoves:
    {
      checkAllUnitUpdates(false);
      m_ResolveState = RS_FindBattles;
      break;
    }
  case RS_FindBattles:
  case RS_ResolveBattle:
    {
      // next player
      Player * pFirstPlayer = (Player*) (*m_pPlayersList)[m_uFirstResolutionListIdx];
      if (m_pCurrentPlayer == m_pNeutralPlayer)
      {
        m_pCurrentPlayer = (Player*) m_pPlayersList->goTo(m_iPlayerIt, m_uFirstResolutionListIdx);
        pFirstPlayer = NULL;
      }
      else
        m_pCurrentPlayer = (Player*) m_pPlayersList->getNext(m_iPlayerIt, true);
      if (m_pCurrentPlayer == pFirstPlayer)
      {
        allowCastSpells(true);
        m_ResolveState = RS_ResolveSpells;
      }
      else
      {
        m_pCurrentPlayer->m_pUnits->getFirst(0);
        NetworkData data(NETWORKMSG_RESOLVING_MOVE_ORDERS);
        data.addLong((long)m_pCurrentPlayer->m_uPlayerId);
        m_pServer->sendMessageToAllClients(&data);
        m_ResolveState = RS_ResolveMoves;
      }
      break;
    }
  case RS_UpdateTowns:
    {
      m_ResolveState = RS_EndResolve;
      break;
    }
  case RS_EndResolve:
    {
      // Resolution phase finished
      resetDataForNextTurn(false);
      if (!m_pServer->isGameOver())
      {
        NetworkData data(NETWORKMSG_RESOLVE_PHASE_ENDS);
        m_uFirstResolutionListIdx++;
        m_uFirstResolutionListIdx %= m_uNbHumanPlayers;
        data.addLong((long)m_uFirstResolutionListIdx);
        m_pServer->sendMessageToAllClients(&data);
      }
      m_ResolveState = RS_NotResolving;
      break;
    }
  default:
    break;
  }
}

// -----------------------------------------------------------------
// Name : resolvePlayerMoves
//  return false if last unit reached
// -----------------------------------------------------------------
bool TurnSolver::resolvePlayerMoves()
{
  assert(m_pCurrentPlayer != NULL);
  m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getCurrent(0);
  if (m_pCurrentUnit == NULL)
    return false;

  m_pCurrentUnit->setHasAttacked(false);
  switch (m_pCurrentUnit->getOrder())
  {
  case OrderMove:
  case OrderAttack:
    resolveUnitMove();
    break;
  case OrderSkill:
    {
      m_pCurrentUnit->unsetOrder();
      m_pCurrentUnit->setHasAttacked(true);
      ChildEffect * pSkill = m_pCurrentUnit->getSkillOrder();
      // pSkill can be null for instance if the skill was in the same turn set as inactive
      if (pSkill != NULL)
      {
        m_pSpellsSolver->resolveChildEffect(m_pCurrentPlayer, m_pCurrentUnit, pSkill);
        if (m_pSpellsSolver->getState() != RSS_NotResolving)
          m_ResolveState = RS_ResolveSpells;
      }
      break;
    }
  default:
    break;
  }
  m_pCurrentPlayer->m_pUnits->getNext(0);
  return true;
}

// -----------------------------------------------------------------
// Name : findDefenders
// -----------------------------------------------------------------
bool TurnSolver::findDefenders(MapTile * pTile, Unit * pAttacker, bool bRange, ObjectList * pDefenders)
{
  bool bFound = false;
  int iRange = bRange ? 1 : 0;
  int _it = pTile->m_pMapObjects->getIterator();
  Unit * pOther = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT, _it);
  while (pOther != NULL)
  {
    if (pOther->getOwner() != pAttacker->getOwner() && pOther->canDefend())
    {
      int bAttacker = 1;
      void * pArgs[3];
      pArgs[0] = &bAttacker;
      pArgs[1] = &iRange;
      pArgs[2] = pOther->getIdentifiers();
      bool bOk = (pAttacker->callEffectHandler(L"isBattleValid", L"iis", pArgs, HANDLER_RESULT_TYPE_BAND) == 1);
      if (bOk)
      {
        bAttacker = 0;
        pArgs[2] = pAttacker->getIdentifiers();
        bOk = (pOther->callEffectHandler(L"isBattleValid", L"iis", pArgs, HANDLER_RESULT_TYPE_BAND) == 1);
        if (bOk)
        {
          bFound = true;
          if (pDefenders != NULL)
            pDefenders->addLast(pOther);
        }
      }
    }
    pOther = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT, _it);
  }
  pTile->m_pMapObjects->releaseIterator(_it);
  return bFound;
}

// -----------------------------------------------------------------
// Name : findPlayerBattles
// -----------------------------------------------------------------
bool TurnSolver::findPlayerBattles()
{
  assert(m_pCurrentPlayer != NULL);
  NetworkData data1(NETWORKMSG_RESOLVE_SELECT_BATTLE);
  data1.addLong((long) m_pCurrentPlayer->m_uPlayerId);
  bool bPotentialBattlesFound = false;
  for (int x = 0; x < m_pServer->getMap()->getWidth(); x++)
  {
    for (int y = 0; y < m_pServer->getMap()->getHeight(); y++)
    {
      // Find tiles where units are in presence with foe units
      MapTile * pTile = m_pServer->getMap()->getTileAt(CoordsMap(x,y));
      Unit * pUnit = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT);
      while (pUnit != NULL)
      {
        if (pUnit->getOwner() == m_pCurrentPlayer->m_uPlayerId && pUnit->canAttack())
        {
          if (findDefenders(pTile, pUnit, false, NULL))
          {
            data1.addLong((long)x);
            data1.addLong((long)y);
            bPotentialBattlesFound = true;
            break;
          }
          if (pUnit->getValue(STRING_RANGE) > 0 && findDefenders(pTile, pUnit, true, NULL))
          {
            data1.addLong((long)x);
            data1.addLong((long)y);
            bPotentialBattlesFound = true;
            break;
          }
        }
        pUnit = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
      }
    }
  }
  if (bPotentialBattlesFound)
  {
    if (m_pCurrentPlayer != m_pNeutralPlayer)
    {
      // Send network message so that attacking player choose a battle
      m_pServer->sendMessageTo(m_pCurrentPlayer->m_uClientId, &data1);
      NetworkData data2(NETWORKMSG_RESOLVE_OTHER_PLAYER_SELECTS_BATTLE);
      data2.addLong((long) m_pCurrentPlayer->m_uPlayerId);
      m_pServer->sendMessageToAllExcept(m_pCurrentPlayer->m_uClientId, &data2);
      return true;
    }
    else
    {
      // Neutral AI: resolve first available battle
      m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(0);
      while (m_pCurrentUnit != NULL)
      {
        if (m_pCurrentUnit->getOrder() == OrderAttack && m_pCurrentUnit->canAttack())
        {
          // Attack
          m_pDefendingUnit = m_pCurrentUnit->getAttackTarget();
          if (m_pDefendingUnit != NULL && m_pDefendingUnit->getStatus() == US_Normal) {
            // Function is only called to know if it's range or melee attack
            m_pAISolver->getOpponentInterest(m_pCurrentUnit, m_pDefendingUnit, &m_bIsRangeAttack);
            allowCastSpells(false);
            m_ResolveState = RS_ResolveSpells;
            return true;
          }
        }
        m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(0);
      }
    }
  }
  return false;
}

// -----------------------------------------------------------------
// Name : playerSelectedBattle
// -----------------------------------------------------------------
void TurnSolver::playerSelectedBattle(NetworkData * pData)
{
  CoordsMap mp;
  mp.x = (int) pData->readLong();
  mp.y = (int) pData->readLong();
  NetworkData data1(NETWORKMSG_SET_RESOLVE_DIALOG_UNITS);
  MapTile * pTile = m_pServer->getMap()->getTileAt(mp);

  // Find all units on that tile
  Unit * pUnit = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT);
  while (pUnit != NULL)
  {
    if (pUnit->getOwner() == m_pCurrentPlayer->m_uPlayerId && pUnit->canAttack())
    {
      ObjectList defenders(false);
      if (findDefenders(pTile, pUnit, false, &defenders))
      {
        data1.addLong((long) pUnit->getOwner());
        data1.addLong((long) pUnit->getId());
        data1.addLong(0); // for melee
        data1.addLong(defenders.size);
        Unit * pOther = (Unit*) defenders.getFirst(0);
        while (pOther != NULL)
        {
          data1.addLong((long) pOther->getOwner());
          data1.addLong((long) pOther->getId());
          pOther = (Unit*) defenders.getNext(0);
        }
      }
      if (pUnit->getValue(STRING_RANGE) > 0 && findDefenders(pTile, pUnit, true, &defenders))
      {
        data1.addLong((long) pUnit->getOwner());
        data1.addLong((long) pUnit->getId());
        data1.addLong(1); // for range
        data1.addLong(defenders.size);
        Unit * pOther = (Unit*) defenders.getFirst(0);
        while (pOther != NULL)
        {
          data1.addLong((long) pOther->getOwner());
          data1.addLong((long) pOther->getId());
          pOther = (Unit*) defenders.getNext(0);
        }
      }
    }
    pUnit = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
  }
  m_pServer->sendMessageTo(m_pCurrentPlayer->m_uClientId, &data1);
}

// -----------------------------------------------------------------
// Name : attackerChoosedUnits
// -----------------------------------------------------------------
void TurnSolver::attackerChoosedUnits(NetworkData * pData)
{
  m_pCurrentUnit = (Unit*) m_pCurrentPlayer->findUnit((u32) pData->readLong());
  m_bIsRangeAttack = (pData->readLong() == 1); // range attack
  Player * pDefender = findPlayer((u8) pData->readLong());
  m_pDefendingUnit = NULL;
  if (pDefender != NULL)
    m_pDefendingUnit = pDefender->findUnit((u32) pData->readLong());

  if (m_pCurrentUnit == NULL || m_pDefendingUnit == NULL)
  {
    m_pServer->getDebug()->notifyErrorMessage(L"Error during resolve battle phase: wrong data sent to server, attacking or defending unit not found.");
    return;
  }
  allowCastSpells(false);
  m_ResolveState = RS_ResolveSpells;
}

// -----------------------------------------------------------------
// Name : allowCastSpells
// -----------------------------------------------------------------
void TurnSolver::allowCastSpells(bool bPostBattle)
{
  NetworkData msg(bPostBattle ? NETWORKMSG_RESOLVE_START_CAST_POST_BATTLE_SPELL : NETWORKMSG_RESOLVE_START_CAST_BATTLE_SPELL);
  if (bPostBattle)
  {
    m_pServer->sendMessageToAllClients(&msg);
    m_pSpellsSolver->waitForInstantSpells(RSS_PostBattleSpells);
  }
  else
  {
    msg.addLong((long)m_pCurrentUnit->getOwner());
    msg.addLong((long)m_pCurrentUnit->getId());
    msg.addLong(m_bIsRangeAttack?1:0);
    msg.addLong((long)m_pDefendingUnit->getOwner());
    msg.addLong((long)m_pDefendingUnit->getId());
    m_pServer->sendMessageToAllClients(&msg);
    m_pSpellsSolver->waitForInstantSpells(RSS_BattleSpells);
  }
}

// -----------------------------------------------------------------
// Name : resolveUnitMove
// -----------------------------------------------------------------
void TurnSolver::resolveUnitMove()
{
  // Reset moves left and calculate again, on the server side, to avoid cheats and to make sure the move hasn't been obstructed by another order.
  bool bPath = m_pCurrentUnit->recomputePath();
  if (bPath) // Ensure the move is still valid
  {
    CoordsMap newpos = m_pCurrentUnit->getPathTurnPosition();
    m_pCurrentUnit->setMapPos(newpos);
    if (m_pCurrentUnit->getOrder() == OrderMove && m_pCurrentUnit->getDestination() == newpos)
      m_pCurrentUnit->resetMoveData();
  }
  else
    m_pCurrentUnit->resetMoveData();
}

// -----------------------------------------------------------------
// Name : resolveBattle
// -----------------------------------------------------------------
void TurnSolver::resolveBattle()
{
  // Units may have been killed by spells
  if (m_pCurrentUnit->getStatus() == US_Normal && m_pDefendingUnit->getStatus() == US_Normal)
  {
    m_bUnitHasAttacked = true;
    if (m_bIsRangeAttack)
    {
      m_iAttackerDamages = m_pCurrentUnit->getValue(STRING_RANGE);
      m_iDefenderDamages = 0;
      m_iAttackerArmor = m_pCurrentUnit->getValue(STRING_ARMOR);
      m_iDefenderArmor = m_pDefendingUnit->getValue(STRING_ARMOR);
      m_pCurrentUnit->callEffectHandler(L"onRangeAttack");
      m_pDefendingUnit->callEffectHandler(L"onRangeDefend");
    }
    else
    {
      m_iAttackerDamages = m_pCurrentUnit->getValue(STRING_MELEE);
      m_iDefenderDamages = m_pDefendingUnit->getValue(STRING_MELEE);
      m_iAttackerArmor = m_pCurrentUnit->getValue(STRING_ARMOR);
      m_iDefenderArmor = m_pDefendingUnit->getValue(STRING_ARMOR);
      m_pCurrentUnit->callEffectHandler(L"onMeleeAttack");
      m_pDefendingUnit->callEffectHandler(L"onMeleeDefend");
    }
    m_iAttackerDamages = max(0, m_iAttackerDamages - m_iDefenderArmor);
    m_iDefenderDamages = max(0, m_iDefenderDamages - m_iAttackerArmor);
    m_iAttackerLife = m_pCurrentUnit->getValue(STRING_LIFE);
    m_iDefenderLife = m_pDefendingUnit->getValue(STRING_LIFE);
    m_pCurrentUnit->callEffectHandler(L"onBattleResolved");
    m_pDefendingUnit->callEffectHandler(L"onBattleResolved");
    m_iAttackerLife -= m_iDefenderDamages;
    m_iDefenderLife -= m_iAttackerDamages;
    m_pCurrentUnit->setBaseValue(STRING_LIFE, m_iAttackerLife);
    if (m_iAttackerLife <= 0)
      m_pCurrentUnit->setStatus(US_Dead);
    m_pDefendingUnit->setBaseValue(STRING_LIFE, m_iDefenderLife);
    if (m_iDefenderLife <= 0)
      m_pDefendingUnit->setStatus(US_Dead);
    m_pCurrentUnit->setHasAttacked(m_bUnitHasAttacked);
    m_pServer->sendCustomLogToAll(L"(1s)_ATTACKED_(2s)", 0, L"pp", m_pCurrentUnit->getOwner(), m_pDefendingUnit->getOwner());
  }
  checkAllUnitUpdates(true);
}

// -----------------------------------------------------------------
// Name : updateTowns
// -----------------------------------------------------------------
void TurnSolver::updateTowns()
{
  // Inform clients we start update towns
  NetworkData msg(NETWORKMSG_RESOLVE_DIALOG_UPDATE_TOWNS);
  m_pServer->sendMessageToAllClients(&msg);

  NetworkData data(NETWORKMSG_SEND_TOWNS_DATA);
  Town * pTown = m_pServer->getMap()->getFirstTown();
  while (pTown != NULL)
  {
    // Calculate owner
    u8 newOwner = pTown->getOwner();
    MapTile * pTile = m_pServer->getMap()->getTileAt(pTown->getMapPos());
    Unit * pUnit = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT);
    if (pUnit != NULL)
    {
      newOwner = pUnit->getOwner();
      pUnit = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
      while (pUnit != NULL)
      {
        if (newOwner != pUnit->getOwner())
        {
          // There's at least 2 different factions on the tile, so we don't change town owner
          newOwner = pTown->getOwner();
          break;
        }
        pUnit = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
      }
    }
    Player * pOwner = findPlayer(newOwner);

    pTown->newTurn(pOwner, m_pServer);
    data.addLong(pTown->getId());
    pTown->serializeForUpdate(&data);
    pTown = m_pServer->getMap()->getNextTown();
  }
  m_pServer->sendMessageToAllClients(&data);
}

// -----------------------------------------------------------------
// Name : drawInitialSpells
// -----------------------------------------------------------------
void TurnSolver::drawInitialSpells()
{
  Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
  while (pPlayer != NULL)
  {
    int iSpell = pPlayer->getValue(STRING_MAXSPELLS);
    pPlayer->shuffleDeck();
    int iMax = m_pServer->getMaxDeckSize();
    if (iMax > 0 && pPlayer->m_pDeck->size > iMax)
    {
      while (pPlayer->m_pDeck->size > iMax)
      {
        pPlayer->m_pDeck->getFirst(0);
        pPlayer->m_pDeck->deleteCurrent(0, true);
      }
    }
    Spell * pSpell = (Spell*) pPlayer->m_pDeck->getFirst(0);
    while (pSpell != NULL)
    {
      if (iSpell-- == 0)
        break;
      pPlayer->m_pHand->addLast(pSpell);
      pSpell = (Spell*) pPlayer->m_pDeck->deleteCurrent(0, true, true);
    }
    pPlayer = (Player*) m_pPlayersList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : drawPlayerSpells
// -----------------------------------------------------------------
void TurnSolver::drawPlayerSpells()
{
  int i = 0;
  bool bDrawn = false;
  NetworkData msg(NETWORKMSG_DRAW_SPELLS);
  msg.addLong((long) m_pCurrentPlayer->m_uPlayerId);
  Spell * pSpell = (Spell*) m_pCurrentPlayer->m_pDeck->getFirst(0);
  while (pSpell != NULL)
  {
    if (++i > m_pCurrentPlayer->getValue(STRING_NBDRAWN) || m_pCurrentPlayer->m_pHand->size >= m_pCurrentPlayer->getValue(STRING_MAXSPELLS))
      break;
    bDrawn = true;
    msg.addLong((long) pSpell->getInstanceId());
    m_pCurrentPlayer->m_pHand->addLast(pSpell);
    pSpell = (Spell*) m_pCurrentPlayer->m_pDeck->deleteCurrent(0, true, true);
  }
  if (bDrawn)
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : resetDataForNextTurn
// -----------------------------------------------------------------
void TurnSolver::resetDataForNextTurn(bool bFirstTurn)
{
  // Remove any dead unit
  NetworkData msgdead(NETWORKMSG_DEAD_UNITS);
  bool bDataDead = false;
  ObjectList * pTiePlayers = new ObjectList(false);
  m_pCurrentPlayer = getFirstPlayerAndNeutral(0);
  while (m_pCurrentPlayer != NULL)
  {
    // Check if avatar is dead
    if (m_pCurrentPlayer->getAvatar() && m_pCurrentPlayer->getAvatar()->getStatus() != US_Normal)
    {
      Player * pDead = m_pCurrentPlayer;
      m_pCurrentPlayer = getNextPlayerAndNeutral(0);
      m_pPlayersList->deleteObject(pDead, true, true);
      pTiePlayers->addLast(pDead);
      removePlayer(pDead);
    }
    else
    {
      // Other units
      Unit * pUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(0);
      while (pUnit != NULL)
      {
        if (pUnit->getStatus() != US_Normal)
        {
          bDataDead = true;
          // Remove any attached spell
          //removeUnitEffects(pUnit); => no: when an unit  dies, it keeps its spells on it
          msgdead.addLong((long)m_pCurrentPlayer->m_uPlayerId);
          msgdead.addLong((long)pUnit->getId());
          pUnit->serializeForUpdate(&msgdead);
          m_pCurrentPlayer->m_pDeadUnits->addLast(pUnit);
          pUnit = (Unit*) m_pCurrentPlayer->m_pUnits->deleteCurrent(0, true, true);
        }
        else
          pUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(0);
      }
      m_pCurrentPlayer = getNextPlayerAndNeutral(0);
    }
  }
  if (m_uNbHumanPlayers <= 1)    // End game
  {
    if (m_uNbHumanPlayers == 1)  // 1 winner
      m_pServer->gameOver(NULL);
    else                    // tie
      m_pServer->gameOver(pTiePlayers);
    delete pTiePlayers;
    return;
  }
  delete pTiePlayers;

  // Reset unit effects (must be done before lua "onNewTurn") and mana
  m_pCurrentPlayer = getFirstPlayerAndNeutral(0);
  while (m_pCurrentPlayer != NULL)
  {
    m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(0);
    while (m_pCurrentUnit != NULL)
    {
      m_pCurrentUnit->m_bNewTurnDone = false;
      m_pCurrentUnit->enableAllEffects();
      m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(0);
    }
    m_pCurrentPlayer->setBaseMana(Mana());
//    m_pCurrentPlayer->m_ManaMax.reset();
    m_pCurrentPlayer = getNextPlayerAndNeutral(0);
  }
  NetworkData msgreset(NETWORKMSG_ENABLE_ALL_EFFECTS);
  m_pServer->sendMessageToAllClients(&msgreset);

  callNewTurnHandlers(0);

  //// Also take mana from temples under player's influence
  //Temple * pTemple = (Temple*) m_pServer->getMap()->getFirstTemple();
  //while (pTemple != NULL)
  //{
  //  u8 uPlayer = m_pServer->getMap()->getTileAt(pTemple->getMapPos())->m_uInfluence;
  //  if (uPlayer > 0)
  //  {
  //    Player * pPlayer = findPlayer(uPlayer);
  //    //if (pPlayer != NULL)
  //    //  pPlayer->m_ManaMax.mana[pTemple->getValue(STRING_MANATYPE)] += pTemple->getValue(STRING_AMOUNT);
  //  }
  //  pTemple = (Temple*) m_pServer->getMap()->getNextTemple();
  //}

  // Regenerate mana (+1 point to type where (max-current) is highest)
  m_pCurrentPlayer = getFirstPlayerAndNeutral(0);
  while (m_pCurrentPlayer != NULL)
  {
    int iColor = -1;
    int maxDiff = 0;
    for (int i = 0; i < 4; i++) {
      if (bFirstTurn) {
        // First turn => init mana to 0 (ie. spent mana to total mana)
        m_pCurrentPlayer->m_SpentMana.mana[i] = m_pCurrentPlayer->getMana(i);
      }
      if (m_pCurrentPlayer->m_SpentMana[i] < 0)
        m_pCurrentPlayer->m_SpentMana.mana[i] = 0;
      else if (m_pCurrentPlayer->m_SpentMana[i] > maxDiff) {
        iColor = i;
        maxDiff = m_pCurrentPlayer->m_SpentMana[i];
      }
    }
    if (iColor >= 0) {
      m_pCurrentPlayer->m_SpentMana.mana[iColor]--;
    }
    m_pCurrentPlayer = getNextPlayerAndNeutral(0);
  }

  // Send data over network
  m_pCurrentPlayer = getFirstPlayerAndNeutral(0);
  while (m_pCurrentPlayer != NULL)
  {
    m_pCurrentPlayer->setState(waiting);
    NetworkData msgplayer(NETWORKMSG_UPDATE_PLAYER);
    msgplayer.addLong(m_pCurrentPlayer->m_uPlayerId);
    m_pCurrentPlayer->serialize(&msgplayer, true);
    m_pServer->sendMessageToAllClients(&msgplayer);

    if (m_pCurrentPlayer->m_uPlayerId != 0)
      drawPlayerSpells();
    m_pCurrentPlayer = getNextPlayerAndNeutral(0);
  }

  if (bDataDead)
    m_pServer->sendMessageToAllClients(&msgdead);

  callNewTurnHandlers(3);
}

// -----------------------------------------------------------------
// Name : callNewTurnHandlers
// -----------------------------------------------------------------
void TurnSolver::callNewTurnHandlers(u8 uStep)
{
  wchar_t sFuncTpl[64];
  wchar_t sFunc[64];
  if (uStep == 0)
    wsafecpy(sFuncTpl, 64, L"onNew%sTurn");
  else
    swprintf(sFuncTpl, 64, L"onNew%sTurn_step%d", L"%s", (int)uStep);
  // Loop through global spells to trigger any effect
  swprintf(sFunc, 64, sFuncTpl, L"");
  int sit = m_pGlobalSpells->getIterator();
  LuaObject * pLua = (LuaObject*) m_pGlobalSpells->getFirst(sit);
  while (pLua != NULL)
  {
    pLua->callLuaFunction(sFunc, 0, L"");
    pLua = (LuaObject*) m_pGlobalSpells->getNext(sit);
  }
  m_pGlobalSpells->releaseIterator(sit);

  // Tiles
  swprintf(sFunc, 64, sFuncTpl, L"Tile");
  int iWidth = m_pServer->getMap()->getWidth();
  int iHeight = m_pServer->getMap()->getHeight();
  for (int x = 0; x < iWidth; x++) {
    for (int y = 0; y < iHeight; y++) {
      MapTile * pTile = m_pServer->getMap()->getTileAt(CoordsMap(x, y));
      pTile->callEffectHandler(sFunc);
    }
  }

  int pit = m_pPlayersList->getIterator();
  m_pCurrentPlayer = getFirstPlayerAndNeutral(pit);
  while (m_pCurrentPlayer != NULL)
  {
    // Loop through units to reset flag
    int uit = m_pCurrentPlayer->m_pUnits->getIterator();
    m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(uit);
    while (m_pCurrentUnit != NULL)
    {
      m_pCurrentUnit->m_bNewTurnDone = false;
      m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(uit);
    }
    m_pCurrentPlayer->m_pUnits->releaseIterator(uit);
    m_pCurrentPlayer = getNextPlayerAndNeutral(pit);
  }

  // Loop through players and units to call new turn handlers
  m_pCurrentPlayer = getFirstPlayerAndNeutral(pit);
  while (m_pCurrentPlayer != NULL)
  {
    // Call new turn effects
    swprintf(sFunc, 64, sFuncTpl, L"Player");
    m_pCurrentPlayer->callEffectHandler(sFunc);
    // Loop through units attached effects to trigger any effect
    swprintf(sFunc, 64, sFuncTpl, L"Unit");
    int uit = m_pCurrentPlayer->m_pUnits->getIterator();
    m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getFirst(uit);
    while (m_pCurrentUnit != NULL)
    {
      if (!m_pCurrentUnit->m_bNewTurnDone)  // Note: when the unit's owner changes, the unit might do its "new turn" twice! so prevent it
      {
        m_pCurrentUnit->callEffectHandler(sFunc);
        if (uStep == 0)
          m_pCurrentUnit->onNewTurn();
      }
      m_pCurrentUnit = (Unit*) m_pCurrentPlayer->m_pUnits->getNext(uit);
    }
    m_pCurrentPlayer->m_pUnits->releaseIterator(uit);
    m_pCurrentPlayer = getNextPlayerAndNeutral(pit);
  }
  m_pPlayersList->releaseIterator(pit);
  checkAllUnitUpdates(true);
}

// -----------------------------------------------------------------
// Name : checkAllUnitUpdates
// -----------------------------------------------------------------
void TurnSolver::checkAllUnitUpdates(bool bUnsetModified)
{
  NetworkData msg(NETWORKMSG_SEND_UNIT_DATA);
  bool bData = false;
  int pit = m_pPlayersList->getIterator();
  Player * pPlayer = getFirstPlayerAndNeutral(pit);
  while (pPlayer != NULL)
  {
    int uit = pPlayer->m_pUnits->getIterator();
    Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(uit);
    while (pUnit != NULL)
    {
      if (pUnit->wasModified())
      {
        bData = true;
        msg.addLong(pUnit->getOwner());
        msg.addLong(pUnit->getId());
        pUnit->serializeForUpdate(&msg);
        // Commented out "setModified". OK, unit information will be sent twice. But else we had problems regarding the path not correctly being updated for a unit that has an attack target which moved.
        if (bUnsetModified)
          pUnit->setModified(false);
      }
      pUnit = (Unit*) pPlayer->m_pUnits->getNext(uit);
    }
    pPlayer->m_pUnits->releaseIterator(uit);
    pPlayer = getNextPlayerAndNeutral(pit);
  }
  m_pPlayersList->releaseIterator(pit);
  if (bData)
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : addUnitToPlayer
// -----------------------------------------------------------------
Unit * TurnSolver::addUnitToPlayer(wchar_t * sEdition, wchar_t * sUnitId, u8 uPlayerId, CoordsMap mapPos, bool bSimulate)
{
  UnitData * pData = m_pServer->getFactory()->getUnitData(sEdition, sUnitId);
  if (pData != NULL)
  {
    Unit * unit = new Unit(mapPos, m_pServer->getMap(), getGlobalSpellsPtr());
    Player * pPlayer = findPlayer(uPlayerId);
    assert(pPlayer != NULL);
    if (bSimulate) {
      unit->init(m_uUnitsCount+1, uPlayerId, pData, m_pServer->getDebug());
      return unit;
    }
    pPlayer->m_pUnits->addFirst(unit);
    unit->init(++m_uUnitsCount, uPlayerId, pData, m_pServer->getDebug());
    return unit;
  }
  // UNIT NOT FOUND
  return NULL;
}

// -----------------------------------------------------------------
// Name : removePlayer
// -----------------------------------------------------------------
void TurnSolver::removePlayer(Player * pDead)
{
  pDead->setState(dead);
  m_pDeadPlayers->addLast(pDead);
  m_uNbHumanPlayers--;
  // Discard all active spells
  NetworkData msgdiscard(NETWORKMSG_DISCARD_SPELLS);
  msgdiscard.addLong((long) pDead->m_uPlayerId);
  Spell * pSpell = (Spell*) pDead->m_pActiveSpells->getFirst(0);
  while (pSpell != NULL)
  {
    // send message to clients to move spell from active to discard
    msgdiscard.addLong(2);  // to say it's in active spells list
    msgdiscard.addLong((long) pSpell->getInstanceId());
    // Delete from all spell's targets
    pSpell->removeFromTargets();
    // Remove from global if necessary
    if (pSpell->isGlobal())
      m_pGlobalSpells->deleteObject(pSpell, false);
    // Move from active to discard
    pDead->m_pDiscard->addLast(pSpell);
    pSpell = (Spell*) pDead->m_pActiveSpells->deleteCurrent(0, true, true);
  }
  m_pServer->sendMessageToAllClients(&msgdiscard);
  // Discard all player's unit attached spells
  //   === > No, when an unit dies it keeps its spells on it
  //Unit * pUnit = (Unit*) pDead->m_pUnits->getFirst(0);
  //while (pUnit != NULL)
  //{
  //  removeUnitEffects(pUnit);
  //  pUnit = (Unit*) pDead->m_pUnits->getNext(0);
  //}
  // Remove town ownership
  Town * pTown = m_pServer->getMap()->getFirstTown();
  while (pTown != NULL)
  {
    if (pTown->getOwner() == pDead->m_uPlayerId)
      pTown->setOwner(0);
    pTown = m_pServer->getMap()->getNextTown();
  }
  NetworkData msg(NETWORKMSG_PLAYER_STATE);
  msg.addLong((long)pDead->m_uPlayerId);
  msg.addLong((long)dead);
  m_pServer->sendMessageToAllClients(&msg);
}

//// -----------------------------------------------------------------
//// Name : removeUnitEffects
//// -----------------------------------------------------------------
//void TurnSolver::removeUnitEffects(Unit * pUnit)
//{
//  bool bDataRemoved = false;
//  NetworkData msgeffects(NETWORKMSG_REMOVE_ACTIVE_EFFECTS);
//  msgeffects.addLong(GOTYPE_UNIT);
//  msgeffects.addLong(pUnit->getOwner());
//  msgeffects.addLong(pUnit->getId());
//  // Remove any attached spell
//  LuaObject * pLua = pUnit->getFirstEffect(0);
//  while (pLua != NULL)
//  {
//    if (pLua->getType() == LUAOBJECT_SPELL)
//    {
//      Spell * pSpell = (Spell*) pLua;
//      // Remove target from spell
//      pSpell->getTargets()->deleteObject(pUnit, true);
//      // Remove effect from unit
//      pLua = (LuaObject*) pUnit->getAllEffects()->deleteCurrent(0, true);
//      bDataRemoved = true;
//      msgeffects.addLong((long) pSpell->getPlayerId());
//      msgeffects.addLong((long) pSpell->getInstanceId());
//      // If removed spell has no other target, discard it
//      if (pSpell->getTargets()->size == 0)
//      {
//        Player * pSpellOwner = findPlayer(pSpell->getPlayerId());
//        assert(pSpellOwner != NULL);
//        pSpellOwner->m_pActiveSpells->deleteObject(pSpell, true, true);
//        pSpellOwner->m_pDiscard->addLast(pSpell);
//        // send message to clients to move spell from active to discard
//        NetworkData msgdiscard(NETWORKMSG_DISCARD_SPELLS);
//        msgdiscard.addLong((long) pSpellOwner->m_uPlayerId);
//        msgdiscard.addLong((long) pSpell->getInstanceId());
//        m_pServer->sendMessageToAllClients(&msgdiscard);
//      }
//    }
//    else
//      pLua = pUnit->getNextEffect(0);
//  }
//  if (bDataRemoved)
//    m_pServer->sendMessageToAllClients(&msgeffects);
//}

// -----------------------------------------------------------------
// Name : updateTilesInfluence
// -----------------------------------------------------------------
void TurnSolver::updateTilesInfluence()
{
  NetworkData msg(NETWORKMSG_SEND_INFLUENCE_DATA);
  for (int x = 0; x < m_pServer->getMap()->getWidth(); x++)
  {
    for (int y = 0; y < m_pServer->getMap()->getHeight(); y++)
    {
      long * pInfl = new long[m_uNbHumanPlayers + 1];
      for (u8 i = 0; i < m_uNbHumanPlayers + 1; i++)
        pInfl[i] = 0;
      // Loop through towns to get influence points
      Town * pTown = m_pServer->getMap()->getFirstTown();
      while (pTown != NULL)
      {
        pInfl[pTown->getOwner()] += pTown->getInfluenceAt(CoordsMap(x, y));
        pTown = m_pServer->getMap()->getNextTown();
      }
      // Find best influence, and update tile influence
      long iBest = pInfl[0];
      MapTile * pTile = m_pServer->getMap()->getTileAt(CoordsMap(x, y));
      pTile->m_uInfluence = 0;
      for (u8 i = 1; i < m_uNbHumanPlayers + 1; i++)
      {
        if (pInfl[i] > iBest)
        {
          iBest = pInfl[i];
          pTile->m_uInfluence = i;
        }
      }
      delete[] pInfl;
      msg.addLong((long) pTile->m_uInfluence);
    }
  }
  m_pServer->sendMessageToAllClients(&msg);
}
