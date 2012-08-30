// -----------------------------------------------------------------
// Name : PlayerManager
// -----------------------------------------------------------------
#include "PlayerManager.h"
#include "Player.h"
#include "Spell.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../Data/LuaContext.h"
#include "../DeckData/UnitData.h"
#include "../Common/ObjectList.h"
#include "../Gameboard/GameboardManager.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Building.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/SpecialTile.h"
#include "../Server/Server.h"
#include "../Interface/InterfaceManager.h"
#include "../Interface/SpellDlg.h"
#include "../Interface/LogDlg.h"
#include "../Interface/ResolveDlg.h"
#include "../Interface/UnitOptionsDlg.h"
#include "../Interface/InfoboxDlg.h"
#include "../Fx/FxManager.h"
#include "../Debug/DebugManager.h"
#include "../Audio/AudioManager.h"

// -----------------------------------------------------------------
// Name : PlayerManager
// -----------------------------------------------------------------
PlayerManager::PlayerManager(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pActiveLocalPlayer = NULL;
  m_pSpellBeingCast = NULL;
  m_pUnitActivatingSkill = NULL;
  m_pSkillBeingActivated = NULL;
  m_bCanEOT = true;
  m_fTurnTimer = 0;
}

// -----------------------------------------------------------------
// Name : ~PlayerManager
// -----------------------------------------------------------------
PlayerManager::~PlayerManager()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy PlayerManager\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy PlayerManager\n");
#endif
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void PlayerManager::Init()
{
  m_pNeutralPlayer->init(m_pLocalClient->getDisplay());
  Unit * pUnit = (Unit*) m_pNeutralPlayer->m_pUnits->getFirst(0);
  while (pUnit != NULL)
  {
    pUnit->initGraphics(m_pLocalClient->getDisplay());
    pUnit = (Unit*) m_pNeutralPlayer->m_pUnits->getNext(0);
  }

  Player * pPlayer = (Player*) m_pPlayersList->getFirst(0);
  while (pPlayer != NULL)
  {
    pPlayer->init(m_pLocalClient->getDisplay());
    Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(0);
    while (pUnit != NULL)
    {
      pUnit->initGraphics(m_pLocalClient->getDisplay());
      pUnit = (Unit*) pPlayer->m_pUnits->getNext(0);
    }
    pPlayer = (Player*) m_pPlayersList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void PlayerManager::Update(double delta)
{
  Player * player = (Player*) m_pPlayersList->getFirst(0);
  while (player != NULL)
  {
    player->update(delta);
    player = (Player*) m_pPlayersList->getNext(0);
  }
  if (m_pActiveLocalPlayer != NULL && m_fTurnTimer > 0)
  {
    int iTimer = (int) m_fTurnTimer;
    m_fTurnTimer -= delta;
    if (m_fTurnTimer <= 0) // Force end turn
    {
      if (!m_bCanEOT)
      {
        extern void clbkSelectTarget_cancelSelection(u32, int);
        clbkSelectTarget_cancelSelection(0, 0);
      }
      requestEndPlayerOrders();
    }
    else if ((int)m_fTurnTimer != iTimer)
      m_pLocalClient->getInterface()->getInfoDialog()->updatePlayersState();
  }
}

// -----------------------------------------------------------------
// Name : enableEOT
// -----------------------------------------------------------------
void PlayerManager::enableEOT(bool bEnabled)
{
  m_bCanEOT = bEnabled;
  m_pLocalClient->getInterface()->getSpellDialog()->disableEOT(!bEnabled);
}

// -----------------------------------------------------------------
// Name : createUnitData
//  Function called when a new unit is born (summoned, or produced)
// -----------------------------------------------------------------
void PlayerManager::createUnitData(NetworkData * pData)
{
  Unit * pUnit = new Unit(CoordsMap(0, 0), m_pLocalClient->getGameboard()->getMap(), getGlobalSpellsPtr());
  pUnit->deserialize(pData, m_pLocalClient, NULL);
  // Hack to properly add skills but don't want to have them doubled
  ObjectList list(false);
  Skill * pSkill = (Skill*) pUnit->getSkillsRef()->getFirst(0);
  while (pSkill != NULL)
  {
    list.addLast(pSkill);
    pSkill = (Skill*) pUnit->getSkillsRef()->deleteCurrent(0, true, true);
  }
  pSkill = (Skill*) list.getFirst(0);
  while (pSkill != NULL)
  {
    pUnit->addSkill(pSkill);
    pSkill = (Skill*) list.getNext(0);
  }
  Player * pPlayer = findPlayer(pUnit->getOwner());
  if (pPlayer != NULL)
  {
    pPlayer->m_pUnits->addLast(pUnit);
    pUnit->setPlayerColor(pPlayer->m_Color);
  }
  if (m_pLocalClient->getGameStep() == GS_InGame)
    pUnit->initGraphics(m_pLocalClient->getDisplay());
}

// -----------------------------------------------------------------
// Name : setPlayerState
// -----------------------------------------------------------------
void PlayerManager::setPlayerState(NetworkData * pData)
{
  Player * pPlayer = findPlayer((u8)pData->readLong());
  if (pPlayer != NULL)
  {
    PlayerState state = (PlayerState)pData->readLong();
    pPlayer->setState(state);
    if (state == dead)
    {
      m_pPlayersList->deleteObject(pPlayer, true, true);
      m_pDeadPlayers->addLast(pPlayer);
      wchar_t sText[256];
      void * pArgs[1];
      pArgs[0] = pPlayer->getAvatarName();
      m_pLocalClient->getInterface()->getLogDialog()->log(i18n->getText(L"PLAYER_(s1)_DEAD", sText, 256, pArgs), 2);
    }
  }
  m_pLocalClient->getInterface()->getInfoDialog()->updatePlayersState();
}

// -----------------------------------------------------------------
// Name : setPlayerMana
// -----------------------------------------------------------------
void PlayerManager::setPlayerMana(NetworkData * pData)
{
  Player * pPlayer = findPlayer((u8)pData->readLong());
  if (pPlayer != NULL)
  {
    // retrieve mana values
    for (int i = 0; i < 4; i++)
      pPlayer->m_Mana.mana[i] = (u8) pData->readLong();
  }
}

// -----------------------------------------------------------------
// Name : drawSpells
// -----------------------------------------------------------------
void PlayerManager::drawSpells(NetworkData * pData)
{
  u8 uPlayer = (u8) pData->readLong();
  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  while (pData->dataYetToRead() > 0)
  {
    u32 uSpellId = (u32) pData->readLong();
    Spell * pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pDeck);
    assert(pSpell != NULL);
    pPlayer->m_pDeck->deleteCurrent(0, true, true);
    pPlayer->m_pHand->addLast(pSpell);
  }
}

// -----------------------------------------------------------------
// Name : recallSpells
// -----------------------------------------------------------------
void PlayerManager::recallSpells(NetworkData * pData)
{
  u8 uPlayer = (u8) pData->readLong();
  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  while (pData->dataYetToRead() > 0)
  {
    wchar_t sType[64];
    pData->readString(sType);
    u32 uSpellId = (u32) pData->readLong();
    if (wcscmp(sType, L"spell_in_play") == 0)
    {
      Spell * pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pActiveSpells);
      assert(pSpell != NULL);
      // Remove from all targets
      pSpell->removeFromTargets();
      // Remove from global if necessary
      if (pSpell->isGlobal())
        m_pGlobalSpells->deleteObject(pSpell, false);
      pPlayer->m_pActiveSpells->deleteCurrent(0, true, true);
      pPlayer->m_pHand->addLast(pSpell);
    }
    else  // spell in discard
    {
      Spell * pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pDiscard);
      assert(pSpell != NULL);
      pPlayer->m_pDiscard->deleteCurrent(0, true, true);
      pPlayer->m_pHand->addLast(pSpell);
    }
  }
}

