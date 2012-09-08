// -----------------------------------------------------------------
// MAP
// -----------------------------------------------------------------
#include "Map.h"
#include "Unit.h"
#include "Town.h"
#include "Temple.h"
#include "SpecialTile.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Ethnicity.h"
#include "../LocalClient.h"
#include "../Server/Server.h"
#include "../Server/MapReader.h"
#include "../Server/TurnSolver.h"
#include "../Server/MapReader.h"
#include "../Debug/DebugManager.h"
#include "../Geometries/GeometryText.h"

// -----------------------------------------------------------------
// Name : Map
//  Constructor
// -----------------------------------------------------------------
Map::Map()
{
    m_iNbTowns = 0;
    m_iNbTemples = 0;
    m_pTiles = NULL;
    m_pPathfinder = NULL;
    m_pTombGeometry = NULL;
    m_pEmptyMapGeometry = NULL;
    m_pTileGeometry = NULL;
    m_pFoeBannerGeometry = NULL;
    m_pCountUnitsBgGeometry1L = NULL;
    m_pCountUnitsBgGeometry2L = NULL;
    m_pUnitsGroups = new ObjectList(true);
    m_iWidth = m_iHeight = 0;
    m_pObjectSelectedFromStack = NULL;
    m_pTownsRef = new ObjectList(true);
    m_pTemplesRef = new ObjectList(true);
}

// -----------------------------------------------------------------
// Name : Map
//  Destructor
// -----------------------------------------------------------------
Map::~Map()
{
    delete m_pTownsRef;
    delete m_pTemplesRef;
    if (m_pTiles != NULL)
    {
        for (u16 x = 0; x < m_iWidth; x++)
        {
            for (u16 y = 0; y < m_iHeight; y++)
                delete m_pTiles[x][y];
            delete[] m_pTiles[x];
        }
        delete m_pTiles;
    }
    if (m_pPathfinder != NULL)
        delete m_pPathfinder;
    FREE(m_pTombGeometry);
    FREE(m_pUnitsGroups);
    FREE(m_pEmptyMapGeometry);
    FREE(m_pTileGeometry);
    FREE(m_pFoeBannerGeometry);
    FREE(m_pCountUnitsBgGeometry1L);
    FREE(m_pCountUnitsBgGeometry2L);
}

