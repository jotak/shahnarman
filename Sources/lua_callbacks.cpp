// -----------------------------------------------------------------
// LUA callbacks functions
// -----------------------------------------------------------------
#include "lua_callbacks.h"
#include "lua_callbacks_utils.h"
#include "Data/DataFactory.h"
#include "GUIClasses/guiLabel.h"
#include "Server/AISolver.h"

//------------------------------------------------------------------------------
// Spell_selectTarget
//  Input : target_type, constraints
//  No output
//------------------------------------------------------------------------------
int LUA_selectTarget(char * sText, bool bThenResolve)
{
  wchar_t sType[64], sConstraints[128] = L"";
  g_bLuaSelectOnResolve = bThenResolve;
  Player * pCaster = g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellDialog()->getCurrentCaster();
  assert(pCaster != NULL);
  if (g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_DECK || g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_HAND || g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_PLAY || g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_DISCARD)
  {
    if (!bThenResolve && (g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_DECK || g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_HAND))
    {
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua interaction error: trying to call \"selectTarget\" for deck or hand spell outside resolve phase. Use \"selectTargetThenResolve\" during resolve phase instead!");
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return 0;
    }
    ObjectList * pList = new ObjectList(false);
    Player * pPlayer = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
      if (!((pPlayer != pCaster && (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED))
          || (pPlayer == pCaster && (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT))))
        pList->addLast(pPlayer);
      pPlayer = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
    }
    if (g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_DECK)
    {
      i18n->getText(L"(A)_SPELL_IN_DECK", sType, 64);
      g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showPlayersSpells(pList, 2, clbkSelectTarget_cancelSelection);
    }
    else if (g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_HAND)
    {
      i18n->getText(L"(A)_SPELL_IN_HAND", sType, 64);
      g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showPlayersSpells(pList, 1, clbkSelectTarget_cancelSelection);
    }
    else if (g_uLuaSelectTargetType == SELECT_TYPE_SPELL_IN_PLAY)
    {
      i18n->getText(L"(A)_SPELL_IN_PLAY", sType, 64);
      g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showPlayersSpells(pList, 0, clbkSelectTarget_cancelSelection);
    }
    else  // discard
    {
      i18n->getText(L"(A)_SPELL_IN_DISCARD", sType, 64);
      g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showPlayersSpells(pList, 3, clbkSelectTarget_cancelSelection);
    }
    delete pList;
    g_pMainGameRoot->m_pLocalClient->getInterface()->setTargetMode(clbkSelectTarget_OnMouseOverInterface, clbkSelectTarget_OnClickInterface);
  }
  else if (g_uLuaSelectTargetType == SELECT_TYPE_PLAYER)
  {
    i18n->getText(L"(A)_PLAYER", sType, 64);
    ObjectList * pList = new ObjectList(false);
    Player * pPlayer = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
      if (!((pPlayer != pCaster && (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED))
          || (pPlayer == pCaster && (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT))))
        pList->addLast(pPlayer);
      pPlayer = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
    }
    g_pMainGameRoot->m_pLocalClient->getInterface()->getPlayerSelectorDialog()->showPlayers(pList, clbkSelectTarget_cancelSelection);
    delete pList;
    g_pMainGameRoot->m_pLocalClient->getInterface()->setTargetMode(clbkSelectTarget_OnMouseOverInterface, clbkSelectTarget_OnClickInterface);
  }
  else
  {
    if (g_uLuaSelectTargetType == SELECT_TYPE_TILE)
      i18n->getText(L"(A)_TILE", sType, 64);
    else if (g_uLuaSelectTargetType == SELECT_TYPE_UNIT)
      i18n->getText(L"(A)_UNIT", sType, 64);
    else if (g_uLuaSelectTargetType == SELECT_TYPE_DEAD_UNIT)
      i18n->getText(L"(A)_DEAD_UNIT", sType, 64);
    else if (g_uLuaSelectTargetType == SELECT_TYPE_TOWN)
      i18n->getText(L"(A)_TOWN", sType, 64);
    else if (g_uLuaSelectTargetType == SELECT_TYPE_BUILDING)
      i18n->getText(L"(A)_BUILDING", sType, 64);
    else if (g_uLuaSelectTargetType == SELECT_TYPE_TEMPLE)
      i18n->getText(L"(A)_TEMPLE", sType, 64);
    g_pMainGameRoot->m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeSelectCustomTarget, clbkSelectTarget_OnMouseOverGameboard, clbkSelectTarget_OnClickGameboard);
    g_pMainGameRoot->m_pLocalClient->getInterface()->setTargetMode(clbkSelectTarget_OnMouseOverInterface, clbkSelectTarget_OnClickInterface);
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_INRANGE)
      g_pMainGameRoot->m_pLocalClient->getGameboard()->highlightMagicCirclesForPlayer(pCaster);
  }
  g_pMainGameRoot->m_pLocalClient->getPlayerManager()->enableEOT(false);

  // Set status text
  wchar_t sStatus[LABEL_MAX_CHARS];
  wchar_t sBuf[LABEL_MAX_CHARS];
  wchar_t s2P[8];
  i18n->getText(L"2P", s2P, 8);
  swprintf_s(sStatus, LABEL_MAX_CHARS, L"%s%s", pCaster->getAvatarName(), s2P);
  if (sText != NULL && strcmp(sText, "") != 0)
  {
    strtow(sBuf, LABEL_MAX_CHARS, sText);
    wsafecat(sStatus, LABEL_MAX_CHARS, sBuf);
  }
  else
  {
    wchar_t sConstr[64];
    wchar_t sSep[4] = L"";
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_INRANGE)
    {
      wsafecat(sConstraints, 128, i18n->getText(L"IN_RANGE", sConstr, 64));
      wsafecpy(sSep, 4, L", ");
    }
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED)
    {
      wsafecat(sConstraints, 128, sSep);
      wsafecat(sConstraints, 128, i18n->getText(L"FRIEND(UNIT)", sConstr, 64));
      wsafecpy(sSep, 4, L", ");
    }
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT)
    {
      wsafecat(sConstraints, 128, sSep);
      wsafecat(sConstraints, 128, i18n->getText(L"ENEMY(UNIT)", sConstr, 64));
    }
    wchar_t sSrcType[64];
    wchar_t sSrc[NAME_MAX_CHARS];
    if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
    {
      i18n->getText(L"(THE)_SPELL", sSrcType, 64);
      wsafecpy(sSrc, NAME_MAX_CHARS, g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSpellBeingCast()->getLocalizedName());
    }
    else
    {
      i18n->getText(L"(THE)_SKILL", sSrcType, 64);
      wsafecpy(sSrc, NAME_MAX_CHARS, g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSkillBeingActivated()->sName);
    }
    void * pArgs[4] = { sSrcType, sSrc, sType, sConstraints };
    i18n->getText(L"%$1s_%$2s_NEEDS_SELECT_TARGET_%$3s_WITH_CONSTRAINTS_%$4s", sBuf, LABEL_MAX_CHARS, pArgs);
    wsafecat(sStatus, LABEL_MAX_CHARS, sBuf);
  }
  g_pMainGameRoot->m_pLocalClient->getInterface()->getStatusDialog()->showStatus(sStatus);

  return 0;
}

