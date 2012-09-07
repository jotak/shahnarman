#include "SpecialTile.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"
#include "../Server/NetworkData.h"

// -----------------------------------------------------------------
// Name : SpecialTile
//  Constructor
// -----------------------------------------------------------------
SpecialTile::SpecialTile(int iFreq, CoordsMap mapPos, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, SPECTILE_OBJECT_NAME, sObjectName, pDebug)
{
  init(iFreq, mapPos, pDebug);
}

// -----------------------------------------------------------------
// Name : SpecialTile
//  Constructor with id
// -----------------------------------------------------------------
SpecialTile::SpecialTile(u32 uId, int iFreq, CoordsMap mapPos, wchar_t * sEdition, wchar_t * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, SPECTILE_OBJECT_NAME, sObjectName, pDebug)
{
  init(iFreq, mapPos, pDebug);
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void SpecialTile::init(int iFreq, CoordsMap mapPos, DebugManager * pDebug)
{
  m_iFreq = iFreq;

  // Init object
  callLuaFunction(L"init", 0, L"ii", mapPos.x, mapPos.y);
  m_bAttractAI = false;

  loadBasicData(pDebug);
}

// -----------------------------------------------------------------
// Name : loadBasicData
// -----------------------------------------------------------------
void SpecialTile::loadBasicData(DebugManager * pDebug)
{
  // Get some basic parameters
  // Object name
  if (callLuaFunction(L"getName", 1, L""))
    getLuaString(m_sName, NAME_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: special tile in file %s has no name defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sName, NAME_MAX_CHARS, L"");
  }

  // Description
  if (callLuaFunction(L"getDescription", 1, L""))
    getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
  else
  {
    wchar_t sError[512];
    swprintf(sError, 512, L"Lua interaction error: special tile in file %s has no description defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, L"");
  }

  // Texture
  wchar_t sFile[MAX_PATH];
  if (!getLuaVarString(L"texture", sFile, MAX_PATH))
  {
	  // error : texture not found
    wchar_t sError[512] = L"";
    swprintf(sError, 512, L"Lua interaction error: special tile in file %s has no texture path defined.", m_sObjectName);
    pDebug->notifyErrorMessage(sError);
    wsafecpy(m_sTexPath, MAX_PATH, L"");
  }
  else
    swprintf(m_sTexPath, MAX_PATH, L"%s/%s", m_sObjectEdition, sFile);

  // Does attract AI?
  double d;
  if (getLuaVarNumber(L"attractAI", &d))
    m_bAttractAI = (d > 0);
}

// -----------------------------------------------------------------
// Name : ~SpecialTile
//  Destructor
// -----------------------------------------------------------------
SpecialTile::~SpecialTile()
{
}

// -----------------------------------------------------------------
// Name : deserialize
//  static
// -----------------------------------------------------------------
SpecialTile * SpecialTile::deserialize(NetworkData * pData, DebugManager * pDebug)
{
  wchar_t sEdition[NAME_MAX_CHARS];
  wchar_t sObjectName[NAME_MAX_CHARS];
  u32 uId = pData->readLong();
  pData->readString(sEdition);
  pData->readString(sObjectName);
  CoordsMap mapPos;
  mapPos.x = pData->readLong();
  mapPos.y = pData->readLong();
  int iFreq = pData->readLong();
  return new SpecialTile(uId, iFreq, mapPos, sEdition, sObjectName, pDebug);
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void SpecialTile::serialize(NetworkData * pData)
{
  pData->addLong(getInstanceId());
  pData->addString(m_sObjectEdition);
  pData->addString(m_sObjectName);
  CoordsMap mapPos = getMapPos();
  pData->addLong(mapPos.x);
  pData->addLong(mapPos.y);
  pData->addLong(m_iFreq);
}

// -----------------------------------------------------------------
// Name : instanciate
// -----------------------------------------------------------------
SpecialTile * SpecialTile::instanciate(CoordsMap mapPos, DebugManager * pDebug)
{
  return new SpecialTile(0, mapPos, getObjectEdition(), getObjectName(), pDebug);
}

// -----------------------------------------------------------------
// Name : getMapPos
// -----------------------------------------------------------------
CoordsMap SpecialTile::getMapPos()
{
  CoordsMap mapPos;
  // Description
  if (callLuaFunction(L"getMapPos", 2, L""))
  {
    // First y then x (got from stack)
    mapPos.y = (int) getLuaNumber();
    mapPos.x = (int) getLuaNumber();
  }
  return mapPos;
}