// -----------------------------------------------------------------
// Name : createFromServer
// -----------------------------------------------------------------
void Map::createFromServer(MapReader * pMapReader, LocalClient * pLocalClient)
{
    m_iWidth = pMapReader->getMapWidth();
    m_iHeight = pMapReader->getMapHeight();
    int * pTiles = pMapReader->getMap();
    m_pTiles = new MapTile**[m_iWidth];
    for (u16 x = 0; x < m_iWidth; x++)
    {
        m_pTiles[x] = new MapTile*[m_iHeight];
        for (u16 y = 0; y < m_iHeight; y++)
        {
            if (IS_VALID_TERRAIN(pTiles[y * m_iWidth + x]))
                m_pTiles[x][y] = new MapTile(pTiles[y * m_iWidth + x], pLocalClient->getServer()->getSolver()->getGlobalSpellsPtr());
            else
            {
                m_pTiles[x][y] = new MapTile(TERRAIN_SEA, pLocalClient->getServer()->getSolver()->getGlobalSpellsPtr());
                char sError[512];
                snprintf(sError, 512, "Invalid terrain type at (%d,%d)", (int)x, (int)y);
                pLocalClient->getDebug()->notifyErrorMessage(sError);
            }
        }
    }
    // Reset used town names and heroes
    Edition * pEdition = pLocalClient->getDataFactory()->getFirstEdition();
    while (pEdition != NULL)
    {
        Ethnicity * pEthn = (Ethnicity*) pEdition->getEthnicities()->getFirst(0);
        while (pEthn != NULL)
        {
            pEthn->resetUsedTownNames();
            pEthn->resetUsedHeroes();
            pEthn = (Ethnicity*) pEdition->getEthnicities()->getNext(0);
        }
        pEdition = pLocalClient->getDataFactory()->getNextEdition();
    }
    // Create towns
    std::vector<TownData> * towns = pMapReader->getTowns();
    for (u16 i = 0; i < towns->size(); i++)
    {
        if (getTileAt((*towns)[i].position)->m_uTerrainType == TERRAIN_SEA)
            continue;
        Edition * pEdition = pLocalClient->getDataFactory()->findEdition((*towns)[i].sEthnEdition);
        if (pEdition != NULL)
        {
            Ethnicity * pEthn = pEdition->findEthnicity((*towns)[i].sEthnId);
            if (pEthn != NULL)
            {
                Town * pTown = new Town((*towns)[i].position, this, pLocalClient->getServer()->getSolver()->getGlobalSpellsPtr());
                pTown->init(m_iNbTowns++, (*towns)[i].size, pEthn, pLocalClient);
                pTown->initServer();
                m_pTownsRef->addLast(pTown);
            }
        }
    }
    // Create temples
    std::vector<TempleData> * temples = pMapReader->getTemples();
    for (u16 i = 0; i < temples->size(); i++)
    {
        if (getTileAt((*temples)[i].position)->m_uTerrainType == TERRAIN_SEA)
            continue;
        Temple * pTemple = new Temple((*temples)[i].position, this, pLocalClient->getServer()->getSolver()->getGlobalSpellsPtr());
        pTemple->init(m_iNbTemples++, (*temples)[i].mana, (*temples)[i].amount);
        m_pTemplesRef->addLast(pTemple);
    }
    // Create special tiles
    std::vector<CoordsMap> * spectiles = pMapReader->getSpecialTiles();
    char pAllTerrains[7][64] = LTERRAIN_NAMES;
    for (u16 i = 0; i < spectiles->size(); i++)
    {
        MapTile * pTile = getTileAt((*spectiles)[i]);
        // First get total frequency
        int totalFreq = 0;
        Edition * pEd = pLocalClient->getDataFactory()->getFirstEdition();
        while (pEd != NULL)
        {
            SpecialTile * pSpec = (SpecialTile*) pEd->getSpecialTiles()->getFirst(0);
            while (pSpec != NULL)
            {
                pSpec->callLuaFunction("isAllowedOn", 1, "s", pAllTerrains[pTile->m_uTerrainType]);
                if (pSpec->getLuaNumber() > 0)
                    totalFreq += pSpec->getFrequency();
                pSpec = (SpecialTile*) pEd->getSpecialTiles()->getNext(0);
            }
            pEd = pLocalClient->getDataFactory()->getNextEdition();
        }
        if (totalFreq > 0)
        {
            bool bBreak = false;
            int iRnd = getRandom(totalFreq);
            pEd = pLocalClient->getDataFactory()->getFirstEdition();
            while (pEd != NULL)
            {
                SpecialTile * pSpec = (SpecialTile*) pEd->getSpecialTiles()->getFirst(0);
                while (pSpec != NULL)
                {
                    pSpec->callLuaFunction("isAllowedOn", 1, "s", pAllTerrains[pTile->m_uTerrainType]);
                    if (pSpec->getLuaNumber() > 0)
                    {
                        iRnd -= pSpec->getFrequency();
                        if (iRnd < 0)
                        {
                            pTile->m_pSpecialTile = pSpec->instanciate((*spectiles)[i], pLocalClient->getDebug());
                            bBreak = true;
                            break;
                        }
                    }
                    pSpec = (SpecialTile*) pEd->getSpecialTiles()->getNext(0);
                }
                if (bBreak)
                    break;
                pEd = pLocalClient->getDataFactory()->getNextEdition();
            }
        }
    }
    // Pathfinder
    m_pPathfinder = new Pathfinder(m_pTiles, m_iWidth, m_iHeight);
}

// -----------------------------------------------------------------
// Name : createFromNetwork
// -----------------------------------------------------------------
void Map::createFromNetwork(NetworkData * pData, LocalClient * pLocalClient)
{
    m_iWidth = pData->readLong();
    m_iHeight = pData->readLong();
    m_pTiles = new MapTile**[m_iWidth];
    for (u16 x = 0; x < m_iWidth; x++)
    {
        m_pTiles[x] = new MapTile*[m_iHeight];
        for (u16 y = 0; y < m_iHeight; y++)
        {
            m_pTiles[x][y] = new MapTile(pData->readLong(), pLocalClient->getPlayerManager()->getGlobalSpellsPtr());
            if (pData->readLong() == 1)
                m_pTiles[x][y]->m_pSpecialTile = SpecialTile::deserialize(pData, pLocalClient->getDebug());
        }
    }
    int nbTowns = pData->readLong();
    for (int i = 0; i < nbTowns; i++)
    {
        Town * pTown = new Town(CoordsMap(0, 0), this, pLocalClient->getPlayerManager()->getGlobalSpellsPtr());
        pTown->deserialize(pData, pLocalClient);
        m_pTownsRef->addLast(pTown);
    }
    int nbTemples = pData->readLong();
    for (int i = 0; i < nbTemples; i++)
    {
        Temple * pTemple = new Temple(CoordsMap(0, 0), this, pLocalClient->getPlayerManager()->getGlobalSpellsPtr());
        pTemple->deserialize(pData, pLocalClient);
        m_pTemplesRef->addLast(pTemple);
    }
    m_pPathfinder = new Pathfinder(m_pTiles, m_iWidth, m_iHeight);
}

