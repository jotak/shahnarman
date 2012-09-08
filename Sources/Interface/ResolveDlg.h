#ifndef _GUI_RESOLVEDLG_H
#define _GUI_RESOLVEDLG_H

#include "../GUIClasses/guiDocument.h"

enum ResolveBattleStep
{
    RBS_Init,
    RBS_Passive,
    RBS_ChooseBattle,
    RBS_ChooseUnits,
    RBS_AskCastBattleSpells,
    RBS_SelectSpellTarget
};

class LocalClient;
class NetworkData;
class guiImage;
class guiContainer;
class Unit;
class Player;
class BattleSpellPopup;
class guiComponent;
class Spell;
class ChildEffect;
class LuaObject;

class ResolveDlg : public guiDocument
{
public:
    // Constructor / destructor
    ResolveDlg(int iWidth, LocalClient * pLocalClient);
    ~ResolveDlg();

    // Inherited functions
    void update(double delta);

    // Events handlers
    guiObject * onButtonEvent(ButtonAction * pEvent);
    virtual guiObject * onCursorMoveEvent(int xPxl, int yPxl);
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    void onMessage(int iMessage, NetworkData * pData);
    virtual BaseObject * getTargetedObject(u8 * isLuaPlayerGO)
    {
        *isLuaPlayerGO = 3;
        return (m_pTarget == NULL) ? NULL : m_pTarget->getAttachment();
    };
    virtual void setTargetValid(bool bValid);
    bool onClickStart();

    // Show / hide components
    void setSimpleStatusScreen();
    void setChooseUnitsScreen();
    void setChooseBattleScreen();
    void setCurrentBattleScreen();

    // Battles
    int getNumberOfBattles()
    {
        return m_iNbBattles;
    };
    CoordsMap getBattlePosition(int idx)
    {
        return m_pBattlesPositions[idx];
    };

    // Specific functions
    void setStatus(char * sStatus);
    void resolveTargetSelectionFinished(bool bCanceled, LuaObject * pLua, ChildEffect * pChild = NULL, Unit * pUnit = NULL);

protected:
    int initSimpleStatusScreen();
    int initChooseUnitsScreen(int yPos);
    void initChooseBattleScreen(int yPos, int boxHeight);
    void updateChooseBattlePanel(NetworkData * pData);
    void updateChooseUnitsPanel(NetworkData * pData);
    guiImage * addUnitToFlowPanel(guiContainer * pPanel, Unit * pUnit, int iOverTex = -1);
    void askNextPlayerCastSpells();
    void mouseOverPanel(guiContainer * pPanel, int xPxl, int yPxl);
    void selectComponent(guiImage * pSelector, guiComponent * pCpnt, guiContainer * pPanel = NULL, int iIndex = 0);
    void needSelectTarget(Player * pPlayer, Spell * pSpell, u8 uType, u32 uConstraints);
    void updateBattlePanelsFromData();

    ResolveBattleStep m_BattleStep;
    int m_iTimerConcerned;
    int m_iTimerNotConcerned;
    Player * m_pAttacker;
    MetaObjectList * m_pAttackingUnit;
    Unit * m_pDefendingUnit;
    int m_iPlayerIt;
    BattleSpellPopup * m_pSpellsPopup;
    bool * m_pCastSpellForThisBattle;
    NetworkData * m_pSpellData;
    guiComponent * m_pTarget;
    ObjectList * m_pAllowedDefendersPerAttacker;

    // Battles list
    CoordsMap * m_pBattlesPositions;
    int m_iNbBattles;
    int m_iSelectedBattle;

    // Player selection
    u8 * m_pPlayers;
    int m_iNbPlayers;
    int m_iSelectTexture;

    // Local client
    LocalClient * m_pLocalClient;

    // Containers and gui data
    guiContainer * m_pAttackersPanel;
    guiContainer * m_pDefendersPanel;
    guiContainer * m_pBattlesListPanel;
    guiImage * m_pBattleSelectorImg;
    guiImage * m_pAttackerSelectorImg;
    guiImage * m_pDefenderSelectorImg;
    int m_iMaxHeight;

    // Other
    int m_iRangeAttackTex;
    char m_sSelectTargetLuaCallback[128];
};

#endif
