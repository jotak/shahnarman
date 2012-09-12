// -----------------------------------------------------------------
// Name : SPELLS SOLVER
// -----------------------------------------------------------------
#include "SpellsSolver.h"
#include "Server.h"
#include "TurnSolver.h"
#include "../Players/Player.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../Debug/DebugManager.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/Building.h"
#include "../Gameboard/SpecialTile.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Edition.h"
#include "../DeckData/AvatarData.h"

// -----------------------------------------------------------------
// Name : SpellsSolver
// -----------------------------------------------------------------
SpellsSolver::SpellsSolver(Server * pServer, TurnSolver * pSolver)
{
    m_pServer = pServer;
    m_pSolver = pSolver;
    m_iPlayerIt = m_pSolver->getPlayersList()->getIterator();
    m_bWaitingClient = NULL;
    m_pLuaBeingResolved = NULL;
    m_State = RSS_NotResolving;
    m_bSpellsCastThisTurn = false;
    m_bPauseResolving = false;
    m_bIdle = false;
}

// -----------------------------------------------------------------
// Name : ~SpellsSolver
// -----------------------------------------------------------------
SpellsSolver::~SpellsSolver()
{
    if (m_bWaitingClient != NULL)
        delete[] m_bWaitingClient;
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void SpellsSolver::Init()
{
    m_State = RSS_NotResolving;
    m_bWaitingClient = new bool[m_pServer->getNbClients()];
    m_bPauseResolving = false;
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void SpellsSolver::Update(double delta)
{
    if (m_State == RSS_NotResolving || m_bIdle || m_bPauseResolving)
        return;

    Player * pPlayer = (Player*) m_pSolver->getPlayersList()->getCurrent(m_iPlayerIt);
    if (!startResolvePlayerSpell(pPlayer))
    {
        // Last player's spell is reached, so loop through players
        Player * pFirstPlayer = (Player*) (*(m_pSolver->getPlayersList()))[m_pSolver->getFirstResolutionIdx()];
        pPlayer = (Player*) m_pSolver->getPlayersList()->getNext(m_iPlayerIt, true);
        if (pPlayer != pFirstPlayer)
        {
            pPlayer->m_pCastSpells->getFirst(0);
            NetworkData data(NETWORKMSG_RESOLVING_SPELL_ORDERS);
            data.addLong((long)pPlayer->m_uPlayerId);
            m_pServer->sendMessageToAllClients(&data);
        }
        else
        {
            // Loop finished ; switch on state
            switch (m_State)
            {
            case RSS_TurnSpells:  // give hand back to turn solver
                m_State = RSS_NotResolving;
                break;
            case RSS_BattleSpells:
            case RSS_PostBattleSpells:
            {
                if (m_bSpellsCastThisTurn)    // if spells have been cast right before, then other players may want to reply
                {
                    m_pSolver->checkAllUnitUpdates(false); // send updated unit data to clients
                    m_pSolver->allowCastSpells(m_State == RSS_PostBattleSpells);
                    waitForInstantSpells(m_State);  // let's do another turn
                }
                else
                    m_State = RSS_NotResolving; // else, give hand back to turn solver
                break;
            }
            default:
                break;
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : startResolveSpells
// -----------------------------------------------------------------
void SpellsSolver::startResolveSpells(ResolveSpellsState state)
{
    // Set iterator 0 to first player in resolve order
    Player * pPlayer = (Player*) m_pSolver->getPlayersList()->goTo(m_iPlayerIt, m_pSolver->getFirstResolutionIdx());
    pPlayer->m_pCastSpells->getFirst(0);
    NetworkData data(NETWORKMSG_RESOLVING_SPELL_ORDERS);
    data.addLong((long)pPlayer->m_uPlayerId);
    m_pServer->sendMessageToAllClients(&data);
    m_bIdle = false;
    m_State = state;
}

// -----------------------------------------------------------------
// Name : waitForInstantSpells
// -----------------------------------------------------------------
void SpellsSolver::waitForInstantSpells(ResolveSpellsState state)
{
    // Now, ask every player if they want to cast battle or post-battle spells.
    // They all will be resolved in the current resolution order.
    // We will continue to ask for spells until no player want to cast any spell.
    for (int i = 0; i < m_pServer->getNbClients(); i++)
        m_bWaitingClient[i] = true;
    m_bIdle = true;
    m_bSpellsCastThisTurn = false;
    m_State = state;
}

// -----------------------------------------------------------------
// Name : startResolvePlayerSpell
//  return false if the last spell is reached
// -----------------------------------------------------------------
bool SpellsSolver::startResolvePlayerSpell(Player * pPlayer)
{
//  m_pCurrentCaster = pPlayer;
    Spell * pSpell = (Spell*) pPlayer->m_pCastSpells->getCurrent(0);
    if (pSpell == NULL)
    {
        // When we've finished to cast all player's spells, send updated mana to clients
        NetworkData msg(NETWORKMSG_SEND_PLAYER_MANA);
        msg.addLong(1); // Stands for "spent mana"
        msg.addLong((long)(pPlayer->m_uPlayerId));
        for (int i = 0; i < 4; i++)
            msg.addLong((long)(pPlayer->m_SpentMana[i]));
        m_pServer->sendMessageToAllClients(&msg);
        return false;
    }
//  m_pSpellBeingCast = pSpell;
    Mana mana = pPlayer->getMana();
    // check if the player still has enough mana
    if (mana < pPlayer->m_SpentMana + pSpell->getCost()) // if no mana left, try next spell
    {
        pPlayer->m_pCastSpells->deleteCurrent(0, true);
        m_pServer->sendCustomLogToAll("(s)_TRIED_CAST_SPELL_BUT_NO_MANA", 0, "ps", pPlayer->m_uPlayerId, pPlayer->m_uPlayerId, "hand", pSpell->getInstanceId());
    }
    else if (pPlayer->m_pHand->goTo(0, pSpell) == false)  // spell may have been discarded for instance: it's not in hand anymore, so ignore it
    {
        pPlayer->m_pCastSpells->deleteCurrent(0, true);
        m_pServer->sendCustomLogToAll("(s)_CANT_FIND_SPELL", 0, "p", pPlayer->m_uPlayerId);
    }
    else
    {
        pPlayer->m_SpentMana += pSpell->getCost();
        NetworkData msg(NETWORKMSG_SEND_CAST_SPELLS_DATA);
        msg.addLong(0); // stands for "start cast spell"
        msg.addLong((long) pPlayer->m_uPlayerId);
        msg.addLong((long) pSpell->getInstanceId());
        m_pServer->sendMessageToAllClients(&msg);
        m_bPauseResolving = false;
        m_pLuaBeingResolved = pSpell;
        pSpell->setCaster(pPlayer);
        pSpell->setCurrentEffect(-1);
        // In what follows, it's possible that m_pSpellNetworkData is modified during the "onResolve" call
        pSpell->callLuaFunction("onResolve", 0, "sil", pSpell->getResolveParameters(), (int) pPlayer->m_uPlayerId, (long) pSpell->getInstanceId());
        if (!m_bPauseResolving)  // During call of "onResolve", it's possible that it was paused for instance to select a target during resolve
            endCastSpell(false);
    }
    return true;
}

// -----------------------------------------------------------------
// Name : resolveChildEffect
// -----------------------------------------------------------------
void SpellsSolver::resolveChildEffect(Player * pPlayer, Unit * pUnit, ChildEffect * pChild)
{
    m_State = RSS_ChildEffect;
    Mana mana = pPlayer->getMana();
    // check if the player still has enough mana
    if (pChild->cost + pPlayer->m_SpentMana <= mana)
    {
        pPlayer->m_SpentMana += pChild->cost;
        // send updated mana to clients
        NetworkData msg(NETWORKMSG_SEND_PLAYER_MANA);
        msg.addLong(1); // Stands for "spent mana"
        msg.addLong((long)(pPlayer->m_uPlayerId));
        for (int i = 0; i < 4; i++)
            msg.addLong((long)(pPlayer->m_SpentMana[i]));
        m_pServer->sendMessageToAllClients(&msg);

        m_bPauseResolving = false;
        LuaObject * pLua = pChild->getLua();
        pLua->setCurrentEffect(pChild->id);
        m_pLuaBeingResolved = pLua;
        // In what follows, it's possible that m_pSpellNetworkData is modified during the "onResolve" call
        switch (pLua->getType())
        {
        case LUAOBJECT_SPELL:
            ((Spell*)pLua)->setCaster(pPlayer);
            pLua->callLuaFunction("onResolveChild", 0, "isi", (int) (pChild->id + 1), pChild->sResolveParams, (int) pPlayer->m_uPlayerId);
            break;
        case LUAOBJECT_SKILL:
            ((Skill*)pLua)->setCaster(pUnit);
            pLua->callLuaFunction("onResolveChild", 0, "isii", (int) (pChild->id + 1), pChild->sResolveParams, (int) pPlayer->m_uPlayerId, (int) pUnit->getId());
            break;
        }
        if (!m_bPauseResolving)  // During call of "onResolve", it's possible that it was paused for instance to select a target during resolve
            endActivateEffect(false);
    }
}

// -----------------------------------------------------------------
// Name : endCastSpell
// -----------------------------------------------------------------
void SpellsSolver::endCastSpell(bool bCancel)
{
    m_bPauseResolving = false;
    assert(m_pLuaBeingResolved != NULL && m_pLuaBeingResolved->getType() == LUAOBJECT_SPELL);
    Spell * pSpell = (Spell*) m_pLuaBeingResolved;
    Player * pPlayer = pSpell->getCaster();
    m_pLuaBeingResolved = NULL;
    pPlayer->m_pCastSpells->deleteCurrent(0, true);
    pPlayer->m_pHand->deleteObject(pSpell, true, true);
    NetworkData msg(NETWORKMSG_SEND_CAST_SPELLS_DATA);
    msg.addLong(1); // stands for "finished cast spell"
    msg.addLong((long) pPlayer->m_uPlayerId);
    msg.addLong((long) pSpell->getInstanceId());
    if (bCancel)
    {
        pPlayer->m_pDiscard->addLast(pSpell);
        msg.addLong(2);  // stands for "spell canceled, move spell from hand to discard"
    }
    else if (pSpell->isGlobal() || pSpell->getTargets()->size > 0)
    {
        pPlayer->m_pActiveSpells->addLast(pSpell);
        msg.addLong(1);  // stands for "move spell from hand to active"
    }
    else
    {
        pPlayer->m_pDiscard->addLast(pSpell);
        msg.addLong(0);  // stands for "move spell from hand to discard"
    }
    msg.addString(pSpell->getResolveParameters());
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : endActivateEffect
// -----------------------------------------------------------------
void SpellsSolver::endActivateEffect(bool bCancel)
{
    m_pLuaBeingResolved = NULL;
    m_bPauseResolving = false;
    m_State = RSS_NotResolving;
}

// -----------------------------------------------------------------
// Name : receiveInstantSpells
//  This function stores received instant spells (battle, or post-battle). They will be resolved when all clients
//  send a response. Until that, nothing is done.
// -----------------------------------------------------------------
void SpellsSolver::receiveInstantSpells(int iClient, NetworkData * pData)
{
    // store received spells
    receiveSpells(pData);
    // update waiting clients boolean
    m_bWaitingClient[iClient] = false;
    for (int i = 0; i < m_pServer->getNbClients(); i++)
    {
        if (m_bWaitingClient[i])
            return; // we're still waiting for at least one client
    }
    // no more waiting client: resolve spells
    startResolveSpells(m_State);  // here we don't change the state ; it was set up when askInstantSpells was called
}

// -----------------------------------------------------------------
// Name : receiveSpells
//  Receives spells cast from clients. It can be called either at the end of the orders phase,
//  or during battles or post-battle phase.
//  It moves the spells from list Player::m_pHand to ServerPlayer::m_pCastSpells.
// -----------------------------------------------------------------
void SpellsSolver::receiveSpells(NetworkData * pData)
{
    while (pData->dataYetToRead() > 0)
    {
        u8 uPlayerId = (u8) pData->readLong();
        int nbSpells = (int) pData->readLong();
        if (nbSpells > 0)
            m_bSpellsCastThisTurn = true;
        Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
        if (pPlayer != NULL)
        {
            for (int i = 0; i < nbSpells; i++)
            {
                u32 uSpellId = (u32) pData->readLong();
                Spell * pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pHand);
                if (pSpell != NULL)
                {
                    char spellParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
                    // Note: keep spell in hand, it still can be discarded by opponent for instance
                    pPlayer->m_pCastSpells->addLast(pSpell);
                    pData->readString(spellParams, LUA_FUNCTION_PARAMS_MAX_CHARS, m_pServer->getDebug(), "Error in Server::receiveSpells: corrupted data (spellParams)");
                    pSpell->resetResolveParameters();
                    pSpell->addResolveParameters(spellParams);
                }
                else
                    m_pServer->getDebug()->notifyErrorMessage("Inconsistant data received from client: spell not found in player's hand.");
            }
        }
        else
            m_pServer->getDebug()->notifyErrorMessage("Inconsistant data received from client: player not found.");
    }
}

// -----------------------------------------------------------------
// Name : receiveTargetOnResolve
// -----------------------------------------------------------------
void SpellsSolver::receiveTargetOnResolve(NetworkData * pData)
{
    bool isChild = (pData->readLong() == 1);
    u32 uType = (u32) pData->readLong();
    //u8 uPlayerId = (u8) pData->readLong();
    //Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    //if (pPlayer != NULL)
    //{
    //  LuaObject * pLua = NULL;
    //  if (uType == LUAOBJECT_SPELL) // Resolving spell
    //  {
    //    u32 uId = (u32) pData->readLong();
    //    if (isChild)
    //      pLua = pPlayer->findSpell(0, uId, pPlayer->m_pActiveSpells);
    //    else
    //      pLua = pPlayer->findSpell(0, uId, pPlayer->m_pCastSpells);
    //  }
    //  else  // Skill
    //  {
    //    u32 uUnitId = (u32) pData->readLong();
    //    u32 uId = (u32) pData->readLong();
    //    Unit * pUnit = pPlayer->findUnit(uUnitId);
    //    if (pUnit != NULL)
    //      pLua = pUnit->findSkill(uId);
    //    else
    //    {
    //      m_pServer->getDebug()->notifyErrorMessage("Inconsistant data received from client: unit doesn't exist.");
    //      return;
    //    }
    //  }
    //  if (pLua == NULL)
    //  {
    //    m_pServer->getDebug()->notifyErrorMessage("Inconsistant data received from client: skill or spell can't be found.");
    //    return;
    //  }
    bool bCancel = (pData->readLong() == 1);
    if (bCancel)
    {
        if (!isChild && uType == LUAOBJECT_SPELL)
            endCastSpell(true);
        else
            endActivateEffect(true);
        return;
    }
    assert(m_pLuaBeingResolved != NULL);
    char sResolveFunc[128];
    pData->readString(sResolveFunc, 128, m_pServer->getDebug(), "Error in SpellsSolver::receiveTargetOnResolve: corrupted data (sResolveFunc)");
    char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
    m_bPauseResolving = false;
    if (isChild)
    {
        int id = (int) pData->readLong();
        ChildEffect * pChild = m_pLuaBeingResolved->getChildEffect(id);
        pData->readString(sParams, LUA_FUNCTION_PARAMS_MAX_CHARS, m_pServer->getDebug(), "Error in SpellsSolver::receiveTargetOnResolve: corrupted data (sParams)");
        m_pLuaBeingResolved->setCurrentEffect(pChild->id);
        m_pLuaBeingResolved->callLuaFunction(sResolveFunc, 0, "is", (int) (pChild->id + 1), sParams);
        if (!m_bPauseResolving)  // During call of "onResolve", it's possible that it was paused for instance to select a target during resolve
            endActivateEffect(false);
    }
    else
    {
        if (uType == LUAOBJECT_SPELL)
        {
            pData->readString(sParams, LUA_FUNCTION_PARAMS_MAX_CHARS, m_pServer->getDebug(), "Error in SpellsSolver::receiveTargetOnResolve: corrupted data (sParams)");
            m_pLuaBeingResolved->callLuaFunction(sResolveFunc, 0, "s", sParams);
            if (!m_bPauseResolving)  // During call of "onResolve", it's possible that it was paused for instance to select a target during resolve
                endCastSpell(false);
        }
        else
        {
            // Case doesn't happen until now
            //pLua->callLuaFunction(sResolveFunc, "", 0);
            //if (!m_bPauseResolving)  // During call of "onResolve", it's possible that it was paused for instance to select a target during resolve
            //  endResolveLua(false, false);
        }
    }

    //}
    //else
    //  m_pServer->getDebug()->notifyErrorMessage("Inconsistant data received from client: player not found.");
}

// -----------------------------------------------------------------
// Name : retrieveLuaContext
// -----------------------------------------------------------------
LuaContext * SpellsSolver::retrieveLuaContext(u32 uExpectedType, NetworkData * pSerialize, int iEffect)
{
    bool bOk = m_LuaContext.retrieve(m_pSolver);
    if (!bOk)
    {
        m_pServer->getDebug()->notifyErrorMessage(m_LuaContext.sError);
        return NULL;
    }
    if (uExpectedType != 0 && m_LuaContext.pLua->getType() != uExpectedType)
    {
        m_pServer->getDebug()->notifyErrorMessage("retrieveLuaContext: got unexpected type!");
        return NULL;
    }
    if (iEffect >= 0)
        m_LuaContext.pLua->setCurrentEffect(iEffect);
    if (pSerialize != NULL)
        m_LuaContext.serialize(pSerialize);
    return &m_LuaContext;
}

// -----------------------------------------------------------------
// Name : onDamageUnit
// -----------------------------------------------------------------
void SpellsSolver::onDamageUnit(u8 uPlayerId, u32 uUnitId, u8 uDamages)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        Unit * pUnit = (Unit*) pPlayer->findUnit(uUnitId);
        if (pUnit != NULL)
        {
            int newlife = pUnit->getValue(STRING_LIFE) - uDamages;
            pUnit->setBaseValue(STRING_LIFE, newlife);
            if (newlife <= 0)
                pUnit->setStatus(US_Dead);
        }
        else
            m_pServer->getDebug()->notifyErrorMessage("Lua interaction error in function damageUnit: unit not found.");
    }
    else
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error in function damageUnit: player not found.");
}

// -----------------------------------------------------------------
// Name : onAddUnit
// -----------------------------------------------------------------
Unit * SpellsSolver::onAddUnit(CoordsMap mapPos, const char * sName, u8 uOwner, bool bSimulate)
{
    retrieveLuaContext();
    assert(m_LuaContext.pLua != NULL);
    Unit * pUnit = m_pSolver->addUnitToPlayer(m_LuaContext.pLua->getObjectEdition(), sName, uOwner, mapPos, bSimulate);
    if (!bSimulate)
    {
        // Send data to clients
        NetworkData unitData(NETWORKMSG_CREATE_UNIT_DATA);
        pUnit->serialize(&unitData);
        m_pServer->sendMessageToAllClients(&unitData);
    }
    return pUnit;
}

// -----------------------------------------------------------------
// Name : onAttachToUnit
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAttachToUnit(u8 uPlayerId, u32 uUnitId)
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        Unit * pUnit = pPlayer->findUnit(uUnitId);
        if (pUnit != NULL)
        {
            pUnit->attachEffect(m_LuaContext.pLua);
            m_LuaContext.pLua->addTarget(pUnit, SELECT_TYPE_UNIT);
            // now we'll complete the current network message m_pSpellNetworkData
            // that is going to be send from function resolvePlayerSpells
            msg.addLong(SELECT_TYPE_UNIT);         // stands for "spell was attached to a unit"
            msg.addLong((long) uPlayerId);       // target player id
            msg.addLong((long) uUnitId);         // target unit id
            m_pServer->sendMessageToAllClients(&msg);
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: lua script %s sent wrong unit id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: lua script %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAddChildEffectToUnit
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAddChildEffectToUnit(int iEffectId, u8 uPlayerId, u32 uUnitId)
{
    NetworkData msg(NETWORKMSG_CHILD_EFFECT_ATTACHED);
    if (!retrieveLuaContext(0, &msg, iEffectId))
        return;
    ChildEffect * pChild = m_LuaContext.pLua->getChildEffect(iEffectId);
    assert(pChild != NULL);
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        Unit * pUnit = pPlayer->findUnit(uUnitId);
        if (pUnit != NULL)
        {
            pUnit->attachChildEffect(pChild);
            pChild->pTargets->addLast(pUnit, SELECT_TYPE_UNIT);
            // now send a network message to notify players that an effect was attached
            msg.addLong(SELECT_TYPE_UNIT);                    // target type, stands for "attached to a unit"
            msg.addLong((long) uPlayerId);                  // target player id
            msg.addLong((long) uUnitId);                    // target unit id
            m_pServer->sendMessageToAllClients(&msg);
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: effect %s sent wrong unit id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: effect %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onRemoveChildEffectFromUnit
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onRemoveChildEffectFromUnit(int iEffectId, u8 uPlayerId, u32 uUnitId)
{
    NetworkData msg(NETWORKMSG_CHILD_EFFECT_DETACHED);
    if (!retrieveLuaContext(0, &msg, iEffectId))
        return;

    ChildEffect * pChild = m_LuaContext.pLua->getChildEffect(iEffectId);
    if (pChild == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: child effect not found while calling SpellsSolver::onRemoveChildEffectFromUnit.");
        return;
    }

    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        Unit * pUnit = pPlayer->findUnit(uUnitId);
        if (pUnit != NULL)
        {
            if (pUnit->detachChildEffect(pChild))
            {
                pChild->pTargets->deleteObject(pUnit, true);
                msg.addLong(SELECT_TYPE_UNIT);                    // target type, stands for "detached from a unit"
                msg.addLong((long) uPlayerId);                  // target player id
                msg.addLong((long) uUnitId);                    // target unit id
                m_pServer->sendMessageToAllClients(&msg);
            }
            else
            {
                char sError[128];
                snprintf(sError, 128, "Lua interaction error: can't detach effect %s.", m_LuaContext.pLua->getLocalizedName());
                m_pServer->getDebug()->notifyErrorMessage(sError);
            }
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: effect %s sent wrong unit id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: effect %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAddChildEffectToTown
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAddChildEffectToTown(int iEffectId, u32 uTownId)
{
    NetworkData msg(NETWORKMSG_CHILD_EFFECT_ATTACHED);
    if (!retrieveLuaContext(0, &msg, iEffectId))
        return;
    ChildEffect * pChild = m_LuaContext.pLua->getChildEffect(iEffectId);
    assert(pChild != NULL);
    Town * pTown = m_pServer->getMap()->findTown(uTownId);
    if (pTown != NULL)
    {
        pTown->attachChildEffect(pChild);
        pChild->pTargets->addLast(pTown, SELECT_TYPE_TOWN);
        // now send a network message to notify players that an effect was attached
        msg.addLong(SELECT_TYPE_TOWN);
        msg.addLong((long) uTownId);
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: effect %s sent wrong town id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onRemoveChildEffectFromTown
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onRemoveChildEffectFromTown(int iEffectId, u32 uTownId)
{
    NetworkData msg(NETWORKMSG_CHILD_EFFECT_DETACHED);
    if (!retrieveLuaContext(0, &msg, iEffectId))
        return;

    ChildEffect * pChild = m_LuaContext.pLua->getChildEffect(iEffectId);
    if (pChild == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: child effect not found while calling SpellsSolver::onRemoveChildEffectFromTown.");
        return;
    }

    Town * pTown = m_pServer->getMap()->findTown(uTownId);
    if (pTown != NULL)
    {
        if (pTown->detachChildEffect(pChild))
        {
            pChild->pTargets->deleteObject(pTown, true);
            msg.addLong(SELECT_TYPE_TOWN);                    // target type, stands for "detached from a town"
            msg.addLong((long) uTownId);
            m_pServer->sendMessageToAllClients(&msg);
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: can't detach effect %s.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: effect %s sent wrong town id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAttachToPlayer
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAttachToPlayer(u8 uPlayerId)
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        pPlayer->attachEffect(m_LuaContext.pLua);
        m_LuaContext.pLua->addTarget(pPlayer, SELECT_TYPE_PLAYER);
        // now we'll complete the current network message m_pSpellNetworkData
        // that is going to be send from function resolvePlayerSpells
        msg.addLong(SELECT_TYPE_PLAYER);                      // stands for "spell was attached to a player"
        msg.addLong((long) uPlayerId);       // target player id
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAttachToTown
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAttachToTown(u32 uTownId)
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    Town * pTown = m_pServer->getMap()->findTown(uTownId);
    if (pTown != NULL)
    {
        pTown->attachEffect(m_LuaContext.pLua);
        m_LuaContext.pLua->addTarget(pTown, SELECT_TYPE_TOWN);
        // now we'll complete the current network message m_pSpellNetworkData
        // that is going to be send from function resolvePlayerSpells
        msg.addLong(SELECT_TYPE_TOWN);                      // stands for "spell was attached to a town"
        msg.addLong((long) uTownId);       // target town id
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong town id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAttachToTemple
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAttachToTemple(u32 uTempleId)
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    Temple * pTemple = m_pServer->getMap()->findTemple(uTempleId);
    if (pTemple != NULL)
    {
        pTemple->attachEffect(m_LuaContext.pLua);
        m_LuaContext.pLua->addTarget(pTemple, SELECT_TYPE_TEMPLE);
        // now we'll complete the current network message m_pSpellNetworkData
        // that is going to be send from function resolvePlayerSpells
        msg.addLong(SELECT_TYPE_TEMPLE);                      // stands for "spell was attached to a town"
        msg.addLong((long) uTempleId);       // target temple id
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong temple id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAttachToTile
//  This function is called from LUA, during a spell resolution
// -----------------------------------------------------------------
void SpellsSolver::onAttachToTile(CoordsMap pos)
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    MapTile * pTile = m_pServer->getMap()->getTileAt(pos);
    if (pTile != NULL)
    {
        pTile->attachEffect(m_LuaContext.pLua);
        m_LuaContext.pLua->addTarget(pTile, SELECT_TYPE_TILE);
        // now we'll complete the current network message m_pSpellNetworkData
        // that is going to be send from function resolvePlayerSpells
        msg.addLong(SELECT_TYPE_TILE);                      // stands for "spell was attached to a town"
        msg.addLong((long) pos.x);
        msg.addLong((long) pos.y);
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong coords at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onDiscardSpell
//  uSrc is the source from where the spell(s) is/are discarded
//  It can be:
//    - 0: from the deck
//    - 1: from the hand
//    - 2: from active spells
//  If it's from the deck, spell id can be negative: in this case it's not a spell id, but the number of spells to discard from the top of the deck
//  If it's from the hand, spell id can be negative: in this case it's not a spell id, but the number of spells to discard randomly
//  If it's from active spells, player id and spell id can be set to -1: in this case, the current running spell (the one who called this function) is discarded.
// -----------------------------------------------------------------
void SpellsSolver::onDiscardSpell(u8 uSrc, int iPlayerId, int iSpellId)
{
    if (!retrieveLuaContext())
        return;
    Player * pPlayer = NULL;
    Spell * pSpell = NULL;
    if (iPlayerId >= 0)
        pPlayer = m_pSolver->findPlayer(iPlayerId);
    else
        pPlayer = m_LuaContext.pPlayer;
    if (pPlayer == NULL)
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
        return;
    }

    ObjectList * pSrc;
    if (iSpellId >= 0 && uSrc == 0)
    {
        pSrc = pPlayer->m_pDeck;
        pSpell = pPlayer->findSpell(0, iSpellId, pSrc);
    }
    else if (iSpellId >= 0 && uSrc == 1)
    {
        pSrc = pPlayer->m_pHand;
        pSpell = pPlayer->findSpell(0, iSpellId, pSrc);
    }
    else if (iSpellId >= 0 && uSrc == 2)
    {
        pSrc = pPlayer->m_pActiveSpells;
        pSpell = pPlayer->findSpell(0, iSpellId, pSrc);
    }
    else if (iSpellId < 0 && uSrc == 2)
    {
        pSrc = pPlayer->m_pActiveSpells;
        pSpell = (Spell*) m_LuaContext.pLua;
        pSrc->goTo(0, pSpell);
    }
    if (pSpell == NULL && (iSpellId >= 0 || uSrc == 2))
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong spell id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
        return;
    }

    if (pSpell != NULL)
    {
        // Move from source list to discard
        pSrc->deleteCurrent(0, true, true);
        pPlayer->m_pDiscard->addLast(pSpell);
        if (uSrc == 2)  // if spell was active, "disconnect" it
        {
            // Now remove any active effect from targets
            pSpell->removeFromTargets();
            // Remove from global if necessary
            if (pSpell->isGlobal())
                m_pSolver->getGlobalSpells()->deleteObject(pSpell, false);
            // Remove any child effect attached
            int nbChildren = pSpell->getNbChildEffects();
            for (int i = 0; i < nbChildren; i++)
            {
                ChildEffect * pChild = pSpell->getChildEffect(i);
                BaseObject * pTarget = (BaseObject*) pChild->pTargets->getFirst(0);
                while (pTarget != NULL)
                {
                    int type = pChild->pTargets->getCurrentType(0);
                    if (type == SELECT_TYPE_UNIT)
                    {
                        if (((Unit*)pTarget)->detachChildEffect(pChild))
                        {
                            pChild->pTargets->deleteObject(((Unit*)pTarget), true);
                        }
                        else
                        {
                            char sError[128];
                            snprintf(sError, 128, "Lua interaction error: can't detach effect %s.", pSpell->getLocalizedName());
                            m_pServer->getDebug()->notifyErrorMessage(sError);
                        }
                    }
                    pTarget = (BaseObject*) pChild->pTargets->getNext(0);
                }
            }
        }
        // Send message to clients
        NetworkData msg(NETWORKMSG_DISCARD_SPELLS);
        msg.addLong(pPlayer->m_uPlayerId);
        msg.addLong(uSrc);
        msg.addLong(pSpell->getInstanceId());
        m_pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        // Here we have to discard several spells, either from deck or from hand
        NetworkData msg(NETWORKMSG_DISCARD_SPELLS);
        msg.addLong(pPlayer->m_uPlayerId);
        if (uSrc == 0)
        {
            Spell * pSpell = (Spell*) pPlayer->m_pDeck->getFirst(0);
            while (pSpell != NULL)
            {
                if (++iSpellId > 0)
                    break;
                // Move from source list to discard
                Spell * pNext = (Spell*) pPlayer->m_pDeck->deleteCurrent(0, true, true);
                pPlayer->m_pDiscard->addLast(pSpell);
                // Spell is in deck, so it shouldn't have any target to remove!
                msg.addLong(uSrc);
                msg.addLong(pSpell->getInstanceId());
                pSpell = pNext;
            }
        }
        else  // uSrc == 1
        {
            for (int i = 0; i < -iSpellId; i++)
            {
                if (pPlayer->m_pHand->size == 0)
                    break;
                int rnd = getRandom(pPlayer->m_pHand->size);
                Spell * pSpell = (Spell*) pPlayer->m_pHand->goTo(0, rnd);
                assert(pSpell != NULL);
                // Move from source list to discard
                pPlayer->m_pHand->deleteCurrent(0, true, true);
                pPlayer->m_pDiscard->addLast(pSpell);
                // Spell is in deck, so it shouldn't have any target to remove!
                msg.addLong(uSrc);
                msg.addLong(pSpell->getInstanceId());
            }
        }
        m_pServer->sendMessageToAllClients(&msg);
    }
}

// -----------------------------------------------------------------
// Name : onDrawSpell
// -----------------------------------------------------------------
void SpellsSolver::onDrawSpell(u8 uPlayerId, u32 uSpellId)
{
    if (!retrieveLuaContext())
        return;
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        Spell * pSpell = pPlayer->findSpell(0, uSpellId, pPlayer->m_pDeck);
        if (pSpell != NULL)
        {
            // Move from deck to hand
            pPlayer->m_pDeck->deleteCurrent(0, true, true);
            pPlayer->m_pHand->addLast(pSpell);
            // Send message to clients
            NetworkData msg(NETWORKMSG_DRAW_SPELLS);
            msg.addLong(uPlayerId);
            msg.addLong(uSpellId);
            m_pServer->sendMessageToAllClients(&msg);
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: spell %s sent wrong spell id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong player id at resolve stage.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onRecallSpell
// -----------------------------------------------------------------
void SpellsSolver::onRecallSpell(const char * sType, u8 uPlayerId, u32 uSpellId)
{
    if (!retrieveLuaContext())
        return;

    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer != NULL)
    {
        ObjectList * pSrc = NULL;
        if (strcmp(sType, "spell_in_play") == 0)
            pSrc = pPlayer->m_pActiveSpells;
        else if (strcmp(sType, "spell_in_discard") == 0)
            pSrc = pPlayer->m_pDiscard;
        else
        {
            m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid type provided to function onRecallSpell");
            return;
        }

        Spell * pSpell = pPlayer->findSpell(0, uSpellId, pSrc);
        if (pSpell != NULL)
        {
            if (pSrc == pPlayer->m_pActiveSpells)  // if spell was active, "disconnect" it
            {
                // Now remove any active effect from targets
                pSpell->removeFromTargets();
                // Remove from global if necessary
                if (pSpell->isGlobal())
                    m_pSolver->getGlobalSpells()->deleteObject(pSpell, false);
            }
            // Move from active to hand
            pSrc->deleteCurrent(0, true, true);
            pPlayer->m_pHand->addLast(pSpell);
            // Send message to clients
            NetworkData msg(NETWORKMSG_RECALL_SPELLS);
            msg.addLong(uPlayerId);
            msg.addString(sType);
            msg.addLong(uSpellId);
            m_pServer->sendMessageToAllClients(&msg);
        }
        else
        {
            char sError[128];
            snprintf(sError, 128, "Lua interaction error: spell %s sent wrong spell id to function onRecallSpell.", m_LuaContext.pLua->getLocalizedName());
            m_pServer->getDebug()->notifyErrorMessage(sError);
        }
    }
    else
    {
        char sError[128];
        snprintf(sError, 128, "Lua interaction error: spell %s sent wrong player id to function onRecallSpell.", m_LuaContext.pLua->getLocalizedName());
        m_pServer->getDebug()->notifyErrorMessage(sError);
    }
}

// -----------------------------------------------------------------
// Name : onAttachAsGlobal
// -----------------------------------------------------------------
void SpellsSolver::onAttachAsGlobal()
{
    NetworkData msg(NETWORKMSG_LUA_ATTACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    m_pSolver->getGlobalSpells()->addLast(m_LuaContext.pLua);
    if (m_LuaContext.pLua->getType() == LUAOBJECT_SPELL)
        ((Spell*)(m_LuaContext.pLua))->setGlobal();
    // now we'll complete the current network message m_pSpellNetworkData
    // that is going to be send from function resolvePlayerSpells
    msg.addLong(0);                      // stands for "spell was attached as global"
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onDetachFromGlobal
// -----------------------------------------------------------------
void SpellsSolver::onDetachFromGlobal()
{
    NetworkData msg(NETWORKMSG_LUA_DETACHED);
    if (!retrieveLuaContext(0, &msg))
        return;
    m_pSolver->getGlobalSpells()->deleteObject(m_LuaContext.pLua, true);
    msg.addLong(0);                      // stands for "spell was detached from global"
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onSelectTargetThenResolve
// -----------------------------------------------------------------
void SpellsSolver::onSelectTargetThenResolve(u8 uType, u32 uConstraints, const char * sCallback)
{
    NetworkData msg(NETWORKMSG_RESOLVE_NEED_SELECT_TARGET);
    if (!retrieveLuaContext(0, &msg))
        return;
    if (m_LuaContext.pPlayer == NULL || m_LuaContext.pPlayer->m_uPlayerId == 0)
        return;
    m_bPauseResolving = true;
    msg.addLong(uType);
    msg.addLong(uConstraints);
    msg.addString(sCallback);
    m_pServer->sendMessageTo(m_LuaContext.pPlayer->m_uClientId, &msg);
}

// -----------------------------------------------------------------
// Name : onDeactivateSkill
// -----------------------------------------------------------------
void SpellsSolver::onDeactivateSkill(long iPlayerId, long iUnitId, long iSkillId)
{
    if (!retrieveLuaContext())
        return;
    Player * pPlayer = NULL;
    Unit * pUnit = NULL;
    Skill * pSkill = NULL;
    if (iPlayerId >= 0)
        pPlayer = m_pSolver->findPlayer(iPlayerId);
    else
        pPlayer = m_LuaContext.pPlayer;
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid player provided to function onDeactivateSkill");
        return;
    }
    if (iUnitId >= 0)
        pUnit = pPlayer->findUnit(iUnitId);
    else
        pUnit = m_LuaContext.pUnit;
    if (pUnit == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid unit provided to function onDeactivateSkill");
        return;
    }
    bool bIsActive;
    if (iSkillId >= 0)
        pSkill = pUnit->findSkill(iSkillId, &bIsActive);
    else
    {
        if (m_LuaContext.pLua->getType() == LUAOBJECT_SKILL)
            pSkill = pUnit->findSkill(((Skill*)m_LuaContext.pLua)->getInstanceId(), &bIsActive);
        else
        {
            m_pServer->getDebug()->notifyErrorMessage("Error in LUA: function onDeactivateSkill expected current LUA to be a skill");
            return;
        }
    }
    if (pSkill == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid skill provided to function onDeactivateSkill");
        return;
    }
    if (bIsActive)
        pUnit->disableEffect(pSkill);

    NetworkData msg(NETWORKMSG_DEACTIVATE_SKILLS);
    msg.addLong((long) pPlayer->m_uPlayerId);
    msg.addLong((long) pUnit->getId());
    msg.addLong((long) pSkill->getInstanceId());
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onChangeSpellOwner
// -----------------------------------------------------------------
void SpellsSolver::onChangeSpellOwner(const char * sType, u8 uOldOwner, u32 uSpellId, u8 uNewOwner)
{
    Player * pOldOwner = m_pSolver->findPlayer(uOldOwner);
    if (pOldOwner == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid old owner provided to function onChangeSpellOwner");
        return;
    }
    Player * pNewOwner = m_pSolver->findPlayer(uNewOwner);
    if (pNewOwner == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid new owner provided to function onChangeSpellOwner");
        return;
    }
    ObjectList * pSrc = NULL;
    ObjectList * pDst = NULL;
    if (strcmp(sType, "spell_in_hand") == 0)
    {
        pSrc = pOldOwner->m_pHand;
        pDst = pNewOwner->m_pHand;
    }
    else if (strcmp(sType, "spell_in_play") == 0)
    {
        pSrc = pOldOwner->m_pActiveSpells;
        pDst = pNewOwner->m_pActiveSpells;
    }
    else if (strcmp(sType, "spell_in_discard") == 0)
    {
        pSrc = pOldOwner->m_pDiscard;
        pDst = pNewOwner->m_pDiscard;
    }
    else
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid type provided to function onChangeSpellOwner");
        return;
    }

    Spell * pSpell = pOldOwner->findSpell(0, uSpellId, pSrc);
    if (pSpell == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid spell provided to function onChangeSpellOwner");
        return;
    }
    pSrc->deleteCurrent(0, true, true);
    pDst->addLast(pSpell);
    pSpell->setCaster(pNewOwner);
    pSpell->callLuaFunction("onChangedOwner", 0, "i", (int)uNewOwner);  // if a spell stores the caster in a variable for whatever reason, it probably wants to know that the owner changed

    NetworkData msg(NETWORKMSG_CHANGE_SPELL_OWNER);
    msg.addString(sType);
    msg.addLong((long) uOldOwner);
    msg.addLong((long) uSpellId);
    msg.addLong((long) uNewOwner);
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onChangeUnitOwner
// -----------------------------------------------------------------
void SpellsSolver::onChangeUnitOwner(u8 uOldOwner, u32 uUnitId, u8 uNewOwner)
{
    Player * pOldOwner = m_pSolver->findPlayer(uOldOwner);
    if (pOldOwner == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid old owner provided to function onChangeUnitOwner");
        return;
    }
    Player * pNewOwner = m_pSolver->findPlayer(uNewOwner);
    if (pNewOwner == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid new owner provided to function onChangeUnitOwner");
        return;
    }
    Unit * pUnit = pOldOwner->findUnit(uUnitId);
    if (pUnit == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid unit provided to function onChangeUnitOwner");
        return;
    }
    if (pUnit == pOldOwner->getAvatar())
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: unit provided to function onChangeUnitOwner is an avatar");
        return;
    }
    pOldOwner->m_pUnits->deleteObject(pUnit, true, true);
    pNewOwner->m_pUnits->addLast(pUnit);
    pUnit->unsetOrder();
    pUnit->setOwner(pNewOwner->m_uPlayerId);

    NetworkData msg(NETWORKMSG_CHANGE_UNIT_OWNER);
    msg.addLong((long) uOldOwner);
    msg.addLong((long) uUnitId);
    msg.addLong((long) uNewOwner);
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onChangeTownOwner
// -----------------------------------------------------------------
void SpellsSolver::onChangeTownOwner(u32 uTownId, u8 uNewOwner)
{
    Player * pNewOwner = m_pSolver->findPlayer(uNewOwner);
    if (pNewOwner == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid new owner provided to function onChangeTownOwner");
        return;
    }
    Town * pTown = m_pServer->getMap()->findTown(uTownId);
    if (pTown == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid town provided to function onChangeTownOwner");
        return;
    }
    pTown->setOwner(uNewOwner);
    pTown->setBaseValue(STRING_HEROECHANCES, 0);

    NetworkData msg(NETWORKMSG_CHANGE_TOWN_OWNER);
    msg.addLong((long) uTownId);
    msg.addLong((long) uNewOwner);
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onBuildBuilding
// -----------------------------------------------------------------
void SpellsSolver::onBuildBuilding(u32 uTownId, const char * sName)
{
    Town * pTown = m_pServer->getMap()->findTown(uTownId);
    if (pTown == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid town id provided to function onBuildBuilding");
        return;
    }
    pTown->buildBuilding(sName, m_pServer);
}

// -----------------------------------------------------------------
// Name : onProduceMana
// -----------------------------------------------------------------
void SpellsSolver::onProduceMana(int playerId, CoordsMap srcPos, u8 * pMana)
{
    Player * pPlayer = (playerId < 0) ? m_pSolver->getCurrentPlayer() : m_pSolver->findPlayer(playerId);
    // If this mana source is on a temple, corresponding mana is doubled
    MapTile * pTile = m_pServer->getMap()->getTileAt(srcPos);
    if (pTile != NULL)
    {
        Temple * pTemple = (Temple*) pTile->getFirstMapObject(GOTYPE_TEMPLE);
        if (pTemple != NULL)
            pMana[pTemple->getValue(STRING_MANATYPE)] *= 2;
    }
    Mana mana = Mana(pMana[0], pMana[1], pMana[2], pMana[3]);
    pPlayer->setBaseMana(pPlayer->getBaseMana() + mana);

    NetworkData msg(NETWORKMSG_SEND_PLAYER_MANA);
    msg.addLong(0); // Stands for "base mana"
    msg.addLong((long)(pPlayer->m_uPlayerId));
    for (int i = 0; i < 4; i++)
        msg.addLong((long)(pPlayer->getBaseMana(i)));
    m_pServer->sendMessageToAllClients(&msg);
}

//// -----------------------------------------------------------------
//// Name : onUpdateMaxMana
//// -----------------------------------------------------------------
//void SpellsSolver::onUpdateMaxMana(int playerId, CoordsMap srcPos, u8 * pMana)
//{
//  Player * pPlayer = (playerId < 0) ? m_pSolver->getCurrentPlayer() : m_pSolver->findPlayer(playerId);
//  // If this mana source is on a temple, corresponding mana is doubled
//  MapTile * pTile = m_pServer->getMap()->getTileAt(srcPos);
//  if (pTile != NULL)
//  {
//    Temple * pTemple = (Temple*) pTile->getFirstMapObject(GOTYPE_TEMPLE);
//    if (pTemple != NULL)
//      pMana[pTemple->getValue(STRING_MANATYPE)] *= 2;
//  }
//  Mana mana = Mana(pMana[0], pMana[1], pMana[2], pMana[3]);
//  pPlayer->m_ManaMax += mana;
//}

// -----------------------------------------------------------------
// Name : onAddSkillToUnit
// -----------------------------------------------------------------
void SpellsSolver::onAddSkillToUnit(const char * sSkillName, const char * sSkillParams, u8 uPlayerId, u32 uUnitId)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid player id provided to function onAddSkillToUnit");
        return;
    }
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid unit id provided to function onAddSkillToUnit");
        return;
    }
    if (!retrieveLuaContext())
        return;
    Skill * pSkill = new Skill(m_LuaContext.pLua->getObjectEdition(), sSkillName, sSkillParams, m_pServer->getDebug());
    if (pSkill->isLoaded())
    {
        pUnit->addSkill(pSkill);
        NetworkData msg(NETWORKMSG_ADD_SKILL);
        msg.addString(pSkill->getObjectEdition());
        msg.addString(pSkill->getObjectName());
        msg.addLong(pSkill->getInstanceId());
        msg.addString(sSkillParams);
        msg.addLong(uPlayerId);
        msg.addLong(uUnitId);
        m_pServer->sendMessageToAllClients(&msg);
    }
}

// -----------------------------------------------------------------
// Name : onHideSpecialTile
// -----------------------------------------------------------------
void SpellsSolver::onHideSpecialTile(u32 uTileId)
{
    Map * pMap = m_pServer->getMap();
    // Loop through map
    for (int x = 0; x < pMap->getWidth(); x++)
    {
        for (int y = 0; y < pMap->getHeight(); y++)
        {
            MapTile * pTile = pMap->getTileAt(CoordsMap(x, y));
            if (pTile->m_pSpecialTile != NULL && pTile->m_pSpecialTile->getInstanceId() == uTileId)
            {
//        m_pServer->addGarbage(pTile->m_pSpecialTile);
                pTile->hideSpecialTile();
                NetworkData msg(NETWORKMSG_HIDE_SPECTILE);
                msg.addLong(uTileId);
                m_pServer->sendMessageToAllClients(&msg);
                return;
            }
        }
    }
    m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid special tile id provided to function onRemoveSpecialTile");
}

// -----------------------------------------------------------------
// Name : onTeleport
// -----------------------------------------------------------------
void SpellsSolver::onTeleport(MapObject * pMapObj, CoordsMap pos)
{
    MapTile * pTile = m_pServer->getMap()->getTileAt(pos);
    if (pTile == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Error in LUA: invalid position provided to function teleport");
        return;
    }
    pMapObj->setMapPos(pos);
    NetworkData msg(NETWORKMSG_MOVE_MAPOBJ);
    if (pMapObj->getType() & GOTYPE_UNIT)
        msg.addLong(SELECT_TYPE_UNIT);
    else if (pMapObj->getType() & GOTYPE_TOWN)
        msg.addLong(SELECT_TYPE_TOWN);
    else if (pMapObj->getType() & GOTYPE_TEMPLE)
        msg.addLong(SELECT_TYPE_TEMPLE);
    msg.addString(pMapObj->getIdentifiers());
    msg.addLong(pos.x);
    msg.addLong(pos.y);
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onResurrect
// -----------------------------------------------------------------
void SpellsSolver::onResurrect(u8 uPlayerId, u32 uUnitId)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"resurrect\".");
        return;
    }
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: unit not found in function \"resurrect\".");
        return;
    }

    pPlayer->m_pDeadUnits->deleteObject(pUnit, true, true);
    pPlayer->m_pUnits->addLast(pUnit);
    pUnit->setStatus(US_Normal);
    pUnit->setBaseValue(STRING_LIFE, pUnit->getValue(STRING_ENDURANCE));
    NetworkData msg(NETWORKMSG_RESURRECT);
    msg.addString(pUnit->getIdentifiers());
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onRemoveUnit
// -----------------------------------------------------------------
void SpellsSolver::onRemoveUnit(u8 uPlayerId, u32 uUnitId)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"removeUnit\".");
        return;
    }
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: unit not found in function \"removeUnit\".");
        return;
    }

    if (pUnit->getStatus() == US_Normal)
    {
        pPlayer->m_pUnits->deleteObject(pUnit, true, true);
        pPlayer->m_pDeadUnits->addLast(pUnit);
    }
    pUnit->setStatus(US_Removed);
    NetworkData msg(NETWORKMSG_REMOVE_UNIT);
    msg.addString(pUnit->getIdentifiers());
    m_pServer->sendMessageToAllClients(&msg);
}

// -----------------------------------------------------------------
// Name : onAddMagicCircle
// -----------------------------------------------------------------
int SpellsSolver::onAddMagicCircle(Player * pPlayer, CoordsMap pos)
{
    for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
    {
        if (pPlayer->m_MagicCirclePos[i].x < 0)
        {
            pPlayer->m_MagicCirclePos[i] = pos;
            NetworkData msg(NETWORKMSG_ADD_MAGIC_CIRCLE);
            msg.addLong(pPlayer->m_uPlayerId);
            msg.addLong(i);
            msg.addLong(pos.x);
            msg.addLong(pos.y);
            m_pServer->sendMessageToAllClients(&msg);
            return i;
        }
    }
    m_pServer->getDebug()->notifyErrorMessage("Warning: max number of magic circles reached.");
    return -1;
}

// -----------------------------------------------------------------
// Name : onRemoveMagicCircle
// -----------------------------------------------------------------
void SpellsSolver::onRemoveMagicCircle(Player * pPlayer, int iCircle)
{
    if (iCircle >= 0 && iCircle < MAX_MAGIC_CIRCLES)
    {
        pPlayer->m_MagicCirclePos[iCircle].x = -1;
        NetworkData msg(NETWORKMSG_ADD_MAGIC_CIRCLE);
        msg.addLong(pPlayer->m_uPlayerId);
        msg.addLong(iCircle);
        m_pServer->sendMessageToAllClients(&msg);
    }
}

// -----------------------------------------------------------------
// Name : onAddGoldToPlayer
// -----------------------------------------------------------------
void SpellsSolver::onAddGoldToPlayer(u8 uPlayerId, int iAmount)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"addGoldToPlayer\".");
        return;
    }
    pPlayer->m_iWonGold += iAmount;
    m_pServer->sendCustomLogToAll("%$1s_WON_%$2d_GOLD", 0, "pi", uPlayerId, iAmount);
}

// -----------------------------------------------------------------
// Name : onAddSpellToPlayer
// -----------------------------------------------------------------
void SpellsSolver::onAddSpellToPlayer(u8 uPlayerId, const char * sName)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"addSpellToPlayer\".");
        return;
    }
    retrieveLuaContext();
    assert(m_LuaContext.pLua != NULL);
    Spell * pSpell = m_pServer->getFactory()->findSpell(m_LuaContext.pLua->getObjectEdition(), sName);
    if (pSpell == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid spell in function \"addSpellToPlayer\".");
        return;
    }
    pPlayer->m_pWonSpells->addLast(pSpell);
    m_pServer->sendCustomLogToAll("%$1s_WON_SPELL_%$2s", 0, "pS", uPlayerId, pSpell->getObjectEdition(), pSpell->getObjectName());
}

// -----------------------------------------------------------------
// Name : onAddArtifactToPlayer
// -----------------------------------------------------------------
void SpellsSolver::onAddArtifactToPlayer(u8 uPlayerId, const char * sName)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"addArtifactToPlayer\".");
        return;
    }
    retrieveLuaContext();
    assert(m_LuaContext.pLua != NULL);
    Edition * pEdition = m_pServer->getFactory()->findEdition(m_LuaContext.pLua->getObjectEdition());
    assert(pEdition != NULL);
    Artifact * pArtifact = pEdition->findArtifact(sName);
    if (pArtifact == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid artifact in function \"addArtifactToPlayer\".");
        return;
    }
    pPlayer->m_pWonArtifacts->addLast(pArtifact);
    m_pServer->sendCustomLogToAll("%$1s_WON_ARTIFACT_%$2s", 0, "pA", uPlayerId, pArtifact->getEdition(), sName);
}

// -----------------------------------------------------------------
// Name : onAddAvatarToPlayer
// -----------------------------------------------------------------
void SpellsSolver::onAddAvatarToPlayer(u8 uPlayerId, const char * sName)
{
    Player * pPlayer = m_pSolver->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid player in function \"addShahamahToPlayer\".");
        return;
    }
    retrieveLuaContext();
    assert(m_LuaContext.pLua != NULL);
    AvatarData * pAvatar = (AvatarData*) m_pServer->getFactory()->getUnitData(m_LuaContext.pLua->getObjectEdition(), sName);
    if (pAvatar == NULL)
    {
        m_pServer->getDebug()->notifyErrorMessage("Lua interaction error: invalid shahmah in function \"addShahamahToPlayer\".");
        return;
    }
    pPlayer->m_pWonAvatars->addLast(pAvatar);
    m_pServer->sendCustomLogToAll("%$1s_WON_AVATAR_%$2s", 0, "pU", uPlayerId, pAvatar->m_sEdition, sName);
}