// -----------------------------------------------------------------
// Name : discardSpells
// -----------------------------------------------------------------
void PlayerManager::discardSpells(NetworkData * pData)
{
  u8 uPlayer = (u8) pData->readLong();
  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  while (pData->dataYetToRead() > 0)
  {
    u8 uSrc = (u8) pData->readLong();
    u32 uSpellId = (u32) pData->readLong();
    ObjectList * pSrc = NULL;
    switch (uSrc)
    {
    case 0:
      pSrc = pPlayer->m_pDeck;
      break;
    case 1:
      pSrc = pPlayer->m_pHand;
      break;
    case 2:
      pSrc = pPlayer->m_pActiveSpells;
      break;
    }
    assert(pSrc != NULL);
    Spell * pSpell = pPlayer->findSpell(0, uSpellId, pSrc);
    assert(pSpell != NULL);
    pSrc->deleteCurrent(0, true, true);
    pPlayer->m_pDiscard->addLast(pSpell);
    if (uSrc == 2)
    {
      // Remove from all targets
      pSpell->removeFromTargets();
      // Remove from global if necessary
      if (pSpell->isGlobal())
        m_pGlobalSpells->deleteObject(pSpell, false);
      // Remove any child effect attached
      int nbChildren = pSpell->getNbChildEffects();
      for (int i = 0; i < nbChildren; i++) {
        ChildEffect * pChild = pSpell->getChildEffect(i);
        BaseObject * pTarget = (BaseObject*) pChild->pTargets->getFirst(0);
        while (pTarget != NULL) {
          int type = pChild->pTargets->getCurrentType(0);
          if (type == LUATARGET_UNIT) {
            if (((Unit*)pTarget)->detachChildEffect(pChild))
            {
              pChild->pTargets->deleteObject(((Unit*)pTarget), true);
            }
            else
            {
              wchar_t sError[128];
              swprintf_s(sError, 128, L"Lua interaction error: can't detach effect %s.", pSpell->getLocalizedName());
              m_pLocalClient->getDebug()->notifyErrorMessage(sError);
            }
          }
          pTarget = (BaseObject*) pChild->pTargets->getNext(0);
        }
      }
    }
  }
}

//// -----------------------------------------------------------------
//// Name : removeActiveEffects
//// -----------------------------------------------------------------
//void PlayerManager::removeActiveEffects(NetworkData * pData)
//{
//  LuaTargetable * pTarget = findLuaTarget(pData);
//  while (pData->dataYetToRead() > 0)
//  {
//    u8 spellOwnerId = (u8) pData->readLong();
//    u32 spellId = (u32) pData->readLong();
//    LuaObject * pLua = pTarget->getFirstEffect(0);
//    while (pLua != NULL)
//    {
//      if (pLua->getType() == LUAOBJECT_SPELL && ((Spell*)pLua)->getPlayerId() == spellOwnerId && pLua->getInstanceId() == spellId)
//        {
//          pUnit->getAllEffects()->deleteCurrent(0, true);
//          ((Spell*)pLua)->getTargets()->deleteObject(pUnit, true);
//          break;
//        }
//        pLua = pUnit->getNextEffect(0);
//      }
//    }
//  }
//}

// -----------------------------------------------------------------
// Name : setResolutionIdx
// -----------------------------------------------------------------
void PlayerManager::setResolutionIdx(NetworkData * pData)
{
  m_uFirstResolutionListIdx = (u8) pData->readLong();
}

// -----------------------------------------------------------------
// Name : isPlayerReady
// -----------------------------------------------------------------
bool PlayerManager::isPlayerReady(u8 uPlayerId)
{
  return (m_pActiveLocalPlayer != NULL && m_pActiveLocalPlayer->m_uPlayerId == uPlayerId);
}

