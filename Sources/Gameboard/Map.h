#ifndef _MAP_H
#define _MAP_H

#include "MapTile.h"
#include "Pathfinder.h"
#include "../Server/NetworkData.h"
#include "../Interface/StackGroupInterface.h"
#include "../Geometries/GeometryQuads.h"

class MapObject;
class Unit;
class Town;
class Temple;
class PlayerManager;
struct TownData;
class LocalClient;
class MapReader;
class Server;
class UnitOptionsDlg;

class Map : public StackGroupInterface
{
public:
  // Constructor / destructor
  Map();
  ~Map();

  // General
  void initGraphics(DisplayEngine * pDisplay);
  void update(double delta);
  void display();
  void displayObjects(PlayerManager * pPlayerMngr);

  // Set map data
  void createFromServer(MapReader * pMapReader, LocalClient * pLocalClient);
  void createFromNetwork(NetworkData * pData, LocalClient * pLocalClient);

  // Member access
  int getNumberOfTiles() { return m_iWidth * m_iHeight; };
  int getWidth() { return m_iWidth; };
  int getHeight() { return m_iHeight; };
  MapTile * getTileAt(CoordsMap position) { if (!isInBounds(position)) return NULL; return m_pTiles[position.x][position.y]; };
  MapObject * getFirstObjectAt(int x, int y, u32 type = GOTYPE_MAPOBJECT);
  MapObject * getNextObjectAt(int x, int y, u32 type = GOTYPE_MAPOBJECT);
  int getObjectsCountAt(int x, int y);
  MapObject * getFirstObjectAt(CoordsMap pos, u32 type = GOTYPE_MAPOBJECT) { return getFirstObjectAt(pos.x, pos.y, type); };
  MapObject * getNextObjectAt(CoordsMap pos, u32 type = GOTYPE_MAPOBJECT) { return getNextObjectAt(pos.x, pos.y, type); };
  int getObjectsCountAt(CoordsMap pos) { return getObjectsCountAt(pos.x, pos.y); };

  // Misc.
  bool isInBounds(CoordsMap position);
  s16 findPath(MapObject * mapObj, CoordsMap goal, CoordsMap ** solution);
  s16 findPath(ObjectList * pGroup, CoordsMap goal, CoordsMap ** solution);
  void changeTerrainType(CoordsMap pos, u8 uType, Server * pServer);

  // Objects
  void addObject(MapObject * mapObj);
  void removeObject(MapObject * mapObj);
  void bringAbove(MapObject * mapObj);

  // Towns
  Town * getFirstTown();
  Town * getNextTown();
  Town * findTown(u32 uTownId);

  // Temples
  Temple * getFirstTemple();
  Temple * getNextTemple();
  Temple * findTemple(u32 uTemplesId);

  // Special tiles
  SpecialTile * getFirstSpecialTile();
  SpecialTile * getNextSpecialTile();
  SpecialTile * findSpecialTile(u32 uId);

  // Unit groups
  MetaObjectList * getFirstPlayerGroup(u8 uPlayer);
  MetaObjectList * getNextPlayerGroup(u8 uPlayer);
  MetaObjectList * resetCurrentGroup(BaseObject * pItem, MetaObjectList * pCurrentGroup);
  MetaObjectList * onClickOnGroupItem(BaseObject * pItem, bool bClickState, MetaObjectList * pCurrentGroup);
  void unsetGroupOrder(Unit * pUnit);
  void setFortifyGroupOrder(Unit * pUnit);
  bool setMoveGroupOrder(Unit * pUnit, CoordsMap mapPos, UnitOptionsDlg * pDlg);
  bool setAttackGroupOrder(Unit * pUnit, Unit * pAttackTarget, UnitOptionsDlg * pDlg);
  void retrievePreviousGroupOrder(Unit * pUnit, UnitOptionsDlg * pDlg);
  void saveCurrentGroupOrder(Unit * pUnit);
  void resetAllPlayerGroups(u8 uPlayer);
  MetaObjectList * createNewGroup();

  MapObject * m_pObjectSelectedFromStack;

private:
  void setTileMask(int x, int y);

  MapTile *** m_pTiles;    // Map data
  int m_iWidth;
  int m_iHeight;
  Pathfinder * m_pPathfinder;
  GeometryQuads * m_pTombGeometry;
  ObjectList * m_pUnitsGroups;
  CoordsMap m_TownFinderPos;
  CoordsMap m_TempleFinderPos;
  CoordsMap m_SpecialTileFinderPos;
  GeometryQuads * m_pEmptyMapGeometry;
  GeometryQuads * m_pFoeBannerGeometry;
  GeometryQuads * m_pCountUnitsBgGeometry1L;
  GeometryQuads * m_pCountUnitsBgGeometry2L;
  int m_iNbTowns;
  int m_iNbTemples;
  Geometry * m_pTileGeometry;
  ObjectList * m_pTownsRef; // just for clearing memory
  ObjectList * m_pTemplesRef; // just for clearing memory
};

#endif
