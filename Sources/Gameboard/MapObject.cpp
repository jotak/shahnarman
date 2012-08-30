#include "MapObject.h"

// -----------------------------------------------------------------
// Name : MapObject
//  Constructor
// -----------------------------------------------------------------
MapObject::MapObject(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects, wchar_t * sIdentifiers) : GraphicObject(), LuaTargetable(pGlobalEffects, sIdentifiers)
{
  m_MapPosition = mapPos;
  m_uOwner = 0;
  m_pMap = pMap;
  m_pMap->addObject(this);
}

// -----------------------------------------------------------------
// Name : setMapPos
// -----------------------------------------------------------------
void MapObject::setMapPos(CoordsMap coords)
{
  m_pMap->removeObject(this);
  m_MapPosition = coords;
  m_pMap->addObject(this);
}

// -----------------------------------------------------------------
// Name : moveBy
// -----------------------------------------------------------------
void MapObject::moveBy(CoordsMap coords)
{
  m_pMap->removeObject(this);
  m_MapPosition += coords;
  m_pMap->addObject(this);
}

// -----------------------------------------------------------------
// Name : setOwner
// -----------------------------------------------------------------
void MapObject::setOwner(u8 uOwner)
{
  m_uOwner = uOwner;
  updateIdentifiers();
}