// -----------------------------------------------------------------
// Name : requestEndPlayerOrders
//  Called from inputs
//  Change active player state
// -----------------------------------------------------------------
bool PlayerManager::requestEndPlayerOrders()
{
  if (m_pActiveLocalPlayer != NULL && m_bCanEOT)
  {
    m_pActiveLocalPlayer->setState(finished);
    m_pLocalClient->getInterface()->getInfoDialog()->updatePlayersState();
    // Send various data to server
    // Send unit orders
    NetworkData unitmsg(NETWORKMSG_SEND_PLAYER_UNITS_ORDERS);
    unitmsg.addLong((long)m_pActiveLocalPlayer->m_uPlayerId);
    Unit * pUnit = (Unit*) m_pActiveLocalPlayer->m_pUnits->getFirst(0);
    while (pUnit != NULL)
    {
      unitmsg.addLong((long)pUnit->getId());
      unitmsg.addLong((long)pUnit->getOrder());
      switch (pUnit->getOrder())
      {
      case OrderNone:
      case OrderFortify:
        break;
      case OrderMove:
        {
          CoordsMap mp = pUnit->getDestination();
          unitmsg.addLong(mp.x);
          unitmsg.addLong(mp.y);
          break;
        }
      case OrderAttack:
        {
          unitmsg.addLong((long) pUnit->getAttackTarget()->getOwner());
          unitmsg.addLong((long) pUnit->getAttackTarget()->getId());
          break;
        }
      case OrderSkill:
        {
          ChildEffect * pEffect = pUnit->getSkillOrder();
          unitmsg.addLong((long) ((Skill*)(pEffect->getAttachment()))->getInstanceId());
          unitmsg.addLong((long) pEffect->id);
          unitmsg.addString(pEffect->sResolveParams);
          break;
        }
      }
      pUnit = (Unit*) m_pActiveLocalPlayer->m_pUnits->getNext(0);
    }
    m_pLocalClient->sendMessage(&unitmsg);
    // Send cast spells
    NetworkData spellmsg(NETWORKMSG_SEND_CAST_SPELLS_DATA);
    spellmsg.addLong((long)m_pActiveLocalPlayer->m_uPlayerId);
    m_pLocalClient->getInterface()->getSpellDialog()->getCastSpellsData(&spellmsg);
    m_pLocalClient->sendMessage(&spellmsg);
    // Send unit groups data
    NetworkData groupsmsg(NETWORKMSG_SEND_UNITS_GROUPS_DATA);
    groupsmsg.addLong((long)m_pActiveLocalPlayer->m_uPlayerId);
    MetaObjectList * pGroup = m_pLocalClient->getGameboard()->getMap()->getFirstPlayerGroup(m_pActiveLocalPlayer->m_uPlayerId);
    while (pGroup != NULL)
    {
      if (pGroup->size > 1)
      {
        groupsmsg.addLong((long) pGroup->size);
        Unit * pUnit = (Unit*) pGroup->getFirst(0);
        while (pUnit != NULL)
        {
          groupsmsg.addLong((long) pUnit->getId());
          pUnit = (Unit*) pGroup->getNext(0);
        }
      }
      pGroup = m_pLocalClient->getGameboard()->getMap()->getNextPlayerGroup(m_pActiveLocalPlayer->m_uPlayerId);
    }
    m_pLocalClient->sendMessage(&groupsmsg);
    // Send towns data
    NetworkData townsmsg(NETWORKMSG_SEND_TOWNS_ORDERS);
    Town * pTown = m_pLocalClient->getGameboard()->getMap()->getFirstTown();
    while (pTown != NULL)
    {
      pTown->getOrders(&townsmsg);
      pTown = m_pLocalClient->getGameboard()->getMap()->getNextTown();
    }
    m_pLocalClient->sendMessage(&townsmsg);
    // Send player state
    NetworkData msg(NETWORKMSG_PLAYER_STATE);
    msg.addLong(m_pActiveLocalPlayer->m_uPlayerId);
    msg.addLong((long)finished);
    m_pLocalClient->sendMessage(&msg);
    m_pActiveLocalPlayer = NULL;
    m_pLocalClient->waitLocalPlayer();
    return true;
  }
  else
    return false;
}

// -----------------------------------------------------------------
// Name : updateUnitsData
// -----------------------------------------------------------------
void PlayerManager::updateUnitsData(NetworkData * pData)
{
  ObjectList * pTempList = new ObjectList(false);
  while (pData->dataYetToRead() > 0)
  {
    u8 uPlayerId = (u8) pData->readLong();
    u32 uUnitId = (u32) pData->readLong();
    Player * pPlayer = findPlayer(uPlayerId);
    assert(pPlayer != NULL);
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    assert(pUnit != NULL);

    pUnit->unsetOrder();
//    CoordsMap oldPos = pUnit->getMapPos();
    pUnit->deserializeForUpdate(pData, m_pLocalClient);
//    if (oldPos != pUnit->getMapPos())
//    {
      //// Translate move
      //ConstantMovement3D * pMove = new ConstantMovement3D(0, 1.0f, m_pLocalClient->getDisplay()->get3DCoords(pUnit->getMapPos()) - m_pLocalClient->getDisplay()->get3DCoords(oldPos));
      //pUnit->bindMovement(pMove);
//    }

    if (pUnit->getOrder() == OrderAttack)
      pTempList->addLast(pUnit);
  }
  Unit * pUnit = (Unit*) pTempList->getFirst(0);
  while (pUnit != NULL)
  {
    pUnit->recomputePath();
    pUnit = (Unit*) pTempList->getNext(0);
  }
  delete pTempList;
}

// -----------------------------------------------------------------
// Name : updateDeadUnits
// -----------------------------------------------------------------
void PlayerManager::updateDeadUnits(NetworkData * pData)
{
  while (pData->dataYetToRead() > 0)
  {
    u8 uPlayerId = (u8) pData->readLong();
    u32 uUnitId = (u32) pData->readLong();
    Player * pPlayer = findPlayer(uPlayerId);
    assert(pPlayer != NULL);
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    assert(pUnit != NULL);

    pUnit->unsetOrder();
    pUnit->deserializeForUpdate(pData, m_pLocalClient);
    pPlayer->m_pUnits->deleteObject(pUnit, true, true);
    pPlayer->m_pDeadUnits->addLast(pUnit);

    wchar_t sBuf[512] = L"";
    void * pPhraseArgs[2];
    pPhraseArgs[0] = pUnit->getName();
    pPhraseArgs[1] = pPlayer->getAvatarName();
    m_pLocalClient->getInterface()->getLogDialog()->log(i18n->getText(L"(1s)_(2s)_DIED", sBuf, 512, pPhraseArgs), 1, LOG_ACTION_UNITSCREEN, pUnit);
  }
}

// -----------------------------------------------------------------
// Name : updateCastSpellData
// -----------------------------------------------------------------
void PlayerManager::updateCastSpellData(NetworkData * pData)
{
  bool bFinished = (pData->readLong() == 1);
  // Find player
  u8 uPlayerId = (u8) pData->readLong();
  Player * pPlayer = findPlayer(uPlayerId);
  assert(pPlayer != NULL);
  u32 uSpellId = (u32) pData->readLong();
  Spell * pSpell = NULL;
  if (!bFinished)
  {
    // Find spell in player's hand
    pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pHand);
    assert(pSpell != NULL);
    pPlayer->m_pHand->deleteCurrent(0, true, true);
    pPlayer->m_pActiveSpells->addLast(pSpell);
  }
  else
  {
    // Finished => may remove from active
    pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pActiveSpells);
    assert(pSpell != NULL);
    long dest = pData->readLong();  // destination of the spell (discarded or attached)
    if (dest == 0 || dest == 2)  // discarded / canceled
    {
      pPlayer->m_pActiveSpells->deleteCurrent(0, true, true);
      pPlayer->m_pDiscard->addLast(pSpell);
    }

    // Log
    wchar_t sText[256];
    void * pPhraseArgs[3];
    pPhraseArgs[0] = pPlayer->getAvatarName();
    pPhraseArgs[1] = pSpell->getLocalizedName();
    pPhraseArgs[2] = pSpell->getParametersInfos();
    if (dest == 2)  // Spell canceled!
      i18n->getText(L"(s1)_CAST_(s2)_FAILED", sText, 256, pPhraseArgs);
    else if (wcscmp(pSpell->getParametersInfos(), L"") != 0)
      i18n->getText(L"%$1s_CAST_%$2s_ON_%$3s", sText, 256, pPhraseArgs);
    else
      i18n->getText(L"%$1s_CAST_%$2s", sText, 256, pPhraseArgs);
    //if (pUnit != NULL)
    //  m_pLocalClient->getInterface()->getLogDialog()->addLog(sText, 1, LOG_ACTION_UNITSCREEN, pUnit);
    //else
    m_pLocalClient->getInterface()->getLogDialog()->log(sText, 1);
  }
}

