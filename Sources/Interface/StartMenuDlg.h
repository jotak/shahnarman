#ifndef _STARTMENU_H
#define _STARTMENU_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiPopup;

class StartMenuDlg : public guiDocument
{
public:
    StartMenuDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
    ~StartMenuDlg();

    virtual void update(double delta);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

private:
    LocalClient * m_pLocalClient;
    guiPopup * m_pPlayLocalDlg;
};

#endif
