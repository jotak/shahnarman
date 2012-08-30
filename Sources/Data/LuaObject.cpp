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
LuaObject::LuaObject(u32 uInstance, wchar_t * sEdition, wchar_t * sObjectType, wchar_t * sObjectName, DebugManager * pDebug)
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
  wchar_t sFilename[MAX_PATH];
  swprintf_s(sFilename, MAX_PATH, L"%s%s/%s/%s.lua", EDITIONS_PATH, sEdition, sObjectType, sObjectName);
  // we must convert unicode file name to ascii
  char sAsciiFilename[MAX_PATH];
  wtostr(sAsciiFilename, MAX_PATH, sFilename);
	if (luaL_dofile(m_pLuaState, sAsciiFilename) != 0)
	{
		// LUA error
    wchar_t sError[512] = L"";
    strtow(sError, 512, lua_tostring(m_pLuaState, -1));
    m_pDebug->notifyErrorMessage(sError);
    m_pLuaState = NULL;
    return;
	}

  // Set language
  char sLanguage[64] = "";
  wtostr(sLanguage, 64, i18n->getCurrentLanguageName());
  lua_settop(m_pLuaState, 0);
  lua_pushstring(m_pLuaState, sLanguage);
  lua_setglobal(m_pLuaState, "language");
  lua_pop(m_pLuaState, 1);

  // Child effects
  m_iNbChildEffects = 0;
  double d;
  if (getLuaVarNumber(L"childEffectsCount", &d))
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
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua interaction error: can't get childEffectsCost, or is not a table - in lua file %s.", sObjectName);
      pDebug->notifyErrorMessage(sError);
    }
    // Get name
    if (callLuaFunction(L"getChildEffectsName", m_iNbChildEffects, L"s", i18n->getCurrentLanguageName()))
    {
      for (int i = 0; i < m_iNbChildEffects; i++)
      {
        m_pChildEffects[i].id = i;
        m_pChildEffects[i].setAttachment(this);
        wsafecpy(m_pChildEffects[i].sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, L"");
        getLuaString(m_pChildEffects[m_iNbChildEffects - i - 1].sName, NAME_MAX_CHARS);
      }
    }
    else
    {
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua interaction error: can't call getChildEffectsName in lua file %s.", sObjectName);
      pDebug->notifyErrorMessage(sError);
    }
    // Get icons
    wchar_t ** pIcons = new wchar_t*[m_iNbChildEffects];
    for (int i = 0; i < m_iNbChildEffects; i++)
      pIcons[i] = new wchar_t[MAX_PATH];
    if (getLuaVarStringArray(L"childEffectsIcon", pIcons, m_iNbChildEffects, MAX_PATH))
    {
      for (int i = 0; i < m_iNbChildEffects; i++)
        swprintf_s(m_pChildEffects[i].sIcon, MAX_PATH, L"%s/%s", m_sObjectEdition, pIcons[i]);
    }
    else
    {
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua interaction error: can't get childEffectsIcon, or is not a table - in lua file %s.", sObjectName);
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
lua_State * LuaObject::prepareLuaFunction(wchar_t * sFunc)
{
  if (m_pLuaState == NULL)
    return NULL;

  // Convert wchar_t arguments to ASCII
  char sAsciiFunc[64];
  wtostr(sAsciiFunc, 64, sFunc);

  // Call lua function
  lua_getglobal(m_pLuaState, sAsciiFunc);
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
bool LuaObject::callPreparedLuaFunction(int iNbParams, int iNbResults, wchar_t * sFunc, wchar_t * sParams)
{
#ifdef DEBUG
  extern GameRoot * g_pMainGameRoot;
  bool bLog = (g_pMainGameRoot->m_pLocalClient->getClientParameters()->iLogLevel >= 3);
  if (bLog)
  {
    wchar_t sText[1024];
    swprintf_s(sText, 1024, L"callPreparedLuaFunction: %s, %s, %s", m_sObjectName, sFunc, sParams);
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
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA runtime error when calling %s::%s(%s).", m_sObjectName, sFunc, sParams);
      m_pDebug->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRMEM:
    {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA memory allocation error when calling %s::%s(%s).", m_sObjectName, sFunc, sParams);
      m_pDebug->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRERR:
    {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA error handling error when calling %s::%s(%s).", m_sObjectName, sFunc, sParams);
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
//    "l" stands for long
//    "f" stands for float
//    "d" stands for double
//    "s" stands for string (wchar_t*)
//  For instance, if sParamsType is "idd", the following parameters must be int, double, double.
// -----------------------------------------------------------------
bool LuaObject::callLuaFunction(wchar_t * sFunc, int iNbResults, wchar_t * sParamsType, ...)
{
  if (!prepareLuaFunction(sFunc))
    return false;

  // Initialize structure to read variable list of parameters
  va_list pArgs;
  va_start(pArgs, sParamsType);
  int i = 0;
  wchar_t sParams[512] = L"";
  wchar_t sBuf[128];
  // Parse sParamsType to read following parameters
  while (sParamsType[i] != L'\0')
  {
    if (sParamsType[i] == L'i')
    {
      int val = va_arg(pArgs, int);
      lua_pushnumber(m_pLuaState, val);
      swprintf_s(sBuf, 128, L"%d,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == L'l')
    {
      long val = va_arg(pArgs, long);
      lua_pushnumber(m_pLuaState, val);
      swprintf_s(sBuf, 128, L"%ld,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == L'f')
    {
      float val = va_arg(pArgs, float);
      lua_pushnumber(m_pLuaState, val);
      swprintf_s(sBuf, 128, L"%f,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == L'd')
    {
      double val = va_arg(pArgs, double);
      lua_pushnumber(m_pLuaState, val);
      swprintf_s(sBuf, 128, L"%lf,", val);
      wsafecat(sParams, 512, sBuf);
    }
    else if (sParamsType[i] == L's')
    {
      wchar_t * val = va_arg(pArgs, wchar_t*);
      // Convert wchar_t arguments to ASCII
      char charval[LUA_FUNCTION_PARAMS_MAX_CHARS];
      wtostr(charval, LUA_FUNCTION_PARAMS_MAX_CHARS, val);
      lua_pushstring(m_pLuaState, charval);
      swprintf_s(sBuf, 128, L"%s,", val);
      wsafecat(sParams, 512, sBuf);
    }
    i++;
  }
  if (sParams[0] != L'\0')
    sParams[wcslen(sParams) - 1] = L'\0';

  return callPreparedLuaFunction(i, iNbResults, sFunc, sParams);
}

// -----------------------------------------------------------------
// Name : getLuaVarNumber
// -----------------------------------------------------------------
bool LuaObject::getLuaVarNumber(wchar_t * sVarName, double * d)
{
  // Convert wchar_t variable name to ASCII
  char sAsciiVar[64];
  wtostr(sAsciiVar, 64, sVarName);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sAsciiVar);
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
bool LuaObject::getLuaVarString(wchar_t * sVarName, wchar_t * sString, int size)
{
  // Convert wchar_t variable name to ASCII
  char sAsciiVar[64];
  wtostr(sAsciiVar, 64, sVarName);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sAsciiVar);
  if (lua_isstring(m_pLuaState, 1))
  {
    // Convert ascii string to unicode
    strtow(sString, size, lua_tostring(m_pLuaState, 1));
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
bool LuaObject::getLuaVarNumberArray(wchar_t * sVarName, double * pArray, int size)
{
  // Convert wchar_t variable name to ASCII
  char sAsciiVar[64];
  wtostr(sAsciiVar, 64, sVarName);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sAsciiVar);
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
bool LuaObject::getLuaVarStringArray(wchar_t * sVarName, wchar_t ** pArray, int tabSize, int strSize)
{
  // Convert wchar_t variable name to ASCII
  char sAsciiVar[64];
  wtostr(sAsciiVar, 64, sVarName);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, sAsciiVar);
  if (lua_istable(m_pLuaState, 1))
  {
    for (int i = 0; i < tabSize; i++)
    {
      lua_pushnumber(m_pLuaState, i+1);
      lua_gettable(m_pLuaState, -2);
      if (lua_isstring(m_pLuaState, -1))
        strtow(pArray[i], strSize, lua_tostring(m_pLuaState, -1));
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
void LuaObject::getLuaString(wchar_t * sString, int size)
{
  assert(m_pLuaState != NULL);
  // convert ascii string to unicode
  strtow(sString, size, lua_tostring(m_pLuaState, -1));
  lua_pop(m_pLuaState, 1);
}

// -----------------------------------------------------------------
// Name : getUniqueId
// -----------------------------------------------------------------
wchar_t * LuaObject::getUniqueId(wchar_t * sId, int iSize)
{
  swprintf_s(sId, iSize, L"%s:%s:%s", m_sObjectEdition, m_sObjectType, m_sObjectName);
  return sId;
}

// -----------------------------------------------------------------
// Name : isUniqueId
// -----------------------------------------------------------------
bool LuaObject::isUniqueId(wchar_t * sEdition, wchar_t * sObjectType, wchar_t * sObjectName)
{
  return wcscmp(sEdition, m_sObjectEdition) == 0
    && wcscmp(sObjectType, m_sObjectType) == 0
    && wcscmp(sObjectName, m_sObjectName) == 0;
}

// -----------------------------------------------------------------
// Name : removeFromTargets
// -----------------------------------------------------------------
void LuaObject::removeFromTargets()
{
  BaseObject * pObj = m_pTargets->getFirst(0);
  while (pObj != NULL)
  {
    // Need here to get the type of pTarget, and re-cast it
    switch (m_pTargets->getCurrentType(0))
    {
    case LUATARGET_PLAYER:
      {
        Player * pTarget = (Player*) m_pTargets->getCurrent(0);
        assert(pTarget != NULL);
        pTarget->getAllEffects()->deleteObject(this, true);
        break;
      }
    case LUATARGET_TILE:
      {
        MapTile * pTarget = (MapTile*) m_pTargets->getCurrent(0);
        assert(pTarget != NULL);
        pTarget->getAllEffects()->deleteObject(this, true);
        break;
      }
    case LUATARGET_TOWN:
      {
        Town * pTarget = (Town*) m_pTargets->getCurrent(0);
        assert(pTarget != NULL);
        pTarget->getAllEffects()->deleteObject(this, true);
        break;
      }
    case LUATARGET_UNIT:
      {
        Unit * pTarget = (Unit*) m_pTargets->getCurrent(0);
        assert(pTarget != NULL);
        pTarget->getAllEffects()->deleteObject(this, true);
        break;
      }
    }
    pObj = m_pTargets->deleteCurrent(0, true);
  }
}