// -----------------------------------------------------------------
// Name : onLuaAttached
// -----------------------------------------------------------------
void PlayerManager::onLuaAttached(NetworkData * pData)
{
  LuaContext context;
  if (!context.deserialize(pData, this, m_pLocalClient->getGameboard()->getMap()))
    return;
  LuaObject * pLua = context.pLua;
  switch (pData->readLong())
  {
  case 0:     // global spell
    {
      m_pGlobalSpells->addLast(pLua);
      if (pLua->getType() == LUAOBJECT_SPELL)
        ((Spell*)pLua)->setGlobal();
      break;
    }
  case LUATARGET_UNIT:     // target is unit
    {
      // Find objects
      u8 uTargetPlayerId = (u8) pData->readLong();
      u32 uTargetUnitId = (u32) pData->readLong();
      Player * pTargetPlayer = findPlayer(uTargetPlayerId);
      assert(pTargetPlayer != NULL);
      Unit * pUnit = pTargetPlayer->findUnit(uTargetUnitId);
      assert(pUnit != NULL);
      pUnit->attachEffect(pLua);  // do attach spell effect to targetted unit
      pLua->addTarget(pUnit, LUATARGET_UNIT);
      break;
    }
  case LUATARGET_PLAYER:     // target is player
    {
      // Find objects
      u8 uTargetPlayerId = (u8) pData->readLong();
      Player * pTargetPlayer = findPlayer(uTargetPlayerId);
      assert(pTargetPlayer != NULL);
      pTargetPlayer->attachEffect(pLua);  // do attach spell effect to targetted player
      pLua->addTarget(pTargetPlayer, LUATARGET_PLAYER);
      break;
    }
  case LUATARGET_TOWN:     // target is town
    {
      // Find objects
      u32 uTargetTownId = (u32) pData->readLong();
      Town * pTargetTown = m_pLocalClient->getGameboard()->getMap()->findTown(uTargetTownId);
      assert(pTargetTown != NULL);
      pTargetTown->attachEffect(pLua);  // do attach spell effect to targetted town
      pLua->addTarget(pTargetTown, LUATARGET_TOWN);
      break;
    }
  case LUATARGET_TEMPLE:     // target is temple
    {
      // Find objects
      u32 uTemple = (u32) pData->readLong();
      Temple * pTemple = m_pLocalClient->getGameboard()->getMap()->findTemple(uTemple);
      assert(pTemple != NULL);
      pTemple->attachEffect(pLua);  // do attach spell effect to targetted temple
      pLua->addTarget(pTemple, LUATARGET_TEMPLE);
      break;
    }
  case LUATARGET_TILE:     // target is tile
    {
      // Find tile
      int x = (int) pData->readLong();
      int y = (int) pData->readLong();
      MapTile * pTile = m_pLocalClient->getGameboard()->getMap()->getTileAt(CoordsMap(x, y));
      assert(pTile != NULL);
      pTile->attachEffect(pLua);  // do attach spell effect to targetted town
      pLua->addTarget(pTile, LUATARGET_TILE);
      break;
    }
  }
}

// -----------------------------------------------------------------
// Name : onLuaDetached
// -----------------------------------------------------------------
void PlayerManager::onLuaDetached(NetworkData * pData)
{
  LuaContext context;
  if (!context.deserialize(pData, this, m_pLocalClient->getGameboard()->getMap()))
    return;
  LuaObject * pLua = context.pLua;
  switch (pData->readLong())
  {
  case 0:     // global spell
    {
      m_pGlobalSpells->deleteObject(pLua, true);
      break;
    }
  case LUATARGET_UNIT:     // target is unit
    {
      // Find objects
      u8 uTargetPlayerId = (u8) pData->readLong();
      u32 uTargetUnitId = (u32) pData->readLong();
      Player * pTargetPlayer = findPlayer(uTargetPlayerId);
      assert(pTargetPlayer != NULL);
      Unit * pUnit = pTargetPlayer->findUnit(uTargetUnitId);
      assert(pUnit != NULL);
      pUnit->detachEffect(pLua);
      pLua->removeTarget(pUnit);
      break;
    }
  case LUATARGET_PLAYER:     // target is player
    {
      // Find objects
      u8 uTargetPlayerId = (u8) pData->readLong();
      Player * pTargetPlayer = findPlayer(uTargetPlayerId);
      assert(pTargetPlayer != NULL);
      pTargetPlayer->detachEffect(pLua);
      pLua->removeTarget(pTargetPlayer);
      break;
    }
  case LUATARGET_TOWN:     // target is town
    {
      // Find objects
      u32 uTargetTownId = (u32) pData->readLong();
      Town * pTargetTown = m_pLocalClient->getGameboard()->getMap()->findTown(uTargetTownId);
      assert(pTargetTown != NULL);
      pTargetTown->detachEffect(pLua);  // do attach spell effect to targetted town
      pLua->removeTarget(pTargetTown);
      break;
    }
  }
}