// -----------------------------------------------------------------
// Name : initGraphics
// -----------------------------------------------------------------
void Map::initGraphics(DisplayEngine * pDisplay)
{
    FREE(m_pTileGeometry);
    QuadData tilequad(0.0f, 1.0f, 0.0f, 1.0f, "maptile init texture", pDisplay);
    m_pTileGeometry = new GeometryQuads(&tilequad, VB_Static);
    assert(m_pTiles != NULL);
    for (u16 x = 0; x < m_iWidth; x++)
    {
        for (u16 y = 0; y < m_iHeight; y++)
        {
            m_pTiles[x][y]->initGraphics(m_pTileGeometry, pDisplay);
            setTileMask(x, y);
            Town * pTown = (Town*) m_pTiles[x][y]->getFirstMapObject(GOTYPE_TOWN);
            if (pTown != NULL)
                pTown->initGraphics(pDisplay);
            Temple * pTemple = (Temple*) m_pTiles[x][y]->getFirstMapObject(GOTYPE_TEMPLE);
            if (pTemple != NULL)
                pTemple->initGraphics(pDisplay);
        }
    }
    FREE(m_pEmptyMapGeometry);
    QuadData mapquad(0.0f, (float) m_iWidth, 0.0f, (float) m_iHeight, "map", pDisplay);
    m_pEmptyMapGeometry = new GeometryQuads(&mapquad, VB_Static);
    FREE(m_pTombGeometry);
    QuadData quad(0.0f, 0.4f, 0.0f, 0.4f, "skull", pDisplay);
    m_pTombGeometry = new GeometryQuads(&quad, VB_Static);
    FREE(m_pFoeBannerGeometry);
    QuadData quad2(0.0f, 0.3f, 0.0f, 0.3f, "attack_icon", pDisplay);
    m_pFoeBannerGeometry = new GeometryQuads(&quad2, VB_Static);
    FREE(m_pCountUnitsBgGeometry1L);
    QuadData quad3(0.0f, 0.4f, 0.0f, 0.3f, "bg-shadowed", pDisplay);
    m_pCountUnitsBgGeometry1L = new GeometryQuads(&quad3, VB_Static);
    FREE(m_pCountUnitsBgGeometry2L);
    QuadData quad4(0.0f, 0.4f, 0.0f, 0.6f, "bg-shadowed", pDisplay);
    m_pCountUnitsBgGeometry2L = new GeometryQuads(&quad4, VB_Static);
}

