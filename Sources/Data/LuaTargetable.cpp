// -----------------------------------------------------------------
// LuaTargetable
// -----------------------------------------------------------------
#include "LuaTargetable.h"
#include "../Server/NetworkData.h"
#include "../Players/PlayerManager.h"
#include "../Players/Spell.h"
#include "../Players/Player.h"
#include "../Gameboard/Skill.h"
#include "../Gameboard/Building.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : LuaTargetable
//  Constructor
// -----------------------------------------------------------------
LuaTargetable::LuaTargetable(ObjectList ** pGlobalEffects, wchar_t * sIdentifiers)
{
  m_pEffects = new ObjectList(false);
  m_pDisabledEffects = new ObjectList(false);
  m_pChildEffects = new ObjectList(false);
  m_pGlobalEffects = pGlobalEffects;
  wsafecpy(m_sIdentifiers, 16, sIdentifiers);
  m_uGetModInstance = 0;
}

// -----------------------------------------------------------------
// Name : ~LuaTargetable
//  Destructor
// -----------------------------------------------------------------
LuaTargetable::~LuaTargetable()
{
  delete m_pEffects;
  delete m_pDisabledEffects;
  delete m_pChildEffects;
}

// -----------------------------------------------------------------
// Name : attachEffect
// -----------------------------------------------------------------
void LuaTargetable::attachEffect(LuaObject * pEffect)
{
  m_pEffects->addLast(pEffect);
}

// -----------------------------------------------------------------
// Name : detachEffect
// -----------------------------------------------------------------
void LuaTargetable::detachEffect(LuaObject * pEffect)
{
  m_pEffects->deleteObject(pEffect, true);
}

// -----------------------------------------------------------------
// Name : disableEffect
// -----------------------------------------------------------------
void LuaTargetable::disableEffect(LuaObject * pEffect)
{
  if (m_pEffects->goTo(0, pEffect))
  {
    m_pDisabledEffects->addLast(pEffect);
    m_pEffects->deleteCurrent(0, true);
  }
}

// -----------------------------------------------------------------
// Name : enableAllEffects
// -----------------------------------------------------------------
void LuaTargetable::enableAllEffects()
{
  LuaObject * pLua = (LuaObject*) m_pDisabledEffects->getFirst(0);
  while (pLua != NULL)
  {
    m_pEffects->addLast(pLua);
    pLua = (LuaObject*) m_pDisabledEffects->deleteCurrent(0, true);
  }
}

// -----------------------------------------------------------------
// Name : attachChildEffect
// -----------------------------------------------------------------
void LuaTargetable::attachChildEffect(ChildEffect * pEffect)
{
  m_pChildEffects->addLast(pEffect);
}

// -----------------------------------------------------------------
// Name : detachChildEffect
// -----------------------------------------------------------------
bool LuaTargetable::detachChildEffect(ChildEffect * pEffect)
{
  return (m_pChildEffects->deleteObject(pEffect, true) == 1);
}

// -----------------------------------------------------------------
// Name : registerValue
// -----------------------------------------------------------------
void LuaTargetable::registerValue(const wchar_t * sName, long initialValue)
{
  m_lValues[sName] = initialValue;
}

// -----------------------------------------------------------------
// Name : setBaseValue
//  return false if variable not found
// -----------------------------------------------------------------
bool LuaTargetable::setBaseValue(const wchar_t * sName, long baseValue)
{
  long_hash::iterator it = m_lValues.find(sName);
  if (it == m_lValues.end())
    return false;
  it->second = baseValue;
  return true;
}

