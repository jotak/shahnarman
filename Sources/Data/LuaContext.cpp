#include "LuaContext.h"
#include "../Players/PlayerManagerAbstract.h"
#include "../Players/Player.h"
#include "../Players/Spell.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/Building.h"
#include "../Gameboard/SpecialTile.h"

// -----------------------------------------------------------------
// Name : retrieve
// -----------------------------------------------------------------
bool LuaContext::retrieve(PlayerManagerAbstract * pMngr)
{
  pPlayer = NULL;
  pUnit = NULL;
  pTown = NULL;
  pLua = LuaObject::static_pCurrentLuaCaller;
  if (pLua == NULL)
  {
    wsafecpy(sError, 256, L"Lua interaction error: no current LUA caller defined while calling SpellsSolver::retrieveLostContext.");
    return false;
  }
  u32 uType = pLua->getType();
  if (uType == LUAOBJECT_SKILL)
  {
    pUnit = ((Skill*)pLua)->getCaster();
    if (pUnit == NULL)
    {
      swprintf_s(sError, 256, L"Lua interaction error: effect %s activated by invalid unit.", pLua->getLocalizedName(), NAME_MAX_CHARS);
      return false;
    }
    pPlayer = pMngr->findPlayer(pUnit->getOwner());
    if (pPlayer == NULL)
    {
      swprintf_s(sError, 256, L"Lua interaction error: effect %s activated by invalid player.", pLua->getLocalizedName(), NAME_MAX_CHARS);
      return false;
    }
  }
  else if (uType == LUAOBJECT_SPELL)
  {
    pPlayer = ((Spell*)pLua)->getCaster();
    if (pPlayer == NULL)
    {
      swprintf_s(sError, 256, L"Lua interaction error: effect %s activated but current player is null.", pLua->getLocalizedName(), NAME_MAX_CHARS);
      return false;
    }
  }
  else if (uType == LUAOBJECT_BUILDING)
  {
    pTown = ((Building*)pLua)->getCaster();
    if (pTown == NULL)
    {
      swprintf_s(sError, 256, L"Lua interaction error: building effect %s activated but current town is null.", pLua->getLocalizedName(), NAME_MAX_CHARS);
      return false;
    }
    pPlayer = pMngr->findPlayer(pTown->getOwner());
    if (pPlayer == NULL)
    {
      swprintf_s(sError, 256, L"Lua interaction error: building effect %s activated by invalid player.", pLua->getLocalizedName(), NAME_MAX_CHARS);
      return false;
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void LuaContext::serialize(NetworkData * pData)
{
  assert(pLua != NULL);
  u32 uType = pLua->getType();
  pData->addLong((long) uType);
  switch (uType)
  {
  case LUAOBJECT_SPELL:
    assert(pPlayer != NULL);
    pData->addLong((long) pPlayer->m_uPlayerId);
    break;
  case LUAOBJECT_SKILL:
    assert(pPlayer != NULL && pUnit != NULL);
    pData->addLong((long) pPlayer->m_uPlayerId);
    pData->addLong((long) pUnit->getId());
    break;
  case LUAOBJECT_BUILDING:
    assert(pTown != NULL);
    pData->addLong((long) pTown->getId());
    break;
  case LUAOBJECT_SPECIALTILE:
    break;
  }
  pData->addLong((long) pLua->getInstanceId());
  pData->addLong((long) pLua->getCurrentEffect());
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
bool LuaContext::deserialize(NetworkData * pData, PlayerManagerAbstract * pMngr, Map * pMap)
{
  pLua = NULL;
  pPlayer = NULL;
  pUnit = NULL;
  pTown = NULL;
  u32 uType = (u32) pData->readLong();
  switch (uType)
  {
  case LUAOBJECT_SPELL:
    {
      // Find player
      u8 uPlayerId = (u8) pData->readLong();
      pPlayer = pMngr->findPlayer(uPlayerId);
      assert(pPlayer != NULL);
      // Find spell
      u32 uLuaId = (u32) pData->readLong();
      pLua = pPlayer->findSpell(0, uLuaId, pPlayer->m_pActiveSpells);
      assert(pLua != NULL);
      ((Spell*)pLua)->setCaster(pPlayer);
      break;
    }
  case LUAOBJECT_SKILL:
    {
      // Find player
      u8 uPlayerId = (u8) pData->readLong();
      pPlayer = pMngr->findPlayer(uPlayerId);
      assert(pPlayer != NULL);
      // Find unit
      u32 uUnitId = (u32) pData->readLong();
      pUnit = pPlayer->findUnit(uUnitId);
      assert(pUnit != NULL);
      // Find skill
      u32 uLuaId = (u32) pData->readLong();
      pLua = pUnit->findSkill(uLuaId);
      assert(pLua != NULL);
      ((Skill*)pLua)->setCaster(pUnit);
      break;
    }
  case LUAOBJECT_BUILDING:
    {
      // Find town
      u32 uTown = (u32) pData->readLong();
      pTown = pMap->findTown(uTown);
      assert(pTown != NULL);
      // Find building
      u32 uLuaId = (u32) pData->readLong();
      Building * pBuild = pTown->getFirstBuilding(0);
      while (pBuild != NULL)
      {
        if (pBuild->getInstanceId() == uLuaId)
        {
          pLua = pBuild;
          break;
        }
        pBuild = pTown->getNextBuilding(0);
      }
      assert(pLua != NULL);
      ((Building*)pLua)->setCaster(pTown);
      break;
    }
  case LUAOBJECT_SPECIALTILE:
    {
      // Find special tile
      u32 uLuaId = (u32) pData->readLong();
      pLua = pMap->findSpecialTile(uLuaId);
      assert(pLua != NULL);
      break;
    }
  }
  pLua->setCurrentEffect((int) pData->readLong());
  return true;
}

// -----------------------------------------------------------------
// Name : serializeTargets
// -----------------------------------------------------------------
void LuaContext::serializeTargets(NetworkData * pData)
{
  assert(pLua != NULL);
  serialize(pData);
  pData->addLong(pLua->getTargets()->size);
  BaseObject * pObj = (BaseObject*) pLua->getTargets()->getFirst(0);
  while (pObj != NULL)
  {
    long type = pLua->getTargets()->getCurrentType(0);
    pData->addLong(type);
    LuaTargetable * pTarget = NULL;
    switch (type)
    {
    case LUATARGET_PLAYER:
      pTarget = (Player*)pObj;
      break;
    case LUATARGET_TILE:
      pTarget = (MapTile*)pObj;
      break;
    case LUATARGET_TOWN:
      pTarget = (Town*)pObj;
      break;
    case LUATARGET_TEMPLE:
      pTarget = (Temple*)pObj;
      break;
    case LUATARGET_UNIT:
      pTarget = (Unit*)pObj;
      break;
    }
    assert(pTarget != NULL);
    if (pTarget->getDisabledEffects()->goTo(0, pLua))
      pData->addLong(0);
    else
      pData->addLong(1);
    pData->addString(pTarget->getIdentifiers());
    pObj = (BaseObject*) pLua->getTargets()->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : deserializeTargets
// -----------------------------------------------------------------
bool LuaContext::deserializeTargets(NetworkData * pData, PlayerManagerAbstract * pMngr, Map * pMap)
{
  if (!deserialize(pData, pMngr, pMap))
    return false;
  pLua->getTargets()->deleteAll();
  int nbTargets = pData->readLong();
  wchar_t ids[16];
  for (int i = 0; i < nbTargets; i++)
  {
    long type = pData->readLong();
    bool bEnabled = (pData->readLong() == 1);
    pData->readString(ids);
    LuaTargetable * pTarget = pMngr->findTargetFromIdentifiers(type, ids, pMap);
    pTarget->attachEffect(pLua);
    if (!bEnabled)
      pTarget->disableEffect(pLua);
    switch (type)
    {
    case LUATARGET_PLAYER:
      pLua->addTarget((Player*)pTarget, type);
      break;
    case LUATARGET_TILE:
      pLua->addTarget((MapTile*)pTarget, type);
      break;
    case LUATARGET_TOWN:
      pLua->addTarget((Town*)pTarget, type);
      break;
    case LUATARGET_TEMPLE:
      pLua->addTarget((Temple*)pTarget, type);
      break;
    case LUATARGET_UNIT:
      pLua->addTarget((Unit*)pTarget, type);
      break;
    }
  }
  return true;
}
