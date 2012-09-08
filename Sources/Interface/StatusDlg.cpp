#include "StatusDlg.h"
#include "../LocalClient.h"
#include "../GUIClasses/guiLabel.h"
#include "../GUIClasses/guiFrame.h"
#include "../Interface/InterfaceManager.h"

// -----------------------------------------------------------------
// Name : StatusDlg
//  Constructor
// -----------------------------------------------------------------
StatusDlg::StatusDlg(LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;

    init("StatusDlg",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, 1, 1, pLocalClient->getDisplay());
}

// -----------------------------------------------------------------
// Name : ~StatusDlg
//  Destructor
// -----------------------------------------------------------------
StatusDlg::~StatusDlg()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy StatusDlg\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy StatusDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : showStatus
// -----------------------------------------------------------------
void StatusDlg::showStatus(char * sMessage)
{
    deleteAllComponents();

    guiLabel * pLbl = new guiLabel();
    pLbl->init(sMessage, TEXT_FONT, TEXT_COLOR, "", 0, 0, 400, 0, getDisplay());
    addComponent(pLbl);

    setDimensions(pLbl->getWidth(), pLbl->getHeight());
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->updateSizeFit();
    pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, 0);
    pFrm->setVisible(true);
}

// -----------------------------------------------------------------
// Name : hide
// -----------------------------------------------------------------
void StatusDlg::hide()
{
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->setVisible(false);
}