// -----------------------------------------------------------------
// Name : getValue
// -----------------------------------------------------------------
long LuaTargetable::getValue(const wchar_t * sName, bool bBase, bool * bFound)
{
  long_hash::iterator it = m_lValues.find(sName);
  if (it == m_lValues.end())
  {
    if (bFound == NULL)
      assert(false);
    else
      *bFound = false;
    return 0;
  }
  if (bFound != NULL)
    *bFound = true;
  if (bBase)
    return it->second;
  wchar_t sFuncName[64];
  swprintf_s(sFuncName, 64, L"getMod_%s", sName);
  double val = (double) it->second;
  // Look in normal effects for modifiers
  LuaObject * pEffect = (LuaObject*) m_pEffects->getFirst(0);
  while (pEffect != NULL)
  {
    if (pEffect->callLuaFunction(sFuncName, 1, L"ds", val, m_sIdentifiers))
      val = pEffect->getLuaNumber();
    pEffect = (LuaObject*) m_pEffects->getNext(0);
  }
  assert(m_pGlobalEffects != NULL && *m_pGlobalEffects != NULL);
  // Look in global effects for modifiers
  pEffect = (LuaObject*) (*m_pGlobalEffects)->getFirst(0);
  while (pEffect != NULL)
  {
    if (pEffect->callLuaFunction(sFuncName, 1, L"ds", val, m_sIdentifiers))
      val = pEffect->getLuaNumber();
    pEffect = (LuaObject*) (*m_pGlobalEffects)->getNext(0);
  }
  // Look in child effects for modifiers
  swprintf_s(sFuncName, 64, L"child_getMod_%s", sName);
  // Note: the same child effect can be attached several times on a target. So, m_uGetModInstance is used to let LUA script know if successive calls of "getMod_xxx" are of the same "instance".
  m_uGetModInstance++;
  m_uGetModInstance %= 999;
  ChildEffect * pChild = (ChildEffect*) m_pChildEffects->getFirst(0);
  while (pChild != NULL)
  {
    LuaObject * pLua = pChild->getLua();
    if (pLua->callLuaFunction(sFuncName, 1, L"idsi", (int) (pChild->id + 1), val, m_sIdentifiers, (int) m_uGetModInstance))
      val = pLua->getLuaNumber();
    pChild = (ChildEffect*) m_pChildEffects->getNext(0);
  }
  return (long) val;
}

// -----------------------------------------------------------------
// Name : serializeValues
// -----------------------------------------------------------------
void LuaTargetable::serializeValues(NetworkData * pData)
{
  pData->addLong(m_lValues.size());
  long_hash::iterator it;
  for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
  {
    pData->addString(it->first.c_str());
    pData->addLong(it->second);
  }
}

// -----------------------------------------------------------------
// Name : deserializeValues
// -----------------------------------------------------------------
void LuaTargetable::deserializeValues(NetworkData * pData)
{
  int nbValues = (int) pData->readLong();
  for (int i = 0; i < nbValues; i++)
  {
    wchar_t sKey[64];
    pData->readString(sKey);
    setBaseValue(sKey, pData->readLong());
  }
}

