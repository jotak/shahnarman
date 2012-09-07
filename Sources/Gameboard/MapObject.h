#ifndef _MAP_OBJECT_H
#define _MAP_OBJECT_H

#include "../Common/GraphicObject.h"
#include "../Data/LuaTargetable.h"
#include "Map.h"

enum InfoDest
{
  Dest_InfoDialog = 0,
  Dest_ShortInfoDialog,
  Dest_MapObjDialog
};

class MapObject : public GraphicObject, public LuaTargetable
{
friend class MapGenerator;
public:
  MapObject(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects, const char * sIdentifiers);

  // Virtual functions
  virtual u32 getType() { return GraphicObject::getType() | GOTYPE_MAPOBJECT; };
  virtual void initServer() {};
  virtual short getMoveCost(unsigned char terrainType) { return -1; }; // For A* algorithm
  virtual void display() = 0;
  virtual int getTexture() = 0;

  // Input
  virtual void inMouseDown(int xPxl, int yPxl) {};
  virtual void inMouseDrag(int xInit, int yInit, int xCur, int yCur) {};
  virtual void inMouseUp(int xPxl, int yPxl) {};

  // Position / movements
  CoordsMap getMapPos() { return m_MapPosition; };
  virtual void setMapPos(CoordsMap coords);
  void moveBy(CoordsMap coords);

  // Map related functions
  Map * getMap() { return m_pMap; };

  // Other
  virtual char * getInfo(char * sBuf, int iSize, InfoDest eDest) { return sBuf; };
  u8 getOwner() { return m_uOwner; };
  void setOwner(u8 uOwner);
  virtual void updateIdentifiers() = 0;

protected:
  Map * m_pMap;
  u8 m_uOwner;

private:
  CoordsMap m_MapPosition;
};

#endif