// -----------------------------------------------------------------
// Name : onChildEffectAttached
// -----------------------------------------------------------------
void PlayerManager::onChildEffectAttached(NetworkData * pData)
{
  LuaContext context;
  if (!context.deserialize(pData, this, m_pLocalClient->getGameboard()->getMap()))
    return;
  LuaObject * pLua = context.pLua;
  wchar_t sSourceName[2*NAME_MAX_CHARS] = L"";
  switch (pLua->getType())
  {
  case LUAOBJECT_SKILL:
    {
      Unit * pUnit = ((Skill*)pLua)->getCaster();
      Player * pPlayer = findPlayer(pUnit->getOwner());
      if (pUnit == pPlayer->getAvatar())
        wsafecpy(sSourceName, 2*NAME_MAX_CHARS, pUnit->getName());
      else
        swprintf_s(sSourceName, 2*NAME_MAX_CHARS, L"%s (%s)", pUnit->getName(), pPlayer->getAvatarName());
      break;
    }
  case LUAOBJECT_SPELL:
    {
      Player * pPlayer = ((Spell*)pLua)->getCaster();
      wsafecpy(sSourceName, 2*NAME_MAX_CHARS, pPlayer->getAvatarName());
      break;
    }
  }
  ChildEffect * pChild = pLua->getChildEffect(pLua->getCurrentEffect());
  u32 uTargetType;
  LuaTargetable * pTarget = findLuaTarget(pData, &uTargetType);
  pTarget->attachChildEffect(pChild);

  wchar_t sTargetName[NAME_MAX_CHARS] = L"";
  u8 uLogAction = 0;
  void * pLogParam = NULL;
  switch (uTargetType)
  {
  case LUATARGET_UNIT:
    pChild->pTargets->addLast((Unit*) pTarget, LUATARGET_UNIT);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Unit*)pTarget)->getName());
    uLogAction = LOG_ACTION_UNITSCREEN;
    pLogParam = (Unit*) pTarget;
    break;
  case LUATARGET_PLAYER:
    pChild->pTargets->addLast((Player*) pTarget, LUATARGET_PLAYER);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Player*)pTarget)->getAvatarName());
    break;
  case LUATARGET_TOWN:
    pChild->pTargets->addLast((Town*) pTarget, LUATARGET_TOWN);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Town*)pTarget)->getName());
    uLogAction = LOG_ACTION_TOWNSCREEN;
    pLogParam = (Town*) pTarget;
    break;
  case LUATARGET_TILE:
    pChild->pTargets->addLast((MapTile*) pTarget, LUATARGET_TILE);
    wsafecpy(sTargetName, NAME_MAX_CHARS, L"");
    break;
  }

  // Log
  wchar_t sText[256];
  if (wcscmp(sSourceName, L"") == 0)
  {
    void * p[2] = { pChild->sName, sTargetName };
    i18n->getText(L"%$1s_WAS_ACTIVATED_ON_%$2s", sText, 256, p);
    m_pLocalClient->getInterface()->getLogDialog()->log(sText, 0, uLogAction, pLogParam);
  }
  else
  {
    void * p[3] = { sSourceName, pChild->sName, sTargetName };
    i18n->getText(L"%$1s_ACTIVATED_%$2s_ON_%$3s", sText, 256, p);
    m_pLocalClient->getInterface()->getLogDialog()->log(sText, 1, uLogAction, pLogParam);
  }
}

// -----------------------------------------------------------------
// Name : onChildEffectDetached
// -----------------------------------------------------------------
void PlayerManager::onChildEffectDetached(NetworkData * pData)
{
  LuaContext context;
  if (!context.deserialize(pData, this, m_pLocalClient->getGameboard()->getMap()))
    return;
  LuaObject * pLua = context.pLua;
  ChildEffect * pChild = pLua->getChildEffect(pLua->getCurrentEffect());
  u32 uTargetType;
  LuaTargetable * pTarget = findLuaTarget(pData, &uTargetType);
  assert(pTarget != NULL);
  pTarget->detachChildEffect(pChild);

  wchar_t sTargetName[NAME_MAX_CHARS] = L"";
  switch (uTargetType)
  {
  case LUATARGET_UNIT:
    pChild->pTargets->deleteObject((Unit*) pTarget, true);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Unit*)pTarget)->getName());
    break;
  case LUATARGET_PLAYER:
    pChild->pTargets->deleteObject((Player*) pTarget, true);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Player*)pTarget)->getAvatarName());
    break;
  case LUATARGET_TOWN:
    pChild->pTargets->deleteObject((Town*) pTarget, true);
    wsafecpy(sTargetName, NAME_MAX_CHARS, ((Town*)pTarget)->getName());
    break;
  case LUATARGET_TILE:
    pChild->pTargets->deleteObject((MapTile*) pTarget, true);
    wsafecpy(sTargetName, NAME_MAX_CHARS, L"");
    break;
  }

  // Log
  wchar_t sText[256];
  wchar_t sBuf[128];
  i18n->getText(L"(s)_EFFECT_ON_(s)_ENDED", sBuf, 128);
  swprintf_s(sText, 256, sBuf, pChild->sName, sTargetName);
  m_pLocalClient->getInterface()->getLogDialog()->log(sText);
}

// -----------------------------------------------------------------
// Name : onCustomLuaUpdate
//  Function is called when LUA script wants to synchronize clients from server
// -----------------------------------------------------------------
void PlayerManager::onCustomLuaUpdate(NetworkData * pData)
{
  LuaContext context;
  if (!context.deserialize(pData, this, m_pLocalClient->getGameboard()->getMap()))
    return;
  LuaObject * pLua = context.pLua;

  // Get callback name
  wchar_t sCallback[NAME_MAX_CHARS];
  pData->readString(sCallback);

  // Prepare calling function
  lua_State * pState = pLua->prepareLuaFunction(sCallback);

  // Get and push params
  int iParams = 0;
  wchar_t sParams[512] = L"";
  wchar_t sBuf[128];
  while (pData->dataYetToRead() > 0)
  {
    long type = pData->readLong();
    if (type == 0)  // lua number
    {
      double val = pData->readDouble();
      lua_pushnumber(pState, val);
      swprintf_s(sBuf, 128, L"%lf,", val);
      wsafecat(sParams, 512, sBuf);
      iParams++;
    }
    else if (type == 1) // string
    {
      wchar_t wcharval[LUA_FUNCTION_PARAMS_MAX_CHARS];
      pData->readString(wcharval);
      // Convert wchar_t arguments to ASCII
      char charval[LUA_FUNCTION_PARAMS_MAX_CHARS];
      wtostr(charval, LUA_FUNCTION_PARAMS_MAX_CHARS, wcharval);
      lua_pushstring(pState, charval);
      swprintf_s(sBuf, 128, L"%s,", wcharval);
      wsafecat(sParams, 512, sBuf);
      iParams++;
    }
  }
  if (sParams[0] != L'\0')
    sParams[wcslen(sParams) - 1] = L'\0';

  // Finalize the call
  if (pLua->callPreparedLuaFunction(iParams, 1, sCallback, sParams))
  {
    double result = pLua->getLuaNumber();
    if (result > 0) // means that we need to reload LUA basic data
    {
      pLua->loadBasicData(m_pLocalClient->getDebug());
    }
  }
}

