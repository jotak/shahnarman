#ifndef _LUA_TARGETABLE_H
#define _LUA_TARGETABLE_H

#include "../Common/ObjectList.h"
#include "LuaObject.h"

#define LUATARGET_PLAYER    1
#define LUATARGET_TILE      2
#define LUATARGET_TOWN      3
#define LUATARGET_UNIT      4
#define LUATARGET_TEMPLE    5

#define HANDLER_RESULT_TYPE_NONE    0
#define HANDLER_RESULT_TYPE_BAND    1
#define HANDLER_RESULT_TYPE_BOR     2

class LocalClient;

class LuaTargetable
{
public:
  LuaTargetable(ObjectList ** pGlobalEffects, wchar_t * sIdentifiers);
  ~LuaTargetable();

  // General
  wchar_t * getIdentifiers() { return m_sIdentifiers; };

  // Effects
  LuaObject * getFirstEffect(int _it) { return (LuaObject*) m_pEffects->getFirst(_it); };
  LuaObject * getNextEffect(int _it) { return (LuaObject*) m_pEffects->getNext(_it); };
  ObjectList * getAllEffects() { return m_pEffects; };
  int getNumberOfEffects() { return m_pEffects->size; };
  void attachEffect(LuaObject * pEffect);
  void detachEffect(LuaObject * pEffect);
  void removeAllEffects() { m_pEffects->deleteAll(); };
  void disableEffect(LuaObject * pEffect);
  void enableAllEffects();
  ObjectList * getDisabledEffects() { return m_pDisabledEffects; }

  // Child effects
  ChildEffect * getFirstChildEffect(int _it) { return (ChildEffect*) m_pChildEffects->getFirst(_it); };
  ChildEffect * getNextChildEffect(int _it) { return (ChildEffect*) m_pChildEffects->getNext(_it); };
  void attachChildEffect(ChildEffect * pEffect);
  bool detachChildEffect(ChildEffect * pEffect);

  // Variables
  void registerValue(const wchar_t * sName, long baseValue);
  virtual long getValue(const wchar_t * sName, bool bBase = false, bool * bFound = NULL);
  virtual bool setBaseValue(const wchar_t * sName, long val);

  // Other
  bool callEffectHandler(wchar_t * sFunc, wchar_t * sArgsType = L"", void ** pArgs = NULL, u8 uResultType = HANDLER_RESULT_TYPE_NONE);
  void getInfo_AddValue(wchar_t * sBuf, int iSize, const wchar_t * sKey, const wchar_t * sSeparator);

protected:
  void serializeValues(NetworkData * pData);
  void deserializeValues(NetworkData * pData);
  //void serializeEffects(NetworkData * pData, ObjectList * pEffectsList);
  //void deserializeEffects(NetworkData * pData, ObjectList * pEffectsList, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData, u8 uRelinkType);

  long_hash m_lValues;
  ObjectList ** m_pGlobalEffects;
  wchar_t m_sIdentifiers[16];

private:
  bool _callEffectHandlerForEffect(LuaObject * pLua, int iChild, wchar_t * sFunc, wchar_t * sArgsType, void ** pArgs, int nbResults);

  ObjectList * m_pEffects;
  ObjectList * m_pDisabledEffects;
  ObjectList * m_pChildEffects; // Child effects are not LuaObjects
  u16 m_uGetModInstance;
};

#endif