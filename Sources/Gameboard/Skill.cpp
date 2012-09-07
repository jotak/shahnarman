#include "Skill.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"
#include "../DeckData/UnitData.h"

// -----------------------------------------------------------------
// Name : Skill
//  Constructor
// -----------------------------------------------------------------
Skill::Skill(wchar_t * sEdition, wchar_t * sObjectName, wchar_t * sParams, DebugManager * pDebug) : LuaObject(0, sEdition, SKILL_OBJECT_NAME, sObjectName, pDebug)
{
  init(sParams, pDebug);
}

// -----------------------------------------------------------------
// Name : Skill
//  Constructor with id
// -----------------------------------------------------------------
Skill::Skill(u32 uId, wchar_t * sEdition, wchar_t * sObjectName, wchar_t * sParams, DebugManager * pDebug) : LuaObject(uId, sEdition, SKILL_OBJECT_NAME, sObjectName, pDebug)
{
  init(sParams, pDebug);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Skill::init(wchar_t * sParams, DebugManager * pDebug)
{
  if (sParams == NULL)
    wsafecpy(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, L"");
  else
    wsafecpy(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams);

  m_bMergeable = false;
  m_bCumulative = false;
  if (wcscmp(m_sParameters, L"") != 0)
    callLuaFunction(L"init", 0, L"s", m_sParameters);

  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void Skill::loadBasicData(DebugManager * pDebug)
{
  // Get some basic parameters
  // Skill name
  if (callLuaFunction(L"getName", 1, L""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: skill in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, L"");
  }

  // Description
  if (callLuaFunction(L"getDescription", 1, L""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: skill in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, L"");
  }

  // Icon texture
  wchar_t sStr[MAX_PATH];
  if (!getLuaVarString(L"icon", sStr, MAX_PATH))
  {
	  // error : icon not found
    wchar_t sError[512] = L"";
    swprintf(sError, 512, L"Lua interaction error: skill in file %s has no icon path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sIconPath, MAX_PATH, L"");
  }
  else
    swprintf(m_sIconPath, MAX_PATH, L"%s/%s", m_sObjectEdition, sStr);

  // is mergeable?
  double d;
  if (getLuaVarNumber(L"isMergeable", &d))
    m_bMergeable = (d == 1);

  // is cumulative?
  if (getLuaVarNumber(L"isCumulative", &d))
    m_bCumulative = (d == 1);
}

// -----------------------------------------------------------------
// Name : ~Skill
//  Destructor
// -----------------------------------------------------------------
Skill::~Skill()
{
}

// -----------------------------------------------------------------
// Name : deserialize
//  static
// -----------------------------------------------------------------
Skill * Skill::deserialize(u32 uInstanceId, NetworkData * pData, DebugManager * pDebug)
{
  wchar_t sEdition[NAME_MAX_CHARS];
  wchar_t sObjectName[NAME_MAX_CHARS];
  wchar_t sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
  pData->readString(sEdition);
  pData->readString(sObjectName);
  pData->readString(sParameters);
  return new Skill(uInstanceId, sEdition, sObjectName, sParameters, pDebug);;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Skill::serialize(NetworkData * pData)
{
  pData->addString(m_sObjectEdition);
  pData->addString(m_sObjectName);
  pData->addString(m_sParameters);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
Skill * Skill::clone(bool bKeepInstance, DebugManager * pDebug)
{
  if (bKeepInstance)
    return new Skill(m_uInstanceId, m_sObjectEdition, m_sObjectName, m_sParameters, pDebug);
  else
    return new Skill(m_sObjectEdition, m_sObjectName, m_sParameters, pDebug);
}

// -----------------------------------------------------------------
// Name : merge
// -----------------------------------------------------------------
void Skill::merge(Skill * pOther)
{
  callLuaFunction(L"merge", 1, L"s", pOther->getParameters());
  getLuaString(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS);

  // Retrieve some basic parameters again, they may have changed
  // Skill name
  if (callLuaFunction(L"getName", 1, L""))
    getLuaString(m_sName, NAME_MAX_CHARS);

  // Description
  if (callLuaFunction(L"getDescription", 1, L""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);

  // Icon texture
  wchar_t sStr[MAX_PATH];
  if (getLuaVarString(L"icon", sStr, MAX_PATH))
    swprintf(m_sIconPath, MAX_PATH, L"%s/%s", m_sObjectEdition, sStr);

  // is mergeable?
  double d;
  if (getLuaVarNumber(L"isMergeable", &d))
    m_bMergeable = (d == 1);
}
