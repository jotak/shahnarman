// -----------------------------------------------------------------
// MAP READER
// -----------------------------------------------------------------
#include "MapReader.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Ethnicity.h"
#include "../Gameboard/Town.h"

#define TILE(x,y)   m_Map[(y) * m_iWidth + (x)]

// -----------------------------------------------------------------
// Name : MapReader
//  Constructor
// -----------------------------------------------------------------
MapReader::MapReader(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pLuaState = NULL;
  m_Map = NULL;
  wsafecpy(m_sLuaFile, MAX_PATH, L"");
  m_iWidth = m_iHeight = 0;
  m_pPlayersPos = NULL;
  m_iNbPlayers = 0;
}

// -----------------------------------------------------------------
// Name : MapReader
//  Destructor
//  Don't delete m_pTiles: it's being passed to TurnSolver
// -----------------------------------------------------------------
MapReader::~MapReader()
{
  // Free m_Map
  if (m_Map != NULL)
    delete[] m_Map;
  m_TownsList.clear();
  m_TemplesList.clear();
  m_SpecTilesList.clear();
  if (m_pPlayersPos != NULL)
    delete[] m_pPlayersPos;
  if (m_pLuaState != NULL)
    lua_close(m_pLuaState);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
bool MapReader::init(const wchar_t * sMapPath)
{
  // Clear any existing data
  if (m_Map != NULL)
  {
    delete[] m_Map;
    m_Map = NULL;
  }
  m_TownsList.clear();
  m_TemplesList.clear();
  m_SpecTilesList.clear();
  if (m_pPlayersPos != NULL)
  {
    delete[] m_pPlayersPos;
    m_pPlayersPos = NULL;
  }

  // Init LUA
  if (m_pLuaState != NULL)
    lua_close(m_pLuaState);

  m_pLuaState = lua_open();
  luaL_openlibs(m_pLuaState);

  // construct the file name
  wchar_t sFilename[MAX_PATH];
  swprintf_s(sFilename, MAX_PATH, L"%s%s", MAPS_PATH, sMapPath);
  // we must convert unicode file name to ascii
  char sAsciiFilename[MAX_PATH];
  wtostr(sAsciiFilename, MAX_PATH, sFilename);
	if (luaL_dofile(m_pLuaState, sAsciiFilename) != 0)
	{
		// LUA error
    wchar_t sError[512] = L"";
    strtow(sError, 512, lua_tostring(m_pLuaState, -1));
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    wsafecpy(m_sLuaFile, MAX_PATH, L"");
    m_pLuaState = NULL;
    return false;
	}
  wsafecpy(m_sLuaFile, MAX_PATH, sMapPath);
  return true;
}

// -----------------------------------------------------------------
// Name : getMapName
// -----------------------------------------------------------------
bool MapReader::getMapName(wchar_t * sString, int size)
{
  if (m_pLuaState == NULL)
    return false;

  char sLanguage[256];
  wtostr(sLanguage, 256, i18n->getCurrentLanguageName());

  // Call lua function
  lua_getglobal(m_pLuaState, "getName");
  if (lua_isfunction(m_pLuaState, -1))
  {
    lua_pushstring(m_pLuaState, sLanguage);
    int err = lua_pcall(m_pLuaState, 1, 1, 0);
    if (isLuaCallValid(err, L"getName", i18n->getCurrentLanguageName()))
    {
      // convert ascii string to unicode
      strtow(sString, size, lua_tostring(m_pLuaState, -1));
      lua_pop(m_pLuaState, 1);
    }
    else
      return false;
  }
  else
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no name defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  return true;
}

// -----------------------------------------------------------------
// Name : getMapParameters
// -----------------------------------------------------------------
bool MapReader::getMapParameters(ObjectList * pList, int iLabelMaxSize)
{
  if (m_pLuaState == NULL)
    return false;

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "nbParams");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    return false;
  }
  int nbParams = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);
  if (nbParams < 1)
    return true;  // No parameter, but no error

  // Create data for each parameter
  for (int i = 0; i < nbParams; i++)
    pList->addLast(new MapParameters());

  // Get parameter labels
  char sLanguage[256];
  wtostr(sLanguage, 256, i18n->getCurrentLanguageName());

  // Call lua function
  lua_getglobal(m_pLuaState, "getParamLabels");
  if (!lua_isfunction(m_pLuaState, -1))
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no label defined for its params.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  lua_pushstring(m_pLuaState, sLanguage);
  int err = lua_pcall(m_pLuaState, 1, nbParams, 0);
  if (!isLuaCallValid(err, L"getParamLabels", i18n->getCurrentLanguageName()))
    return false;

  // Get 1 result per parameter
  MapParameters * pParam = (MapParameters*) pList->getLast(0);
  while (pParam != NULL)
  {
    // convert ascii string to unicode
    strtow(pParam->sLabel, 256, lua_tostring(m_pLuaState, -1));
    lua_pop(m_pLuaState, 1);
    pParam = (MapParameters*) pList->getPrev(0);
  }

  // Get number of possible values for each parameter
  int nbTotalPossibleValues = 0;
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "nbParamsValues");
  if (!lua_istable(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: variable 'nbParamsValues' incorrect in file %s.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  int i = 0;
  pParam = (MapParameters*) pList->getFirst(0);
  while (pParam != NULL)
  {
    lua_pushnumber(m_pLuaState, ++i);
    lua_gettable(m_pLuaState, -2);
    if (!lua_isnumber(m_pLuaState, -1))
    {
      lua_pop(m_pLuaState, 2);
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua map error: variable 'nbParamsValues' incorrect in file %s. Content is not a number.", m_sLuaFile);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return false;
    }
    pParam->nbValues = (int) lua_tonumber(m_pLuaState, -1);
    pParam->pPossibleValues = new int[pParam->nbValues];
    pParam->pPossibleValueLabels = new wchar_t*[pParam->nbValues];
    for (int i = 0; i < pParam->nbValues; i++)
      pParam->pPossibleValueLabels[i] = new wchar_t[iLabelMaxSize];
    nbTotalPossibleValues += pParam->nbValues;
    lua_pop(m_pLuaState, 1);
    pParam = (MapParameters*) pList->getNext(0);
  }
  lua_pop(m_pLuaState, 1);

  // Get default value index for each parameter
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "paramDefaultValues");
  if (!lua_istable(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: variable 'paramDefaultValues' incorrect in file %s.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  i = 0;
  pParam = (MapParameters*) pList->getFirst(0);
  while (pParam != NULL)
  {
    lua_pushnumber(m_pLuaState, ++i);
    lua_gettable(m_pLuaState, -2);
    if (!lua_isnumber(m_pLuaState, -1))
    {
      lua_pop(m_pLuaState, 2);
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua map error: variable 'paramDefaultValues' incorrect in file %s. Content is not a number.", m_sLuaFile);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return false;
    }
    pParam->defaultValueIndex = (int) lua_tonumber(m_pLuaState, -1) - 1;
    lua_pop(m_pLuaState, 1);
    pParam = (MapParameters*) pList->getNext(0);
  }
  lua_pop(m_pLuaState, 1);

  // Get value labels for each parameter
  // Call lua function
  lua_getglobal(m_pLuaState, "getParamValueLabels");
  if (!lua_isfunction(m_pLuaState, -1))
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no function 'getParamValueLabels' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  lua_pushstring(m_pLuaState, sLanguage);
  err = lua_pcall(m_pLuaState, 1, nbTotalPossibleValues, 0);
  if (!isLuaCallValid(err, L"getParamValueLabels", i18n->getCurrentLanguageName()))
    return false;

  // Get 1 result per parameter
  pParam = (MapParameters*) pList->getLast(0);
  while (pParam != NULL)
  {
    for (int i = pParam->nbValues-1; i >= 0; i--)
    {
      // convert ascii string to unicode
      strtow(pParam->pPossibleValueLabels[i], iLabelMaxSize, lua_tostring(m_pLuaState, -1));
      lua_pop(m_pLuaState, 1);
    }
    pParam = (MapParameters*) pList->getPrev(0);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : deleteMapParameters
//  static
// -----------------------------------------------------------------
void MapReader::deleteMapParameters(ObjectList * pList)
{
  //MapParameters * pParam = (MapParameters*) pList->getFirst(0);
  //while (pParam != NULL)
  //{
  //  if (pParam->pPossibleValues != NULL)
  //    delete[] pParam->pPossibleValues;
  //  if (pParam->pPossibleValueLabels != NULL)
  //  {
  //    for (int i = 0; i < pParam->nbValues; i++)
  //      delete[] pParam->pPossibleValueLabels[i];
  //    delete[] pParam->pPossibleValueLabels;
  //  }
  //  pParam = (MapParameters*) pList->getNext(0);
  //}
  pList->deleteAll();
}

// -----------------------------------------------------------------
// Name : isLuaCallValid
// -----------------------------------------------------------------
bool MapReader::isLuaCallValid(int iError, const wchar_t * sFuncName, const wchar_t * sParams)
{
  switch (iError)
  {
  case LUA_ERRRUN:
    {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA runtime error in file %s, when calling %s with params %s.", m_sLuaFile, sFuncName, sParams);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRMEM:
    {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA memory allocation error in file %s, when calling %s::%s with params %s.", m_sLuaFile, sFuncName, sParams);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return false;
    }
  case LUA_ERRERR:
    {
      wchar_t sError[512] = L"";
      swprintf_s(sError, 512, L"LUA error handling error in file %s, when calling %s::%s with params %s.", m_sLuaFile, sFuncName, sParams);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      return false;
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : setMapParameters
// -----------------------------------------------------------------
void MapReader::setMapParameters(int * pCustomParams, int nbParams, int nbPlayers)
{
  // Count ethnicities
  int nbEthn = 0;
  Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
  while (pEdition != NULL)
  {
    nbEthn += pEdition->getEthnicities()->size;
    pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
  }

  // Set nbEthnicities
  lua_settop(m_pLuaState, 0);
  lua_pushnumber(m_pLuaState, nbEthn);
  lua_setglobal(m_pLuaState, "nbEthnicities");
  lua_pop(m_pLuaState, 1);

  if (pCustomParams != NULL)
  {
    // Set params array
    lua_settop(m_pLuaState, 0);
    lua_getglobal(m_pLuaState, "params");
    if (lua_istable(m_pLuaState, 1))
    {
      for (int i = 0; i < nbParams; i++)
      {
        lua_pushnumber(m_pLuaState, i+1);
        lua_pushnumber(m_pLuaState, pCustomParams[i] + 1);
        lua_settable(m_pLuaState, -3);
      }
      lua_pop(m_pLuaState, 1);
    }
    else
    {
      wchar_t sError[512];
      swprintf_s(sError, 512, L"Lua map error: 'params' array not correctly defined in file %s.", m_sLuaFile);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
      lua_pop(m_pLuaState, 1);
    }
  }

  // Set nbPlayers
  m_iNbPlayers = nbPlayers;
  lua_settop(m_pLuaState, 0);
  lua_pushnumber(m_pLuaState, nbPlayers);
  lua_setglobal(m_pLuaState, "nbPlayers");
  lua_pop(m_pLuaState, 1);
  m_pPlayersPos = new CoordsMap[nbPlayers];
}

// -----------------------------------------------------------------
// Name : generate
// -----------------------------------------------------------------
bool MapReader::generate()
{
  // Call lua function "generate"
  lua_getglobal(m_pLuaState, "generate");
  if (!lua_isfunction(m_pLuaState, -1))
  {
    lua_pop(m_pLuaState, 1);
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no function 'generate' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  lua_call(m_pLuaState, 0, 0);

  // Get results from LUA
  // Width
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "width");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'width' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  m_iWidth = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);

  // Height
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "height");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'height' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  m_iHeight = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);

  // Prepare data
  assert(m_Map == NULL);
  assert(m_iWidth > 0 && m_iHeight > 0);
  m_Map = new int[m_iWidth * m_iHeight];
  memset(m_Map, TERRAIN_UNSET, m_iWidth * m_iHeight * sizeof(int));

  // Map tiles
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "map");
  if (!lua_istable(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'map' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  else
  {
    for (int i = 0; i < m_iWidth * m_iHeight; i++)
    {
      lua_pushnumber(m_pLuaState, i+1);
      lua_gettable(m_pLuaState, -2);
      if (lua_isnumber(m_pLuaState, -1))
        m_Map[i] = lua_tonumber(m_pLuaState, -1);
      lua_pop(m_pLuaState, 1);
    }
    lua_pop(m_pLuaState, 1);
  }

  // Players position
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "players");
  if (!lua_istable(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'players' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    return false;
  }
  for (int i = 0; i < m_iNbPlayers * 2; i++)
  {
    lua_pushnumber(m_pLuaState, i+1);
    lua_gettable(m_pLuaState, -2);
    if (lua_isnumber(m_pLuaState, -1))
    {
      if (i%2)
        m_pPlayersPos[i/2].y = lua_tonumber(m_pLuaState, -1) - 1;
      else
        m_pPlayersPos[i/2].x = lua_tonumber(m_pLuaState, -1) - 1;
    }
    lua_pop(m_pLuaState, 1);
  }
  lua_pop(m_pLuaState, 1);

  // Towns
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "nbTowns");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'nbTowns' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int nbTowns = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "towns");
  if (!lua_istable(m_pLuaState, 1))
  {
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'towns' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int ** towns = new int*[nbTowns];
  for (int i = 0; i < nbTowns; i++)
  {
    towns[i] = new int[3];

    lua_pushnumber(m_pLuaState, i+1);
    lua_gettable(m_pLuaState, -2);
    if (lua_istable(m_pLuaState, -1))
    {
      for (int j = 0; j < 3; j++)
      {
        lua_pushnumber(m_pLuaState, j+1);
        lua_gettable(m_pLuaState, -2);
        if (lua_isnumber(m_pLuaState, -1))
          towns[i][j] = (int) lua_tonumber(m_pLuaState, -1);
        else
        {
          towns[i][0] = -1; // invalid town data => ignore town
          break;
        }
        lua_pop(m_pLuaState, 1);
      }
    }
    else
      towns[i][0] = -1; // invalid town data => ignore town
    lua_pop(m_pLuaState, 1);
  }
  lua_pop(m_pLuaState, 1);

  // Build an array of available ethnicities
  ObjectList * pList = new ObjectList(false);
  Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
  while (pEdition != NULL)
  {
    Ethnicity * pEthn = (Ethnicity*) pEdition->getEthnicities()->getFirst(0);
    while (pEthn != NULL)
    {
      pList->addLast(pEthn);
      pEthn = (Ethnicity*) pEdition->getEthnicities()->getNext(0);
    }
    pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
  }

  // From extracted data, build Towns
  for (int i = 0; i < nbTowns; i++)
  {
    if (towns[i][0] < 1)  // Invalid town data => ignore town
      continue;

    // Extract position ; note that towns[i][0] is a 1-based index array representing position on map
    int mapx = (towns[i][0] - 1) % m_iWidth;
    int mapy = (towns[i][0] - 1) / m_iWidth;
    if (mapx < 0 || mapx >= m_iWidth || mapy < 0 || mapy >= m_iHeight)
      continue;

    TownData town;
    town.position = CoordsMap(mapx, mapy);
    town.size = towns[i][2];
    if (town.size < 1 || town.size > 5) // Invalid town size => set it to 1
      town.size = 1;

    Ethnicity * pEthn = NULL;
    if (towns[i][1] >= 0)
      pEthn = (Ethnicity*) ((*pList)[towns[i][1]]);
    if (pEthn == NULL)
      pEthn = computeEthnicity(mapx, mapy, pList);
    if (pEthn != NULL)
    {
      wsafecpy(town.sEthnEdition, NAME_MAX_CHARS, pEthn->m_sEdition);
      wsafecpy(town.sEthnId, NAME_MAX_CHARS, pEthn->m_sObjectId);
      m_TownsList.push_back(town);
    }
  }

  delete pList;
  for (int i = 0; i < nbTowns; i++)
    delete[] towns[i];
  delete[] towns;

  // Temples
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "nbTemples");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'nbTemples' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int nbTemples = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "temples");
  if (!lua_istable(m_pLuaState, 1))
  {
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'temples' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int ** temples = new int*[nbTemples];
  for (int i = 0; i < nbTemples; i++)
  {
    temples[i] = new int[3];

    lua_pushnumber(m_pLuaState, i+1);
    lua_gettable(m_pLuaState, -2);
    if (lua_istable(m_pLuaState, -1))
    {
      for (int j = 0; j < 3; j++)
      {
        lua_pushnumber(m_pLuaState, j+1);
        lua_gettable(m_pLuaState, -2);
        if (lua_isnumber(m_pLuaState, -1))
          temples[i][j] = (int) lua_tonumber(m_pLuaState, -1);
        else
        {
          temples[i][0] = -1; // invalid temple data => ignore temple
          break;
        }
        lua_pop(m_pLuaState, 1);
      }
    }
    else
      temples[i][0] = -1; // invalid temple data => ignore temple
    lua_pop(m_pLuaState, 1);

    if (temples[i][0] < 1)
      continue;

    // Extract position ; note that temples[i][0] is a 1-based index array representing position on map
    int mapx = (temples[i][0] - 1) % m_iWidth;
    int mapy = (temples[i][0] - 1) / m_iWidth;
    if (mapx < 0 || mapx >= m_iWidth || mapy < 0 || mapy >= m_iHeight)
      continue;

    TempleData temple;
    temple.position = CoordsMap(mapx, mapy);
    temple.mana = temples[i][1] - 1;
    temple.amount = temples[i][2];
    m_TemplesList.push_back(temple);
  }
  lua_pop(m_pLuaState, 1);

  for (int i = 0; i < nbTemples; i++)
    delete[] temples[i];
  delete[] temples;

  // Special tiles
  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "nbSpecTiles");
  if (!lua_isnumber(m_pLuaState, 1))
  {
    lua_pop(m_pLuaState, 1);
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'nbSpecTiles' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int nbSpecTiles = (int)lua_tonumber(m_pLuaState, 1);
  lua_pop(m_pLuaState, 1);

  lua_settop(m_pLuaState, 0);
  lua_getglobal(m_pLuaState, "spectiles");
  if (!lua_istable(m_pLuaState, 1))
  {
    // error : variable not found
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua map error: file %s has no variable 'spectiles' defined.", m_sLuaFile);
    m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    lua_pop(m_pLuaState, 1);
    return false;
  }
  int * spectiles = new int[nbSpecTiles];
  for (int i = 0; i < nbSpecTiles; i++)
  {
    lua_pushnumber(m_pLuaState, i+1);
    lua_gettable(m_pLuaState, -2);
    if (lua_isnumber(m_pLuaState, -1))
      spectiles[i] = (int) lua_tonumber(m_pLuaState, -1);
    else
      spectiles[i] = -1; // invalid data => ignore it
    lua_pop(m_pLuaState, 1);

    if (spectiles[i] < 1)
      continue;

    // Extract position ; note that spectiles[i] is a 1-based index array representing position on map
    int mapx = (spectiles[i] - 1) % m_iWidth;
    int mapy = (spectiles[i] - 1) / m_iWidth;
    if (mapx < 0 || mapx >= m_iWidth || mapy < 0 || mapy >= m_iHeight)
      continue;

    CoordsMap pos(mapx, mapy);
    m_SpecTilesList.push_back(pos);
  }
  lua_pop(m_pLuaState, 1);
  delete[] spectiles;

  return true;
}

#define COMPUTE_TOTAL_WEIGHT(freqVar) { Ethnicity * pEthn = (Ethnicity*) pList->getFirst(0); while (pEthn != NULL) { totalWeight += pEthn->freqVar; pEthn = (Ethnicity*) pList->getNext(0); }}
#define TEST_RND(freqVar) { Ethnicity * pEthn = (Ethnicity*) pList->getFirst(0); while (pEthn != NULL) { rnd -= pEthn->freqVar; if (rnd < 0) return pEthn; pEthn = (Ethnicity*) pList->getNext(0); }}
// -----------------------------------------------------------------
// Name : computeEthnicity
// -----------------------------------------------------------------
Ethnicity * MapReader::computeEthnicity(int x, int y, ObjectList * pList)
{
  // Find most used terrain
  u8 terrains[5];
  for (int k = 0; k < 5; k++)
    terrains[k] = 0;
  int iMostUsed = -1;
  for (int dx = -2; dx <= 2; dx++)
  {
    if (x + dx < 0)
      continue;
    if (x + dx >= m_iWidth)
      break;
    for (int dy = -2; dy <= 2; dy++)
    {
      if (y + dy < 0)
        continue;
      if (y + dy >= m_iHeight)
        break;
      int tile = TILE(x + dx, y + dy);
      if (tile >= 1 && tile <= 5)
      {
        terrains[tile - 1]++;
        if (iMostUsed < 0 || terrains[tile - 1] > terrains[iMostUsed - 1])
          iMostUsed = tile;
      }
    }
  }
  if (iMostUsed < 0) // for instance when all tiles are SEA
    return NULL;
  int totalWeight = 0;
  switch (iMostUsed)
  {
  case TERRAIN_PLAIN:
    COMPUTE_TOTAL_WEIGHT(m_uTownsFreqOnPlain)
    break;
  case TERRAIN_FOREST:
    COMPUTE_TOTAL_WEIGHT(m_uTownsFreqOnForest)
    break;
  case TERRAIN_MOUNTAIN:
    COMPUTE_TOTAL_WEIGHT(m_uTownsFreqOnMountain)
    break;
  case TERRAIN_TOUNDRA:
    COMPUTE_TOTAL_WEIGHT(m_uTownsFreqOnToundra)
    break;
  case TERRAIN_DESERT:
    COMPUTE_TOTAL_WEIGHT(m_uTownsFreqOnDesert)
    break;
  }
  if (totalWeight == 0)
    return NULL;
  int rnd = getRandom(totalWeight);
  switch (iMostUsed)
  {
  case TERRAIN_PLAIN:
    TEST_RND(m_uTownsFreqOnPlain)
    break;
  case TERRAIN_FOREST:
    TEST_RND(m_uTownsFreqOnForest)
    break;
  case TERRAIN_MOUNTAIN:
    TEST_RND(m_uTownsFreqOnMountain)
    break;
  case TERRAIN_TOUNDRA:
    TEST_RND(m_uTownsFreqOnToundra)
    break;
  case TERRAIN_DESERT:
    TEST_RND(m_uTownsFreqOnDesert)
    break;
  }
  return NULL;
}