int LUA_selectTarget(lua_State * pState)
{
  int nbParams = checkNumberOfParams(pState, 2, 4, L"selectTarget");
  if (nbParams < 0)
    return 0;
  char sType[64];
  char sConstraints[128];
  char sText[256] = "";
  char sCallback[128] = "";
  strcpy_s(sType, 64, lua_tostring(pState, 1));
  strcpy_s(sConstraints, 128, lua_tostring(pState, 2));
  if (nbParams >= 3)
    strcpy_s(sText, 256, lua_tostring(pState, 3));
  if (nbParams == 4)
    strcpy_s(sCallback, 128, lua_tostring(pState, 4));

  if (!readTargetData(sType, sConstraints, sCallback, L"selectTarget"))
    return 0;

  g_pLuaStateForTarget = pState;

  if (g_bLuaEvaluationMode) {
    AILuaSolver::EvaluationTargetInfo * pEvalTarget = new AILuaSolver::EvaluationTargetInfo(g_uLuaSelectTargetType, g_uLuaSelectConstraints);
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    pServer->getSolver()->getAISolver()->addEvaluationTarget(pEvalTarget);
    return 0;
  }

  return LUA_selectTarget(sText, false);
}

int LUA_selectTargetThenResolve(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"selectTargetThenResolve");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"selectTargetThenResolve"))
    return 0;

  char sType[64];
  char sConstraints[128];
  char sCallback[128];
  strcpy_s(sType, 64, lua_tostring(pState, 1));
  strcpy_s(sConstraints, 128, lua_tostring(pState, 2));
  strcpy_s(sCallback, 128, lua_tostring(pState, 3));

  if (!readTargetData(sType, sConstraints, "", L"selectTargetThenResolve"))
    return 0;

  g_pLuaStateForTarget = pState;
  wchar_t sCallbackW[128];
  strtow(sCallbackW, 128, sCallback);
  pServer->getSolver()->getSpellsSolver()->onSelectTargetThenResolve(g_uLuaSelectTargetType, g_uLuaSelectConstraints, sCallbackW);
  return 0;
}

int LUA_damageUnit(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"damageUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"damageUnit"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  u8 damages = (u8)lua_tonumber(pState, 3);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
    if (pPlayer != NULL) {
      Unit * pUnit = pPlayer->findUnit(uUnitId);
      if (pUnit != NULL) {
        pServer->getSolver()->getAISolver()->addCurrentSpellInterestForDamages(pPlayer, pUnit, damages);
      }
    }
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDamageUnit(uPlayerId, uUnitId, damages);
  return 0;
}

int LUA_summon(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"summon");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 4, L"summon"))
    return 0;

  u8 uPlayer = (u8) lua_tonumber(pState, 1);
  CoordsMap mp;
  mp.x = (int)lua_tonumber(pState, 2);
  mp.y = (int)lua_tonumber(pState, 3);
  const char * pName = lua_tostring(pState, 4);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    Player * pPlayer = pServer->getSolver()->findPlayer(uPlayer);
    if (pPlayer != NULL) {
      int iSign = (uPlayer == pServer->getSolver()->getAISolver()->getCurrentPlayer()->m_uPlayerId) ? 1 : -1;
      float fUnitInterest = pServer->getSolver()->getAISolver()->evaluateUnit(mp, pName, uPlayer);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(iSign * fUnitInterest);
    }
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddUnit(mp, pName, uPlayer);
  return 0;
}

int LUA_attachToUnit(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachToUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"attachToUnit"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachToUnit(uPlayerId, uUnitId);
  return 0;
}

int LUA_attachToPlayer(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachToPlayer");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"attachToPlayer"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachToPlayer(uPlayerId);
  return 0;
}

int LUA_attachToTown(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachToTown");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"attachToTown"))
    return 0;

  u32 uTownId = (u32)lua_tonumber(pState, 1);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachToTown(uTownId);
  return 0;
}

int LUA_attachToTemple(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachToTemple");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"attachToTemple"))
    return 0;

  u32 uTempleId = (u32)lua_tonumber(pState, 1);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachToTemple(uTempleId);
  return 0;
}

int LUA_attachToTile(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachToTile");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"attachToTile"))
    return 0;

  int x = (int) lua_tonumber(pState, 1);
  int y = (int) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachToTile(CoordsMap(x, y));
  return 0;
}

int LUA_addChildEffectToUnit(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addChildEffectToUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"addChildEffectToUnit"))
    return 0;

  int iEffectId = (int)lua_tonumber(pState, 1) - 1;
  u8 uPlayerId = (u8)lua_tonumber(pState, 2);
  u32 uUnitId = (u32)lua_tonumber(pState, 3);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddChildEffectToUnit(iEffectId, uPlayerId, uUnitId);
  return 0;
}

int LUA_removeChildEffectFromUnit(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"removeChildEffectFromUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"removeChildEffectFromUnit"))
    return 0;

  int iEffectId = (int)lua_tonumber(pState, 1) - 1;
  u8 uPlayerId = (u8)lua_tonumber(pState, 2);
  u32 uUnitId = (u32)lua_tonumber(pState, 3);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onRemoveChildEffectFromUnit(iEffectId, uPlayerId, uUnitId);
  return 0;
}

int LUA_addChildEffectToTown(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addChildEffectToTown");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"addChildEffectToTown"))
    return 0;

  int iEffectId = (int)lua_tonumber(pState, 1) - 1;
  u32 uTownId = (u32)lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddChildEffectToTown(iEffectId, uTownId);
  return 0;
}

int LUA_removeChildEffectFromTown(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"removeChildEffectFromTown");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"removeChildEffectFromTown"))
    return 0;

  int iEffectId = (int)lua_tonumber(pState, 1) - 1;
  u32 uTownId = (u32)lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onRemoveChildEffectFromTown(iEffectId, uTownId);
  return 0;
}

int getUnitDataBaseOrNot(lua_State * pState, bool bBase, wchar_t * sFuncName)
{
  int nbParams = lua_gettop(pState);
  if (nbParams < 3)
  {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error: \"%s\" should receive at least 3 arguments.", sFuncName);
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return 0;
  }

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  Player * pPlayer = getServerOrLocalPlayer(uPlayerId);
  if (pPlayer != NULL)
  {
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit != NULL)
    {
      for (int iParam = 0; iParam < nbParams - 2; iParam++)
      {
        const char * varname = lua_tostring(pState, nbParams - iParam);
        wchar_t sName[64];
        strtow(sName, 64, varname);
        bool bFound;
        double val = pUnit->getValue(sName, bBase, &bFound);
        if (bFound)
          lua_pushnumber(pState, val);
        else
        {
          wchar_t sError[512] = L"";
          swprintf_s(sError, 512, L"Lua interaction error in function %s: variable '%s' doesn't exist.", sFuncName, sName);
          g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
          return iParam;
        }
        return nbParams - 2;
      }
    }
    else {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"Lua interaction error in function %s: unit not found.", sFuncName);
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
  }
  else {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error in function %s: player not found.", sFuncName);
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
  }
  return 0;
}

int LUA_getUnitData(lua_State * pState)
{
  return getUnitDataBaseOrNot(pState, false, L"getUnitData");
}

int LUA_getUnitBaseData(lua_State * pState)
{
  return getUnitDataBaseOrNot(pState, true, L"getUnitBaseData");
}

