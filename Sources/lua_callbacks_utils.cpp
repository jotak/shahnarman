#include "lua_callbacks_utils.h"

u8 g_uLuaSelectTargetType;
u32 g_uLuaSelectConstraints = SELECT_CONSTRAINT_NONE;
char g_sLuaSelectCallback[128] = "";
bool g_bLuaSelectOnResolve = false;
u32 g_uLuaCurrentObjectType;
bool g_bLuaEvaluationMode = false;
lua_State * g_pLuaStateForTarget = NULL;

extern GameRoot * g_pMainGameRoot;

//************************************************************************************
// Utility functions used in callbacks
//************************************************************************************
bool setValidFor(u32 uTypes, const char * sFuncName)
{
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    assert(pServer != NULL && pServer->isResolving());
    LuaContext * pCtxt = pServer->getSolver()->getSpellsSolver()->retrieveLuaContext();
    assert(pCtxt != NULL);
    if (pCtxt->pLua->getType() & uTypes)
        return true;
    else
    {
        char sText[512];
        snprintf(sText, 512, "Lua interaction error: function \"%s\" called by invalid LUA type.", sFuncName);
        pServer->getDebug()->notifyErrorMessage(sText);
        return false;
    }
}

Server * checkServerResolving(const char * sFuncName)
{
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer == NULL || !pServer->isResolving())
    {
        char sText[512];
        snprintf(sText, 512, "Lua interaction error: trying to call \"%s\" outside resolve phase.", sFuncName);
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sText);
        return NULL;
    }
    return pServer;
}

Map * getServerOrLocalMap()
{
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer != NULL && pServer->isResolving())
        return pServer->getMap();
    else
        return g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap();
}

PlayerManagerAbstract * getServerOrLocalPlayerManager()
{
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer != NULL && pServer->isResolving())
        return pServer->getSolver();
    else
        return g_pMainGameRoot->m_pLocalClient->getPlayerManager();
}

Player * getServerOrLocalPlayer(u8 uId)
{
    Server * pServer = g_pMainGameRoot->m_pLocalClient->getServer();
    if (pServer != NULL && pServer->isResolving())
        return pServer->getSolver()->findPlayer(uId);
    else
        return g_pMainGameRoot->m_pLocalClient->getPlayerManager()->findPlayer(uId);
}

bool checkNumberOfParams(lua_State * pState, int iExpected, const char * sFuncName)
{
#ifdef DEBUG
    bool bLog = (g_pMainGameRoot->m_pLocalClient->getClientParameters()->iLogLevel >= 3);
    if (bLog)
    {
        char sText[1024];
        snprintf(sText, 1024, "call from LUA: %s", sFuncName);
        g_pMainGameRoot->m_pLocalClient->getDebug()->log(sText);
    }
#endif
    int nbParams = lua_gettop(pState);
    if (nbParams != iExpected)
    {
        char sText[512];
        snprintf(sText, 512, "Lua interaction error: \"%s\" should receive %d arguments.", sFuncName, iExpected);
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sText);
        return false;
    }
    return true;
}

int checkNumberOfParams(lua_State * pState, int iMinExpected, int iMaxExpected, const char * sFuncName)
{
#ifdef DEBUG
    bool bLog = (g_pMainGameRoot->m_pLocalClient->getClientParameters()->iLogLevel >= 3);
    if (bLog)
    {
        char sText[1024];
        snprintf(sText, 1024, "call from LUA: %s", sFuncName);
        g_pMainGameRoot->m_pLocalClient->getDebug()->log(sText);
    }
#endif
    int nbParams = lua_gettop(pState);
    if (nbParams < iMinExpected || nbParams > iMaxExpected)
    {
        char sText[512];
        snprintf(sText, 512, "Lua interaction error: \"%s\" should receive between %d and %d arguments.", sFuncName, iMinExpected, iMaxExpected);
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sText);
        return -1;
    }
    return nbParams;
}