//// -----------------------------------------------------------------
//// Name : serializeEffects
//// -----------------------------------------------------------------
//void LuaTargetable::serializeEffects(NetworkData * pData, ObjectList * pEffectsList)
//{
//  pData->addLong(pEffectsList->size);
//  LuaObject * pLua = (LuaObject*) pEffectsList->getFirst(0);
//  while (pLua != NULL)
//  {
//    pData->addLong(pLua->getType());
//    if (pLua->getType() == LUAOBJECT_SPELL)
//      pData->addLong(((Spell*)pLua)->getPlayerId());
//    pData->addLong(pLua->getInstanceId());
//    pLua = (LuaObject*) pEffectsList->getNext(0);
//  }
//}
//
//// -----------------------------------------------------------------
//// Name : deserializeEffects
//// -----------------------------------------------------------------
//void LuaTargetable::deserializeEffects(NetworkData * pData, ObjectList * pEffectsList, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData, u8 uRelinkType)
//{
//  long nbEffects = pData->readLong();
//  for (int i = 0; i < nbEffects; i++)
//  {
//    u32 uType = pData->readLong();
//    switch (uType)
//    {
//    case LUAOBJECT_SPELL:
//      {
//        u8 owner = (u8) pData->readLong();
//        u32 id = (u32) pData->readLong();
//        Player * pPlayer = pLocalClient->getPlayerManager()->findPlayer(owner);
//        assert(pPlayer != NULL);
//        Spell * pSpell = pPlayer->findSpell(0, id, pPlayer->m_pActiveSpells);
//        assert(pSpell != NULL);
//        pEffectsList->addLast(pSpell);
//        break;
//      }
//    case LUAOBJECT_SKILL:
//    case LUAOBJECT_BUILDING:
//    case LUAOBJECT_SPECIALTILE:
//      {
//        u32 id = (u32) pData->readLong();
//        LuaObject * pLua = (LuaObject*) pEffectsRef->getFirst(0);
//        while (pLua != NULL)
//        {
//          if (uType == pLua->getType() && id == pLua->getInstanceId())
//          {
//            pEffectsList->addLast(pLua);
//            break;
//          }
//          pLua = (LuaObject*) pEffectsRef->getNext(0);
//        }
//        break;
//      }
//      assert(relinkPtrData != NULL);  // if there's a spell attached, then we MUST have a relinkPtrData (it can be NULL only when the unit is just born, so when it doesn't have any spell attached)
//      // Use relink structure to later link this unit in spell's targets
//      RELINK_PTR_DATA data;
//      data.type = uRelinkType;
//      data.data1 = (u32) this;
//      data.data2 = uType;
//      data.data3 = owner;
//      data.data4 = id;
//      relinkPtrData->push(data);
//    }
//  }
//}

// -----------------------------------------------------------------
// Name : _callEffectHandlerForEffect
//  Private
// -----------------------------------------------------------------
bool LuaTargetable::_callEffectHandlerForEffect(LuaObject * pLua, int iChild, wchar_t * sFunc, wchar_t * sArgsType, void ** pArgs, int nbResults)
{
  lua_State * pState = pLua->prepareLuaFunction(sFunc);
  if (pState)
  {
    int i = 0;
    wchar_t sParams[512] = L"";
    wchar_t sBuf[128];
    char ids[LUA_FUNCTION_PARAMS_MAX_CHARS];
    wtostr(ids, LUA_FUNCTION_PARAMS_MAX_CHARS, m_sIdentifiers);
    // First argument may be child effect id
    if (iChild >= 0)
      lua_pushnumber(pState, iChild);
    // Then object's identifiers
    lua_pushstring(pState, ids);
    if (iChild >= 0)
      swprintf_s(sParams, 512, L"%d,%s", iChild, m_sIdentifiers);
    else
      swprintf_s(sParams, 512, L"%s,", m_sIdentifiers);
    // Parse sParamsType to read following parameters
    while (sArgsType[i] != L'\0')
    {
      if (sArgsType[i] == L'i')
      {
        int val = *(int*)(pArgs[i]);
        lua_pushnumber(pState, val);
        swprintf_s(sBuf, 128, L"%d,", val);
        wsafecat(sParams, 512, sBuf);
      }
      else if (sArgsType[i] == L'l')
      {
        long val = *(long*)(pArgs[i]);
        lua_pushnumber(pState, val);
        swprintf_s(sBuf, 128, L"%ld,", val);
        wsafecat(sParams, 512, sBuf);
      }
      else if (sArgsType[i] == L'f')
      {
        float val = *(float*)(pArgs[i]);
        lua_pushnumber(pState, val);
        swprintf_s(sBuf, 128, L"%f,", val);
        wsafecat(sParams, 512, sBuf);
      }
      else if (sArgsType[i] == L'd')
      {
        double val = *(double*)(pArgs[i]);
        lua_pushnumber(pState, val);
        swprintf_s(sBuf, 128, L"%lf,", val);
        wsafecat(sParams, 512, sBuf);
      }
      else if (sArgsType[i] == L's')
      {
        wchar_t * val = (wchar_t*)(pArgs[i]);
        // Convert wchar_t arguments to ASCII
        char charval[LUA_FUNCTION_PARAMS_MAX_CHARS];
        wtostr(charval, LUA_FUNCTION_PARAMS_MAX_CHARS, val);
        lua_pushstring(pState, charval);
        swprintf_s(sBuf, 128, L"%s,", val);
        wsafecat(sParams, 512, sBuf);
      }
      i++;
    }
    if (sParams[0] != L'\0')
      sParams[wcslen(sParams) - 1] = L'\0';
    return pLua->callPreparedLuaFunction(iChild >= 0 ? i+2 : i+1, nbResults, sFunc, sParams);
  }
  return false;
}

