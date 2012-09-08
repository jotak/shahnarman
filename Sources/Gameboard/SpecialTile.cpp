#include "SpecialTile.h"
#include "../Debug/DebugManager.h"
#include "../lua_callbacks.h"
#include "../Data/LocalisationTool.h"
#include "../Server/NetworkData.h"

// -----------------------------------------------------------------
// Name : SpecialTile
//  Constructor
// -----------------------------------------------------------------
SpecialTile::SpecialTile(int iFreq, CoordsMap mapPos, char * sEdition, char * sObjectName, DebugManager * pDebug) : LuaObject(0, sEdition, SPECTILE_OBJECT_NAME, sObjectName, pDebug)
{
    init(iFreq, mapPos, pDebug);
}

// -----------------------------------------------------------------
// Name : SpecialTile
//  Constructor with id
// -----------------------------------------------------------------
SpecialTile::SpecialTile(u32 uId, int iFreq, CoordsMap mapPos, char * sEdition, char * sObjectName, DebugManager * pDebug) : LuaObject(uId, sEdition, SPECTILE_OBJECT_NAME, sObjectName, pDebug)
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
    callLuaFunction("init", 0, "ii", mapPos.x, mapPos.y);
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
    if (callLuaFunction("getName", 1, ""))
        getLuaString(m_sName, NAME_MAX_CHARS);
    else
    {
        char sError[512];
        snprintf(sError, 512, "Lua interaction error: special tile in file %s has no name defined.", m_sObjectName);
        pDebug->notifyErrorMessage(sError);
        wsafecpy(m_sName, NAME_MAX_CHARS, "");
    }

    // Description
    if (callLuaFunction("getDescription", 1, ""))
        getLuaString(m_sDescription, DESCRIPTION_MAX_CHARS);
    else
    {
        char sError[512];
        snprintf(sError, 512, "Lua interaction error: special tile in file %s has no description defined.", m_sObjectName);
        pDebug->notifyErrorMessage(sError);
        wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, "");
    }

    // Texture
    char sFile[MAX_PATH];
    if (!getLuaVarString("texture", sFile, MAX_PATH))
    {
        // error : texture not found
        char sError[512] = "";
        snprintf(sError, 512, "Lua interaction error: special tile in file %s has no texture path defined.", m_sObjectName);
        pDebug->notifyErrorMessage(sError);
        wsafecpy(m_sTexPath, MAX_PATH, "");
    }
    else
        snprintf(m_sTexPath, MAX_PATH, "%s/%s", m_sObjectEdition, sFile);

    // Does attract AI?
    double d;
    if (getLuaVarNumber("attractAI", &d))
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
    char sEdition[NAME_MAX_CHARS];
    char sObjectName[NAME_MAX_CHARS];
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
    if (callLuaFunction("getMapPos", 2, ""))
    {
        // First y then x (got from stack)
        mapPos.y = (int) getLuaNumber();
        mapPos.x = (int) getLuaNumber();
    }
    return mapPos;
}
