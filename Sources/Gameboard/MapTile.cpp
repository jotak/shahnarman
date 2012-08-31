#include "MapTile.h"
#include "../Geometries/GeometryQuads.h"
#include "../Geometries/GeometryText.h"
#include "../Data/LocalisationTool.h"
#include "MapObject.h"
#include "SpecialTile.h"

// -----------------------------------------------------------------
// Name : MapTile
//  Constructor
// -----------------------------------------------------------------
MapTile::MapTile(u8 uTerrainType, ObjectList ** pGlobalEffects) : GraphicObject(), LuaTargetable(pGlobalEffects, L"")
{
  m_iTexture = -1;
  m_uTerrainType = uTerrainType;
  m_pMapObjects = new ObjectList(false);  // So that it doesn't try to delete these objects memory ; it will be deleted from PlayerManager
  m_pGeometryPtr = NULL;
  m_pSpecialTile = NULL;
  m_pSpecialTileGeometry = NULL;
  m_pNbAlliesGeo = m_pNbFoesGeo = NULL;
  u8 uFood = 0;
  u8 uProd = 0;
  switch (m_uTerrainType)
  {
  case TERRAIN_SEA:
    uFood = 2;
    uProd = 0;
    break;
  case TERRAIN_PLAIN:
    uFood = 3;
    uProd = 1;
    break;
  case TERRAIN_FOREST:
    uFood = 2;
    uProd = 2;
    break;
  case TERRAIN_MOUNTAIN:
    uFood = 0;
    uProd = 2;
    break;
  case TERRAIN_DESERT:
    uFood = 0;
    uProd = 0;
    break;
  case TERRAIN_TOUNDRA:
    uFood = 1;
    uProd = 0;
    break;
  }
  registerValue(STRING_FOOD, (long) uFood);
  registerValue(STRING_PROD, (long) uProd);
  m_iMaskTexture = -1;
  swprintf_s(m_sIdentifiers, 16, L"tile %d", (int) m_uTerrainType);
  m_uInfluence = 0;
}

// -----------------------------------------------------------------
// Name : ~MapTile
//  Destructor
// -----------------------------------------------------------------
MapTile::~MapTile()
{
  delete m_pMapObjects;
  FREE(m_pSpecialTile);
  FREE(m_pSpecialTileGeometry);
  FREE(m_pNbAlliesGeo);
  FREE(m_pNbFoesGeo);
}

// -----------------------------------------------------------------
// Name : resetTexture
// -----------------------------------------------------------------
void MapTile::resetTexture(DisplayEngine * pDisplay)
{
  int rnd;
  wchar_t sTex[64];
  switch (m_uTerrainType)
  {
  case TERRAIN_SEA:
    wsafecpy(sTex, 64, L"sea1");
    break;
  case TERRAIN_PLAIN:
    swprintf_s(sTex, 64, L"plain1");
    break;
  case TERRAIN_FOREST:
    rnd = getRandom(7) + 1;
    swprintf_s(sTex, 64, L"forest%d", rnd);
    break;
  case TERRAIN_MOUNTAIN:
    rnd = getRandom(7) + 1;
    swprintf_s(sTex, 64, L"mountain%d", rnd);
    break;
  case TERRAIN_DESERT:
    swprintf_s(sTex, 64, L"desert1");
    break;
  case TERRAIN_TOUNDRA:
    swprintf_s(sTex, 64, L"toundra1");
    break;
  }
  m_iTexture = pDisplay->getTextureEngine()->loadTexture(sTex, true);
}

