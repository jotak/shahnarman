#include "LuaObject.h"
#include "../Debug/DebugManager.h"
#include "../Data/LocalisationTool.h"
#include "../lua_callbacks.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Unit.h"
#include "../Players/Player.h"
#include "../GameRoot.h"
#include "../LocalClient.h"
#include "../Data/Parameters.h"

u32 LuaObject::static_uIdGenerator = 0;
LuaObject * LuaObject::static_pCurrentLuaCaller = NULL;

// -----------------------------------------------------------------
// Name : LuaObject
//  Constructor
// -----------------------------------------------------------------
LuaObject::LuaObject(u32 uInstance, const char * sEdition, const char * sObjectType, const char * sObjectName, DebugManager * pDebug)
{
  m_uInstanceId = (uInstance == 0) ? ++static_uIdGenerator : uInstance;
  wsafecpy(m_sObjectName, NAME_MAX_CHARS, sObjectName);
  wsafecpy(m_sObjectType, NAME_MAX_CHARS, sObjectType);
  wsafecpy(m_sObjectEdition, NAME_MAX_CHARS, sEdition);
  m_pDebug = pDebug;
  m_pTargets = new ObjectList(false);

  m_pLuaState = lua_open();
  luaL_openlibs(m_pLuaState);

  // register LUA callbacks
  registerLuaCallbacks(m_pLuaState);

  // construct the file name
  char sFilename[MAX_PATH];
  snprintf(sFilename, MAX_PATH, "%s%s/%s/%s.lua", EDITIONS_PATH, sEdition, sObjectType, sObjectName);
	if (luaL_dofile(m_pLuaState, sFilename) != 0)
	{
		// LUA error
    m_pDebug->notifyErrorMessage(lua_tostring(m_pLuaState, -1));
    m_pLuaState = NULL;
    return;
	}

  // Set language
  lua_settop(m_pLuaState, 0);
  lua_pushstring(m_pLuaState, i18n->getCurrentLanguageName());
  lua_setglobal(m_pLuaState, "language");
  lua_pop(m_pLuaState, 1);

  // Child effects
  m_iNbChildEffects = 0;
  double d;
  if (getLuaVarNumber("childEffectsCount", &d))
  {
    m_iNbChildEffects = (int) d;
    if (m_iNbChildEffects > 0)
      m_pChildEffects = new ChildEffect[m_iNbChildEffects];
  }
  if (m_iNbChildEffects > 0)
  {
    lua_settop(m_pLuaState, 0);
    lua_getglobal(m_pLuaState, "childEffectsCost");
    if (lua_istable(m_pLuaState, 1))
    {
      // Get cost
      for (int i = 0; i < m_iNbChildEffects; i++)
      {
        lua_pushnumber(m_pLuaState, i+1);
        lua_gettable(m_pLuaState, -2);
        if (lua_istable(m_pLuaState, -1))
        {
          for (int iMana = 0; iMana < 4; iMana++)
          {
            lua_pushnumber(m_pLuaState, iMana + 1);
            lua_gettable(m_pLuaState, -2);
            if (lua_isnumber(m_pLuaState, -1))
              m_pChildEffects[i].cost.mana[iMana] = (int) lua_tonumber(m_pLuaState, -1);
            lua_pop(m_pLuaState, 1);
          }
        }
        lua_pop(m_pLuaState, 1);
      }
      lua_pop(m_pLuaState, 1);
    }
    else
    {
      char sError[512];
      snprintf(sError, 512, "Lua interaction error: can't get childEffectsCost, or is not a table - in lua file %s.", sObjectName);
      pDebug->notifyErrorMessage(sError);
    }
    // Get name
    if (callLuaFunction("getChildEffectsName", m_iNbChildEffects, "s", i18n->getCurrentLanguageName()))
    {
      for (int i = 0; i < m_iNbChildEffects; i++)
      {
        m_pChildEffects[i].id = i;
        m_pChildEffects[i].setAttachment(this);
        wsafecpy(m_pChildEffects[i].sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
        getLuaString(m_pChildEffects[m_iNbChildEffects - i - 1].sName, NAME_MAX_CHARS);
      }
    }
    else
    {
      char sError[512];
      snprintf(sError, 512, "Lua interaction error: can't call getChildEffectsName in lua file %s.", sObjectName);
      pDebug->notifyErrorMessage(sError);
    }
    // Get icons
    char ** pIcons = new char*[m_iNbChildEffects];
    for (int i = 0; i < m_iNbChildEffects; i++)
      pIcons[i] = new char[MAX_PATH];
    if (getLuaVarStringArray("childEffectsIcon", pIcons, m_iNbChildEffects, MAX_PATH))
    {
      for (int i = 0; i < m_iNbChildEffects; i++)
        snprintf(m_pChildEffects[i].sIcon, MAX_PATH, "%s/%s", m_sObjectEdition, pIcons[i]);
    }
    else
    {
      char sError[512];
      snprintf(sError, 512, "Lua interaction error: can't get childEffectsIcon, or is not a table - in lua file %s.", sObjectName);
      pDebug->notifyErrorMessage(sError);
    }
    for (int i = 0; i < m_iNbChildEffects; i++)
      delete[] pIcons[i];
    delete[] pIcons;
  }
  m_iCurrentEffect = 0;
}

// -----------------------------------------------------------------
// Name : ~LuaObject
//  Destructor
// -----------------------------------------------------------------
LuaObject::~LuaObject()
{
  delete m_pTargets;
  if (m_iNbChildEffects > 0)
    delete[] m_pChildEffects;
  lua_close(m_pLuaState);
}

// -----------------------------------------------------------------
// Name : prepareLuaFunction
// -----------------------------------------------------------------
lua_State * LuaObject::prepareLuaFunction(const char * sFunc)
{
  if (m_pLuaState == NULL)
    return NULL;

  // Call lua function
  lua_getglobal(m_pLuaState, sFunc);
  if (lua_isfunction(m_pLuaState, -1))
    return m_pLuaState;
  else
  {
    lua_pop(m_pLuaState, 1);
    return NULL;
  }
}

// -----------------------------------------------------------------
// Name : callPreparedLuaFunction
// -----------------------------------------------------------------
bool LuaObject::callPreparedLuaFunction(int iNbParams, int iNbResults, const char * sFunc, const char * sParams)
{
#ifdef DEBUG
  extern GameRoot * g_pMainGameRoot;
  bool bLog = (g_pMainGameRoot->m_pLocalClient->getClientParameters()->iLogLevel >= 3);
  if (bLog)
  {
    char sText[1024];
    snprintf(sText, 1024, "callPreparedLuaFunction: %s, %s, %s", m_sObjectName, sFunc, sParams);
    m_pDebug->log(sText);
  }
#endif
  // Store context of previous caller
  LuaObject * pPrevCaller = LuaObject::static_pCurrentLuaCaller;
  // Override caller context
  LuaObject::static_pCurrentLuaCaller = this;
  // Call LUA
  int err = lua_pcall(m_pLuaState, iNbParams, iNbResults, 0);
  // Restore previous caller context
  LuaObject::static_pCurrentLuaCaller = pPrevCaller;
  // Manage errors
  switch (err)
  {
  case LUA_ERRRUN:
    {
      char sError[1024] = "";
      snprintf(sError, 1024, "LUA runtime error when calling %s::%s(%s). %s", m_sObjectName, sFunc, sParams, lua_tostring(m_pLuaState, -1));
      m_pDebug->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRMEM:
    {
      char sError[512] = "";
      snprintf(sError, 512, "LUA memory allocation error when calling %s::%s(%s).", m_sObjectName, sFunc, sParams);
      m_pDebug->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRERR:
    {
      char sError[512] = "";
      snprintf(sError, 512, "LUA error handling error when calling %s::%s(%s).", m_sObjectName, sFunc, sParams);
      m_pDebug->notifyErrorMessage(sError);
      return false;
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : callLuaFunction
//  This function takes a varaible number of aguments to call a lua function with them.
//  sParamsType is a string that defines the types of these arguments.
//    "i" stands for int
//    "" stands for long
//    "f" stands for float
//    "d" stands for double
//    "s" stands for string (char*)
//  For instance, if sParamsType is "idd", the following parameters must be int, double, double.
// -----------------------------------------------------------------
bool LuaObject::callLuaFunction(const char * sFunc, int iNbResults, const char * sParamsType, ...)
{
  if (!prepareLuaFunction(sFunc))
    return false;

  // Initialize structure to read variable list of parameters
  va_list pArgs;
  va_start(pArgs, sParamsType);
  int i = 0;
  char sParams[512] = "";
  char sBuf[128];
  // Parse sParamsType to read following parameters
  while (sParamsType[i] != '\0')
  {
    if (sParamsType[i] == 'i')
    {
      int val = va_arg(pArgs, int);
      lua_pushnumber(m_pLuaState, val);
      snprintf(sBuf, 128, "%d,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == 'l')
    {
      long val = va_arg(pArgs, long);
      lua_pushnumber(m_pLuaState, val);
      snprintf(sBuf, 128, "%ld,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == 'f')
    {
      float val = va_arg(pArgs, float);
      lua_pushnumber(m_pLuaState, val);
      snprintf(sBuf, 128, "%f,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == 'd')
    {
      double val = va_arg(pArgs, double);
      lua_pushnumber(m_pLuaState, val);
      snprintf(sBuf, 128, "%lf,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == 's')
    {
      char * val = va_arg(pArgs, char*);
      lua_pushstring(m_pLuaState, val);
      snprintf(sBuf, 128, "%s,", val);
      wsafecat(sParams, 512, sBuf);
    }
    i++;
  }
  if (sParams[0] != '\0')
    sParams[strlen(sParams) - 1] = '\0';

  return callPreparedLuaFunction(i, iNbResults, sFunc, sParams);
}

// -----------------------------------------------------------------
// Name : getLuaVarNumber
// -----------------------------------------------------------------
bool LuaObject::getLuaVarNumber(const char * sVarName, double * d)
{
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sVarName);
  if (lua_isnumber(m_pLuaState, 1))
  {
    *d = lua_tonumber(m_pLuaState, 1);
    lua_pop(m_pLuaState, 1);
    return true;
  }
  lua_pop(m_pLuaState, 1);
  // error : variable not found
  return false;
}

// -----------------------------------------------------------------
// Name : getLuaVarString
// -----------------------------------------------------------------
bool LuaObject::getLuaVarString(const char * sVarName, char * sString, int size)
{
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sVarName);
  if (lua_isstring(m_pLuaState, 1))
  {
    wsafecpy(sString, size, lua_tostring(m_pLuaState, 1));
    lua_pop(m_pLuaState, 1);
    return true;
  }
  lua_pop(m_pLuaState, 1);
  // error : variable not found
  return false;
}

// -----------------------------------------------------------------
// Name : getLuaVarNumberArray
// -----------------------------------------------------------------
bool LuaObject::getLuaVarNumberArray(const char * sVarName, double * pArray, int size)
{
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sVarName);
  if (lua_istable(m_pLuaState, 1))
  {
    for (int i = 0; i < size; i++)
    {
      lua_pushnumber(m_pLuaState, i+1);
      lua_gettable(m_pLuaState, -2);
      if (lua_isnumber(m_pLuaState, -1))
        pArray[i] = lua_tonumber(m_pLuaState, -1);
      lua_pop(m_pLuaState, 1);
    }
    lua_pop(m_pLuaState, 1);
    return true;
  }
  lua_pop(m_pLuaState, 1);
  // error : variable not found
  return false;
}

// -----------------------------------------------------------------
// Name : getLuaVarStringArray
// -----------------------------------------------------------------
bool LuaObject::getLuaVarStringArray(const char * sVarName, char ** pArray, int tabSize, int strSize)
{
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sVarName);
  if (lua_istable(m_pLuaState, 1))
  {
    for (int i = 0; i < tabSize; i++)
    {
      lua_pushnumber(m_pLuaState, i+1);
      lua_gettable(m_pLuaState, -2);
      if (lua_isstring(m_pLuaState, -1))
        wsafecpy(pArray[i], strSize, lua_tostring(m_pLuaState, -1));
      lua_pop(m_pLuaState, 1);
    }
    lua_pop(m_pLuaState, 1);
    return true;
  }
  lua_pop(m_pLuaState, 1);
  // error : variable not found
  return false;
}

// -----------------------------------------------------------------
// Name : getLuaNumber
// -----------------------------------------------------------------
double LuaObject::getLuaNumber()
{
  assert(m_pLuaState != NULL);
  double d = lua_tonumber(m_pLuaState, -1);
  lua_pop(m_pLuaState, 1);
  return d;
}

// -----------------------------------------------------------------
// Name : getLuaString
// -----------------------------------------------------------------
void LuaObject::getLuaString(char * sString, int size)
{
  assert(m_pLuaState != NULL);
  // convert ascii string to unicode
  wsafecpy(sString, size, lua_tostring(m_pLuaState, -1));
  lua_pop(m_pLuaState, 1);
}

// -----------------------------------------------------------------
// Name : getUniqueId
// -----------------------------------------------------------------
char * LuaObject::getUniqueId(char * sId, int iSize)
{
  snprintf(sId, iSize, "%s:%s:%s", m_sObjectEdition, m_sObjectType, m_sObjectName);
  return sId;
}

// -----------------------------------------------------------------
// Name : isUniqueId
// -----------------------------------------------------------------
bool LuaObject::isUniqueId(const char * sEdition, const char * sObjectType, const char * sObjectName)
{
  return strcmp(sEdition, m_sObjectEdition) == 0
    && strcmp(sObjectType, m_sObjectType) == 0
    && strcmp(sObjectName, m_sObjectName) == 0;
}

// -----------------------------------------------------------------
// Name : removeFromTargets
// -----------------------------------------------------------------
void LuaObject::removeFromTargets()
{
  BaseObject * pObj = m_pTargets->getFirst(0);
  while (pObj != NULL)
  {
    LuaTargetable * pTarget = LuaTargetable::convertFromBaseObject(pObj, m_pTargets->getCurrentType(0));
    assert(pTarget != NULL);
    pTarget->getAllEffects()->deleteObject(this, true);
    pObj = m_pTargets->deleteCurrent(0, true);
  }
}