// -----------------------------------------------------------------
// Name : callEffectHandler
// -----------------------------------------------------------------
bool LuaTargetable::callEffectHandler(wchar_t * sFunc, wchar_t * sArgsType, void ** pArgs, u8 uResultType)
{
  bool bResult = true;
  if (uResultType == HANDLER_RESULT_TYPE_BOR)
    bResult = false;
  int nResults = 1;
  if (uResultType == HANDLER_RESULT_TYPE_NONE)
    nResults = 0;

  LuaObject * pEffect = getFirstEffect(0);
  while (pEffect != NULL)
  {
    if (_callEffectHandlerForEffect(pEffect, -1, sFunc, sArgsType, pArgs, nResults))
    {
      if (uResultType == HANDLER_RESULT_TYPE_BOR)
        bResult |= (pEffect->getLuaNumber() == 1);
      if (uResultType == HANDLER_RESULT_TYPE_BAND)
        bResult &= (pEffect->getLuaNumber() == 1);
    }
    pEffect = getNextEffect(0);
  }
  // Look in global effects
  pEffect = (LuaObject*) (*m_pGlobalEffects)->getFirst(0);
  while (pEffect != NULL)
  {
    if (_callEffectHandlerForEffect(pEffect, -1, sFunc, sArgsType, pArgs, nResults))
    {
      if (uResultType == HANDLER_RESULT_TYPE_BOR)
        bResult |= (pEffect->getLuaNumber() == 1);
      if (uResultType == HANDLER_RESULT_TYPE_BAND)
        bResult &= (pEffect->getLuaNumber() == 1);
    }
    pEffect = (LuaObject*) (*m_pGlobalEffects)->getNext(0);
  }
  wchar_t sChildFunc[128];
  swprintf_s(sChildFunc, 128, L"child_%s", sFunc);
  ChildEffect * pChild = getFirstChildEffect(0);
  while (pChild != NULL)
  {
    pEffect = pChild->getLua();
    if (_callEffectHandlerForEffect(pEffect, pChild->id + 1, sChildFunc, sArgsType, pArgs, nResults))
    {
      if (uResultType == HANDLER_RESULT_TYPE_BOR)
        bResult |= (pEffect->getLuaNumber() == 1);
      if (uResultType == HANDLER_RESULT_TYPE_BAND)
        bResult &= (pEffect->getLuaNumber() == 1);
    }
    pChild = getNextChildEffect(0);
  }
  return bResult;
}

// -----------------------------------------------------------------
// Name : getInfo_AddValue
// -----------------------------------------------------------------
void LuaTargetable::getInfo_AddValue(wchar_t * sBuf, int iSize, const wchar_t * sKey, const wchar_t * sSeparator)
{
  wchar_t sKeyText[64];
  wchar_t sTemp[128];
  wchar_t s2P[8];
  i18n->getText1stUp(sKey, sKeyText, 64);
  i18n->getText1stUp(L"2P", s2P, 8);
  swprintf_s(sTemp, 128, L"%s%s%ld%s",
        sKeyText,
        s2P,
        getValue(sKey),
        sSeparator
  );
  wsafecat(sBuf, iSize, sTemp);
}