int LUA_setUnitData(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setUnitData");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 4, L"setUnitData"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  const char * varname = lua_tostring(pState, 3);
  double value = lua_tonumber(pState, 4);
  Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
  if (pPlayer != NULL)
  {
    Unit * pUnit = (Unit*) pPlayer->findUnit(uUnitId);
    if (pUnit != NULL)
    {
      wchar_t sName[64];
      strtow(sName, 64, varname);

      if (g_bLuaEvaluationMode) {
        Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
        assert(pServer != NULL);
        bool bFound = true;
        long iCurVal = pUnit->getValue(sName, true, &bFound);
        if (!bFound)
          iCurVal = 0;
        int iSign = (uPlayerId == pServer->getSolver()->getAISolver()->getCurrentPlayer()->m_uPlayerId) ? 1 : -1;
        float fCoef = 0.0f;
        if (_wcsicmp(STRING_MELEE, sName) == 0)
          fCoef = AI_INTEREST_MELEE;
        else if (_wcsicmp(STRING_RANGE, sName) == 0)
          fCoef = AI_INTEREST_RANGE;
        else if (_wcsicmp(STRING_ARMOR, sName) == 0)
          fCoef = AI_INTEREST_ARMOR;
        else if (_wcsicmp(STRING_ENDURANCE, sName) == 0)
          fCoef = AI_INTEREST_ENDURANCE;
        else if (_wcsicmp(STRING_SPEED, sName) == 0)
          fCoef = AI_INTEREST_SPEED;
        else if (_wcsicmp(STRING_LIFE, sName) == 0)
          fCoef = AI_INTEREST_LIFE;
        pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(iSign * (value - (double)iCurVal) * fCoef);
        return 0;
      }

      if (!pUnit->setBaseValue(sName, value))
        pServer->getDebug()->notifyErrorMessage(L"Lua interaction error in function setUnitData: variable not found.");
    }
    else
      pServer->getDebug()->notifyErrorMessage(L"Lua interaction error in function setUnitData: unit not found.");
  }
  else
    pServer->getDebug()->notifyErrorMessage(L"Lua interaction error in function setUnitData: player not found.");
  return 0;
}

int LUA_produceMana(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"produceMana");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 7, L"produceMana"))
    return 0;

  int playerId = (int) lua_tonumber(pState, 1);
  int x = (int) lua_tonumber(pState, 2);
  int y = (int) lua_tonumber(pState, 3);
  u8 pMana[4];
  pMana[0] = (u8) lua_tonumber(pState, 4);
  pMana[1] = (u8) lua_tonumber(pState, 5);
  pMana[2] = (u8) lua_tonumber(pState, 6);
  pMana[3] = (u8) lua_tonumber(pState, 7);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    int iSign = (playerId == pServer->getSolver()->getAISolver()->getCurrentPlayer()->m_uPlayerId) ? 1 : -1;
    pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(iSign * (pMana[0] + pMana[1] + pMana[2] + pMana[3]) * 2.0f);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onProduceMana(playerId, CoordsMap(x, y), pMana);
  return 0;
}

int LUA_getAttacker(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getAttacker");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getAttacker"))
    return 0;

  Unit * pUnit = pServer->getSolver()->getAttacker();
  if (pUnit == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getAttacker: Attacker is null.");
    return 0;
  }
  char str[16];
  sprintf_s(str, 16, "%d", (int) pUnit->getOwner());
  lua_pushstring(pState, str);
  sprintf_s(str, 16, "%ld", (long) pUnit->getId());
  lua_pushstring(pState, str);
  return 2;
}

int LUA_getDefender(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getDefender");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getDefender"))
    return 0;

  Unit * pUnit = pServer->getSolver()->getDefender();
  if (pUnit == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getDefender: Defender is null.");
    return 0;
  }
  char str[16];
  sprintf_s(str, 16, "%d", (int) pUnit->getOwner());
  lua_pushstring(pState, str);
  sprintf_s(str, 16, "%ld", (long) pUnit->getId());
  lua_pushstring(pState, str);
  return 2;
}

int LUA_getAttackerLife(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getAttackerLife");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getAttackerLife"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iAttackerLife);
  return 1;
}

int LUA_setAttackerLife(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setAttackerLife");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setAttackerLife"))
    return 0;

  int iNewLife = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewLife != pServer->getSolver()->m_iAttackerLife) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(&iNewLife, NULL, NULL, NULL, NULL, NULL)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iAttackerLife = iNewLife;
  return 0;
}

int LUA_getDefenderLife(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getDefenderLife");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getDefenderLife"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iDefenderLife);
  return 1;
}

int LUA_setDefenderLife(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setDefenderLife");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setDefenderLife"))
    return 0;
  
  int iNewLife = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewLife != pServer->getSolver()->m_iDefenderLife) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(NULL, &iNewLife, NULL, NULL, NULL, NULL)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iDefenderLife = iNewLife;
  return 0;
}

int LUA_getAttackerDamages(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getAttackerDamages");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getAttackerDamages"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iAttackerDamages);
  return 1;
}

int LUA_setAttackerDamages(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setAttackerDamages");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setAttackerDamages"))
    return 0;

  int iNewDamages = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewDamages != pServer->getSolver()->m_iAttackerDamages) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(NULL, NULL, NULL, NULL, &iNewDamages, NULL)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iAttackerDamages = iNewDamages;
  return 0;
}

int LUA_getDefenderDamages(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getDefenderDamages");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getDefenderDamages"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iDefenderDamages);
  return 1;
}

int LUA_setDefenderDamages(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setDefenderDamages");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setDefenderDamages"))
    return 0;

  int iNewDamages = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewDamages != pServer->getSolver()->m_iDefenderDamages) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(NULL, NULL, NULL, NULL, NULL, &iNewDamages)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iDefenderDamages = iNewDamages;
  return 0;
}

int LUA_getAttackerArmor(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getAttackerArmor");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getAttackerArmor"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iAttackerArmor);
  return 1;
}

int LUA_setAttackerArmor(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setAttackerArmor");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setAttackerArmor"))
    return 0;

  int iNewArmor = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewArmor != pServer->getSolver()->m_iAttackerArmor) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(NULL, NULL, &iNewArmor, NULL, NULL, NULL)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iAttackerArmor = iNewArmor;
  return 0;
}

int LUA_getDefenderArmor(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"getDefenderArmor");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 0, L"getDefenderArmor"))
    return 0;
  lua_pushnumber(pState, (double) pServer->getSolver()->m_iDefenderArmor);
  return 1;
}

int LUA_setDefenderArmor(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setDefenderArmor");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"setDefenderArmor"))
    return 0;

  int iNewArmor = (int) lua_tonumber(pState, 1);
  if (g_bLuaEvaluationMode) {
    if (iNewArmor != pServer->getSolver()->m_iDefenderArmor) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      pServer->getSolver()->getAISolver()->addInterestForCurrentSpell(
              pServer->getSolver()->getAISolver()->evaluateBattleModifications(NULL, NULL, NULL, &iNewArmor, NULL, NULL)
      );
    }
    return 0;
  }

  pServer->getSolver()->m_iDefenderArmor = iNewArmor;
  return 0;
}

int LUA_discardActiveSpell(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"discardActiveSpell");
  if (pServer == NULL)
    return 0;
  int nbParams = checkNumberOfParams(pState, 0, 2, L"discardActiveSpell");
  if (nbParams < 0)
    return 0;
  long iPlayerId = -1, iSpellId = -1;
  if (nbParams == 2)
  {
    iPlayerId = (long) lua_tonumber(pState, 1);
    iSpellId = (long) lua_tonumber(pState, 2);
  }

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDiscardSpell(2, iPlayerId, iSpellId);
  return 0;
}

