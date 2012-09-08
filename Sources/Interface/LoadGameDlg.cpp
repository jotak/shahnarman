#include "LoadGameDlg.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "../GUIClasses/guiButton.h"
#include "../GUIClasses/guiContainer.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"

#define MAX_SAVED_GAMES   1000
#define SPACING           4

// -----------------------------------------------------------------
// Name : LoadGameDlg
//  Constructor
// -----------------------------------------------------------------
LoadGameDlg::LoadGameDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;

    init("Load game",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, iWidth, iHeight, pLocalClient->getDisplay());

    // Create top label "Load game"
    int yPxl = 10;
    char str[64];
    guiLabel * pLbl = new guiLabel();
    pLbl->init(i18n->getText("LOAD_GAME", str, 64), TEXT_FONT, TEXT_COLOR, "LoadGameLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, yPxl);
    addComponent(pLbl);

    // Create button "Cancel"
    float fx = getWidth() / 8;
    guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText("CANCEL", str, 64), "CancelButton", pLocalClient->getDisplay());
    yPxl = getHeight() - (int)(1.25f * (float)pBtn->getHeight());
    pBtn->moveTo((int)fx, yPxl);
    pBtn->setWidth(2*fx);
    addComponent(pBtn);

    // Create button "Ok"
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(i18n->getText("OK", str, 64));
    pBtn->setId("OkButton");
    pBtn->moveBy((int)(4 * fx), 0);
    pBtn->setEnabled(false);
    addComponent(pBtn);

    // Create saved games list panel
    int bottom = yPxl - 10;
    yPxl = pLbl->getYPos() + pLbl->getHeight() + 5;
    m_pGamesPanel = guiContainer::createDefaultPanel(getWidth() / 2 - SPACING - SPACING / 2, bottom - yPxl, "GamesListPane", pLocalClient->getDisplay());
    m_pGamesPanel->moveTo(SPACING, yPxl);
    addComponent(m_pGamesPanel);

    // Create game info panel
    m_pGameInfoPanel = (guiContainer*) m_pGamesPanel->clone();
    m_pGameInfoPanel->setId("GameInfoPane");
    m_pGameInfoPanel->moveBy(m_pGamesPanel->getWidth() + SPACING, 0);
    m_pGameInfoPanel->setDocument((guiDocument*) m_pGamesPanel->getDocument()->clone());
    addComponent(m_pGameInfoPanel);

    loadGamesList();
}

// -----------------------------------------------------------------
// Name : ~LoadGameDlg
//  Destructor
// -----------------------------------------------------------------
LoadGameDlg::~LoadGameDlg()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy LoadGameDlg\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy LoadGameDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool LoadGameDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (strcmp(pCpnt->getId(), "CancelButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "OkButton") == 0)
    {
        m_pLocalClient->loadServer(m_sSelectedGameName);
        return false;
    }
    else if (strcmp(pCpnt->getId(), "SavedGameButton") == 0)
    {
        if (pEvent->eEvent == Event_DoubleClick)
        {
            m_pLocalClient->loadServer(m_sSelectedGameName);
            return false;
        }
        else
            loadGameInfo(((guiButton*)pCpnt)->getText());
    }
    return true;
}

// -----------------------------------------------------------------
// Name : loadGamesList
// -----------------------------------------------------------------
void LoadGameDlg::loadGamesList()
{
    char ** pSavedGameFiles;
    pSavedGameFiles = new char*[MAX_SAVED_GAMES];
    for (int i = 0; i < MAX_SAVED_GAMES; i++)
        pSavedGameFiles[i] = new char[MAX_PATH];
    int count = getSavedGames(pSavedGameFiles, MAX_SAVED_GAMES, MAX_PATH);
    int yPxl = 0;
    for (int i = 0; i < count; i++)
    {
        char sName[64];
        wsafecpy(sName, 64, pSavedGameFiles[i]);
        assert(strlen(sName) > 4);
        sName[strlen(sName) - 4] = '\0'; // remove ".sav" extension
        guiButton * pBtn = guiButton::createDefaultSmallButton(sName, m_pGamesPanel->getInnerWidth(), "SavedGameButton", getDisplay());
        pBtn->setOwner(this); // catch button events
        pBtn->moveTo(0, yPxl);
        pBtn->setCatchDoubleClicks(true);
        m_pGamesPanel->getDocument()->addComponent(pBtn);
        yPxl += pBtn->getHeight();
    }
    for (int i = 0; i < MAX_SAVED_GAMES; i++)
        delete[] pSavedGameFiles[i];
    delete[] pSavedGameFiles;
}

// -----------------------------------------------------------------
// Name : loadGameInfo
// -----------------------------------------------------------------
void LoadGameDlg::loadGameInfo(char * sGameName)
{
    wsafecpy(m_sSelectedGameName, MAX_PATH, sGameName);
    m_pGameInfoPanel->getDocument()->deleteAllComponents();

    char sFileName[MAX_PATH];
    snprintf(sFileName, MAX_PATH, "%s.sav", sGameName);

    int iDocWidth = m_pGameInfoPanel->getInnerWidth();

    // Write title (Game name)
    int yPxl = 5;
    guiLabel * pLbl = new guiLabel();
    pLbl->init(sGameName, H2_FONT, H2_COLOR, "TitleLabe", 0, 0, 0, 0, getDisplay());
    pLbl->moveTo((iDocWidth - pLbl->getWidth()) / 2, yPxl);
    m_pGameInfoPanel->getDocument()->addComponent(pLbl);

    getComponent("OkButton")->setEnabled(true);
}
