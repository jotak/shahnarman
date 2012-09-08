#ifndef _SPELLSSELECTOR_DLG_H
#define _SPELLSSELECTOR_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiContainer;
class MapTile;

typedef void CLBK_ON_CANCEL();

class SpellsSelectorDlg : public guiDocument
{
public:
    SpellsSelectorDlg(LocalClient * pLocalClient, int iWidth, int iHeight);
    ~SpellsSelectorDlg();

    void showActiveSpellsOnTile(MapTile * pTile);
    void showPlayersSpells(ObjectList * pPlayers, int iSrc, CLBK_ON_CANCEL * pCancelCallback);  // iSrc : 0 = active, 1 = hand, 2 = deck, 3 = discard
    void hide();
    virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO)
    {
        *isLuaPlayerGO = 1;
        return (m_pTarget == NULL) ? NULL : m_pTarget->getAttachment();
    };
    virtual void setTargetValid(bool bValid);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);

protected:
    LocalClient * m_pLocalClient;
    guiContainer * m_pSpellsContainer;
    guiComponent * m_pTarget;
    CLBK_ON_CANCEL * m_pCancelCallback;
};

#endif
