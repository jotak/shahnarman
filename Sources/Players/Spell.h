#ifndef _SPELL_H
#define _SPELL_H

#include "../Data/LuaObject.h"
#include "Mana.h"
#include "../Server/NetworkData.h"

#define SPELL_OBJECT_NAME   "spells"

class Player;

class Spell : public LuaObject
{
public:
  Spell(u8 uPlayerId, char * sEdition, short iFreq, char * sObjectName, DebugManager * pDebug);
  Spell(u32 uId, u8 uPlayerId, char * sEdition, short iFreq, char * sObjectName, DebugManager * pDebug);
  virtual ~Spell();

  void serialize(NetworkData * pData);
  static Spell * deserialize(NetworkData * pData, DebugManager * pDebug);

  virtual u32 getType() { return LUAOBJECT_SPELL; };
  virtual void loadBasicData(DebugManager * pDebug);
  char * getIconPath() { return m_sIconPath; };
  char * getResolveParameters() { return m_sResolveParameters; };
  void resetResolveParameters();
  void addResolveParameters(char * sParams);
  char * getInfo(char * sBuf, int iSize);
  char * getManaText(char * sBuf, int iSize);
  Mana getCost() { return m_CastingCost; };
  char * getLocalizedName() { return m_sName; };
  char * getLocalizedDescription() { return m_sDescription; };
  u8 getPlayerId() { return m_uPlayerId; };
  bool isAllowedInBattle() { return m_bAllowedInBattle; };
  bool isGlobal() { return m_bGlobal; };
  void setGlobal() { m_bGlobal = true; };
  short getFrequency() { return m_iFrequency; };
  void setCaster(Player * pCaster) { m_pCaster = pCaster; };
  Player * getCaster() { return m_pCaster; };
  void setTargetInfo(char * sInfos) { wsafecpy(m_sTargetInfo, 256, sInfos); };
  char * getTargetInfo() { return m_sTargetInfo; };

protected:
  void init(u8 uPlayerId, short iFreq, DebugManager * pDebug);

  u8 m_uPlayerId;
  char m_sIconPath[MAX_PATH];
  Mana m_CastingCost;
  char m_sName[NAME_MAX_CHARS];
  char m_sDescription[DESCRIPTION_MAX_CHARS];
  char m_sResolveParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
  char m_sTargetInfo[256];
  bool m_bAllowedInBattle;
  bool m_bGlobal;
  short m_iFrequency;   // Frequency is usefull during deck building / shopping. It's not set during game.

  Player * m_pCaster; // Temporary data used when activating skill
};

#endif
