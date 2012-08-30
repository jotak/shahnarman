#ifndef _SKILL_H
#define _SKILL_H

#include "../Data/LuaObject.h"
#include "../Server/NetworkData.h"

#define SKILL_OBJECT_NAME   L"skills"

class UnitData;
class Unit;

class Skill : public LuaObject
{
public:
  Skill(wchar_t * sEdition, wchar_t * sObjectName, wchar_t * sParams, DebugManager * pDebug);
  Skill(u32 uId, wchar_t * sEdition, wchar_t * sObjectName, wchar_t * sParams, DebugManager * pDebug);
  ~Skill();

  virtual u32 getType() { return LUAOBJECT_SKILL; };
  virtual void loadBasicData(DebugManager * pDebug);
  static Skill * deserialize(u32 uInstanceId, NetworkData * pData, DebugManager * pDebug);
  void serialize(NetworkData * pData);
  Skill * clone(bool bKeepInstance, DebugManager * pDebug);
  wchar_t * getLocalizedName() { return m_sName; };
  wchar_t * getLocalizedDescription() { return m_sDescription; };
  wchar_t * getParameters() { return m_sParameters; };
  void setCaster(Unit * pCaster) { m_pCaster = pCaster; };
  Unit * getCaster() { return m_pCaster; };
  wchar_t * getIconPath() { return m_sIconPath; };
  bool isMergeable() { return m_bMergeable; };
  bool isCumulative() { return m_bCumulative; };
  void merge(Skill * pOther);

protected:
  void init(wchar_t * sParams, DebugManager * pDebug);

  wchar_t m_sName[NAME_MAX_CHARS];
  wchar_t m_sDescription[DESCRIPTION_MAX_CHARS];
  wchar_t m_sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
  wchar_t m_sIconPath[MAX_PATH];
  bool m_bMergeable;
  bool m_bCumulative;

  Unit * m_pCaster; // Temporary data used when activating skill
};

#endif