int LUA_discardDeckSpell(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"discardDeckSpell");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"discardDeckSpell"))
    return 0;
  long iPlayerId = (long) lua_tonumber(pState, 1);
  long iSpellId = (long) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDiscardSpell(0, iPlayerId, iSpellId);
  return 0;
}

int LUA_discardHandSpell(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"discardHandSpell");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"discardHandSpell"))
    return 0;
  long iPlayerId = (long) lua_tonumber(pState, 1);
  long iSpellId = (long) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDiscardSpell(1, iPlayerId, iSpellId);
  return 0;
}

int LUA_drawSpell(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"drawSpell");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"drawSpell"))
    return 0;
  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uSpellId = (u32)lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDrawSpell(uPlayerId, uSpellId);
  return 0;
}

int LUA_attachAsGlobal(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"attachAsGlobal");
  if (pServer == NULL)
    return 0;

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAttachAsGlobal();
  return 0;
}

int LUA_detachFromGlobal(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"detachFromGlobal");
  if (pServer == NULL)
    return 0;

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDetachFromGlobal();
  return 0;
}

int LUA_getUnitAlignment(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 2, L"getUnitAlignment"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  Player * pPlayer = getServerOrLocalPlayer(uPlayerId);
  if (pPlayer != NULL)
  {
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit != NULL)
    {
      int iAlign = pUnit->getValue(STRING_ALIGNMENT);
      lua_pushnumber(pState, (iAlign & ALIGNMENT_LIFE) ? 1 : 0);
      lua_pushnumber(pState, (iAlign & ALIGNMENT_LAW) ? 1 : 0);
      lua_pushnumber(pState, (iAlign & ALIGNMENT_DEATH) ? 1 : 0);
      lua_pushnumber(pState, (iAlign & ALIGNMENT_CHAOS) ? 1 : 0);
      return 4;
    }
    else
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getUnitAlignment: unit not found.");
  }
  else
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getUnitAlignment: player not found.");
  return 0;
}

int LUA_askForExtraMana(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 5, L"askForExtraMana"))
    return 0;

  char sDesc[256];
  strcpy_s(sDesc, 256, lua_tostring(pState, 1));
  wchar_t sDescW[256];
  strtow(sDescW, 256, sDesc);
  u16 mana = (u16) lua_tonumber(pState, 2);
  int min = (int) lua_tonumber(pState, 3);
  int max = (int) lua_tonumber(pState, 4);
  char sCallback[128];
  strcpy_s(sCallback, 128, lua_tostring(pState, 5));
  wchar_t sCallbackW[128];
  strtow(sCallbackW, 128, sCallback);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  g_pMainGameRoot->m_pLocalClient->getInterface()->askForExtraMana(sDescW, mana, min, max, sCallbackW, g_uLuaCurrentObjectType);
  return 0;
}

int LUA_addResolveParameter(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 1, L"addResolveParameter"))
    return 0;

  char sParam[256];
  strcpy_s(sParam, 256, lua_tostring(pState, 1));
  wchar_t sParamW[256];
  strtow(sParamW, 256, sParam);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
    g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSpellBeingCast()->addResolveParameters(sParamW);
  else if (g_uLuaCurrentObjectType == LUAOBJECT_SKILL)
    g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSkillBeingActivated()->addResolveParameters(sParamW);
  return 0;
}

int LUA_dispatchToClients(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"dispatchToClients");
  if (pServer == NULL)
    return 0;

  if (g_bLuaEvaluationMode) {
    return 0;
  }

  int nbParams = lua_gettop(pState);
  if (nbParams < 2)
  {
    wchar_t sText[512];
    swprintf_s(sText, 512, L"Lua interaction error: \"dispatchToClients\" should receive at least 2 arguments (client callback name and at least 1 data to dispatch).");
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sText);
    return 0;
  }

  NetworkData msg(NETWORKMSG_CUSTOM_LUA_UPDATE);
  if (!pServer->getSolver()->getSpellsSolver()->retrieveLuaContext(0, &msg))
    return 0;

  // Get callback name
  char sCallback[NAME_MAX_CHARS];
  strcpy_s(sCallback, NAME_MAX_CHARS, lua_tostring(pState, 1));
  wchar_t sCallbackW[NAME_MAX_CHARS];
  strtow(sCallbackW, NAME_MAX_CHARS, sCallback);
  msg.addString(sCallbackW);

  for (int i = 0; i < nbParams - 1; i++)
  {
    if (lua_type(pState, i+2) == LUA_TNUMBER)
    {
      msg.addLong(0);
      msg.addDouble(lua_tonumber(pState, i+2));
    }
    else if (lua_isstring(pState, i+2))
    {
      msg.addLong(1);
      char sStr[256];
      strcpy_s(sStr, 256, lua_tostring(pState, i+2));
      wchar_t sStrW[256];
      strtow(sStrW, 256, sStr);
      msg.addString(sStrW);
    }
  }
  pServer->sendMessageToAllClients(&msg);

  return 0;
}

int LUA_getPlayersList(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 0, L"getPlayersList"))
    return 0;
  char str[16];
  PlayerManagerAbstract * pMngr = getServerOrLocalPlayerManager();
  int count = 0;
  if (0 == lua_checkstack(pState, pMngr->getPlayersCount()+1))
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: getPlayersList, stack too small!");
    return 0;
  }
  Player * pPlayer = pMngr->getFirstPlayerAndNeutral(0);
  while (pPlayer != NULL)
  {
    count++;
    sprintf_s(str, 16, "%d", (int) pPlayer->m_uPlayerId);
    lua_pushstring(pState, str);
    pPlayer = pMngr->getNextPlayerAndNeutral(0);
  }
  return count;
}

int LUA_getUnitsList(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 1, L"getUnitsList"))
    return 0;

  int player = (int) lua_tonumber(pState, 1);
  Player * pPlayer = getServerOrLocalPlayer(player);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid player provided to function getUnitsList");
    return 0;
  }
  char str[16];
  int count = 0;
  if (0 == lua_checkstack(pState, pPlayer->m_pUnits->size))
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: getUnitsList, stack too small!");
    return 0;
  }
  Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(0);
  while (pUnit != NULL)
  {
    count++;
    sprintf_s(str, 16, "%ld", (long) pUnit->getId());
    lua_pushstring(pState, str);
    pUnit = (Unit*) pPlayer->m_pUnits->getNext(0);
  }
  return count;
}

int LUA_getSkillsList(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 2, L"getSkillsList"))
    return 0;

  int player = (int) lua_tonumber(pState, 1);
  Player * pPlayer = getServerOrLocalPlayer(player);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid player provided to function getSkillsList");
    return 0;
  }

  u32 unit = (u32) lua_tonumber(pState, 2);
  Unit * pUnit = pPlayer->findUnit(unit);
  if (pUnit == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid unit provided to function getSkillsList");
    return 0;
  }
  char str[16];
  int count = 0;
  if (0 == lua_checkstack(pState, pUnit->getAllEffects()->size))
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: getSkillsList, stack too small!");
    return 0;
  }
  LuaObject * pLua = pUnit->getFirstEffect(0);
  while (pLua != NULL)
  {
    if (pLua->getType() == LUAOBJECT_SKILL)
    {
      count++;
      sprintf_s(str, 16, "%ld", (long) pLua->getInstanceId());
      lua_pushstring(pState, str);
    }
    pLua = pUnit->getNextEffect(0);
  }
  return count;
}