// -----------------------------------------------------------------
// Name : setTileMask
// -----------------------------------------------------------------
void Map::setTileMask(int x, int y)
{
    // Tile masks
    u16 uMask = MASK_NONE;
    if (y > 0 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x][y-1]->m_uTerrainType != TERRAIN_DESERT)
                  || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x][y-1]->m_uTerrainType != TERRAIN_TOUNDRA)
                  || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x][y-1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_NORTH;
    if (y < m_iHeight - 1 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x][y+1]->m_uTerrainType != TERRAIN_DESERT)
                              || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x][y+1]->m_uTerrainType != TERRAIN_TOUNDRA)
                              || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x][y+1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_SOUTH;
    if (x > 0 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x-1][y]->m_uTerrainType != TERRAIN_DESERT)
                  || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x-1][y]->m_uTerrainType != TERRAIN_TOUNDRA)
                  || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x-1][y]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_WEST;
    if (x < m_iWidth - 1 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x+1][y]->m_uTerrainType != TERRAIN_DESERT)
                             || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x+1][y]->m_uTerrainType != TERRAIN_TOUNDRA)
                             || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x+1][y]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_EAST;

    // Check corners
    if (!(uMask & MASK_NORTH) && !(uMask & MASK_WEST) && y > 0 && x > 0 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x-1][y-1]->m_uTerrainType != TERRAIN_DESERT)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x-1][y-1]->m_uTerrainType != TERRAIN_TOUNDRA)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x-1][y-1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_CORNER_NW;
    if (!(uMask & MASK_NORTH) && !(uMask & MASK_EAST) && x < m_iWidth - 1 && y > 0 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x+1][y-1]->m_uTerrainType != TERRAIN_DESERT)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x+1][y-1]->m_uTerrainType != TERRAIN_TOUNDRA)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x+1][y-1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_CORNER_NE;
    if (!(uMask & MASK_SOUTH) && !(uMask & MASK_EAST) && x < m_iWidth - 1 && y < m_iHeight - 1 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x+1][y+1]->m_uTerrainType != TERRAIN_DESERT)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x+1][y+1]->m_uTerrainType != TERRAIN_TOUNDRA)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x+1][y+1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_CORNER_SE;
    if (!(uMask & MASK_SOUTH) && !(uMask & MASK_WEST) && x > 0 && y < m_iHeight - 1 && ((m_pTiles[x][y]->m_uTerrainType == TERRAIN_DESERT && m_pTiles[x-1][y+1]->m_uTerrainType != TERRAIN_DESERT)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_TOUNDRA && m_pTiles[x-1][y+1]->m_uTerrainType != TERRAIN_TOUNDRA)
            || (m_pTiles[x][y]->m_uTerrainType == TERRAIN_SEA && m_pTiles[x-1][y+1]->m_uTerrainType != TERRAIN_SEA)))
        uMask |= MASK_CORNER_SW;
    m_pTiles[x][y]->setMask(uMask);
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void Map::update(double delta)
{
    // remove any empty UnitsGroup
    MetaObjectList * pGroup = (MetaObjectList*) m_pUnitsGroups->getFirst(0);
    while (pGroup != NULL)
    {
        if (pGroup->size <= 0)  // we don't delete one-unit group since new groups are created with only one unit inside
            pGroup = (MetaObjectList*) m_pUnitsGroups->deleteCurrent(0, true);
        else
            pGroup = (MetaObjectList*) m_pUnitsGroups->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void Map::display()
{
    DisplayEngine * pDisplay = m_pEmptyMapGeometry->getDisplay();
    Coords3D pos3D = pDisplay->get3DCoords(CoordsMap(0, 0), BOARDPLANE);
    m_pEmptyMapGeometry->display(pos3D, F_RGBA_NULL);
    CoordsMap screenTL = pDisplay->getMapCoords(CoordsScreen(0, 0));
    CoordsMap screenBR = pDisplay->getMapCoords(CoordsScreen(pDisplay->getParameters()->screenXSize, pDisplay->getParameters()->screenYSize));
    screenTL.x = 0;//max(0, screenTL.x);
    screenTL.y = 0;//max(0, screenTL.y);
    screenBR.x = m_iWidth - 1;//min(m_iWidth - 1, screenBR.x);
    screenBR.y = m_iHeight - 1;//min(m_iHeight - 1, screenBR.y);
    for (int x = screenTL.x; x <= screenBR.x; x++)
    {
        for (int y = screenTL.y; y <= screenBR.y; y++)
            m_pTiles[x][y]->display(CoordsMap(x, y));
    }
}

// -----------------------------------------------------------------
// Name : displayObjects
// -----------------------------------------------------------------
void Map::displayObjects(PlayerManager * pPlayerMngr)
{
    DisplayEngine * pDisplay = m_pTombGeometry->getDisplay();
    for (int x = 0; x < m_iWidth; x++)
    {
        for (int y = 0; y < m_iHeight; y++)
        {
            Coords3D pos3D = pDisplay->get3DCoords(CoordsMap(x, y), BOARDPLANE);
            MapObject * mapObj = m_pTiles[x][y]->getFirstMapObject(GOTYPE_TOWN);
            if (mapObj != NULL)
                mapObj->display();
            mapObj = m_pTiles[x][y]->getFirstMapObject(GOTYPE_TEMPLE);
            if (mapObj != NULL)
                mapObj->display();
            mapObj = m_pTiles[x][y]->getFirstMapObject(GOTYPE_DEAD_UNIT);
            if (mapObj != NULL)
                m_pTombGeometry->display(pos3D + Coords3D(0.6f, 0.1f), F_RGBA_NULL);
            mapObj = m_pTiles[x][y]->getFirstMapObject(GOTYPE_UNIT);
            if (mapObj != NULL)
            {
                mapObj->display();
                Player * pPlayer = pPlayerMngr->findPlayer(mapObj->getOwner());
                assert(pPlayer != NULL);
                pPlayer->m_pBannerGeometry->display(pos3D + Coords3D(0.7f, 0.0f), pPlayer->m_Color);
            }
            Player * pPlayer = pPlayerMngr->getActiveLocalPlayer();
            if (pPlayer != NULL)
            {
                if (m_pTiles[x][y]->m_pNbAlliesGeo != NULL)
                {
//          float fy = 0.31f;
                    if (m_pTiles[x][y]->m_pNbFoesGeo == NULL)
                    {
//            fy = 0.62f;
                        m_pCountUnitsBgGeometry1L->display(pos3D + Coords3D(-0.1f, 0.0f), F_RGBA_NULL);
                    }
                    else
                        m_pCountUnitsBgGeometry2L->display(pos3D + Coords3D(-0.1f, 0.0f), F_RGBA_NULL);
                    pPlayer->m_pBannerGeometry->display(pos3D + Coords3D(-0.1f, 0.0f), pPlayer->m_Color);
                    m_pTiles[x][y]->m_pNbAlliesGeo->display(pos3D + Coords3D(0.15f, 0.0f), F_RGBA_NULL);
                }
                if (m_pTiles[x][y]->m_pNbFoesGeo != NULL)
                {
                    float fy = 0.31f;
                    if (m_pTiles[x][y]->m_pNbAlliesGeo == NULL)
                    {
                        fy = 0.0f;
                        m_pCountUnitsBgGeometry1L->display(pos3D + Coords3D(-0.1f, 0.0f), F_RGBA_NULL);
                    }
                    m_pFoeBannerGeometry->display(pos3D + Coords3D(-0.1f, fy), rgb(0, 0, 0));
                    m_pTiles[x][y]->m_pNbFoesGeo->display(pos3D + Coords3D(0.15f, fy), F_RGBA_NULL);
                }
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : isInBounds
// -----------------------------------------------------------------
bool Map::isInBounds(CoordsMap position)
{
    return (position.x >= 0 && position.y >= 0 && position.x < m_iWidth && position.y < m_iHeight);
}

// -----------------------------------------------------------------
// Name : addObject
// -----------------------------------------------------------------
void Map::addObject(MapObject * mapObj)
{
    m_pTiles[mapObj->getMapPos().x][mapObj->getMapPos().y]->m_pMapObjects->addFirst(mapObj);
}

// -----------------------------------------------------------------
// Name : removeObject
// -----------------------------------------------------------------
void Map::removeObject(MapObject * mapObj)
{
    m_pTiles[mapObj->getMapPos().x][mapObj->getMapPos().y]->m_pMapObjects->deleteObject(mapObj, true);
}

// -----------------------------------------------------------------
// Name : bringAbove
// -----------------------------------------------------------------
void Map::bringAbove(MapObject * mapObj)
{
    if (m_pTiles[mapObj->getMapPos().x][mapObj->getMapPos().y]->m_pMapObjects->goTo(0, mapObj))
        m_pTiles[mapObj->getMapPos().x][mapObj->getMapPos().y]->m_pMapObjects->moveCurrentToBegin(0);
}

// -----------------------------------------------------------------
// Name : getFirstObjectAt
// -----------------------------------------------------------------
MapObject * Map::getFirstObjectAt(int x, int y, u32 type)
{
    GraphicObject * pObj = (GraphicObject*) m_pTiles[x][y]->m_pMapObjects->getFirst(0);
    while (pObj != NULL)
    {
        if (pObj->getType() & type)
            return (MapObject*) pObj;
        pObj = (GraphicObject*) m_pTiles[x][y]->m_pMapObjects->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextObjectAt
// -----------------------------------------------------------------
MapObject * Map::getNextObjectAt(int x, int y, u32 type)
{
    GraphicObject * pObj = (GraphicObject*) m_pTiles[x][y]->m_pMapObjects->getNext(0);
    while (pObj != NULL)
    {
        if (pObj->getType() & type)
            return (MapObject*) pObj;
        pObj = (GraphicObject*) m_pTiles[x][y]->m_pMapObjects->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getObjectsCountAt
// -----------------------------------------------------------------
int Map::getObjectsCountAt(int x, int y)
{
    return m_pTiles[x][y]->m_pMapObjects->size;
}

// -----------------------------------------------------------------
// Name : findPath
// -----------------------------------------------------------------
s16 Map::findPath(MapObject * mapObj, CoordsMap goal, CoordsMap ** solution)
{
    return m_pPathfinder->aStar(mapObj, goal, solution);
}

// -----------------------------------------------------------------
// Name : findPath
// -----------------------------------------------------------------
s16 Map::findPath(ObjectList * pGroup, CoordsMap goal, CoordsMap ** solution)
{
    return m_pPathfinder->aStar(pGroup, goal, solution);
}

// -----------------------------------------------------------------
// Name : getFirstPlayerGroup
// -----------------------------------------------------------------
MetaObjectList * Map::getFirstPlayerGroup(u8 uPlayer)
{
    MetaObjectList * pGroup = (MetaObjectList*) m_pUnitsGroups->getFirst(0);
    while (pGroup != NULL)
    {
        Unit * pUnit = (Unit*) pGroup->getFirst(0);
        if (pUnit != NULL && pUnit->getOwner() == uPlayer)
            return pGroup;
        pGroup = (MetaObjectList*) m_pUnitsGroups->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextPlayerGroup
// -----------------------------------------------------------------
MetaObjectList * Map::getNextPlayerGroup(u8 uPlayer)
{
    MetaObjectList * pGroup = (MetaObjectList*) m_pUnitsGroups->getNext(0);
    while (pGroup != NULL)
    {
        Unit * pUnit = (Unit*) pGroup->getFirst(0);
        if (pUnit != NULL && pUnit->getOwner() == uPlayer)
            return pGroup;
        pGroup = (MetaObjectList*) m_pUnitsGroups->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : resetCurrentGroup
// -----------------------------------------------------------------
MetaObjectList * Map::resetCurrentGroup(BaseObject * pItem, MetaObjectList * pCurrentGroup)
{
    if (((GraphicObject*)pItem)->getType() & GOTYPE_UNIT)
    {
        MetaObjectList * pGroup = ((Unit*)pItem)->getGroup();
        if (pGroup != NULL && pGroup == pCurrentGroup)
        {
            pGroup->deleteObject(pItem, false);
            ((Unit*)pItem)->setGroup(NULL);
        }
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : onClickOnGroupItem
// -----------------------------------------------------------------
MetaObjectList * Map::onClickOnGroupItem(BaseObject * pItem, bool bClickState, MetaObjectList * pCurrentGroup)
{
    if (((GraphicObject*)pItem)->getType() & GOTYPE_UNIT)
    {
        MetaObjectList * pGroup = ((Unit*)pItem)->getGroup();
        if (bClickState)
        {
            if (pCurrentGroup == NULL && pGroup == NULL)
            {
                // Create a new group
                MetaObjectList * pNewGroup = new MetaObjectList(false);
                m_pUnitsGroups->addFirst(pNewGroup);
                pNewGroup->addFirst(pItem);
                ((Unit*)pItem)->setGroup(pNewGroup);
                return pNewGroup;
            }
            else if (pCurrentGroup == NULL)
            {
                // Return existing unit's group
                return pGroup;
            }
            else if (pGroup == NULL)
            {
                // Add unit to current group
                pCurrentGroup->addFirst(pItem);
                ((Unit*)pItem)->setGroup(pCurrentGroup);
                return pCurrentGroup;
            }
            else
            {
                // Remove unit from its previous group, and add it to the new current group
                pGroup->deleteObject(pItem, false);
                pCurrentGroup->addFirst(pItem);
                ((Unit*)pItem)->setGroup(pCurrentGroup);
                return pCurrentGroup;
            }
        }
        else
        {
            if (pCurrentGroup == NULL)
            {
                // Nothing to do
                return NULL;
            }
            else
            {
                if (pGroup == NULL)
                {
                    // Remove unit from current group (although this case should not happen)
                    pCurrentGroup->deleteObject(pItem, false);
                    if (pCurrentGroup->size == 0)
                        return NULL;
                    else
                        return pCurrentGroup;
                }
                else
                {
                    pCurrentGroup->deleteObject(pItem, false);
                    pGroup->deleteObject(pItem, false);
                    ((Unit*)pItem)->setGroup(NULL);
                    if (pCurrentGroup->size == 0)
                        return NULL;
                    else
                        return pCurrentGroup;
                }
            }
        }
    }
    else
        return pCurrentGroup;
}

// -----------------------------------------------------------------
// Name : saveCurrentGroupOrder
// -----------------------------------------------------------------
void Map::saveCurrentGroupOrder(Unit * pUnit)
{
    if (pUnit->getGroup() != NULL)
    {
        Unit * pOtherUnit = (Unit*) pUnit->getGroup()->getFirst(0);
        while (pOtherUnit != NULL)
        {
            pOtherUnit->saveCurrentOrder();
            pOtherUnit = (Unit*) pUnit->getGroup()->getNext(0);
        }
    }
    else
        pUnit->saveCurrentOrder();
}

// -----------------------------------------------------------------
// Name : retrievePreviousGroupOrder
// -----------------------------------------------------------------
void Map::retrievePreviousGroupOrder(Unit * pUnit, UnitOptionsDlg * pDlg)
{
    if (pUnit->getGroup() != NULL)
    {
        Unit * pOtherUnit = (Unit*) pUnit->getGroup()->getFirst(0);
        while (pOtherUnit != NULL)
        {
            pOtherUnit->retrievePreviousOrder(pDlg);
            pOtherUnit = (Unit*) pUnit->getGroup()->getNext(0);
        }
    }
    else
        pUnit->retrievePreviousOrder(pDlg);
}

// -----------------------------------------------------------------
// Name : unsetGroupOrder
// -----------------------------------------------------------------
void Map::unsetGroupOrder(Unit * pUnit)
{
    if (pUnit->getGroup() != NULL)
    {
        Unit * pOtherUnit = (Unit*) pUnit->getGroup()->getFirst(0);
        while (pOtherUnit != NULL)
        {
            pOtherUnit->unsetOrder();
            pOtherUnit = (Unit*) pUnit->getGroup()->getNext(0);
        }
    }
    else
        pUnit->unsetOrder();
}

// -----------------------------------------------------------------
// Name : setFortifyGroupOrder
// -----------------------------------------------------------------
void Map::setFortifyGroupOrder(Unit * pUnit)
{
    if (pUnit->getGroup() != NULL)
    {
        Unit * pOtherUnit = (Unit*) pUnit->getGroup()->getFirst(0);
        while (pOtherUnit != NULL)
        {
            pOtherUnit->setFortifyOrder();
            pOtherUnit = (Unit*) pUnit->getGroup()->getNext(0);
        }
    }
    else
        pUnit->setFortifyOrder();
}

// -----------------------------------------------------------------
// Name : setMoveGroupOrder
//  returns true if the move is valid
// -----------------------------------------------------------------
bool Map::setMoveGroupOrder(Unit * pUnit, CoordsMap mapPos, UnitOptionsDlg * pDlg)
{
    if (pUnit->getGroup() != NULL && pUnit->getGroup()->size > 1)
        return pUnit->setGroupMoveOrder(mapPos, pDlg);
    else
        return pUnit->setMoveOrder(mapPos, pDlg);
}

// -----------------------------------------------------------------
// Name : setAttackGroupOrder
// -----------------------------------------------------------------
bool Map::setAttackGroupOrder(Unit * pUnit, Unit * pAttackTarget, UnitOptionsDlg * pDlg)
{
    if (pUnit->getGroup() != NULL)
        return pUnit->setGroupAttackOrder(pAttackTarget, pDlg);
    else
        return pUnit->setAttackOrder(pAttackTarget, pDlg);
}

// -----------------------------------------------------------------
// Name : createNewGroup
// -----------------------------------------------------------------
MetaObjectList * Map::createNewGroup()
{
    MetaObjectList * pGroup = new MetaObjectList(false);
    m_pUnitsGroups->addLast(pGroup);
    return pGroup;
}

// -----------------------------------------------------------------
// Name : resetAllPlayerGroups
// -----------------------------------------------------------------
void Map::resetAllPlayerGroups(u8 uPlayer)
{
    MetaObjectList * pGroup = (MetaObjectList*) m_pUnitsGroups->getFirst(0);
    while (pGroup != NULL)
    {
        Unit * pUnit = (Unit*) pGroup->getFirst(0);
        if (pUnit != NULL && pUnit->getOwner() == uPlayer)
        {
            pUnit->setGroup(NULL);
            pGroup = (MetaObjectList*) m_pUnitsGroups->deleteCurrent(0, true);
        }
        else
            pGroup = (MetaObjectList*) m_pUnitsGroups->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : findTown
// -----------------------------------------------------------------
Town * Map::findTown(u32 uTownId)
{
    // Loop through map
    for (int x = 0; x < m_iWidth; x++)
    {
        for (int y = 0; y < m_iHeight; y++)
        {
            Town * pTown = (Town*) (m_pTiles[x][y])->getFirstMapObject(GOTYPE_TOWN);
            if (pTown != NULL && pTown->getId() == uTownId)
                return pTown;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getFirstTown
// -----------------------------------------------------------------
Town * Map::getFirstTown()
{
    // Init TownFinder position to (0,0)
    m_TownFinderPos = CoordsMap(0, 0);

    // Loop through map
    while (m_TownFinderPos.x < m_iWidth)
    {
        while (m_TownFinderPos.y < m_iHeight)
        {
            Town * pTown = (Town*) (m_pTiles[m_TownFinderPos.x][m_TownFinderPos.y])->getFirstMapObject(GOTYPE_TOWN);
            if (pTown != NULL)
                return pTown;
            m_TownFinderPos.y++;
        }
        m_TownFinderPos.x++;
        m_TownFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextTown
// -----------------------------------------------------------------
Town * Map::getNextTown()
{
    // Increase TownFinder position (it was stopped at last town position)
    m_TownFinderPos.y++;
    if (m_TownFinderPos.y >= m_iHeight)
    {
        m_TownFinderPos.x++;
        m_TownFinderPos.y = 0;
    }

    // Loop through map
    while (m_TownFinderPos.x < m_iWidth)
    {
        while (m_TownFinderPos.y < m_iHeight)
        {
            Town * pTown = (Town*) (m_pTiles[m_TownFinderPos.x][m_TownFinderPos.y])->getFirstMapObject(GOTYPE_TOWN);
            if (pTown != NULL)
                return pTown;
            m_TownFinderPos.y++;
        }
        m_TownFinderPos.x++;
        m_TownFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : findTemple
// -----------------------------------------------------------------
Temple * Map::findTemple(u32 uTempleId)
{
    // Loop through map
    for (int x = 0; x < m_iWidth; x++)
    {
        for (int y = 0; y < m_iHeight; y++)
        {
            Temple * pTemple = (Temple*) (m_pTiles[x][y])->getFirstMapObject(GOTYPE_TEMPLE);
            if (pTemple != NULL && pTemple->getId() == uTempleId)
                return pTemple;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getFirstTemple
// -----------------------------------------------------------------
Temple * Map::getFirstTemple()
{
    // Init TownFinder position to (0,0)
    m_TempleFinderPos = CoordsMap(0, 0);

    // Loop through map
    while (m_TempleFinderPos.x < m_iWidth)
    {
        while (m_TempleFinderPos.y < m_iHeight)
        {
            Temple * pTemple = (Temple*) (m_pTiles[m_TempleFinderPos.x][m_TempleFinderPos.y])->getFirstMapObject(GOTYPE_TEMPLE);
            if (pTemple != NULL)
                return pTemple;
            m_TempleFinderPos.y++;
        }
        m_TempleFinderPos.x++;
        m_TempleFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextTemple
// -----------------------------------------------------------------
Temple * Map::getNextTemple()
{
    // Increase TownFinder position (it was stopped at last town position)
    m_TempleFinderPos.y++;
    if (m_TempleFinderPos.y >= m_iHeight)
    {
        m_TempleFinderPos.x++;
        m_TempleFinderPos.y = 0;
    }

    // Loop through map
    while (m_TempleFinderPos.x < m_iWidth)
    {
        while (m_TempleFinderPos.y < m_iHeight)
        {
            Temple * pTemple = (Temple*) (m_pTiles[m_TempleFinderPos.x][m_TempleFinderPos.y])->getFirstMapObject(GOTYPE_TEMPLE);
            if (pTemple != NULL)
                return pTemple;
            m_TempleFinderPos.y++;
        }
        m_TempleFinderPos.x++;
        m_TempleFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : findSpecialTile
// -----------------------------------------------------------------
SpecialTile * Map::findSpecialTile(u32 uId)
{
    // Loop through map
    for (int x = 0; x < m_iWidth; x++)
    {
        for (int y = 0; y < m_iHeight; y++)
        {
            if (m_pTiles[x][y]->m_pSpecialTile != NULL && m_pTiles[x][y]->m_pSpecialTile->getInstanceId() == uId)
                return m_pTiles[x][y]->m_pSpecialTile;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getFirstSpecialTile
// -----------------------------------------------------------------
SpecialTile * Map::getFirstSpecialTile()
{
    // Init SpecialTileFinder position to (0,0)
    m_SpecialTileFinderPos = CoordsMap(0, 0);

    // Loop through map
    while (m_SpecialTileFinderPos.x < m_iWidth)
    {
        while (m_SpecialTileFinderPos.y < m_iHeight)
        {
            if (m_pTiles[m_SpecialTileFinderPos.x][m_SpecialTileFinderPos.y]->m_pSpecialTile != NULL)
                return m_pTiles[m_SpecialTileFinderPos.x][m_SpecialTileFinderPos.y]->m_pSpecialTile;
            m_SpecialTileFinderPos.y++;
        }
        m_SpecialTileFinderPos.x++;
        m_SpecialTileFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getNextSpecialTile
// -----------------------------------------------------------------
SpecialTile * Map::getNextSpecialTile()
{
    // Increase SpecialTileFinder position (it was stopped at last SpecialTile position)
    m_SpecialTileFinderPos.y++;
    if (m_SpecialTileFinderPos.y >= m_iHeight)
    {
        m_SpecialTileFinderPos.x++;
        m_SpecialTileFinderPos.y = 0;
    }

    // Loop through map
    while (m_SpecialTileFinderPos.x < m_iWidth)
    {
        while (m_SpecialTileFinderPos.y < m_iHeight)
        {
            if (m_pTiles[m_SpecialTileFinderPos.x][m_SpecialTileFinderPos.y]->m_pSpecialTile != NULL)
                return m_pTiles[m_SpecialTileFinderPos.x][m_SpecialTileFinderPos.y]->m_pSpecialTile;
            m_SpecialTileFinderPos.y++;
        }
        m_SpecialTileFinderPos.x++;
        m_SpecialTileFinderPos.y = 0;
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : changeTerrainType
// -----------------------------------------------------------------
void Map::changeTerrainType(CoordsMap pos, u8 uType, Server * pServer)
{
    MapTile * pTile = getTileAt(pos);
    pTile->m_uTerrainType = uType;
    if (pServer != NULL)
    {
        NetworkData msg(NETWORKMSG_CHANGE_TERRAIN);
        msg.addLong(pos.x);
        msg.addLong(pos.y);
        msg.addLong(uType);
        pServer->sendMessageToAllClients(&msg);
    }
    else
    {
        pTile->resetTexture(pTile->getDisplay());
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                int x = pos.x + i;
                int y = pos.y + j;
                if (x < 0 || y < 0 || x >= m_iWidth || y >= m_iHeight)
                    continue;
                setTileMask(x, y);
            }
        }
    }
}