// -----------------------------------------------------------------
// Name : initGraphics
// -----------------------------------------------------------------
void MapTile::initGraphics(Geometry * pTileGeo, DisplayEngine * pDisplay)
{
  m_pGeometryPtr = pTileGeo;
  resetTexture(pDisplay);
  if (m_pSpecialTile != NULL)
  {
    QuadData quad2(0.0f, 1.0f, 0.0f, 1.0f, m_pSpecialTile->getIconPath(), pDisplay);
    m_pSpecialTileGeometry = new GeometryQuads(&quad2, VB_Static);
  }
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void MapTile::display(CoordsMap mapPos)
{
  Coords3D pos3D = getDisplay()->get3DCoords(mapPos, BOARDPLANE);
  if (m_iMaskTexture >= 0)
  {
    if (m_uTerrainType == TERRAIN_SEA)
      getDisplay()->setMaskBlending(true);
    else
      getDisplay()->setMaskBlending(false);
    ((GeometryQuads*)m_pGeometryPtr)->setTexture(m_iMaskTexture);
    m_pGeometryPtr->display(pos3D, F_RGBA_NULL);
    getDisplay()->startMaskBlending();
    ((GeometryQuads*)m_pGeometryPtr)->setTexture(m_iTexture);
    m_pGeometryPtr->display(pos3D, F_RGBA_NULL);
    getDisplay()->stopMaskBlending();
  }
  else
  {
    ((GeometryQuads*)m_pGeometryPtr)->setTexture(m_iTexture);
    m_pGeometryPtr->display(pos3D, F_RGBA_NULL);
  }

  if (m_pSpecialTileGeometry != NULL)
    m_pSpecialTileGeometry->display(pos3D, F_RGBA_NULL);
}

// -----------------------------------------------------------------
// Name : setMask
// -----------------------------------------------------------------
void MapTile::setMask(u16 uMask)
{
  wchar_t sCornerStr[64] = L"";
  // first check corners
  if ((uMask & MASK_CORNER_NW) && (uMask & MASK_CORNER_NE) && (uMask & MASK_CORNER_SE) && (uMask & MASK_CORNER_SW))
  {
    wsafecpy(sCornerStr, 64, L"_corner_all");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_NE | MASK_CORNER_SE | MASK_CORNER_SW);
  }
  else if ((uMask & MASK_CORNER_NW) && (uMask & MASK_CORNER_NE) && (uMask & MASK_CORNER_SE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_wnes");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_NE | MASK_CORNER_SE);
  }
  else if ((uMask & MASK_CORNER_NE) && (uMask & MASK_CORNER_SE) && (uMask & MASK_CORNER_SW))
  {
    wsafecpy(sCornerStr, 64, L"_corner_nesw");
    uMask &= ~(MASK_CORNER_SW | MASK_CORNER_NE | MASK_CORNER_SE);
  }
  else if ((uMask & MASK_CORNER_SE) && (uMask & MASK_CORNER_SW) && (uMask & MASK_CORNER_NW))
  {
    wsafecpy(sCornerStr, 64, L"_corner_eswn");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_SW | MASK_CORNER_SE);
  }
  else if ((uMask & MASK_CORNER_SW) && (uMask & MASK_CORNER_NW) && (uMask & MASK_CORNER_NE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_swne");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_NE | MASK_CORNER_SW);
  }
  else if ((uMask & MASK_CORNER_NW) && (uMask & MASK_CORNER_SE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_nswe");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_SE);
  }
  else if ((uMask & MASK_CORNER_SW) && (uMask & MASK_CORNER_NE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_nsew");
    uMask &= ~(MASK_CORNER_SW | MASK_CORNER_NE);
  }
  else if ((uMask & MASK_CORNER_NW) && (uMask & MASK_CORNER_NE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_wne");
    uMask &= ~(MASK_CORNER_NW | MASK_CORNER_NE);
  }
  else if ((uMask & MASK_CORNER_NE) && (uMask & MASK_CORNER_SE))
  {
    wsafecpy(sCornerStr, 64, L"_corner_nes");
    uMask &= ~(MASK_CORNER_NE | MASK_CORNER_SE);
  }
  else if ((uMask & MASK_CORNER_SE) && (uMask & MASK_CORNER_SW))
  {
    wsafecpy(sCornerStr, 64, L"_corner_esw");
    uMask &= ~(MASK_CORNER_SE | MASK_CORNER_SW);
  }
  else if ((uMask & MASK_CORNER_SW) && (uMask & MASK_CORNER_NW))
  {
    wsafecpy(sCornerStr, 64, L"_corner_swn");
    uMask &= ~(MASK_CORNER_SW | MASK_CORNER_NW);
  }
  else if (uMask & MASK_CORNER_NW)
  {
    wsafecpy(sCornerStr, 64, L"_corner_nw");
    uMask &= ~MASK_CORNER_NW;
  }
  else if (uMask & MASK_CORNER_NE)
  {
    wsafecpy(sCornerStr, 64, L"_corner_ne");
    uMask &= ~MASK_CORNER_NE;
  }
  else if (uMask & MASK_CORNER_SW)
  {
    wsafecpy(sCornerStr, 64, L"_corner_sw");
    uMask &= ~MASK_CORNER_SW;
  }
  else if (uMask & MASK_CORNER_SE)
  {
    wsafecpy(sCornerStr, 64, L"_corner_se");
    uMask &= ~MASK_CORNER_SE;
  }

  wchar_t sMainStr[64] = L"";
  switch (uMask)
  {
  case MASK_NONE:
    wsafecpy(sMainStr, 64, L"tile_mask");
    break;
  case MASK_NORTH:
    wsafecpy(sMainStr, 64, L"tile_mask_north");
    break;
  case MASK_EAST:
    wsafecpy(sMainStr, 64, L"tile_mask_east");
    break;
  case MASK_SOUTH:
    wsafecpy(sMainStr, 64, L"tile_mask_south");
    break;
  case MASK_WEST:
    wsafecpy(sMainStr, 64, L"tile_mask_west");
    break;
  case MASK_NORTH | MASK_EAST:
    wsafecpy(sMainStr, 64, L"tile_mask_ne");
    break;
  case MASK_EAST | MASK_SOUTH:
    wsafecpy(sMainStr, 64, L"tile_mask_se");
    break;
  case MASK_SOUTH | MASK_WEST:
    wsafecpy(sMainStr, 64, L"tile_mask_sw");
    break;
  case MASK_WEST | MASK_NORTH:
    wsafecpy(sMainStr, 64, L"tile_mask_nw");
    break;
  case MASK_NORTH | MASK_EAST | MASK_SOUTH:
    wsafecpy(sMainStr, 64, L"tile_mask_nes");
    break;
  case MASK_EAST | MASK_SOUTH | MASK_WEST:
    wsafecpy(sMainStr, 64, L"tile_mask_esw");
    break;
  case MASK_SOUTH | MASK_WEST | MASK_NORTH:
    wsafecpy(sMainStr, 64, L"tile_mask_swn");
    break;
  case MASK_WEST | MASK_NORTH | MASK_EAST:
    wsafecpy(sMainStr, 64, L"tile_mask_wne");
    break;
  case MASK_WEST | MASK_EAST:
    wsafecpy(sMainStr, 64, L"tile_mask_ew");
    break;
  case MASK_SOUTH | MASK_NORTH:
    wsafecpy(sMainStr, 64, L"tile_mask_ns");
    break;
  case MASK_WEST | MASK_NORTH | MASK_EAST | MASK_SOUTH:
    wsafecpy(sMainStr, 64, L"tile_mask_all");
    break;
  }
  wchar_t sPath[MAX_PATH];
  swprintf_s(sPath, MAX_PATH, L"%s%s", sMainStr, sCornerStr);
  if (wcscmp(sPath, L"tile_mask") != 0)
    m_iMaskTexture = getDisplay()->getTextureEngine()->loadTexture(sPath, true);
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
wchar_t * MapTile::getInfo(wchar_t * sBuf, int iSize)
{
  wchar_t sTerrain[128] = L"";
  switch (m_uTerrainType)
  {
  case TERRAIN_PLAIN:
    i18n->getText1stUp(L"PLAIN", sTerrain, 128);
    break;
  case TERRAIN_FOREST:
    i18n->getText1stUp(L"FOREST", sTerrain, 128);
    break;
  case TERRAIN_MOUNTAIN:
    i18n->getText1stUp(L"MOUNTAIN", sTerrain, 128);
    break;
  case TERRAIN_TOUNDRA:
    i18n->getText1stUp(L"TOUNDRA", sTerrain, 128);
    break;
  case TERRAIN_DESERT:
    i18n->getText1stUp(L"DESERT", sTerrain, 128);
    break;
  case TERRAIN_SEA:
    i18n->getText1stUp(L"SEA", sTerrain, 128);
    break;
  }
  wsafecpy(sBuf, iSize, sTerrain);
  wsafecat(sBuf, iSize, L" (");
  getInfo_AddValue(sBuf, iSize, STRING_FOOD, L", ");
  getInfo_AddValue(sBuf, iSize, STRING_PROD, L"");
  wsafecat(sBuf, iSize, L")");
  if (m_pSpecialTile != NULL)
  {
    wsafecat(sBuf, iSize, L"\n");
    wsafecat(sBuf, iSize, m_pSpecialTile->getLocalizedName());
  }
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getFirstMapObject
// -----------------------------------------------------------------
MapObject * MapTile::getFirstMapObject(u32 uType, int _it)
{
  MapObject * pMapObj = (MapObject*) m_pMapObjects->getFirst(_it);
  while (pMapObj != NULL)
  {
    if (pMapObj->getType() & uType)
      return pMapObj;
    pMapObj = (MapObject*) m_pMapObjects->getNext(_it);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getNextMapObject
// -----------------------------------------------------------------
MapObject * MapTile::getNextMapObject(u32 uType, int _it)
{
  MapObject * pMapObj = (MapObject*) m_pMapObjects->getNext(_it);
  while (pMapObj != NULL)
  {
    if (pMapObj->getType() & uType)
      return pMapObj;
    pMapObj = (MapObject*) m_pMapObjects->getNext(_it);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : hideSpecialTile
// -----------------------------------------------------------------
void MapTile::hideSpecialTile()
{
  FREE(m_pSpecialTileGeometry);
}