int LUA_getTownsList(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 0, L"getTownsList"))
    return 0;

  char str[16];
  Map * pMap = getServerOrLocalMap();
  int count = 0;
  Town * pTown = pMap->getFirstTown();
  while (pTown != NULL)
  {
    count++;
    sprintf_s(str, 16, "%ld", (long) pTown->getId());
    if (0 == lua_checkstack(pState, count))
    {
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: getTownsList, stack too small!");
      return 0;
    }
    lua_pushstring(pState, str);
    pTown = pMap->getNextTown();
  }
  return count;
}

int LUA_deactivateSkill(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"deactivateSkill");
  if (pServer == NULL)
    return 0;
  int nbParams = checkNumberOfParams(pState, 0, 3, L"deactivateSkill");
  if (nbParams != 0 && !checkNumberOfParams(pState, 3, L"deactivateSkill"))
    return 0;
  long iPlayerId = -1, iUnitId = -1, iSkillId = -1;
  if (nbParams == 3)
  {
    iPlayerId = (long) lua_tonumber(pState, 1);
    iUnitId = (long) lua_tonumber(pState, 2);
    iSkillId = (long) lua_tonumber(pState, 3);
  }

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onDeactivateSkill(iPlayerId, iUnitId, iSkillId);

  return 0;
}

int LUA_changeSpellOwner(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"changeSpellOwner");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 4, L"changeSpellOwner"))
    return 0;

  const char * type = lua_tostring(pState, 1);
  wchar_t typeW[64];
  strtow(typeW, 64, type);
  int player = (int) lua_tonumber(pState, 2);
  int spell = (int) lua_tonumber(pState, 3);
  int newowner = (int) lua_tonumber(pState, 4);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onChangeSpellOwner(typeW, player, spell, newowner);

  return 0;
}

int LUA_changeUnitOwner(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"changeUnitOwner");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"changeUnitOwner"))
    return 0;

  int player = (int) lua_tonumber(pState, 1);
  int unit = (int) lua_tonumber(pState, 2);
  int newowner = (int) lua_tonumber(pState, 3);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onChangeUnitOwner(player, unit, newowner);
  return 0;
}

int LUA_changeTownOwner(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"changeTownOwner");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"changeTownOwner"))
    return 0;

  int town = (int) lua_tonumber(pState, 1);
  int newowner = (int) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onChangeTownOwner(town, newowner);
  return 0;
}

int LUA_getTownData(lua_State * pState)
{
  int nbParams = checkNumberOfParams(pState, 1, 2, L"getTownData");
  if (nbParams < 0)
    return 0;

  u32 uTownId = (u32)lua_tonumber(pState, 1);
  Town * pTown = NULL;
  // If server is resolving, take information from server; else from client
  Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
  if (pServer != NULL && pServer->isResolving())
    pTown = pServer->getMap()->findTown(uTownId);
  else
    pTown = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap()->findTown(uTownId);

  if (pTown != NULL)
  {
    if (nbParams == 2)
    {
      // Only 1 data is requested
      const char * varname = lua_tostring(pState, 2);
      wchar_t sName[64];
      strtow(sName, 64, varname);
      bool bFound;
      double val = pTown->getValue(sName, false, &bFound);
      if (bFound)
        lua_pushnumber(pState, val);
      else
      {
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getTownData: variable not found.");
        return 0;
      }
      return 1;
    }
    else
    {
      // Send basic town data
      char str[64];
      wtostr(str, 64, pTown->getName());
      lua_pushstring(pState, str);
      wtostr(str, 64, pTown->getEthnicityId());
      lua_pushstring(pState, str);
      lua_pushnumber(pState, pTown->getOwner());
      return 3;
    }
  }
  else
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getTownData: town not found.");
  return 0;
}

int LUA_townHasBuilding(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 2, L"townHasBuilding"))
    return 0;

  u32 uTownId = (u32)lua_tonumber(pState, 1);
  Town * pTown = NULL;
  // If server is resolving, take information from server; else from client
  Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
  if (pServer != NULL && pServer->isResolving())
    pTown = pServer->getMap()->findTown(uTownId);
  else
    pTown = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap()->findTown(uTownId);

  if (pTown != NULL)
  {
    const char * varname = lua_tostring(pState, 2);
    wchar_t sName[64];
    strtow(sName, 64, varname);
    int it = pTown->getBuildingsList()->getIterator();
    Building * pBuild = pTown->getFirstBuilding(it);
    while (pBuild != NULL)
    {
      if (wcscmp(pBuild->getObjectName(), sName) == 0 && pBuild->isBuilt())
      {
        pTown->getBuildingsList()->releaseIterator(it);
        lua_pushnumber(pState, 1);
        return 1;
      }
      pBuild = pTown->getNextBuilding(it);
    }
    pTown->getBuildingsList()->releaseIterator(it);
    lua_pushnumber(pState, 0);
    return 1;
  }
  else
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function townHasBuilding: town not found.");
  return 0;
}

int LUA_buildBuilding(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"buildBuilding");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"buildBuilding"))
    return 0;

  u32 uTownId = (u32)lua_tonumber(pState, 1);
  const char * varname = lua_tostring(pState, 2);
  wchar_t sName[64];
  strtow(sName, 64, varname);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onBuildBuilding(uTownId, sName);
  return 0;
}

int LUA_getObjectPosition(lua_State * pState)
{
  int nbParams = lua_gettop(pState);
  if (nbParams < 2)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: \"getObjectPosition\" should receive at least 2 arguments.");
    return 0;
  }

  const char * objtype = lua_tostring(pState, 1);
  if (strcmp(objtype, "unit") == 0)
  {
    if (nbParams < 3)
    {
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: \"getObjectPosition\" should receive at least 3 arguments.");
      return 0;
    }
    u8 uPlayerId = (u8)lua_tonumber(pState, 2);
    u32 uUnitId = (u32)lua_tonumber(pState, 3);
    Unit * pUnit = NULL;
    // If server is resolving, take information from server; else from client
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer != NULL && pServer->isResolving())
    {
      Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
      if (pPlayer == NULL)
      {
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"getObjectPosition\".");
        return 0;
      }
      pUnit = pPlayer->findUnit(uUnitId);
    }
    else
    {
      Player * pPlayer = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->findPlayer(uPlayerId);
      if (pPlayer == NULL)
      {
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"getObjectPosition\".");
        return 0;
      }
      pUnit = pPlayer->findUnit(uUnitId);
    }

    if (pUnit != NULL)
    {
      CoordsMap mp = pUnit->getMapPos();
      lua_pushnumber(pState, mp.x);
      lua_pushnumber(pState, mp.y);
      return 2;
    }
    else
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getObjectPosition: unit not found.");
    return 0;
  }
  else if (strcmp(objtype, "town") == 0)
  {
    u32 uTownId = (u32)lua_tonumber(pState, 2);
    Town * pTown = NULL;
    // If server is resolving, take information from server; else from client
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer != NULL && pServer->isResolving())
      pTown = pServer->getMap()->findTown(uTownId);
    else
      pTown = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap()->findTown(uTownId);

    if (pTown != NULL)
    {
      CoordsMap mp = pTown->getMapPos();
      lua_pushnumber(pState, mp.x);
      lua_pushnumber(pState, mp.y);
      return 2;
    }
    else
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getObjectPosition: town not found.");
    return 0;
  }
  else
  {
    wchar_t sText[512];
    swprintf_s(sText, 512, L"Lua interaction error: object type in \"getObjectPosition\" is invalid.");
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sText);
    return 0;
  }
}