bool readTargetData(const char * sType, const char * sConstraints, const char * sCallback, const char * sFuncName)
{
    // Get target type
    g_uLuaSelectTargetType = getTargetTypeFromName(sType);
    if (g_uLuaSelectTargetType == 0)
    {
        char sError[256];
        snprintf(sError, 256, "Lua interaction error: parameter \"type\" in \"%s\" is unknown.", sFuncName);
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
        return false;
    }

    // Get target constraints: parse string
    g_uLuaSelectConstraints = SELECT_CONSTRAINT_NONE;
    int iParse = 0;
    int iWord = 0;
    char sWord[64] = "";
    while (sConstraints[iParse] == ' ')
        iParse++;
    while (sConstraints[iParse] != '\0')
    {
        sWord[iWord++] = sConstraints[iParse++];
        if (sConstraints[iParse] == ' ' || sConstraints[iParse] == '\0')
        {
            while (sConstraints[iParse] == ' ')
                iParse++;
            sWord[iWord] = '\0';
            iWord = 0;
            if (strcasecmp(sWord, "inrange") == 0)
                g_uLuaSelectConstraints |= SELECT_CONSTRAINT_INRANGE;
            else if (strcasecmp(sWord, "owned") == 0)
                g_uLuaSelectConstraints |= SELECT_CONSTRAINT_OWNED;
            else if (strcasecmp(sWord, "opponent") == 0)
                g_uLuaSelectConstraints |= SELECT_CONSTRAINT_OPPONENT;
            else if (strcasecmp(sWord, "not_avatar") == 0)
                g_uLuaSelectConstraints |= SELECT_CONSTRAINT_NOT_AVATAR;
            else if (strcasecmp(sWord, "custom") == 0)
                g_uLuaSelectConstraints |= SELECT_CONSTRAINT_CUSTOM;
            else
            {
                char sError[256];
                snprintf(sError, 256, "Lua interaction warning: parameter \"constraints\" in \"%s\" has an invalid constraint.", sFuncName);
                g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage(sError);
            }
        }
    }
    wsafecpy(g_sLuaSelectCallback, 128, sCallback);
    return true;
}

bool checkCustomConstraints(CoordsMap mp, BaseObject * pObj, u8 uType)
{
    if (!(g_uLuaSelectConstraints & SELECT_CONSTRAINT_CUSTOM))
        return true;
    if (g_pLuaStateForTarget == NULL)
        return false;

    // Call lua function
    lua_getglobal(g_pLuaStateForTarget, "onCheckSelect");
    if (lua_isfunction(g_pLuaStateForTarget, -1))
    {
        char sParams[64];
        switch (uType)
        {
        case SELECT_TYPE_TILE:
            snprintf(sParams, 64, "tile %d %d", mp.x, mp.y);
            break;
        case SELECT_TYPE_ANY_SPELL:
        case SELECT_TYPE_SPELL_IN_PLAY:
        case SELECT_TYPE_SPELL_IN_HAND:
        case SELECT_TYPE_SPELL_IN_DECK:
        case SELECT_TYPE_SPELL_IN_DISCARD:
            snprintf(sParams, 64, "spell %d %ld", (int)((Spell*)pObj)->getPlayerId(), (long)((Spell*)pObj)->getInstanceId());
            break;
        default:
        {
            LuaTargetable * pTarget = LuaTargetable::convertFromBaseObject(pObj, uType);
            assert(pTarget != NULL);
            wsafecpy(sParams, 64, pTarget->getIdentifiers());
        }
        }
        lua_pushstring(g_pLuaStateForTarget, sParams);
        int err = lua_pcall(g_pLuaStateForTarget, 1, 1, 0);
        switch (err)
        {
        case LUA_ERRRUN:
        {
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("LUA runtime error when calling onCheckSelect.");
            return false;
        }
        case LUA_ERRMEM:
        {
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("LUA memory allocation error when calling onCheckSelect.");
            return false;
        }
        case LUA_ERRERR:
        {
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("LUA error handling error when calling onCheckSelect.");
            return false;
        }
        }
        double d = lua_tonumber(g_pLuaStateForTarget, -1);
        lua_pop(g_pLuaStateForTarget, 1);
        return (d > 0);
    }
    else
    {
        g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("LUA error: function onCheckSelect not found.");
        lua_pop(g_pLuaStateForTarget, 1);
        return false;
    }
}