// -----------------------------------------------------------------
// Name : deactivateSkills
// -----------------------------------------------------------------
void PlayerManager::deactivateSkills(NetworkData * pData)
{
  u8 player = (u8) pData->readLong();
  u32 unit = (u32) pData->readLong();
  u32 skill = (u32) pData->readLong();

  Player * pPlayer = findPlayer(player);
  assert(pPlayer != NULL);
  Unit * pUnit = pPlayer->findUnit(unit);
  assert(unit != NULL);
  bool bIsActive;
  Skill * pSkill = pUnit->findSkill(skill, &bIsActive);
  assert(pSkill != NULL);
  if (bIsActive)
    pUnit->disableEffect(pSkill);
}

// -----------------------------------------------------------------
// Name : enableAllEffects
// -----------------------------------------------------------------
void PlayerManager::enableAllEffects(NetworkData * pData)
{
  Player * pPlayer = getFirstPlayerAndNeutral(0);
  while (pPlayer != NULL)
  {
    Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(0);
    while (pUnit != NULL)
    {
      pUnit->enableAllEffects();
      pUnit = (Unit*) pPlayer->m_pUnits->getNext(0);
    }
    pPlayer = getNextPlayerAndNeutral(0);
  }
}

// -----------------------------------------------------------------
// Name : changeSpellOwner
// -----------------------------------------------------------------
void PlayerManager::changeSpellOwner(NetworkData * pData)
{
  wchar_t sType[128];
  pData->readString(sType);
  u8 uOld = (u8) pData->readLong();
  u32 uSpell = (u32) pData->readLong();
  u8 uNew = (u8) pData->readLong();

  Player * pOld = findPlayer(uOld);
  assert(pOld != NULL);
  Player * pNew = findPlayer(uNew);
  assert(pNew != NULL);

  ObjectList * pSrc = NULL;
  ObjectList * pDst = NULL;
  if (wcscmp(sType, L"spell_in_hand") == 0)
  {
    pSrc = pOld->m_pHand;
    pDst = pNew->m_pHand;
  }
  else if (wcscmp(sType, L"spell_in_play") == 0)
  {
    pSrc = pOld->m_pActiveSpells;
    pDst = pNew->m_pActiveSpells;
  }
  else if (wcscmp(sType, L"spell_in_discard") == 0)
  {
    pSrc = pOld->m_pDiscard;
    pDst = pNew->m_pDiscard;
  }
  assert(pSrc != NULL && pDst != NULL);

  Spell * pSpell = pOld->findSpell(0, uSpell, pSrc);
  assert(pSpell != NULL);
  pSrc->deleteCurrent(0, true, true);
  pDst->addLast(pSpell);
  pSpell->setCaster(pNew);
}

// -----------------------------------------------------------------
// Name : changeUnitOwner
// -----------------------------------------------------------------
void PlayerManager::changeUnitOwner(NetworkData * pData)
{
  u8 uOld = (u8) pData->readLong();
  u32 uUnit = (u32) pData->readLong();
  u8 uNew = (u8) pData->readLong();

  Player * pOld = findPlayer(uOld);
  assert(pOld != NULL);
  Player * pNew = findPlayer(uNew);
  assert(pNew != NULL);

  Unit * pUnit = pOld->findUnit(uUnit);
  assert(pUnit != NULL);
  pOld->m_pUnits->deleteObject(pUnit, true, true);
  pNew->m_pUnits->addLast(pUnit);

  pUnit->unsetOrder();
  pUnit->setOwner(pNew->m_uPlayerId);
  pUnit->setPlayerColor(pNew->m_Color);
}

// -----------------------------------------------------------------
// Name : changeTownOwner
// -----------------------------------------------------------------
void PlayerManager::changeTownOwner(NetworkData * pData)
{
  u32 uTown = (u32) pData->readLong();
  u8 uNew = (u8) pData->readLong();

  Player * pNew = findPlayer(uNew);
  assert(pNew != NULL);

  Town * pTown = m_pLocalClient->getGameboard()->getMap()->findTown(uTown);
  assert(pTown != NULL);
  pTown->setOwner(uNew);
  pTown->setBaseValue(STRING_HEROECHANCES, 0);
}

// -----------------------------------------------------------------
// Name : addSkillToUnit
// -----------------------------------------------------------------
void PlayerManager::addSkillToUnit(NetworkData * pData)
{
  wchar_t sEdition[NAME_MAX_CHARS];
  wchar_t sSkill[NAME_MAX_CHARS];
  wchar_t sParams[256];
  pData->readString(sEdition);
  pData->readString(sSkill);
  u32 uId = (u32) pData->readLong();
  pData->readString(sParams);
  u8 uPlayer = (u8) pData->readLong();
  u32 uUnit = (u32) pData->readLong();

  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  Unit * pUnit = pPlayer->findUnit(uUnit);
  assert(pUnit != NULL);

  Skill * pSkill = new Skill(uId, sEdition, sSkill, sParams, m_pLocalClient->getDebug());
  assert(pSkill->isLoaded());
  pUnit->addSkill(pSkill);
}