int LUA_addSkillToUnit(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addSkillToUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 4, L"addSkillToUnit"))
    return 0;

  const char * skill = lua_tostring(pState, 1);
  wchar_t sSkill[NAME_MAX_CHARS];
  strtow(sSkill, NAME_MAX_CHARS, skill);
  const char * params = lua_tostring(pState, 2);
  wchar_t sParams[256];
  strtow(sParams, 256, params);
  u8 uPlayer = (u8)lua_tonumber(pState, 3);
  u32 uUnit = (u32)lua_tonumber(pState, 4);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddSkillToUnit(sSkill, sParams, uPlayer, uUnit);
  return 0;
}

int LUA_hideSpecialTile(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"hideSpecialTile");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 1, L"hideSpecialTile"))
    return 0;

  u32 uSpec = (u32)lua_tonumber(pState, 1);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onHideSpecialTile(uSpec);
  return 0;
}

int LUA_isShahmah(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 2, L"isShahmah"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  Player * pPlayer = getServerOrLocalPlayer(uPlayerId);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"isShahmah\".");
    return 0;
  }
  Unit * pUnit = pPlayer->findUnit(uUnitId);
  if (pUnit == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function isShahmah: unit not found.");
    return 0;
  }
  if (pUnit == pPlayer->getAvatar())
    lua_pushnumber(pState, 1);
  else
    lua_pushnumber(pState, 0);
  return 1;
}

int LUA_getUnitDescription(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 1, L"getUnitDescription"))
    return 0;

  const char * sName = lua_tostring(pState, 1);
  wchar_t sWName[NAME_MAX_CHARS];
  strtow(sWName, NAME_MAX_CHARS, sName);
  LuaObject * pLua = LuaObject::static_pCurrentLuaCaller;
  UnitData * pData = g_pMainGameRoot->m_pLocalClient->getDataFactory()->getUnitData(pLua->getObjectEdition(), sWName);
  if (pData == NULL) {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid unit provided to function getUnitDescription");
    return 0;
  }
  char sDesc[DESCRIPTION_MAX_CHARS];
  wchar_t sWDesc[DESCRIPTION_MAX_CHARS];
  pData->getInfos(sWDesc, DESCRIPTION_MAX_CHARS, L", ", false, NULL, false, true, true, false);
  wtostr(sDesc, DESCRIPTION_MAX_CHARS, sWDesc);
  lua_pushstring(pState, sDesc);
  return 1;
}

int LUA_getSkillData(lua_State * pState)
{
  if (!checkNumberOfParams(pState, 3, L"getSkillData"))
    return 0;

  u8 player = (u8) lua_tonumber(pState, 1);
  u32 unit = (u32) lua_tonumber(pState, 2);
  u32 skill = (u32) lua_tonumber(pState, 3);

  Player * pPlayer = getServerOrLocalPlayer(player);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid player provided to function getSkillData");
    return 0;
  }

  Unit * pUnit = pPlayer->findUnit(unit);
  if (pUnit == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid unit provided to function getSkillData");
    return 0;
  }

  Skill * pSkill = pUnit->findSkill(skill);
  if (pSkill == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid skill provided to function getSkillData");
    return 0;
  }

  char sName[NAME_MAX_CHARS];
  wtostr(sName, NAME_MAX_CHARS, pSkill->getObjectName());
  lua_pushstring(pState, sName);
  return 1;
}

int LUA_getTileData(lua_State * pState)
{
  int nbParams = checkNumberOfParams(pState, 3, 4, L"getTileData");
  if (nbParams < 0)
    return 0;

  int x = (int) lua_tonumber(pState, 1);
  int y = (int) lua_tonumber(pState, 2);
  const char * varname = lua_tostring(pState, 3);
  bool bBase = (nbParams == 4) ? lua_tonumber(pState, 4) == 1 : false;

  Map * pMap = getServerOrLocalMap();
  MapTile * pTile = pMap->getTileAt(CoordsMap(x, y));
  if (pTile == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid coords provided to function getTileData");
    return 0;
  }

  if (strcmp(varname, "terrain") == 0)
  {
    char pAllTerrains[7][64] = TERRAIN_NAMES;
    lua_pushstring(pState, pAllTerrains[pTile->m_uTerrainType]);
    return 1;
  }
  else
  {
    wchar_t sName[64];
    strtow(sName, 64, varname);
    bool bFound;
    double val = pTile->getValue(sName, bBase, &bFound);
    if (bFound)
      lua_pushnumber(pState, val);
    else
    {
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error in function getTileData: variable not found.");
      return 0;
    }
  }
  return 1;
}

int LUA_setTileData(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"setTileData");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 4, L"setTileData"))
    return 0;

  int x = (int) lua_tonumber(pState, 1);
  int y = (int) lua_tonumber(pState, 2);
  MapTile * pTile = pServer->getMap()->getTileAt(CoordsMap(x, y));
  if (pTile == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: invalid coords provided to function setTileData");
    return 0;
  }

  const char * varname = lua_tostring(pState, 3);
  if (strcmp(varname, "terrain") == 0)
  {
    const char * type = lua_tostring(pState, 4);
    char pAllTerrains[7][64] = TERRAIN_NAMES;
    for (int i = 0; i < 7; i++)
    {
      if (strcmp(type, pAllTerrains[i]) == 0)
      {
        if (g_bLuaEvaluationMode) {
          Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
          assert(pServer != NULL);
          return 0;
        }

        pServer->getMap()->changeTerrainType(CoordsMap(x, y), (u8)i, pServer);
        return 0;
      }
    }
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Error in LUA: unrecognize terrain type in setTileData");
    return 0;
  }
  else
  {
    wchar_t sName[64];
    strtow(sName, 64, varname);
    double d = lua_tonumber(pState, 4);

    if (g_bLuaEvaluationMode) {
      Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
      assert(pServer != NULL);
      return 0;
    }

    if (!pTile->setBaseValue(sName, d))
      pServer->getDebug()->notifyErrorMessage(L"Lua interaction error in function setTileData: variable not found.");
  }
  return 0;
}

int LUA_teleport(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"teleport");
  if (pServer == NULL)
    return 0;
  int nbParams = checkNumberOfParams(pState, 4, 5, L"teleport");
  if (nbParams < 0)
    return 0;

  MapObject * pMapObj = NULL;
  const char * objtype = lua_tostring(pState, 1);
  int iNextParam = 3;
  if (strcmp(objtype, "unit") == 0)
  {
    if (nbParams != 5)
    {
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: \"teleport\" for unit should receive 5 arguments.");
      return 0;
    }
    u8 uPlayerId = (u8) lua_tonumber(pState, 2);
    u32 uUnitId = (u32) lua_tonumber(pState, 3);
    iNextParam = 4;
    Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
    if (pPlayer == NULL)
    {
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"teleport\".");
      return 0;
    }
    pMapObj = pPlayer->findUnit(uUnitId);
  }
  else if (strcmp(objtype, "town") == 0)
  {
    u32 uTownId = (u32) lua_tonumber(pState, 2);
    pMapObj = pServer->getMap()->findTown(uTownId);
  }
  else
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: object type in \"teleport\" is invalid.");
    return 0;
  }
  if (pMapObj == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: object not found in function \"teleport\".");
    return 0;
  }
  int x = (int) lua_tonumber(pState, iNextParam);
  int y = (int) lua_tonumber(pState, iNextParam+1);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onTeleport(pMapObj, CoordsMap(x, y));
  return 0;
}

