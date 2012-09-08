#ifndef _MAPTILE_H
#define _MAPTILE_H

#include "../Common/GraphicObject.h"
#include "../Data/LuaTargetable.h"

#define TERRAIN_UNSET       0
#define TERRAIN_PLAIN       1
#define TERRAIN_FOREST      2
#define TERRAIN_MOUNTAIN    3
#define TERRAIN_TOUNDRA     4
#define TERRAIN_DESERT      5
#define TERRAIN_SEA         6
#define IS_VALID_TERRAIN(t) (t>=TERRAIN_PLAIN && t<=TERRAIN_SEA)

#define MASK_NONE           0
#define MASK_NORTH          1
#define MASK_EAST           2
#define MASK_SOUTH          4
#define MASK_WEST           8
#define MASK_CORNER_NE      16
#define MASK_CORNER_SE      32
#define MASK_CORNER_SW      64
#define MASK_CORNER_NW      128

#define STRING_FOOD         "food"
#define STRING_PROD         "production"

#define TERRAIN_NAMES       { "unset", "plain", "forest", "mountain", "toundra", "desert", "sea" }
#define LTERRAIN_NAMES      { "unset", "plain", "forest", "mountain", "toundra", "desert", "sea" }

class MapObject;
class SpecialTile;
class GeometryQuads;
class GeometryText;

class MapTile : public GraphicObject, public LuaTargetable
{
public:
    MapTile(u8 uTerrainType, ObjectList ** pGlobalEffects);
    ~MapTile();

    virtual u32 getType()
    {
        return GraphicObject::getType() | GOTYPE_MAPTILE;
    };

    void initGraphics(Geometry * pTileGeo, DisplayEngine * pDisplay);
    void resetTexture(DisplayEngine * pDisplay);
    void display(CoordsMap mapPos);
    char * getInfo(char * sBuf, int iSize);
    int getTexture()
    {
        return m_iTexture;
    };
    MapObject * getFirstMapObject(u32 uType = GOTYPE_MAPOBJECT, int _it = 0);
    MapObject * getNextMapObject(u32 uType = GOTYPE_MAPOBJECT, int _it = 0);
    void setMask(u16 uMask);
    DisplayEngine * getDisplay()
    {
        if (m_pGeometryPtr != NULL) return m_pGeometryPtr->getDisplay();
        else return NULL;
    };
    Geometry * getGeometry()
    {
        return m_pGeometryPtr;
    };
    void hideSpecialTile();

    u8 m_uTerrainType;
    u8 m_uInfluence;
    ObjectList * m_pMapObjects;
    SpecialTile * m_pSpecialTile;
    GeometryText * m_pNbAlliesGeo;
    GeometryText * m_pNbFoesGeo;

private:
    int m_iMaskTexture;
    GeometryQuads * m_pSpecialTileGeometry;
    Geometry * m_pGeometryPtr;
    int m_iTexture;
};

#endif
