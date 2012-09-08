#ifndef _GAMEBOARDMANAGER_H
#define _GAMEBOARDMANAGER_H

#include "Map.h"
#include "MapCursor.h"

#define MAPCURSOR_SELECTION     0
#define MAPCURSOR_MOVETARGET    1
#define MAPCURSOR_MOVETURNPOS   2
#define MAPCURSOR_SPECIALTARGET 3
#define MAPCURSOR_WRONGACTION   4
#define NB_MAPCURSORS           5

enum GameboardMode
{
    GM_Orders,
    GM_SelectBattle
};

class GameboardInputs;
class GeometryQuads;
class Unit;
class LocalClient;
class Player;

class GameboardManager
{
public:
    // Constructor / destructor
    GameboardManager(LocalClient * pLocalClient);
    ~GameboardManager();

    // Manager functions
    void Init();
    void Update(double delta);
    void Display();

    // Member access
    Map * getMap()
    {
        return &m_Map;
    };
    GameboardInputs * getInputs()
    {
        return m_pInputs;
    };
    int getBattleTexture()
    {
        return m_pBattleIcon->getTexture();
    };
    MapCursor * getMapCursor(int iCursor)
    {
        return &(m_AllCursors[iCursor]);
    };

    // Unit functions
    Unit * getFoeAt(CoordsMap position, u8 unitOwnerId, bool * bMultipleTargets);
    bool setUnitMoveOrder(Unit * pUnit, CoordsMap mapPos);
    char setUnitAttackOrder(Unit * pUnit, CoordsMap mapPos);
    bool setCurrentUnitAttackOrder(Unit * pTarget);
    void updateUnitOrder(Unit * pUnit);

    // Other
    void displayBackground();
    void disableNoPlayer();
    void enableNextPlayer(Player * pPlayer);
    void selectMapObject(MapObject * mapObj, bool bUpdateStack = true);
    bool isCurrentTargetValid()
    {
        return m_AllCursors[MAPCURSOR_MOVETARGET].isEnabled();
    };
    void updateTownsData(NetworkData * pData);
    void updateTilesInfluence(NetworkData * pData);
    void unsetTargetMode();
    ObjectList * getMagicCircles()
    {
        return m_pMagicCircles;
    };
    void highlightMagicCirclesForPlayer(Player * pPlayer);

    // Select battles mode
    void setBattleMode();
    void unsetBattleMode();

private:
    void updateUnitTargetCursors(Unit * pUnit);

    LocalClient * m_pLocalClient;
    Map m_Map;                    // Map object
    GameboardInputs * m_pInputs;  // Gameboard inputs
    GameboardMode m_Mode;
    MapCursor m_AllCursors[NB_MAPCURSORS];
    ObjectList * m_pMagicCircles;
    Player * m_pHighlightedPlayerCircles;

    // Geometry objects and textures
    GeometryQuads * m_pBackgroundGeometry;
    GeometryQuads * m_pTerraIncognitaGeometry;
    GeometryQuads * m_pPathDot;
    GeometryQuads * m_pBattleIcon;
    GeometryQuads * m_pMagicCircleGeometry;
};

#endif
