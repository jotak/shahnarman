#ifndef _LUA_OBJECT_H
#define _LUA_OBJECT_H

#include "../Common/ObjectList.h"
#include "../Players/Mana.h"
#include <lua5.1/lua.hpp>

class DebugManager;
class NetworkData;
class LuaObject;

#define LUA_FUNCTION_PARAMS_MAX_CHARS   256
#define LUAOBJECT_SPELL                 0x00000001
#define LUAOBJECT_SKILL                 0x00000002
#define LUAOBJECT_BUILDING              0x00000004
#define LUAOBJECT_SPECIALTILE           0x00000008

class ChildEffect : public BaseObject
{
public:
  ChildEffect() { pTargets = new ObjectList(false); };
  ~ChildEffect() { delete pTargets; };
  int id;
  wchar_t sName[NAME_MAX_CHARS];
  wchar_t sIcon[MAX_PATH];
  Mana cost;
  ObjectList * pTargets;
  void resetResolveParameters() { wsafecpy(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, L""); };
  void addResolveParameters(wchar_t * sParams) { wsafecat(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, L" "); wsafecat(sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams); };
  wchar_t sResolveParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
  LuaObject * getLua() { return (LuaObject*) getAttachment(); };
};

class LuaObject : public BaseObject
{
public:
  LuaObject(u32 uInstance, const wchar_t * sEdition, const wchar_t * sObjectType, const wchar_t * sObjectName, DebugManager * pDebug);
  ~LuaObject();

  virtual u32 getType() = 0;
  virtual void loadBasicData(DebugManager * pDebug) = 0;
  bool isLoaded() { return m_pLuaState != NULL; };
  bool callLuaFunction(const wchar_t * sFunc, int iNbResults, const wchar_t * sParamsType, ...);
  lua_State * prepareLuaFunction(const wchar_t * sFunc);
  bool callPreparedLuaFunction(int iNbParams, int iNbResults, const wchar_t * sFunc, const wchar_t * sParams);
  double getLuaNumber();
  void getLuaString(wchar_t * sString, int size);
  bool getLuaVarNumber(const wchar_t * sVarName, double * d);
  bool getLuaVarString(const wchar_t * sVarName, wchar_t * sString, int size);
  bool getLuaVarNumberArray(const wchar_t * sVarName, double * pArray, int size);
  bool getLuaVarStringArray(const wchar_t * sVarName, wchar_t ** pArray, int tabSize, int strSize);
  u32 getInstanceId() { return m_uInstanceId; };
  wchar_t * getUniqueId(wchar_t * sId, int iSize);
  bool isUniqueId(const wchar_t * sEdition, const wchar_t * sObjectType, const wchar_t * sObjectName);
  wchar_t * getObjectEdition() { return m_sObjectEdition; };
  wchar_t * getObjectName() { return m_sObjectName; };
  virtual wchar_t * getLocalizedName() { return NULL; };
  virtual wchar_t * getLocalizedDescription() { return NULL; };
  virtual wchar_t * getIconPath() = 0;
  void setExtraMana(Mana mana) { m_ExtraMana = mana; };
  Mana getExtraMana() { return m_ExtraMana; };

  // Child effects
  int getNbChildEffects() { return m_iNbChildEffects; };
  ChildEffect * getChildEffect(int idx) { return &(m_pChildEffects[idx]); };
  void setCurrentEffect(int id) { m_iCurrentEffect = id; };
  int getCurrentEffect() { return m_iCurrentEffect; };

  // Targets
  ObjectList * getTargets() { return m_pTargets; };
  void addTarget(BaseObject * pTarget, long iType) { m_pTargets->addLast(pTarget, iType); };
  void removeTarget(BaseObject * pTarget) { m_pTargets->deleteObject(pTarget, true); };
  void removeFromTargets();

  static u32 static_uIdGenerator;
  static LuaObject * static_pCurrentLuaCaller;

protected:
  u32 m_uInstanceId;
  wchar_t m_sObjectEdition[NAME_MAX_CHARS];
  wchar_t m_sObjectName[NAME_MAX_CHARS];
  wchar_t m_sObjectType[NAME_MAX_CHARS];
  lua_State * m_pLuaState;
  DebugManager * m_pDebug;
  int m_iNbChildEffects;
  ChildEffect * m_pChildEffects;
  ObjectList * m_pTargets;
  int m_iCurrentEffect;
  Mana m_ExtraMana;
};

#endif