bool checkRange(Player * pCaster, Unit * pUnit, CoordsMap mp)
{
    int range = (int) pCaster->getValue(STRING_SPELLSRANGE);
    if (pUnit == NULL)
    {
        // Check distance between mp and caster or its magic circles (typically, for spells)
        CoordsMap mp2 = pCaster->getAvatar()->getMapPos();
        if (abs(mp2.x-mp.x) <= range && abs(mp2.y-mp.y) <= range)
            return true;
        for (int i = 0; i < MAX_MAGIC_CIRCLES; i++)
        {
            if (pCaster->m_MagicCirclePos[i].x >= 0 && mp == pCaster->m_MagicCirclePos[i])
                return true;
        }
        return false;
    }
    else
    {
        // Check distance between mp and unit (typically, for skill actions)
        CoordsMap mp2 = pUnit->getMapPos();
        return (abs(mp2.x-mp.x) <= range && abs(mp2.y-mp.y) <= range);
    }
}

bool checkTileConstraints(CoordsMap mp)
{
    if (!checkCustomConstraints(mp, NULL, SELECT_TYPE_TILE))
        return false;
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_INRANGE)
    {
        Player * pPlayer = g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellDialog()->getCurrentCaster();
        assert(pPlayer != NULL);
        if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
            return checkRange(pPlayer, NULL, mp);
        else if (g_uLuaCurrentObjectType == LUAOBJECT_SKILL)
        {
            Unit * pUnit = g_pMainGameRoot->m_pLocalClient->getInterface()->getUnitOptionsDialog()->getUnit();
            assert(pUnit != NULL);
            return checkRange(pPlayer, pUnit, mp);
        }
    }
    return true;
}

bool checkMapObjConstraints(MapObject * pObj)
{
    if (!checkCustomConstraints(CoordsMap(), pObj, g_uLuaSelectTargetType))
        return false;
    // Check constraints.
    bool bConstraints = true;
    Player * pPlayer = g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellDialog()->getCurrentCaster();
    assert(pPlayer != NULL);
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_INRANGE)
    {
        if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
            bConstraints = checkRange(pPlayer, NULL, pObj->getMapPos());
        else if (g_uLuaCurrentObjectType == LUAOBJECT_SKILL)
        {
            Unit * pUnit = g_pMainGameRoot->m_pLocalClient->getInterface()->getUnitOptionsDialog()->getUnit();
            assert(pUnit != NULL);
            bConstraints = checkRange(pPlayer, pUnit, pObj->getMapPos());
        }
    }
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED)
    {
        if (pObj->getOwner() != pPlayer->m_uPlayerId)
            bConstraints = false;
    }
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT)
    {
        if (pObj->getOwner() == pPlayer->m_uPlayerId)
            bConstraints = false;
    }
    if (g_uLuaSelectConstraints & SELECT_CONSTRAINT_NOT_AVATAR)
    {
        if (pObj->getType() & GOTYPE_UNIT)
        {
            Player * pPlayer2 = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
            while (pPlayer2 != NULL)
            {
                if (pPlayer2->getAvatar() == (Unit*) pObj)
                {
                    bConstraints = false;
                    break;
                }
                pPlayer2 = (Player*) g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
            }
        }
    }
    return bConstraints;
}