int LUA_resurrect(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"resurrect");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"resurrect"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  u32 uUnitId = (u32) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onResurrect(uPlayerId, uUnitId);

  return 0;
}

int LUA_addMagicCircle(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addMagicCircle");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"addMagicCircle"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  int x = (int) lua_tonumber(pState, 2);
  int y = (int) lua_tonumber(pState, 3);

  Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"addMagicCircle\".");
    return 0;
  }

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  int id = pServer->getSolver()->getSpellsSolver()->onAddMagicCircle(pPlayer, CoordsMap(x, y));
  lua_pushnumber(pState, id);
  return 1;
}

int LUA_removeMagicCircle(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"removeMagicCircle");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"removeMagicCircle"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  int id = (int) lua_tonumber(pState, 2);

  Player * pPlayer = pServer->getSolver()->findPlayer(uPlayerId);
  if (pPlayer == NULL)
  {
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(L"Lua interaction error: invalid player in function \"removeMagicCircle\".");
    return 0;
  }

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onRemoveMagicCircle(pPlayer, id);
  return 0;
}

int LUA_recallSpell(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"recallSpell");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 3, L"recallSpell"))
    return 0;

  wchar_t sType[64];
  const char * objtype = lua_tostring(pState, 1);
  strtow(sType, 64, objtype);
  u8 uPlayerId = (u8) lua_tonumber(pState, 2);
  u32 id = (u32) lua_tonumber(pState, 3);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onRecallSpell(sType, uPlayerId, id);
  return 0;
}

int LUA_addGoldToPlayer(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addGoldToPlayer");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"addGoldToPlayer"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  int amount = (int) lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddGoldToPlayer(uPlayerId, amount);
  return 0;
}

int LUA_addSpellToPlayer(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addSpellToPlayer");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"addSpellToPlayer"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  wchar_t sName[64];
  const char * name = lua_tostring(pState, 2);
  strtow(sName, 64, name);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddSpellToPlayer(uPlayerId, sName);
  return 0;
}

int LUA_addArtifactToPlayer(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addArtifactToPlayer");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"addArtifactToPlayer"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  wchar_t sName[64];
  const char * name = lua_tostring(pState, 2);
  strtow(sName, 64, name);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddArtifactToPlayer(uPlayerId, sName);
  return 0;
}

int LUA_addShahmahToPlayer(lua_State * pState)
{
  Server * pServer = checkServerResolving(L"addShahmahToPlayer");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"addShahmahToPlayer"))
    return 0;

  u8 uPlayerId = (u8) lua_tonumber(pState, 1);
  wchar_t sName[64];
  const char * name = lua_tostring(pState, 2);
  strtow(sName, 64, name);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onAddAvatarToPlayer(uPlayerId, sName);
  return 0;
}

int LUA_getUnitStatus(lua_State * pState) {
  if (!checkNumberOfParams(pState, 2, L"getUnitStatus"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  Player * pPlayer = getServerOrLocalPlayer(uPlayerId);
  if (pPlayer != NULL) {
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit != NULL) {
      switch (pUnit->getStatus())
      {
      case US_Dead:
        lua_pushstring(pState, "dead");
        break;
      case US_Removed:
        lua_pushstring(pState, "removed");
        break;
      case US_Normal:
      default:
        lua_pushstring(pState, "normal");
        break;
      }
      return 1;
    }
    else {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"Lua interaction error in function getUnitStatus: unit not found.");
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
  }
  else {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error in function getUnitStatus: player not found.");
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
  }
  return 0;
}

int LUA_getUnitNameAndEdition(lua_State * pState) {
  if (!checkNumberOfParams(pState, 2, L"getUnitNameAndEdition"))
    return 0;

  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);
  Player * pPlayer = getServerOrLocalPlayer(uPlayerId);
  if (pPlayer != NULL) {
    Unit * pUnit = pPlayer->findUnit(uUnitId);
    if (pUnit != NULL) {
      char sName[NAME_MAX_CHARS] = "";
      char sEdition[NAME_MAX_CHARS] = "";
      wtostr(sName, NAME_MAX_CHARS, pUnit->getUnitModelId());
      wtostr(sEdition, NAME_MAX_CHARS, pUnit->getUnitEdition());
      lua_pushstring(pState, sName);
      lua_pushstring(pState, sEdition);
      return 2;
    }
    else {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"Lua interaction error in function getUnitNameAndEdition: unit not found.");
      g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
  }
  else {
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error in function getUnitNameAndEdition: player not found.");
    g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
  }
  return 0;
}

int LUA_removeUnit(lua_State * pState) {
  Server * pServer = checkServerResolving(L"removeUnit");
  if (pServer == NULL)
    return 0;
  if (!checkNumberOfParams(pState, 2, L"removeUnit"))
    return 0;
  
  u8 uPlayerId = (u8)lua_tonumber(pState, 1);
  u32 uUnitId = (u32)lua_tonumber(pState, 2);

  if (g_bLuaEvaluationMode) {
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL);
    return 0;
  }

  pServer->getSolver()->getSpellsSolver()->onRemoveUnit(uPlayerId, uUnitId);
  return 0;
}

int LUA_getMapDimensions(lua_State * pState) {
  if (!checkNumberOfParams(pState, 0, L"getMapDimensions"))
    return 0;

  Map * pMap = getServerOrLocalMap();
  lua_pushnumber(pState, pMap->getWidth());
  lua_pushnumber(pState, pMap->getHeight());
  return 2;
}

u8 getTargetTypeFromName(char * sType)
{
  u8 uType = 0;
  if (_stricmp(sType, "unit") == 0)
    uType = SELECT_TYPE_UNIT;
  else if (_stricmp(sType, "dead_unit") == 0)
    uType = SELECT_TYPE_DEAD_UNIT;
  else if (_stricmp(sType, "town") == 0)
    uType = SELECT_TYPE_TOWN;
  else if (_stricmp(sType, "temple") == 0)
    uType = SELECT_TYPE_TEMPLE;
  else if (_stricmp(sType, "building") == 0)
    uType = SELECT_TYPE_BUILDING;
  else if (_stricmp(sType, "spell_in_play") == 0)
    uType = SELECT_TYPE_SPELL_IN_PLAY;
  else if (_stricmp(sType, "spell_in_hand") == 0)
    uType = SELECT_TYPE_SPELL_IN_HAND;
  else if (_stricmp(sType, "spell_in_deck") == 0)
    uType = SELECT_TYPE_SPELL_IN_DECK;
  else if (_stricmp(sType, "spell_in_discard") == 0)
    uType = SELECT_TYPE_SPELL_IN_DISCARD;
  else if (_stricmp(sType, "tile") == 0)
    uType = SELECT_TYPE_TILE;
  else if (_stricmp(sType, "player") == 0)
    uType = SELECT_TYPE_PLAYER;
  return uType;
}

