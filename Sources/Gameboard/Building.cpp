#include "Building.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : Building
//  Constructor
// -----------------------------------------------------------------
Building::Building(int iX, int iY, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, BUILDING_OBJECT_NAME, sObjectName, pDebug)
{
  init(iX, iY, pDebug);
}

// -----------------------------------------------------------------
// Name : Building
//  Constructor with id
// -----------------------------------------------------------------
Building::Building(u32 uId, int iX, int iY, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, BUILDING_OBJECT_NAME, sObjectName, pDebug)
{
  init(iX, iY, pDebug);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Building::init(int iX, int iY, DebugManager * pDebug)
{
  m_bIsBuilt = false;
  m_iXPxl = iX;
  m_iYPxl = iY;
  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void Building::loadBasicData(DebugManager * pDebug)
{
  // Get some basic parameters
  // Building name
  if (callLuaFunction(L"getName", 1, L""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua interaction error: building in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, L"");
  }

  // Description
  if (callLuaFunction(L"getDescription", 1, L""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf_s(sError, 512, L"Lua interaction error: building in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, L"");
  }

  // Texture
  wchar_t sStr[MAX_PATH];
  if (!getLuaVarString(L"texture", sStr, MAX_PATH))
  {
	  // error : texture not found
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error: building in file %s has no texture path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sTexture, NAME_MAX_CHARS, L"");
  }
  else
    swprintf_s(m_sTexture, MAX_PATH, L"%s/%s", m_sObjectEdition, sStr);

  // Production cost
  double d;
  if (getLuaVarNumber(L"cost", &d))
    m_uCost = (u16) d;
  else
  {
	  // error : cost not found
    wchar_t sError[512] = L"";
    swprintf_s(sError, 512, L"Lua interaction error: building in file %s has no cost defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    m_uCost = 9999;
  }
}

// -----------------------------------------------------------------
// Name : ~Building
//  Destructor
// -----------------------------------------------------------------
Building::~Building()
{
}

// -----------------------------------------------------------------
// Name : getNetworkData
// -----------------------------------------------------------------
void Building::getNetworkData(NetworkData * pData)
{
  pData->addString(m_sObjectEdition);
  pData->addString(m_sObjectName);
}