bool checkSpellConstraints(Spell * pObj)
{
    if (!checkCustomConstraints(CoordsMap(), pObj, SELECT_TYPE_ANY_SPELL))
        return false;
    // Check constraints.
    // Ignore SELECT_CONSTRAINT_INRANGE for spell target ; spells are not located on map (only spell effects can be, but it's not the target)
    Player * pPlayer = g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellDialog()->getCurrentCaster();
    assert(pPlayer != NULL);
    return !((g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED) && pObj->getPlayerId() != pPlayer->m_uPlayerId)
           || ((g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT) && pObj->getPlayerId() == pPlayer->m_uPlayerId);
}

bool checkTargetPlayer(Player * pPlayer)
{
    if (g_uLuaSelectTargetType == SELECT_TYPE_PLAYER)
    {
        Player * pCaster = g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellDialog()->getCurrentCaster();
        return !((g_uLuaSelectConstraints & SELECT_CONSTRAINT_OWNED) && pPlayer != pCaster)
               || ((g_uLuaSelectConstraints & SELECT_CONSTRAINT_OPPONENT) && pPlayer == pCaster);
    }
    return false;
}

bool checkTargetObject(MapObject * pObj)
{
    // Valid target type?
    if ((g_uLuaSelectTargetType == SELECT_TYPE_UNIT && (pObj->getType() & GOTYPE_UNIT))
            || (g_uLuaSelectTargetType == SELECT_TYPE_DEAD_UNIT && (pObj->getType() & GOTYPE_DEAD_UNIT))
            || (g_uLuaSelectTargetType == SELECT_TYPE_TOWN && (pObj->getType() & GOTYPE_TOWN))
            || (g_uLuaSelectTargetType == SELECT_TYPE_TEMPLE && (pObj->getType() & GOTYPE_TEMPLE)))
    {
        // Target type valid. Now check constraints.
        return checkMapObjConstraints(pObj);
    }
    return false;
}

bool checkTargetSpell(Spell * pObj)
{
    // Check that the spell is in the right list
    u8 uPlayerId = pObj->getPlayerId();
    Player * pPlayer = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->findPlayer(uPlayerId);
    assert(pPlayer != NULL);

    switch (g_uLuaSelectTargetType)
    {
    case SELECT_TYPE_SPELL_IN_HAND:
        if (!pPlayer->m_pHand->goTo(0, pObj))
            return false;
        break;
    case SELECT_TYPE_SPELL_IN_DECK:
        if (!pPlayer->m_pDeck->goTo(0, pObj))
            return false;
        break;
    case SELECT_TYPE_SPELL_IN_PLAY:
        if (!pPlayer->m_pActiveSpells->goTo(0, pObj))
            return false;
        break;
    case SELECT_TYPE_SPELL_IN_DISCARD:
        if (!pPlayer->m_pDiscard->goTo(0, pObj))
            return false;
        break;
    default:
        return false;
    }
    // Target type valid. Now check constraints.
    return checkSpellConstraints(pObj);
}

