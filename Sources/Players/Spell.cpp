#include "Spell.h"
#include "../Common/ObjectList.h"
#include "../Data/LuaTargetable.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : Spell
//  Constructor
// -----------------------------------------------------------------
Spell::Spell(u8 uPlayerId, char * sEdition, short iFreq, char * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, SPELL_OBJECT_NAME, sObjectName, pDebug)
{
  init(uPlayerId, iFreq, pDebug);
}

// -----------------------------------------------------------------
// Name : Spell
//  Constructor with id
// -----------------------------------------------------------------
Spell::Spell(u32 uId, u8 uPlayerId, char * sEdition, short iFreq, char * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, SPELL_OBJECT_NAME, sObjectName, pDebug)
{
  init(uPlayerId, iFreq, pDebug);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Spell::init(u8 uPlayerId, short iFreq, DebugManager * pDebug)
{
  m_uPlayerId = uPlayerId;
  m_iFrequency = iFreq;
  wsafecpy(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
  wsafecpy(m_sTargetInfo, 256, "");
  m_bGlobal = false;
  m_bAllowedInBattle = false;
  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void Spell::loadBasicData(DebugManager * pDebug)
{
  wsafecpy(m_sIconPath, MAX_PATH, "");

  // Get some basic parameters
  // Spell name
  if (callLuaFunction("getName", 1, ""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: spell in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, "");
  }

  // Casting cost
  double dMana[4];
  if (!getLuaVarNumberArray("cost", dMana, 4))
  {
	  // error : cost not found
    char sError[512] = "";
    snprintf(sError, 512, "Lua interaction error: spell in file %s has no cost defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
  }
  else
  {
    for (int i = 0; i < 4; i++)
      m_CastingCost.mana[i] = (u8) dMana[i];
  }

  // Description
  if (callLuaFunction("getDescription", 1, ""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: spell in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, "");
  }

  // Icon texture
  char sStr[MAX_PATH];
  if (!getLuaVarString("icon", sStr, MAX_PATH))
  {
	  // error : icon not found
    char sError[512] = "";
    snprintf(sError, 512, "Lua interaction error: spell in file %s has no icon path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sIconPath, MAX_PATH, "");
  }
  else
    snprintf(m_sIconPath, MAX_PATH, "%s/%s", m_sObjectEdition, sStr);

  double val;
  // Battle spell?
  if (getLuaVarNumber("allowedInBattle", &val))
    m_bAllowedInBattle = (val >= 1);
}

// -----------------------------------------------------------------
// Name : ~Spell
//  Destructor
// -----------------------------------------------------------------
Spell::~Spell()
{
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Spell::serialize(NetworkData * pData)
{
  pData->addLong((long)m_uPlayerId);
  pData->addLong((long)m_uInstanceId);
  pData->addString(m_sObjectEdition);
  pData->addString(m_sObjectName);
  pData->addLong((long)m_bGlobal);
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
Spell * Spell::deserialize(NetworkData * pData, DebugManager * pDebug)
{
  char sEdition[NAME_MAX_CHARS];
  char sFilename[NAME_MAX_CHARS];
  u8 uPlayerId = (u8) pData->readLong();
  u32 uSpellId = (u32) pData->readLong();
  pData->readString(sEdition);
  pData->readString(sFilename);
  long iGlobal = pData->readLong();
  Spell * pSpell = new Spell(uSpellId, uPlayerId, sEdition, 1, sFilename, pDebug);
  if (iGlobal)
    pSpell->setGlobal();
  return pSpell;
}

// -----------------------------------------------------------------
// Name : resetResolveParameters
// -----------------------------------------------------------------
void Spell::resetResolveParameters()
{
  wsafecpy(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
}

// -----------------------------------------------------------------
// Name : addResolveParameters
// -----------------------------------------------------------------
void Spell::addResolveParameters(char * sParams)
{
  wsafecat(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, " ");
  wsafecat(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams);
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
char * Spell::getInfo(char * sBuf, int iSize)
{
  char sMana[32] = "";
  getManaText(sMana, 32);
  snprintf(sBuf, iSize, "%s (%s)\n%s",
        m_sName,
        sMana,
        m_sDescription
  );
  if (strcmp(m_sTargetInfo, "") != 0)
  {
    char sBuf1[256] = "";
    char sBuf2[256] = "";
    i18n->getText("CAST_ON_(s)", sBuf1, 256);
    snprintf(sBuf2, 256, sBuf1, m_sTargetInfo);
    wsafecat(sBuf, iSize, "\n");
    wsafecat(sBuf, iSize, sBuf2);
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getManaText
// -----------------------------------------------------------------
char * Spell::getManaText(char * sMana, int iSize)
{
  char signs[4] = MANA_SIGNS;
  for (int i = 0; i < 4; i++)
  {
    if (m_CastingCost[i] > 0)
    {
      char sBuf[8];
      snprintf(sBuf, 8, "%c %d, ", signs[i], (int)m_CastingCost[i]);
      wsafecat(sMana, 32, sBuf);
    }
  }
  if (sMana[0] != '\0')
    sMana[strlen(sMana)-2] = '\0'; // remove coma
  return sMana;
}
