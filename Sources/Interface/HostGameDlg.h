#ifndef _HOSTGAME_DLG_H
#define _HOSTGAME_DLG_H

#include "../GUIClasses/guiDocument.h"

#define MAX_COLORS  16

class LocalClient;
class guiContainer;
class guiComboBox;
class guiButton;
class guiLabel;
class guiPopup;
class AvatarData;
class AIData;
class Profile;
class MapReader;

class HostGameDlg : public guiDocument
{
    enum PlayerType
    {
        LocalPlayer,
        AI,
        LAN,
        Closed
    };

    class PlayerData : public BaseObject
    {
    public:
        guiComboBox * m_pPlayerCmb;
        guiComboBox * m_pAvatarCmb;
        guiButton * m_pViewInfoBtn;
        guiButton * m_pColorBtn;
//    guiButton * m_pReadyBtn;
//    guiLabel * m_pReadyLbl;
        PlayerType m_Type;
        Profile * m_pSelectedPlayer;
        AvatarData * m_pSelectedAvatar;
        AIData * m_pSelectedAI;
        int m_iColor;
    };

public:
    HostGameDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
    ~HostGameDlg();

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    virtual void update(double delta);
    virtual void bringAbove(guiComponent * pCpnt);
    void onShow();
    void onHide();

private:
    void addPlayerRow();
    void onClose(PlayerData * pData);
    void onOpenPlayer(char * sName, PlayerData * pData);
    void onOpenAI(PlayerData * pData);
    void onOpenLAN(PlayerData * pData);
    void onSelectAvatar(AvatarData * pAvatar, PlayerData * pData);
    void onSelectAI(AIData * pAI, PlayerData * pData);
    void releaseAvatar(PlayerData * pData);
    void releaseAI(PlayerData * pData);
    void onSelectRow(PlayerData * pData);
    void onSelectMap(char * sMapId);
    bool checkIfAvatarAvailable(AvatarData * pAvatar, PlayerData * pExceptHere);
    void checkStartEnable();
    void checkGameNameWarning();
    void startGame();
    void _reallyStartGame();
    int getNextColor(int iColor);

    LocalClient * m_pLocalClient;
    MapReader * m_pMapReader;
    ObjectList * m_pMapParameters;
    guiContainer * m_pAllPlayersPanel;
    guiContainer * m_pPlayerPanel;
    guiContainer * m_pAvatarPanel;
    guiContainer * m_pMapOptionsPanel;
    guiPopup * m_pServerOptionsPopup;
    guiPopup * m_pGameNameWarningPopup;
    ObjectList * m_pAllPlayersData;
    int m_iNbRemoteClients;
    bool m_bIsLocalClient;
    F_RGBA m_AllColors[MAX_COLORS];
    float m_fStartGameTimer;
    ObjectList * m_pAvailableAIList;
    ObjectList * m_pSelectedAIList;
};

#endif
