#include "Building.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"

// -----------------------------------------------------------------
// Name : Building
//  Constructor
// -----------------------------------------------------------------
Building::Building(int iX, int iY, char * sEdition, char * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, BUILDING_OBJECT_NAME, sObjectName, pDebug)
{
  init(iX, iY, pDebug);
}

// -----------------------------------------------------------------
// Name : Building
//  Constructor with id
// -----------------------------------------------------------------
Building::Building(u32 uId, int iX, int iY, char * sEdition, char * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, BUILDING_OBJECT_NAME, sObjectName, pDebug)
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
  if (callLuaFunction("getName", 1, ""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: building in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, "");
  }

  // Description
  if (callLuaFunction("getDescription", 1, ""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    char sError[512];
    snprintf(sError, 512, "Lua interaction error: building in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, "");
  }

  // Texture
  char sStr[MAX_PATH];
  if (!getLuaVarString("texture", sStr, MAX_PATH))
  {
	  // error : texture not found
    char sError[512] = "";
    snprintf(sError, 512, "Lua interaction error: building in file %s has no texture path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sTexture, NAME_MAX_CHARS, "");
  }
  else
    snprintf(m_sTexture, MAX_PATH, "%s/%s", m_sObjectEdition, sStr);

  // Production cost
  double d;
  if (getLuaVarNumber("cost", &d))
    m_uCost = (u16) d;
  else
  {
	  // error : cost not found
    char sError[512] = "";
    snprintf(sError, 512, "Lua interaction error: building in file %s has no cost defined.", m_sObjectName);
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