LuaTargetable * checkTargetTile(CoordsMap mp, bool * bMultipleTargets)
{
    if (!g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap()->isInBounds(mp))
        return NULL;

    // Look at target type
    if (bMultipleTargets != NULL)
        *bMultipleTargets = false;
    MapTile * pTile = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMap()->getTileAt(mp);
    switch (g_uLuaSelectTargetType)
    {
    case SELECT_TYPE_TILE:
        return checkTileConstraints(mp) ? pTile : NULL;
    case SELECT_TYPE_UNIT:
    case SELECT_TYPE_DEAD_UNIT:
    {
        u32 uType = (g_uLuaSelectTargetType == SELECT_TYPE_UNIT) ? GOTYPE_UNIT : GOTYPE_DEAD_UNIT;
        int nbValid = 0;
        MapObject * mapobj = pTile->getFirstMapObject(uType);
        MapObject * lastvalid = NULL;
        while (mapobj != NULL)
        {
            if (checkMapObjConstraints(mapobj))
            {
                nbValid++;
                lastvalid = mapobj;
            }
            mapobj = pTile->getNextMapObject(uType);
        }
        if (bMultipleTargets == NULL)
            return lastvalid;
        else if (nbValid == 1) // single target
        {
            *bMultipleTargets = false;
            return lastvalid;
        }
        else if (nbValid > 1) // multiple targets
        {
            *bMultipleTargets = true;
            return pTile;
        }
        else
            return NULL;
    }
    case SELECT_TYPE_TOWN:
    case SELECT_TYPE_BUILDING:
    {
        if (bMultipleTargets != NULL)
            *bMultipleTargets = false;  // never 2 cities in one tile
        MapObject * mapobj = pTile->getFirstMapObject(GOTYPE_TOWN);
        if (mapobj != NULL && checkMapObjConstraints(mapobj))
            return mapobj;
        else
            return NULL;
    }
    case SELECT_TYPE_TEMPLE:
    {
        if (bMultipleTargets != NULL)
            *bMultipleTargets = false;  // never 2 temples in one tile
        MapObject * mapobj = pTile->getFirstMapObject(GOTYPE_TEMPLE);
        if (mapobj != NULL && checkMapObjConstraints(mapobj))
            return mapobj;
        else
            return NULL;
    }
    case SELECT_TYPE_SPELL_IN_PLAY:
    {
        MapObject * mapobj = pTile->getFirstMapObject();
        while (mapobj != NULL)
        {
            LuaObject * pLua = mapobj->getFirstEffect(0);
            while (pLua != NULL)
            {
                if (pLua->getType() == LUAOBJECT_SPELL)
                {
                    if (checkSpellConstraints((Spell*)pLua))
                        return pTile;
                }
                pLua = mapobj->getNextEffect(0);
            }
            mapobj = pTile->getNextMapObject();
        }
        return NULL;
    }
    }
    return NULL;
}
/*
void getObjectParameters(LuaTargetable * pTarget, u8 uType, char * wParams, char * sParams)
{
  switch (pTarget->get)
  {
  case SELECT_TYPE_TILE:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "tile %d %d", mp.x, mp.y);
      if (wParams != NULL)
        snprintf(wParams, 64, "tile %d %d", mp.x, mp.y);
      break;
    }
  case SELECT_TYPE_UNIT:
  case SELECT_TYPE_DEAD_UNIT:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "%d %d %ld", ((Unit*)pObj)->getOwner(), (long)((Unit*)pObj)->getId());
      if (wParams != NULL)
        snprintf(wParams, 64, "%d %d %ld", ((Unit*)pObj)->getOwner(), (long)((Unit*)pObj)->getId());
      break;
    }
  case SELECT_TYPE_TOWN:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "%d %ld", (long)((Town*)pObj)->getId());
      if (wParams != NULL)
        snprintf(wParams, 64, "%d %ld", (long)((Town*)pObj)->getId());
      break;
    }
  case SELECT_TYPE_TEMPLE:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "%d %ld", (long)((Temple*)pObj)->getId());
      if (wParams != NULL)
        snprintf(wParams, 64, "%d %ld", (long)((Temple*)pObj)->getId());
      break;
    }
  case SELECT_TYPE_ANY_SPELL:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "%d %d %ld", (int)((Spell*)pObj)->getPlayerId(), (long)((Spell*)pObj)->getInstanceId());
      if (wParams != NULL)
        snprintf(wParams, 64, "%d %d %ld", (int)((Spell*)pObj)->getPlayerId(), (long)((Spell*)pObj)->getInstanceId());
      break;
    }
  case SELECT_TYPE_PLAYER:
    {
      if (sParams != NULL)
        snprintf(sParams, 64, "%d %d", (int)((Player*)pObj)->m_uPlayerId);
      if (wParams != NULL)
        snprintf(wParams, 64, "%d %d", (int)((Player*)pObj)->m_uPlayerId);
      break;
    }
  }
}*/


