#ifndef _MAPOBJECT_DLG_H
#define _MAPOBJECT_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class MapTile;
class MapObject;
class Unit;
class Town;
class guiPopup;
class Building;

class MapObjectDlg : public guiDocument
{
public:
    MapObjectDlg(LocalClient * pLocalClient, int iWidth, int iHeight);
    ~MapObjectDlg();

    virtual void update(double delta);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);
    bool onClickStart();

    // Misc.
    void setTile(MapTile * pTile, bool bLoadObject = true);
    void setSelectedObject(MapObject * pObj);
    static int getRemainingTurns(int cost, int delta, int stock);

protected:
    void loadObject(MapObject * pObj);
    void loadUnit(Unit * pUnit);
    void loadTown(Town * pTown);
    void raiseBuildingPopup(Building * pBuilding);

    LocalClient * m_pLocalClient;
    guiContainer * m_pListPanel;
    guiContainer * m_pContentPanel;
    guiComponent * m_pPointedBuilding;
    guiPopup * m_pBuildingPopup;
    Town * m_pCurrentTown;
};

#endif