// -----------------------------------------------------------------
// Name : hideSpecialTile
// -----------------------------------------------------------------
void PlayerManager::hideSpecialTile(NetworkData * pData)
{
  u32 uTileId = (u32) pData->readLong();
  Map * pMap = m_pLocalClient->getGameboard()->getMap();
  // Loop through map
  for (int x = 0; x < pMap->getWidth(); x++)
  {
    for (int y = 0; y < pMap->getHeight(); y++)
    {
      MapTile * pTile = pMap->getTileAt(CoordsMap(x, y));
      if (pTile->m_pSpecialTile != NULL && pTile->m_pSpecialTile->getInstanceId() == uTileId)
      {
        pTile->hideSpecialTile();
        return;
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : setNextPlayerReady
//  Directly called from LocalClient
// -----------------------------------------------------------------
void PlayerManager::setNextPlayerReady(Player * pPlayer)
{
  m_pActiveLocalPlayer = pPlayer;
  pPlayer->setState(playing);
  setFocusUnit(m_pActiveLocalPlayer->getFirstUnplayedUnit());
  m_fTurnTimer = (double) m_pLocalClient->getTurnTimer();
  m_pLocalClient->getInterface()->getInfoDialog()->updatePlayersState();
}

// -----------------------------------------------------------------
// Name : updateMagicCirclePositions
// -----------------------------------------------------------------
void PlayerManager::updateMagicCirclePositions()
{
  ObjectList * pCircles = m_pLocalClient->getGameboard()->getMagicCircles();
  pCircles->deleteAll();
  Player * pPlayer = m_pLocalClient->getPlayerManager()->getFirstPlayerAndNeutral(0);
  while (pPlayer != NULL)
  {
    for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
    {
      if (pPlayer->m_MagicCirclePos[i].x >= 0)
      {
        CoordsObject * pCoords = new CoordsObject(pPlayer->m_MagicCirclePos[i]);
        pCoords->setAttachment(pPlayer);
        pCircles->addLast(pCoords);
      }
    }
    pPlayer = m_pLocalClient->getPlayerManager()->getNextPlayerAndNeutral(0);
  }
}

// -----------------------------------------------------------------
// Name : getLocalPlayersCount
// -----------------------------------------------------------------
u8 PlayerManager::getLocalPlayersCount()
{
  u8 count = 0;
  Player * player = (Player*) m_pPlayersList->getFirst(0);
  while (player != NULL)
  {
    if (player->m_uClientId == m_pLocalClient->getClientId())
      count++;
    player = (Player*) m_pPlayersList->getNext(0);
  }
  return count;
}

// -----------------------------------------------------------------
// Name : setFocusUnit
// -----------------------------------------------------------------
void PlayerManager::setFocusUnit(Unit * unit)
{
  m_pLocalClient->getGameboard()->selectMapObject(unit);
  if (unit != NULL)
    m_pLocalClient->getFx()->zoomToMapPos(unit->getMapPos());
}

// -----------------------------------------------------------------
// Name : castSpell
// -----------------------------------------------------------------
void PlayerManager::castSpell(Player * pPlayer, Spell * pSpell)
{
  m_pSpellBeingCast = pSpell;
  extern u32 g_uLuaCurrentObjectType;
  g_uLuaCurrentObjectType = LUAOBJECT_SPELL;
  pSpell->resetResolveParameters();
  pSpell->setExtraMana(Mana());
  pPlayer->castSpell(pSpell);
}

// -----------------------------------------------------------------
// Name : doSkillEffect
// -----------------------------------------------------------------
void PlayerManager::doSkillEffect(Unit * pUnit, ChildEffect * pSkillEffect)
{
  m_pUnitActivatingSkill = pUnit;
  m_pSkillBeingActivated = pSkillEffect;
  Skill * pSkill = (Skill*) pSkillEffect->getAttachment();
  pUnit->setSkillOrder(pSkill->getInstanceId(), pSkillEffect->id);
  extern u32 g_uLuaCurrentObjectType;
  g_uLuaCurrentObjectType = LUAOBJECT_SKILL;
  pSkillEffect->resetResolveParameters();
  pSkill->setExtraMana(Mana());
  pSkill->callLuaFunction(L"onActivateEffect", 0, L"i", (int) (m_pSkillBeingActivated->id + 1));
}

// -----------------------------------------------------------------
// Name : castSpellFinished
// -----------------------------------------------------------------
void PlayerManager::castSpellFinished(bool bCastOk, bool bOnResolve)
{
  // Audio
  if (bCastOk)
    AudioManager::getInstance()->playSound(SOUND_CAST_SPELL);
  else
    AudioManager::getInstance()->playSound(SOUND_CANCEL_SPELL);

  if (bOnResolve)
    m_pLocalClient->getInterface()->getResolveDialog()->resolveTargetSelectionFinished(!bCastOk, m_pSpellBeingCast);
  else if (!bCastOk)
    m_pLocalClient->getInterface()->getSpellDialog()->cancelCastSpell(m_pSpellBeingCast);
  m_pLocalClient->getInterface()->getSpellDialog()->updateSpellInformation(m_pSpellBeingCast);
  m_pSpellBeingCast = NULL;
}

// -----------------------------------------------------------------
// Name : skillWasActivated
// -----------------------------------------------------------------
void PlayerManager::skillWasActivated(bool bOk, bool bOnResolve)
{
  if (bOnResolve)
    m_pLocalClient->getInterface()->getResolveDialog()->resolveTargetSelectionFinished(!bOk, (LuaObject*) m_pSkillBeingActivated->getAttachment(), m_pSkillBeingActivated, m_pUnitActivatingSkill);
  else if (!bOk)
    m_pLocalClient->getInterface()->getUnitOptionsDialog()->cancelSkillAction();
  m_pSkillBeingActivated = NULL;
}

// -----------------------------------------------------------------
// Name : setSpellBeingCastOnResolve
// -----------------------------------------------------------------
void PlayerManager::setSpellBeingCastOnResolve(Spell * pSpell)
{
  m_pSpellBeingCast = pSpell;
  extern u32 g_uLuaCurrentObjectType;
  g_uLuaCurrentObjectType = LUAOBJECT_SPELL;
  pSpell->resetResolveParameters();
}

// -----------------------------------------------------------------
// Name : setSkillBeingActivatedOnResolve
// -----------------------------------------------------------------
void PlayerManager::setSkillBeingActivatedOnResolve(Unit * pUnit, ChildEffect * pSkillEffect)
{
  m_pUnitActivatingSkill = pUnit;
  m_pSkillBeingActivated = pSkillEffect;
  extern u32 g_uLuaCurrentObjectType;
  g_uLuaCurrentObjectType = LUAOBJECT_SKILL;
  pSkillEffect->resetResolveParameters();
  ((LuaObject*) pSkillEffect->getAttachment())->setExtraMana(Mana());
}

// -----------------------------------------------------------------
// Name : findLuaTarget
// -----------------------------------------------------------------
LuaTargetable * PlayerManager::findLuaTarget(NetworkData * pData, u32 * pType)
{
  u32 uType = (u32) pData->readLong();
  LuaTargetable * pTarget = NULL;
  switch (uType)
  {
  case LUATARGET_PLAYER:
    {
      u8 uPlayer = (u8) pData->readLong();
      pTarget = findPlayer(uPlayer);
      break;
    }
  case LUATARGET_TILE:
    {
      int x = (int) pData->readLong();
      int y = (int) pData->readLong();
      pTarget = m_pLocalClient->getGameboard()->getMap()->getTileAt(CoordsMap(x, y));
      break;
    }
  case LUATARGET_TOWN:
    {
      u32 uTown = (u32) pData->readLong();
      pTarget = m_pLocalClient->getGameboard()->getMap()->findTown(uTown);
      break;
    }
  case LUATARGET_UNIT:
    {
      u8 uPlayer = (u8) pData->readLong();
      u32 uUnit = (u32) pData->readLong();
      Player * pPlayer = findPlayer(uPlayer);
      if (pPlayer != NULL)
        pTarget = pPlayer->findUnit(uUnit);
      break;
    }
  }
  if (pType != NULL)
    *pType = uType;
  return pTarget;
}

// -----------------------------------------------------------------
// Name : buildBuilding
// -----------------------------------------------------------------
void PlayerManager::buildBuilding(NetworkData * pData)
{
  u32 uTownId = (u32) pData->readLong();
  wchar_t sName[NAME_MAX_CHARS];
  pData->readString(sName);
  Town * pTown = m_pLocalClient->getGameboard()->getMap()->findTown(uTownId);
  assert(pTown != NULL);
  pTown->buildBuilding(sName, NULL);
}

// -----------------------------------------------------------------
// Name : teleport
// -----------------------------------------------------------------
void PlayerManager::teleport(NetworkData * pData)
{
  long type = pData->readLong();
  wchar_t sIds[16];
  pData->readString(sIds);
  MapObject * pMapObj = (MapObject*) findTargetFromIdentifiers(type, sIds, m_pLocalClient->getGameboard()->getMap());
  assert(pMapObj != NULL);
  int x = (int) pData->readLong();
  int y = (int) pData->readLong();
  pMapObj->setMapPos(CoordsMap(x, y));
}

// -----------------------------------------------------------------
// Name : resurrectUnit
// -----------------------------------------------------------------
void PlayerManager::resurrectUnit(NetworkData * pData)
{
  wchar_t sIds[16];
  pData->readString(sIds);
  Unit * pUnit = (Unit*) findTargetFromIdentifiers(LUATARGET_UNIT, sIds, m_pLocalClient->getGameboard()->getMap());
  assert(pUnit != NULL);
  Player * pPlayer = findPlayer(pUnit->getOwner());
  assert(pPlayer != NULL);
  pPlayer->m_pDeadUnits->deleteObject(pUnit, true, true);
  pPlayer->m_pUnits->addLast(pUnit);
  pUnit->setStatus(US_Normal);
  pUnit->setBaseValue(STRING_LIFE, pUnit->getValue(STRING_ENDURANCE));
}

// -----------------------------------------------------------------
// Name : addMagicCircle
// -----------------------------------------------------------------
void PlayerManager::addMagicCircle(NetworkData * pData)
{
  u8 uPlayer = (u8) pData->readLong();
  int i = (int) pData->readLong();
  int x = (int) pData->readLong();
  int y = (int) pData->readLong();
  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  assert(i >= 0 && i < MAX_MAGIC_CIRCLES);
  pPlayer->m_MagicCirclePos[i] = CoordsMap(x, y);
}

// -----------------------------------------------------------------
// Name : removeMagicCircle
// -----------------------------------------------------------------
void PlayerManager::removeMagicCircle(NetworkData * pData)
{
  u8 uPlayer = (u8) pData->readLong();
  int i = (int) pData->readLong();
  Player * pPlayer = findPlayer(uPlayer);
  assert(pPlayer != NULL);
  assert(i >= 0 && i < MAX_MAGIC_CIRCLES);
  pPlayer->m_MagicCirclePos[i].x = -1;
}

// -----------------------------------------------------------------
// Name : addExtraMana
// -----------------------------------------------------------------
void PlayerManager::addExtraMana(u32 uType, bool bOk, wchar_t * sCallback, Mana amount)
{
  if (bOk && m_pLocalClient->getInterface()->getSpellDialog()->takeMana(amount))
  {
    int total = amount.amount();
    if (uType == LUAOBJECT_SPELL)
    {
      Spell * pSpell = getSpellBeingCast();
      pSpell->setExtraMana(amount);
      pSpell->callLuaFunction(sCallback, 0, L"i", total);
    }
    else
    {
      Skill * pSkill = (Skill*) getSkillBeingActivated()->getAttachment();
      pSkill->setExtraMana(amount);
      pSkill->callLuaFunction(sCallback, 0, L"i", total);
    }
  }
  else
  {
    if (uType == LUAOBJECT_SPELL)
      castSpellFinished(false, false);
    else
      skillWasActivated(false, false);
  }
}

// -----------------------------------------------------------------
// Name : deepFindLuaObject
//  This function searches in all game data, so don't use it when not necessary
// -----------------------------------------------------------------
//LuaObject * PlayerManager::deepFindLuaObject(u32 uId, ObjectList * pPlayers, Map * pMap)
//{
//  int it = pPlayers->getIterator();
//  Player * pPlayer = (Player*) pPlayers->getFirst(it);
//  while (pPlayer != NULL)
//  {
//    // Player spell?
//    Spell * pSpell = pPlayer->findSpell(0, uId, pPlayer->m_pActiveSpells);
//    if (pSpell != NULL)
//      return pSpell;
//    pSpell = pPlayer->findSpell(0, uId, pPlayer->m_pDeck);
//    if (pSpell != NULL)
//      return pSpell;
//    pSpell = pPlayer->findSpell(0, uId, pPlayer->m_pHand);
//    if (pSpell != NULL)
//      return pSpell;
//    pSpell = pPlayer->findSpell(0, uId, pPlayer->m_pDiscard);
//    if (pSpell != NULL)
//      return pSpell;
//    // Unit skill?
//    Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(0);
//    while (pUnit != NULL)
//    {
//      Skill * pSkill = pUnit->findSkill(uId);
//      if (pSkill != NULL)
//        return pSkill;
//      pUnit = (Unit*) pPlayer->m_pUnits->getNext(0);
//    }
//    pPlayer = (Player*) pPlayers->getNext(it);
//  }
//  pPlayers->releaseIterator(it);
//  // Building?
//  Town * pTown = pMap->getFirstTown();
//  while (pTown != NULL)
//  {
//    Building * pBuild = pTown->getFirstBuildableBuilding(m_pLocalClient->getDebug());
//    while (pBuild != NULL)
//    {
//      if (pBuild->getInstanceId() == uId)
//        return pBuild;
//      pBuild = pTown->getNextBuildableBuilding();
//    }
//    pBuild = pTown->getFirstBuiltBuilding();
//    while (pBuild != NULL)
//    {
//      if (pBuild->getInstanceId() == uId)
//        return pBuild;
//      pBuild = pTown->getNextBuiltBuilding();
//    }
//    pTown = pMap->getNextTown();
//  }
//  // Not found
//  return NULL;
//}
