#ifndef _GAMEOVER_DLG_H
#define _GAMEOVER_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class Player;

class GameOverDlg : public guiDocument
{
public:
    GameOverDlg(LocalClient * pLocalClient, int iWidth, int iHeight);
    ~GameOverDlg();

    void onShow();
    void setWinners(ObjectList * pWinners);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

protected:
    int addPlayerGains(Player * pPlayer, int yPxl);

    LocalClient * m_pLocalClient;
    guiContainer * m_pGainsPanel;
    guiContainer * m_pStatsPanel;
    ObjectList * m_pWinners;
};

#endif