u8 getTargetTypeFromName(wchar_t * sType)
{
  u8 uType = 0;
  if (_wcsicmp(sType, L"unit") == 0)
    uType = SELECT_TYPE_UNIT;
  else if (_wcsicmp(sType, L"dead_unit") == 0)
    uType = SELECT_TYPE_DEAD_UNIT;
  else if (_wcsicmp(sType, L"town") == 0)
    uType = SELECT_TYPE_TOWN;
  else if (_wcsicmp(sType, L"temple") == 0)
    uType = SELECT_TYPE_TEMPLE;
  else if (_wcsicmp(sType, L"building") == 0)
    uType = SELECT_TYPE_BUILDING;
  else if (_wcsicmp(sType, L"spell_in_play") == 0)
    uType = SELECT_TYPE_SPELL_IN_PLAY;
  else if (_wcsicmp(sType, L"spell_in_hand") == 0)
    uType = SELECT_TYPE_SPELL_IN_HAND;
  else if (_wcsicmp(sType, L"spell_in_deck") == 0)
    uType = SELECT_TYPE_SPELL_IN_DECK;
  else if (_wcsicmp(sType, L"spell_in_discard") == 0)
    uType = SELECT_TYPE_SPELL_IN_DISCARD;
  else if (_wcsicmp(sType, L"tile") == 0)
    uType = SELECT_TYPE_TILE;
  else if (_wcsicmp(sType, L"player") == 0)
    uType = SELECT_TYPE_PLAYER;
  return uType;
}

//------------------------------------------------------------------------------
// Call to register lua callbacks
//------------------------------------------------------------------------------
void registerLuaCallbacks(lua_State * pState)
{
	lua_register(pState, "split", LUA_split);
	lua_register(pState, "splitint", LUA_splitint);
	lua_register(pState, "dispatchToClients", LUA_dispatchToClients);
	lua_register(pState, "selectTarget", LUA_selectTarget);
	lua_register(pState, "selectTargetThenResolve", LUA_selectTargetThenResolve);
	lua_register(pState, "damageUnit", LUA_damageUnit);
	lua_register(pState, "summon", LUA_summon);
	lua_register(pState, "attachToUnit", LUA_attachToUnit);
	lua_register(pState, "attachToPlayer", LUA_attachToPlayer);
	lua_register(pState, "attachToTown", LUA_attachToTown);
	lua_register(pState, "attachToTemple", LUA_attachToTemple);
	lua_register(pState, "attachToTile", LUA_attachToTile);
	lua_register(pState, "attachAsGlobal", LUA_attachAsGlobal);
	lua_register(pState, "detachFromGlobal", LUA_detachFromGlobal);
	lua_register(pState, "addChildEffectToUnit", LUA_addChildEffectToUnit);
	lua_register(pState, "removeChildEffectFromUnit", LUA_removeChildEffectFromUnit);
	lua_register(pState, "addChildEffectToTown", LUA_addChildEffectToTown);
	lua_register(pState, "removeChildEffectFromTown", LUA_removeChildEffectFromTown);
	lua_register(pState, "getUnitData", LUA_getUnitData);
	lua_register(pState, "getUnitBaseData", LUA_getUnitBaseData);
	lua_register(pState, "setUnitData", LUA_setUnitData);
	lua_register(pState, "drawSpell", LUA_drawSpell);
	lua_register(pState, "discardActiveSpell", LUA_discardActiveSpell);
	lua_register(pState, "discardDeckSpell", LUA_discardDeckSpell);
	lua_register(pState, "discardHandSpell", LUA_discardHandSpell);
	lua_register(pState, "produceMana", LUA_produceMana);
	lua_register(pState, "getAttacker", LUA_getAttacker);
	lua_register(pState, "getDefender", LUA_getDefender);
	lua_register(pState, "getAttackerLife", LUA_getAttackerLife);
	lua_register(pState, "getDefenderLife", LUA_getDefenderLife);
	lua_register(pState, "setAttackerLife", LUA_setAttackerLife);
	lua_register(pState, "setDefenderLife", LUA_setDefenderLife);
	lua_register(pState, "getAttackerArmor", LUA_getAttackerArmor);
	lua_register(pState, "getDefenderArmor", LUA_getDefenderArmor);
	lua_register(pState, "setAttackerArmor", LUA_setAttackerArmor);
	lua_register(pState, "setDefenderArmor", LUA_setDefenderArmor);
	lua_register(pState, "getAttackerDamages", LUA_getAttackerDamages);
	lua_register(pState, "getDefenderDamages", LUA_getDefenderDamages);
	lua_register(pState, "setAttackerDamages", LUA_setAttackerDamages);
	lua_register(pState, "setDefenderDamages", LUA_setDefenderDamages);
	lua_register(pState, "getUnitAlignment", LUA_getUnitAlignment);
	lua_register(pState, "askForExtraMana", LUA_askForExtraMana);
	lua_register(pState, "addResolveParameter", LUA_addResolveParameter);
	lua_register(pState, "getPlayersList", LUA_getPlayersList);
	lua_register(pState, "getUnitsList", LUA_getUnitsList);
	lua_register(pState, "getSkillsList", LUA_getSkillsList);
	lua_register(pState, "getTownsList", LUA_getTownsList);
	lua_register(pState, "deactivateSkill", LUA_deactivateSkill);
	lua_register(pState, "changeSpellOwner", LUA_changeSpellOwner);
	lua_register(pState, "changeUnitOwner", LUA_changeUnitOwner);
	lua_register(pState, "changeTownOwner", LUA_changeTownOwner);
	lua_register(pState, "getTownData", LUA_getTownData);
	lua_register(pState, "townHasBuilding", LUA_townHasBuilding);
	lua_register(pState, "buildBuilding", LUA_buildBuilding);
	lua_register(pState, "getObjectPosition", LUA_getObjectPosition);
	lua_register(pState, "addSkillToUnit", LUA_addSkillToUnit);
	lua_register(pState, "hideSpecialTile", LUA_hideSpecialTile);
	lua_register(pState, "isShahmah", LUA_isShahmah);
	lua_register(pState, "getUnitDescription", LUA_getUnitDescription);
	lua_register(pState, "getSkillData", LUA_getSkillData);
	lua_register(pState, "getTileData", LUA_getTileData);
	lua_register(pState, "setTileData", LUA_setTileData);
	lua_register(pState, "teleport", LUA_teleport);
	lua_register(pState, "resurrect", LUA_resurrect);
	lua_register(pState, "addMagicCircle", LUA_addMagicCircle);
	lua_register(pState, "removeMagicCircle", LUA_removeMagicCircle);
	lua_register(pState, "recallSpell", LUA_recallSpell);
	lua_register(pState, "addGoldToPlayer", LUA_addGoldToPlayer);
	lua_register(pState, "addSpellToPlayer", LUA_addSpellToPlayer);
	lua_register(pState, "addArtifactToPlayer", LUA_addArtifactToPlayer);
	lua_register(pState, "addShahmahToPlayer", LUA_addShahmahToPlayer);
	lua_register(pState, "getUnitStatus", LUA_getUnitStatus);
	lua_register(pState, "getUnitNameAndEdition", LUA_getUnitNameAndEdition);
	lua_register(pState, "removeUnit", LUA_removeUnit);
	lua_register(pState, "getMapDimensions", LUA_getMapDimensions);
}
