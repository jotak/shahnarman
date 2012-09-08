#ifndef _GAMEBOARDINPUTS_H
#define _GAMEBOARDINPUTS_H

#include "../Input/EventListener.h"

class GraphicObject;
class MapObject;

typedef void CLBK_CUSTOM_TARGET_ON_MOUSE_OVER(CoordsMap);
typedef void CLBK_CUSTOM_TARGET_ON_CLICK(CoordsMap);

enum GameboardMouseMode
{
    ModeNormal = 0,
    ModeAttackTarget,
    ModeMoveToTarget,
    ModeSelectCustomTarget
};

class GameboardManager;
class LocalClient;
class Unit;

class GameboardInputs : public EventListener
{
public:
    // Constructor / destructor
    GameboardInputs(GameboardManager * pGameboard, LocalClient * pLocalClient);
    ~GameboardInputs();

    // Common functions
    void init();
    void update(double delta);

    // Input functions
    bool onCatchButtonEvent(ButtonAction * pEvent);
    bool onCursorMoveEvent(int xPxl, int yPxl);

    // Mouse mode
    void setMouseMode(GameboardMouseMode mouseMode, CLBK_CUSTOM_TARGET_ON_MOUSE_OVER * pCallback1 = NULL, CLBK_CUSTOM_TARGET_ON_CLICK * pCallback2 = NULL);
    GameboardMouseMode getMouseMode()
    {
        return m_MouseMode;
    };

    // Member access functions
    MapObject * getSelectedMapObject()
    {
        return m_pSelectedMapObj;
    };
    void setSelectedMapObject(MapObject * mapObj)
    {
        m_pSelectedMapObj = mapObj;
    };

    // Other functions
    Unit * getActiveSelectedUnit();
    void onMoveOrAttackResponse(bool bCancel, Unit * pTarget);

private:
    // Private input
    bool onButton1Down(int xPxl, int yPxl);
    bool onButton1Drag(int xPxl, int yPxl);
    bool onButton1Up(int xPxl, int yPxl);
    bool onButton1Click(int xPxl, int yPxl);
    bool onButton1DoubleClick(int xPxl, int yPxl);

    GameboardManager * m_pGameboard;
    LocalClient * m_pLocalClient;

    CoordsMap m_PreviousPointedTile;  // Some calculations are skipped when current pointed tile is same as previous

    // Dragging map object
    CoordsMap m_DragToPosition;
    CoordsMap m_DragNextTurnPosition;
    bool m_bIsDraggingSelection;
    CLBK_CUSTOM_TARGET_ON_MOUSE_OVER * m_pCustomTargetOnMouseOverClbk;
    CLBK_CUSTOM_TARGET_ON_CLICK * m_pCustomTargetOnClickClbk;

    // Mouse modes
    GameboardMouseMode m_MouseMode;

    // Selected map object
    MapObject * m_pSelectedMapObj;
    MapObject * m_pPreSelectedMapObj;
    float m_fMoveOrAttackColorTimer;
};

#endif
