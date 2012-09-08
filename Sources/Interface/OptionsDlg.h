#ifndef _OPTIONS_DLG_H
#define _OPTIONS_DLG_H

#include "../GUIClasses/guiDocument.h"

class LocalClient;
class guiPopup;

class OptionsDlg : public guiDocument
{
public:
    OptionsDlg(int iWidth, int iHeight, LocalClient * pLocalClient);
    ~OptionsDlg();

    virtual void update(double delta);

    // Handlers
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

private:
    void onAcceptGameParameters();
    void onAcceptVideoParameters();
    void onAcceptAudioParameters();

    LocalClient * m_pLocalClient;
    guiPopup * m_pGameOptionsDlg;
    guiPopup * m_pVideoOptionsDlg;
    guiPopup * m_pAudioOptionsDlg;
    guiPopup * m_pConfirmVideoModeDlg;
    float m_fConfirmVideoModeTimer;
    int m_iSelectedLanguage;
};

#endif
