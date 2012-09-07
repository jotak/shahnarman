#include "Skill.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"
#include "../DeckData/UnitData.h"

// -----------------------------------------------------------------
// Name : Skill
//  Constructor
// -----------------------------------------------------------------
Skill::Skill(const char * sEdition, const char * sObjectName, const char * sParams, DebugManager * pDebug) : LuaObject(0, sEdition, SKILL_OBJECT_NAME, sObjectName, pDebug)
{
  init(sParams, pDebug);
}

// -----------------------------------------------------------------
// Name : Skill
//  Constructor with id
// -----------------------------------------------------------------
Skill::Skill(u32 uId, const char * sEdition, const char * sObjectName, const char * sParams, DebugManager * pDebug) : LuaObject(uId, sEdition, SKILL_OBJECT_NAME, sObjectName, pDebug)
{
  init(sParams, pDebug);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Skill::init(const char * sParams, DebugManager * pDebug)
{
  if (sParams == NULL)
    wsafecpy(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
  else
    wsafecpy(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams);

  m_bMergeable = false;
  m_bCumulative = false;
  if (strcmp(m_sParameters, "") != 0)
    callLuaFunction("init", 0, "s", m_sParameters);

  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void Skill::loadBasicData(DebugManager * pDebug)
{
  // Get some basic parameters
  // Skill name
  if (callLuaFunction("getName", 1, ""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: skill in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, "");
  }

  // Description
  if (callLuaFunction("getDescription", 1, ""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: skill in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, "");
  }

  // Icon texture
  char sStr[MAX_PATH];
  if (!getLuaVarString("icon", sStr, MAX_PATH))
  {
	  // error : icon not found
    char sError[512] = "";
    snprintf(sError, 512, "Lua interaction error: skill in file %s has no icon path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sIconPath, MAX_PATH, "");
  }
  else
    snprintf(m_sIconPath, MAX_PATH, "%s/%s", m_sObjectEdition, sStr);

  // is mergeable?
  double d;
  if (getLuaVarNumber("isMergeable", &d))
    m_bMergeable = (d == 1);

  // is cumulative?
  if (getLuaVarNumber("isCumulative", &d))
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
  char sEdition[NAME_MAX_CHARS];
  char sObjectName[NAME_MAX_CHARS];
  char sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
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
  callLuaFunction("merge", 1, "s", pOther->getParameters());
  getLuaString(m_sParameters, LUA_FUNCTION_PARAMS_MAX_CHARS);

  // Retrieve some basic parameters again, they may have changed
  // Skill name
  if (callLuaFunction("getName", 1, ""))
    getLuaString(m_sName, NAME_MAX_CHARS);

  // Description
  if (callLuaFunction("getDescription", 1, ""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);

  // Icon texture
  char sStr[MAX_PATH];
  if (getLuaVarString("icon", sStr, MAX_PATH))
    snprintf(m_sIconPath, MAX_PATH, "%s/%s", m_sObjectEdition, sStr);

  // is mergeable?
  double d;
  if (getLuaVarNumber("isMergeable", &d))
    m_bMergeable = (d == 1);
}