//************************************************************************************
// Then, here are some callbacks used with the game classes
//************************************************************************************
void clbkSelectTarget_cancelSelection(u32 uType, int iResolve)
{
    if (uType == 0)
        uType = g_uLuaCurrentObjectType;
    bool bResolve = (iResolve == -1) ? g_bLuaSelectOnResolve : (iResolve == 1);
    g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->hide();
    g_pMainGameRoot->m_pLocalClient->getInterface()->getPlayerSelectorDialog()->hide();
    g_pMainGameRoot->m_pLocalClient->getInterface()->getStatusDialog()->hide();
    g_pMainGameRoot->m_pLocalClient->getGameboard()->unsetTargetMode();
    if (uType == LUAOBJECT_SPELL)
        g_pMainGameRoot->m_pLocalClient->getPlayerManager()->castSpellFinished(false, bResolve);
    else if (uType == LUAOBJECT_SKILL)
        g_pMainGameRoot->m_pLocalClient->getPlayerManager()->skillWasActivated(false, bResolve);
    g_pMainGameRoot->m_pLocalClient->getPlayerManager()->enableEOT(true);
}
void clbkSelectTarget_cancelSelection()
{
    clbkSelectTarget_cancelSelection(0, -1);
}

void clbkSelectTarget_OnMouseOverGameboard(CoordsMap mp)
{
    if (checkTargetTile(mp, NULL) != NULL)
    {
        MapCursor * pCur = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMapCursor(MAPCURSOR_SPECIALTARGET);
        pCur->moveTo(mp);
        pCur->setEnabled(true);
        g_pMainGameRoot->m_pLocalClient->getGameboard()->getMapCursor(MAPCURSOR_WRONGACTION)->setEnabled(false);
    }
    else
    {
        // Invalid target
        MapCursor * pCur = g_pMainGameRoot->m_pLocalClient->getGameboard()->getMapCursor(MAPCURSOR_WRONGACTION);
        pCur->moveTo(mp);
        pCur->setEnabled(true);
        g_pMainGameRoot->m_pLocalClient->getGameboard()->getMapCursor(MAPCURSOR_SPECIALTARGET)->setEnabled(false);
    }
}

void clbkSelectTarget_OnClickGameboard(CoordsMap mp)
{
    bool bMultipleTargets;
    LuaTargetable * pObj = checkTargetTile(mp, &bMultipleTargets);
    if (pObj != NULL)
    {
        switch (g_uLuaSelectTargetType)
        {
        case SELECT_TYPE_UNIT:
        case SELECT_TYPE_DEAD_UNIT:
        {
            // Multiple targets?
            if (bMultipleTargets)
            {
                // Not finished yet: still have to select 1 unit among the other
                u32 uFilter = 0;
                if (g_uLuaSelectTargetType == SELECT_TYPE_UNIT)
                    uFilter = GOTYPE_UNIT;
                else if (g_uLuaSelectTargetType == SELECT_TYPE_DEAD_UNIT)
                    uFilter = GOTYPE_DEAD_UNIT;
                InterfaceManager * pIntfc = g_pMainGameRoot->m_pLocalClient->getInterface();
                pIntfc->showStack((MapTile*)pObj, GOTYPE_MAPTILE | GOTYPE_MAPOBJECT, uFilter, NULL, NULL);
                guiFrame * pFrm = pIntfc->findFrameFromDoc(pIntfc->getInfoDialog());
                pFrm->flash(1.0f);
                pIntfc->bringFrameAbove(pFrm);
                return;
            }
        }
        break;
        case SELECT_TYPE_BUILDING:
        {
            // Not finished yet: still have to open town screen to select a building
            // TODO
            return;
        }
        //case SELECT_TYPE_SPELL_IN_PLAY:
        //  {
        //    // Not finished yet: still have to open a popup with all units and spells
        //    g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showActiveSpellsOnTile((MapTile*) pObj);
        //    return;
        //  }
        }
        // Ok, target is valid
        g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->hide();
        g_pMainGameRoot->m_pLocalClient->getInterface()->getPlayerSelectorDialog()->hide();
        g_pMainGameRoot->m_pLocalClient->getInterface()->getStatusDialog()->hide();
        g_pMainGameRoot->m_pLocalClient->getGameboard()->unsetTargetMode();
        g_pMainGameRoot->m_pLocalClient->getPlayerManager()->enableEOT(true);
        char sParams[64];
        char sInfos[256] = "";
        wsafecpy(sParams, 64, pObj->getIdentifiers());
        if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
        {
            Spell * pSpell = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSpellBeingCast();
            pSpell->addResolveParameters(sParams);
            if (strcmp(g_sLuaSelectCallback, "") == 0)
                g_pMainGameRoot->m_pLocalClient->getPlayerManager()->castSpellFinished(true, g_bLuaSelectOnResolve);
            else
                pSpell->callLuaFunction(g_sLuaSelectCallback, 0, "");
        }
        else if (g_uLuaCurrentObjectType == LUAOBJECT_SKILL)
        {
            ChildEffect * pEffect = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSkillBeingActivated();
            pEffect->addResolveParameters(sParams);
            if (strcmp(g_sLuaSelectCallback, "") == 0)
                g_pMainGameRoot->m_pLocalClient->getPlayerManager()->skillWasActivated(true, g_bLuaSelectOnResolve);
            else
                pEffect->getLua()->callLuaFunction(g_sLuaSelectCallback, 0, "");
        }
    }
    else  // Invalid target
        clbkSelectTarget_cancelSelection();
}

