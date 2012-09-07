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
Spell::Spell(u8 uPlayerId, wchar_t * sEdition, short iFreq, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, SPELL_OBJECT_NAME, sObjectName, pDebug)
{
  init(uPlayerId, iFreq, pDebug);
}

// -----------------------------------------------------------------
// Name : Spell
//  Constructor with id
// -----------------------------------------------------------------
Spell::Spell(u32 uId, u8 uPlayerId, wchar_t * sEdition, short iFreq, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, SPELL_OBJECT_NAME, sObjectName, pDebug)
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
  wsafecpy(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, L"");
  wsafecpy(m_sTargetInfo, 256, L"");
  m_bGlobal = false;
  m_bAllowedInBattle = false;
  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void Spell::loadBasicData(DebugManager * pDebug)
{
  wsafecpy(m_sIconPath, MAX_PATH, L"");

  // Get some basic parameters
  // Spell name
  if (callLuaFunction(L"getName", 1, L""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: spell in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, L"");
  }

  // Casting cost
  double dMana[4];
  if (!getLuaVarNumberArray(L"cost", dMana, 4))
  {
	  // error : cost not found
    wchar_t sError[512] = L"";
    swprintf(sError, 512, L"Lua interaction error: spell in file %s has no cost defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
  }
  else
  {
    for (int i = 0; i < 4; i++)
      m_CastingCost.mana[i] = (u8) dMana[i];
  }

  // Description
  if (callLuaFunction(L"getDescription", 1, L""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: spell in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, L"");
  }

  // Icon texture
  wchar_t sStr[MAX_PATH];
  if (!getLuaVarString(L"icon", sStr, MAX_PATH))
  {
	  // error : icon not found
    wchar_t sError[512] = L"";
    swprintf(sError, 512, L"Lua interaction error: spell in file %s has no icon path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sIconPath, MAX_PATH, L"");
  }
  else
    swprintf(m_sIconPath, MAX_PATH, L"%s/%s", m_sObjectEdition, sStr);

  double val;
  // Battle spell?
  if (getLuaVarNumber(L"allowedInBattle", &val))
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
  wchar_t sEdition[NAME_MAX_CHARS];
  wchar_t sFilename[NAME_MAX_CHARS];
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
  wsafecpy(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, L"");
}

// -----------------------------------------------------------------
// Name : addResolveParameters
// -----------------------------------------------------------------
void Spell::addResolveParameters(wchar_t * sParams)
{
  wsafecat(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, L" ");
  wsafecat(m_sResolveParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams);
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
wchar_t * Spell::getInfo(wchar_t * sBuf, int iSize)
{
  wchar_t sMana[32] = L"";
  getManaText(sMana, 32);
  swprintf(sBuf, iSize, L"%s (%s)\n%s",
        m_sName,
        sMana,
        m_sDescription
  );
  if (wcscmp(m_sTargetInfo, L"") != 0)
  {
    wchar_t sBuf1[256] = L"";
    wchar_t sBuf2[256] = L"";
    i18n->getText(L"CAST_ON_(s)", sBuf1, 256);
    swprintf(sBuf2, 256, sBuf1, m_sTargetInfo);
    wsafecat(sBuf, iSize, L"\n");
    wsafecat(sBuf, iSize, sBuf2);
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getManaText
// -----------------------------------------------------------------
wchar_t * Spell::getManaText(wchar_t * sMana, int iSize)
{
  wchar_t signs[4] = MANA_SIGNS;
  for (int i = 0; i < 4; i++)
  {
    if (m_CastingCost[i] > 0)
    {
      wchar_t sBuf[8];
      swprintf(sBuf, 8, L"%c %d, ", signs[i], (int)m_CastingCost[i]);
      wsafecat(sMana, 32, sBuf);
    }
  }
  if (sMana[0] != L'\0')
    sMana[wcslen(sMana)-2] = L'\0'; // remove coma
  return sMana;
}
