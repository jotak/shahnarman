#ifndef _SELECT_PLAYER_AVATAR_H
#define _SELECT_PLAYER_AVATAR_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiComponent;
class guiContainer;
class guiPopup;
class Profile;

class SelectPlayerAvatarDlg : public guiDocument
{
public:
    SelectPlayerAvatarDlg(LocalClient * pLocalClient);
    ~SelectPlayerAvatarDlg();

    virtual void update(double delta);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    void onShow();
    bool onClickStart();

    // Misc
    Profile * getCurrentPlayer()
    {
        return m_pCurrentPlayer;
    };

protected:
    bool createPlayer(char * sName);
    void loadPlayer(char * sName);
    void onPlayerDataChanged();
    void unloadPlayer();
    void loadPlayersList(char * sSelect = NULL);

    LocalClient * m_pLocalClient;
    guiPopup * m_pTextInput;
    guiPopup * m_pConfirmDelete;
    Profile * m_pCurrentPlayer;
};

#endif