bool clbkSelectTarget_OnMouseOverInterface(BaseObject * pBaseObj, u8 enum1Spell2Player3Mapobj)
{
    if (enum1Spell2Player3Mapobj == 1)
    {
        LuaObject * pObj = (LuaObject*) pBaseObj;
        switch (g_uLuaSelectTargetType)
        {
        case SELECT_TYPE_SPELL_IN_PLAY:
        case SELECT_TYPE_SPELL_IN_HAND:
        case SELECT_TYPE_SPELL_IN_DECK:
        case SELECT_TYPE_SPELL_IN_DISCARD:
            return (pObj->getType() == LUAOBJECT_SPELL) && checkTargetSpell((Spell*)pObj);
        }
    }
    else if (enum1Spell2Player3Mapobj == 2)
    {
        if (g_uLuaSelectTargetType == SELECT_TYPE_PLAYER)
            return checkTargetPlayer((Player*)pBaseObj);
    }
    else if (enum1Spell2Player3Mapobj == 3)
    {
        GraphicObject * pObj = (GraphicObject*) pBaseObj;
        switch (g_uLuaSelectTargetType)
        {
        case SELECT_TYPE_UNIT:
        case SELECT_TYPE_DEAD_UNIT:
        case SELECT_TYPE_TOWN:
        case SELECT_TYPE_TEMPLE:
            return (pObj->getType() & GOTYPE_MAPOBJECT) && checkTargetObject((MapObject*)pObj);
        }
    }
    return false;
}

