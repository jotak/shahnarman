#ifndef _SPELL_H
#define _SPELL_H

#include "../Data/LuaObject.h"
#include "Mana.h"
#include "../Server/NetworkData.h"

#define SPELL_OBJECT_NAME   L"spells"

class Player;

class Spell : public LuaObject
{
public:
  Spell(u8 uPlayerId, wchar_t * sEdition, short iFreq, wchar_t * sObjectName, DebugManager * pDebug);
  Spell(u32 uId, u8 uPlayerId, wchar_t * sEdition, short iFreq, wchar_t * sObjectName, DebugManager * pDebug);
  virtual ~Spell();

  void serialize(NetworkData * pData);
  static Spell * deserialize(NetworkData * pData, DebugManager * pDebug);

  virtual u32 getType() { return LUAOBJECT_SPELL; };
  virtual void loadBasicData(DebugManager * pDebug);
  wchar_t * getIconPath() { return m_sIconPath; };
  wchar_t * getResolveParameters() { return m_sResolveParameters; };
  void resetResolveParameters();
  void addResolveParameters(wchar_t * sParams);
  wchar_t * getInfo(wchar_t * sBuf, int iSize);
  wchar_t * getManaText(wchar_t * sBuf, int iSize);
  Mana getCost() { return m_CastingCost; };
  wchar_t * getLocalizedName() { return m_sName; };
  wchar_t * getLocalizedDescription() { return m_sDescription; };
  u8 getPlayerId() { return m_uPlayerId; };
  bool isAllowedInBattle() { return m_bAllowedInBattle; };
  bool isGlobal() { return m_bGlobal; };
  void setGlobal() { m_bGlobal = true; };
  short getFrequency() { return m_iFrequency; };
  void setCaster(Player * pCaster) { m_pCaster = pCaster; };
  Player * getCaster() { return m_pCaster; };
  void setTargetInfo(wchar_t * sInfos) { wsafecpy(m_sTargetInfo, 256, sInfos); };
  wchar_t * getTargetInfo() { return m_sTargetInfo; };

protected:
  void init(u8 uPlayerId, short iFreq, DebugManager * pDebug);

  u8 m_uPlayerId;
  wchar_t m_sIconPath[MAX_PATH];
  Mana m_CastingCost;
  wchar_t m_sName[NAME_MAX_CHARS];
  wchar_t m_sDescription[DESCRIPTION_MAX_CHARS];
  wchar_t m_sResolveParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
  wchar_t m_sTargetInfo[256];
  bool m_bAllowedInBattle;
  bool m_bGlobal;
  short m_iFrequency;   // Frequency is usefull during deck building / shopping. It's not set during game.

  Player * m_pCaster; // Temporary data used when activating skill
};

#endif