bool clbkSelectTarget_OnClickInterface(BaseObject * pObj, u8 enum1Spell2Player3Mapobj)
{
    if (pObj == NULL)
        return false;
    char sParams[64] = "";
    char sInfos[256] = "";
    bool bOk = false;
    if (enum1Spell2Player3Mapobj == 1)
    {
        bOk = true;
        snprintf(sParams, 64, "spell %d %ld", (int)((Spell*)pObj)->getPlayerId(), (long)((Spell*)pObj)->getInstanceId());
        g_pMainGameRoot->m_pLocalClient->getInterface()->getSpellsSelectorDialog()->hide();
    }
    else if (enum1Spell2Player3Mapobj == 2)
    {
        bOk = true;
        wsafecpy(sParams, 64, ((Player*) pObj)->getIdentifiers());
        g_pMainGameRoot->m_pLocalClient->getInterface()->getPlayerSelectorDialog()->hide();
    }
    else
    {
        if ((((GraphicObject*)pObj)->getType() & GOTYPE_MAPOBJECT) && checkTargetObject((MapObject*)pObj))
        {
            // Ok, target is valid
            bOk = true;
            wsafecpy(sParams, 64, ((MapObject*) pObj)->getIdentifiers());
        }
    }
    g_pMainGameRoot->m_pLocalClient->getInterface()->getStatusDialog()->hide();
    g_pMainGameRoot->m_pLocalClient->getPlayerManager()->enableEOT(true);

    if (bOk)
    {
        if (g_uLuaCurrentObjectType == LUAOBJECT_SPELL)
        {
            g_pMainGameRoot->m_pLocalClient->getGameboard()->unsetTargetMode();
            Spell * pSpell = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSpellBeingCast();
            pSpell->addResolveParameters(sParams);
            if (strcmp(g_sLuaSelectCallback, "") == 0)
                g_pMainGameRoot->m_pLocalClient->getPlayerManager()->castSpellFinished(true, g_bLuaSelectOnResolve);
            else
                pSpell->callLuaFunction(g_sLuaSelectCallback, 0, "");
        }
        else if (g_uLuaCurrentObjectType == LUAOBJECT_SKILL)
        {
            g_pMainGameRoot->m_pLocalClient->getGameboard()->unsetTargetMode();
            ChildEffect * pEffect = g_pMainGameRoot->m_pLocalClient->getPlayerManager()->getSkillBeingActivated();
            pEffect->addResolveParameters(sParams);
            if (strcmp(g_sLuaSelectCallback, "") == 0)
                g_pMainGameRoot->m_pLocalClient->getPlayerManager()->skillWasActivated(true, g_bLuaSelectOnResolve);
            else
                pEffect->getLua()->callLuaFunction(g_sLuaSelectCallback, 0, "");
        }
    }
    else
        clbkSelectTarget_cancelSelection();
    return true;
}

int LUA_split(lua_State * pState)
{
    if (!checkNumberOfParams(pState, 2, "split"))
        return 0;
    char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
    strncpy(sParams, lua_tostring(pState, 1), LUA_FUNCTION_PARAMS_MAX_CHARS);
    char sSep[2];
    strncpy(sSep, lua_tostring(pState, 2), 2);

    char sSingleParam[128] = "";
    int i = 0;
    int i2 = 0;
    int countParams = 0;
    while (true)
    {
        if (sParams[i] == sSep[0] || sParams[i] == '\0')
        {
            if (i2 > 0)
            {
                sSingleParam[i2] = '\0';
                lua_pushstring(pState, sSingleParam);
                countParams++;
                i2 = 0;
                sSingleParam[0] = '\0';
            }
            if (sParams[i] == '\0')
                break;
        }
        else
            sSingleParam[i2++] = sParams[i];
        i++;
    }

    return countParams;
}

int LUA_splitint(lua_State * pState)
{
    if (!checkNumberOfParams(pState, 2, "splitint"))
        return 0;
    char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
    strncpy(sParams, lua_tostring(pState, 1), LUA_FUNCTION_PARAMS_MAX_CHARS);
    char sSep[2];
    strncpy(sSep, lua_tostring(pState, 2), 2);

    int sign = 1;
    int iVal = 0;
    bool bStarted = false;
    int i = 0;
    int countParams = 0;
    while (true)
    {
        if (sParams[i] == sSep[0] || sParams[i] == '\0')
        {
            if (bStarted)
            {
                lua_pushnumber(pState, sign * iVal);
                countParams++;
                bStarted = false;
                iVal = 0;
                sign = 1;
            }
            if (sParams[i] == '\0')
                break;
        }
        else
        {
            if (!bStarted && sParams[i] == '-')
                sign = -1;
            bStarted = true;
            int digit = sParams[i] - '0';
            if (digit >= 0 && digit <= 9)
                iVal = 10 * iVal + digit;
        }
        i++;
    }

    return countParams;
}
