#include "HostGameDlg.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiPopup.h"
#include "../GUIClasses/guiEditBox.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../DeckData/Profile.h"
#include "../DeckData/AvatarData.h"
#include "../DeckData/Edition.h"
#include "../DeckData/AIData.h"
#include "../DeckData/ShopItem.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "../Debug/DebugManager.h"
#include "../Data/DataFactory.h"
#include "../Players/Player.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../Server/Server.h"
#include "../Server/TurnSolver.h"
#include "../Server/MapReader.h"
#include "../Gameboard/Unit.h"

#define SPACING    4

#define TURN_TIMER_VALUES     {                 -1,             3600,                 2400,                 1800,                 1200,                  600,                 300,                 120,                 60,                   30,                   15 };
#define NB_TURN_TIMER_VALUES  11

// -----------------------------------------------------------------
// Name : HostGameDlg
//  Constructor
// -----------------------------------------------------------------
HostGameDlg::HostGameDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_pAllPlayersData = new ObjectList(true);
    m_iNbRemoteClients = 0;
    m_bIsLocalClient = false;
    m_pGameNameWarningPopup = NULL;
    m_pMapReader = new MapReader(m_pLocalClient);
    m_pMapParameters = new ObjectList(true);
    m_fStartGameTimer = -1;
    m_pAvailableAIList = new ObjectList(false);
    m_pSelectedAIList = new ObjectList(false);

    int i = 0;
    m_AllColors[i++] = rgb(1, 0, 0);
    m_AllColors[i++] = rgb(0, 1, 0);
    m_AllColors[i++] = rgb(0, 0, 1);
    m_AllColors[i++] = rgb(1, 1, 0);
    m_AllColors[i++] = rgb(0, 1, 1);
    m_AllColors[i++] = rgb(1, 0, 1);
    m_AllColors[i++] = rgb(0, 0.369, 0.125);
    m_AllColors[i++] = rgb(0.459, 0.298, 0.141);
    m_AllColors[i++] = rgb(0.949, 0.396, 0.133);
    m_AllColors[i++] = rgb(1, 1, 1);
    m_AllColors[i++] = rgb(0, 0, 0);
    m_AllColors[i++] = rgb(0.51, 0.478, 0);
    m_AllColors[i++] = rgb(0.49, 0.655, 0.847);
    m_AllColors[i++] = rgb(0.961, 0.596, 0.616);
    m_AllColors[i++] = rgb(0.635, 0.827, 0.612);
    m_AllColors[i++] = rgb(0.4, 0.176, 0.569);

    init("Host game",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, iWidth, iHeight, pLocalClient->getDisplay());

    // Create label "Game name"
    int xPxl = SPACING;
    int yPxl = SPACING;
    char str[128];
    guiLabel * pLbl = new guiLabel();
    pLbl->init(i18n->getText("GAME_NAME", str, 128), H2_FONT, H2_COLOR, "GameNameLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo(SPACING, yPxl);
    addComponent(pLbl);

    // Create game name input zone
    xPxl += pLbl->getWidth() + SPACING;
    yPxl += 2;
    if (m_pLocalClient->getInput()->hasKeyboard())
    {
        guiEditBox * pEdit = guiEditBox::createDefaultEditBox(1, false, getWidth() - xPxl - SPACING, "GameName", (KeyboardInputEngine*) m_pLocalClient->getInput(), getDisplay());
        pEdit->moveTo(xPxl, yPxl);
        addComponent(pEdit);
        yPxl += pEdit->getHeight() + SPACING;
        pEdit->setText("No name");
    }
    else
    {
        // TODO : no keyboard
    }

    // Create players list panel
    m_pAllPlayersPanel = guiContainer::createDefaultPanel(getWidth() - 2*SPACING, getHeight() / 3 - 2 * SPACING, "PlayerPane", pLocalClient->getDisplay());
    m_pAllPlayersPanel->moveTo(SPACING, yPxl);
    addComponent(m_pAllPlayersPanel);

    // Create player panel
    yPxl += m_pAllPlayersPanel->getHeight() + 10;
    m_pPlayerPanel = guiContainer::createDefaultPanel(getWidth() / 2 - SPACING - SPACING / 2, 110, "ChatPane", pLocalClient->getDisplay());
    m_pPlayerPanel->moveTo(SPACING, yPxl);
    addComponent(m_pPlayerPanel);

    // Create avatar panel
    m_pAvatarPanel = (guiContainer*) m_pPlayerPanel->clone();
    m_pAvatarPanel->setId("InfoPane");
    m_pAvatarPanel->moveBy(m_pAvatarPanel->getWidth() + SPACING, 0);
    m_pAvatarPanel->setDocument((guiDocument*) m_pPlayerPanel->getDocument()->clone());
    addComponent(m_pAvatarPanel);

    // Server & map options
    char sText[LABEL_MAX_CHARS];
    char sDefault[LABEL_MAX_CHARS];
    i18n->getText("DEFAULT", sDefault, LABEL_MAX_CHARS);

    // Map type label
    yPxl += m_pAvatarPanel->getHeight() + SPACING;
    i18n->getText("MAP_TYPE", sText, LABEL_MAX_CHARS);
    pLbl = new guiLabel();
    pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, -1, -1, pLocalClient->getDisplay());
    xPxl = max(0, getWidth() / 2 - pLbl->getWidth() / 2);
    pLbl->moveTo(xPxl, yPxl);
    addComponent(pLbl);

    // Map type combo
    yPxl += pLbl->getHeight() + SPACING;
    guiComboBox * pCombo = guiComboBox::createDefaultComboBox("MapTypeCombo", pLocalClient->getInterface(), pLocalClient->getDisplay());
    xPxl = max(0, getWidth() / 2 - pCombo->getWidth() / 2);
    pCombo->moveTo(xPxl, yPxl);
    addComponent(pCombo);

    // Fill map types combo
    char sFirstMap[MAX_PATH];
    char ** sMapsList;
    sMapsList = new char*[1000];
    for (int i = 0; i < 1000; i++)
        sMapsList[i] = new char[MAX_PATH];
    int nbMaps = getMaps(sMapsList, 1000, MAX_PATH);
    for (int i = 0; i < nbMaps; i++)
    {
        if (m_pMapReader->init(sMapsList[i]))
        {
            char sName[NAME_MAX_CHARS];
            if (m_pMapReader->getMapName(sName, NAME_MAX_CHARS))
                pCombo->addString(sName, sMapsList[i]);
        }
    }
    pCombo->setItem(0);
    wsafecpy(sFirstMap, MAX_PATH, sMapsList[0]);
    for (int i = 0; i < 1000; i++)
        delete[] sMapsList[i];
    delete[] sMapsList;

    // Create button "back"
    guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText("BACK", str, 128), "BackButton", pLocalClient->getDisplay());
    pBtn->setDimensions(getWidth() / 2 - 5, 45);
    yPxl = getHeight() - pBtn->getHeight() - 4;
    pBtn->moveTo(2, yPxl);
    addComponent(pBtn);

    // Create button "next"
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(i18n->getText("START", str, 128));
    pBtn->setId("NextButton");
    pBtn->moveTo(getWidth() - pBtn->getWidth() - 2, yPxl);
    pBtn->setEnabled(false);
    addComponent(pBtn);
    pBtn->setTooltipText(i18n->getText("NEED_TWO_PLAYERS", str, 128));

    // Create map options panel
    yPxl = pCombo->getYPos() + pCombo->getHeight() + SPACING;
    iHeight = pBtn->getYPos() - SPACING - yPxl;
    m_pMapOptionsPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, iHeight, "MapOptionsPane", pLocalClient->getDisplay());
    m_pMapOptionsPanel->moveTo(SPACING, yPxl);
    addComponent(m_pMapOptionsPanel);

    // Create server options popup
    m_pServerOptionsPopup = guiPopup::createEmptyPopup(pLocalClient->getDisplay());
    m_pServerOptionsPopup->setWidthFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pServerOptionsPopup->setHeightFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pServerOptionsPopup->setDimensions(250, 250);
    m_pServerOptionsPopup->setVisible(false);
    m_pLocalClient->getInterface()->registerFrame(m_pServerOptionsPopup);
    guiDocument * pDoc = m_pServerOptionsPopup->getDocument();

    // Create label "Server options"
    yPxl = SPACING;
    pLbl = new guiLabel();
    pLbl->init(i18n->getText("SERVER_OPTIONS", str, 128), H2_FONT, H2_COLOR, "", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo(m_pServerOptionsPopup->getInnerWidth() / 2 - pLbl->getWidth() / 2, yPxl);
    pDoc->addComponent(pLbl);

    // Create label "Turn timer"
    yPxl += pLbl->getHeight() + 2 * SPACING;
    pLbl = new guiLabel();
    pLbl->init(i18n->getText("TURN_TIMER", str, 128), H2_FONT, H2_COLOR, "", SPACING, yPxl, 0, 0, pLocalClient->getDisplay());
    pDoc->addComponent(pLbl);

    // Turn timer combo
    yPxl += pLbl->getHeight() + SPACING;
    pCombo = guiComboBox::createDefaultComboBox("TurnTimerCombo", pLocalClient->getInterface(), pLocalClient->getDisplay());
    pCombo->moveTo(SPACING, yPxl);
    pDoc->addComponent(pCombo);

    // Fill turn timer combo
    int iValues[NB_TURN_TIMER_VALUES] = TURN_TIMER_VALUES;
    for (int i = 0; i < NB_TURN_TIMER_VALUES; i++)
    {
        if (iValues[i] < 0)
            pCombo->addString(i18n->getText("INFINITE(TIMER)", str, 128), "TurnTimerComboItem");
        else if (iValues[i] <= 1)
        {
            snprintf(sText, LABEL_MAX_CHARS, "%d %s", iValues[i], i18n->getTextLow("SECOND", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
        else if (iValues[i] < 60)
        {
            snprintf(sText, LABEL_MAX_CHARS, "%d %s", iValues[i], i18n->getTextLow("SECONDS", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
        else if (iValues[i] == 60)
        {
            snprintf(sText, LABEL_MAX_CHARS, "1 %s", i18n->getTextLow("MINUTE", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
        else if (iValues[i] < 3600)
        {
            snprintf(sText, LABEL_MAX_CHARS, "%d %s", iValues[i] / 60, i18n->getTextLow("MINUTES", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
        else if (iValues[i] == 3600)
        {
            snprintf(sText, LABEL_MAX_CHARS, "1 %s", i18n->getTextLow("HOUR", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
        else
        {
            snprintf(sText, LABEL_MAX_CHARS, "%d %s", iValues[i] / 3600, i18n->getTextLow("HOURS", str, 128));
            pCombo->addString(sText, "TurnTimerComboItem");
        }
    }
    pCombo->setItem(0);

    // Create label "Deck max size"
    yPxl += pCombo->getHeight() + 2 * SPACING;
    pLbl = new guiLabel();
    pLbl->init(i18n->getText("DECK_MAX_SIZE", str, 128), H2_FONT, H2_COLOR, "", SPACING, yPxl, 0, 0, pLocalClient->getDisplay());
    pDoc->addComponent(pLbl);

    // Deck size combo
    yPxl += pLbl->getHeight() + SPACING;
    pCombo = guiComboBox::createDefaultComboBox("DeckSizeCombo", pLocalClient->getInterface(), pLocalClient->getDisplay());
    pCombo->moveTo(SPACING, yPxl);
    pCombo->setOwner(this);
    pDoc->addComponent(pCombo);

    // Fill deck size combo
    pCombo->addString(i18n->getText("UNLIMITED(DECK_SIZE)", str, 128), "DeckSizeComboItem");
    for (int i = 1; i <= 10; i++)
    {
        snprintf(str, 128, "%d", (int) i * 10);
        pCombo->addString(str, "DeckSizeComboItem");
    }
    pCombo->setItem(0);

    // Fill available AI list
    Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
    while (pEdition != NULL)
    {
        AIData * pAI = (AIData*) pEdition->getAIs()->getFirst(0);
        while (pAI != NULL)
        {
            m_pAvailableAIList->addLast(pAI);
            pAI = (AIData*) pEdition->getAIs()->getNext(0);
        }
        pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
    }

    onSelectMap(sFirstMap);
}

// -----------------------------------------------------------------
// Name : ~HostGameDlg
//  Destructor
// -----------------------------------------------------------------
HostGameDlg::~HostGameDlg()
{
    delete m_pAllPlayersData;
    delete m_pMapReader;
    MapReader::deleteMapParameters(m_pMapParameters);
    delete m_pMapParameters;
    delete m_pAvailableAIList;
    delete m_pSelectedAIList;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void HostGameDlg::update(double delta)
{
    guiDocument::update(delta);
    if (m_fStartGameTimer > 0)
    {
        m_fStartGameTimer -= delta;
        if (m_fStartGameTimer < 0)
            _reallyStartGame();
    }
    if (m_pGameNameWarningPopup != NULL)
    {
        guiComponent * pCpnt = m_pGameNameWarningPopup->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "YesButton") == 0)
            {
                // Remove frame
                m_pLocalClient->getInterface()->deleteFrame(m_pGameNameWarningPopup);
                m_pGameNameWarningPopup = NULL;
                setEnabled(true);
                // Delete file
                guiEditBox * pEdit = (guiEditBox*) getComponent("GameName");
                char sFilePath[MAX_PATH];
                snprintf(sFilePath, MAX_PATH, "%s%s.sav", SAVES_PATH, pEdit->getText());
                remove(sFilePath);
                // Start game
                startGame();
            }
            else if (strcmp(pCpnt->getId(), "NoButton") == 0)
            {
                m_pLocalClient->getInterface()->deleteFrame(m_pGameNameWarningPopup);
                m_pGameNameWarningPopup = NULL;
                setEnabled(true);
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void HostGameDlg::onShow()
{
    // Remove data
    m_pAllPlayersPanel->getDocument()->deleteAllComponents();
    m_pPlayerPanel->getDocument()->deleteAllComponents();
    m_pAvatarPanel->getDocument()->deleteAllComponents();
    m_pAllPlayersData->deleteAll();
    m_iNbRemoteClients = 0;
    m_bIsLocalClient = false;

    addPlayerRow();

    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    assert(pFrm != NULL);
    m_pServerOptionsPopup->moveTo(pFrm->getXPos() + pFrm->getWidth() + 1, pFrm->getYPos());
    m_pServerOptionsPopup->setVisible(true);
}

// -----------------------------------------------------------------
// Name : onHide
// -----------------------------------------------------------------
void HostGameDlg::onHide()
{
    m_pServerOptionsPopup->setVisible(false);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool HostGameDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (strcmp(pCpnt->getId(), "BackButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "NextButton") == 0)
    {
        checkGameNameWarning();
        return false;
    }
    else if (strcmp(pCpnt->getId(), "LoadPlayer") == 0)
    {
        PlayerData * pData = (PlayerData*) ((guiComboBox*)(pCpnt->getOwner()))->getAttachment();
        assert(pData != NULL);
        onOpenPlayer(((guiComboBox*)(pCpnt->getOwner()))->getText(), pData);
        if (pData == m_pAllPlayersData->getLast(0)) // Last line
            addPlayerRow();
    }
    else if (strcmp(pCpnt->getId(), "LANPlayer") == 0)
    {
        PlayerData * pData = (PlayerData*) ((guiComboBox*)(pCpnt->getOwner()))->getAttachment();
        assert(pData != NULL);
        onOpenLAN(pData);
        if (pData == m_pAllPlayersData->getLast(0)) // Last line
            addPlayerRow();
    }
    else if (strcmp(pCpnt->getId(), "AI") == 0)
    {
        PlayerData * pData = (PlayerData*) ((guiComboBox*)(pCpnt->getOwner()))->getAttachment();
        assert(pData != NULL);
        onOpenAI(pData);
        if (pData == m_pAllPlayersData->getLast(0)) // Last line
            addPlayerRow();
    }
    else if (strcmp(pCpnt->getId(), "Closed") == 0)
    {
        PlayerData * pData = (PlayerData*) ((guiComboBox*)(pCpnt->getOwner()))->getAttachment();
        assert(pData != NULL);
        if (pData != m_pAllPlayersData->getLast(0)) // Closed not last : remove row
        {
            onClose(pData);
            return false;
        }
    }
    else if (strcmp(pCpnt->getId(), "ViewButton") == 0)
    {
        PlayerData * pData = (PlayerData*) pCpnt->getAttachment();
        assert(pData != NULL);
        onSelectRow(pData);
    }
    else if (strcmp(pCpnt->getId(), "ColorButton") == 0)
    {
        PlayerData * pData = (PlayerData*) pCpnt->getAttachment();
        assert(pData != NULL);
        int iColor = getNextColor(pData->m_iColor);
        if (iColor >= 0)
        {
            pData->m_iColor = iColor;
            pCpnt->setDiffuseColor(m_AllColors[iColor]);
        }
    }
    else
    {
        // Check if it's Map Type combo
        guiComboBox * pCombo = (guiComboBox*) getComponent("MapTypeCombo");
        if (pCpnt->getOwner() == pCombo)
            this->onSelectMap(pCpnt->getId());

        ComponentOwnerInterface * pOwner = pCpnt->getOwner();
        PlayerData * pData = (PlayerData*) m_pAllPlayersData->getFirst(0);
        while (pData != NULL)
        {
            // Clicked on avatar in list?
            if (pOwner == pData->m_pAvatarCmb)
            {
                // Avatar combo can hold either avatars or AIs
                // If previous avatar or AI was selected, release it for other combos
                // Then select new one
                if (pData->m_Type == LocalPlayer)
                {
                    if (pData->m_pSelectedAvatar != NULL)
                        releaseAvatar(pData);
                    AvatarData * pAvatar = (AvatarData*) pCpnt->getAttachment();
                    assert(pAvatar != NULL);
                    onSelectAvatar(pAvatar, pData);
                    break;
                }
                else if (pData->m_Type == AI)
                {
                    if (pData->m_pSelectedAI != NULL)
                        releaseAI(pData);
                    AIData * pAI = (AIData*) pCpnt->getAttachment();
                    assert(pAI != NULL);
                    onSelectAI(pAI, pData);
                    break;
                }
            }
            pData = (PlayerData*) m_pAllPlayersData->getNext(0);
        }
    }
    return true;
}

// -----------------------------------------------------------------
// Name : addPlayerRow
// -----------------------------------------------------------------
void HostGameDlg::addPlayerRow()
{
    // Create new PlayerData object
    PlayerData * pData = new PlayerData();
    pData->m_Type = Closed;
    pData->m_pSelectedAvatar = NULL;
    pData->m_pSelectedPlayer = NULL;
    pData->m_pSelectedAI = NULL;
    m_pAllPlayersData->addLast(pData);

    // Create "view info" button
    char sText[LABEL_MAX_CHARS];
    int xPxl = SPACING;
    int iTex = getDisplay()->getTextureEngine()->loadTexture("eye", false, 0, 25, 0, 16);
    pData->m_pViewInfoBtn = guiButton::createDefaultImageButton(iTex, "ViewButton", getDisplay());
    pData->m_pViewInfoBtn->moveTo(xPxl, 0);
    pData->m_pViewInfoBtn->setEnabled(false);
    pData->m_pViewInfoBtn->setAttachment(pData);
    pData->m_pViewInfoBtn->setOwner(this);  // catch events (instead of panel)
    pData->m_pViewInfoBtn->setTooltipText(i18n->getText("SHOW_INFORMATION", sText, LABEL_MAX_CHARS));
    m_pAllPlayersPanel->getDocument()->addComponent(pData->m_pViewInfoBtn);

    // Create combo box for listing players
    xPxl += pData->m_pViewInfoBtn->getWidth() + 10;
    pData->m_pPlayerCmb = guiComboBox::createDefaultComboBox("ChoosePlayer", m_pLocalClient->getInterface(), getDisplay());
    pData->m_pPlayerCmb->moveTo(xPxl, 0);
    pData->m_pPlayerCmb->setAttachment(pData);
    pData->m_pPlayerCmb->setOwner(this);  // catch events (instead of panel)
    m_pAllPlayersPanel->getDocument()->addComponent(pData->m_pPlayerCmb);

    // Create combo box for listing avatars
    xPxl += pData->m_pPlayerCmb->getWidth() + 10;
    pData->m_pAvatarCmb = guiComboBox::createDefaultComboBox("ChooseAvatar", m_pLocalClient->getInterface(), getDisplay());
    pData->m_pAvatarCmb->moveTo(xPxl, 0);
    pData->m_pAvatarCmb->setEnabled(false);
    pData->m_pAvatarCmb->setAttachment(pData);
    pData->m_pAvatarCmb->setOwner(this);  // catch events (instead of panel)
    m_pAllPlayersPanel->getDocument()->addComponent(pData->m_pAvatarCmb);

    // Create color button
    xPxl += pData->m_pAvatarCmb->getWidth() + 10;
    iTex = getDisplay()->getTextureEngine()->loadTexture("blason1");
    pData->m_pColorBtn = guiButton::createDefaultImageButton(iTex, "ColorButton", getDisplay());
    pData->m_pColorBtn->setDimensions(30, 30);
    pData->m_iColor = getNextColor(-1);
    if (pData->m_iColor >= 0)
        pData->m_pColorBtn->setDiffuseColor(m_AllColors[pData->m_iColor]);
    else
        pData->m_pColorBtn->setDiffuseColor(F_RGBA_NULL);
    pData->m_pColorBtn->moveTo(xPxl, 0);
    pData->m_pColorBtn->setEnabled(false);
    pData->m_pColorBtn->setAttachment(pData);
    pData->m_pColorBtn->setOwner(this);  // catch events (instead of panel)
    pData->m_pColorBtn->setTooltipText(i18n->getText("CHANGE_PLAYER_COLOR", sText, LABEL_MAX_CHARS));
    m_pAllPlayersPanel->getDocument()->addComponent(pData->m_pColorBtn);

    // Find last row position
    int rowHeight = pData->m_pPlayerCmb->getHeight() + 5;
    int yPxl = 5 + (m_pAllPlayersData->size - 1) * rowHeight;

    pData->m_pViewInfoBtn->moveBy(0, yPxl + 7);
    pData->m_pPlayerCmb->moveBy(0, yPxl);
    pData->m_pAvatarCmb->moveBy(0, yPxl);
    pData->m_pColorBtn->moveBy(0, yPxl);

    m_pAllPlayersPanel->getDocument()->setHeight(5 + m_pAllPlayersData->size * rowHeight);

    Profile * pProfile = m_pLocalClient->getDataFactory()->getFirstProfile();
    int nbProfiles = 0;
    while (pProfile != NULL)
    {
        nbProfiles++;
        pData->m_pPlayerCmb->addString(pProfile->getName(), "LoadPlayer");
        pProfile = m_pLocalClient->getDataFactory()->getNextProfile();
    }
    char str[64];
    pData->m_pPlayerCmb->addString(i18n->getText("AI", str, 64), "AI");
    pData->m_pPlayerCmb->addString(i18n->getText("LAN", str, 64), "LANPlayer");
    pData->m_pPlayerCmb->addString(i18n->getText("CLOSED", str, 64), "Closed");
    pData->m_pPlayerCmb->setItem(nbProfiles + 2);
}

// -----------------------------------------------------------------
// Name : onClose
// -----------------------------------------------------------------
void HostGameDlg::onClose(PlayerData * pData)
{
    // If avatar or AI was selected, release it for other combos
    if (pData->m_Type == LocalPlayer && pData->m_pSelectedAvatar != NULL)
        releaseAvatar(pData);
    else if (pData->m_Type == AI && pData->m_pSelectedAI != NULL)
        releaseAI(pData);

    // Remove components
    int rowHeight = pData->m_pPlayerCmb->getHeight() + 5;
    int rowY = pData->m_pPlayerCmb->getYPos();
    m_pAllPlayersPanel->getDocument()->deleteComponent(pData->m_pAvatarCmb);
    m_pAllPlayersPanel->getDocument()->deleteComponent(pData->m_pPlayerCmb);
    m_pAllPlayersPanel->getDocument()->deleteComponent(pData->m_pViewInfoBtn);
    m_pAllPlayersPanel->getDocument()->deleteComponent(pData->m_pColorBtn);

    // Remove pData
    m_pAllPlayersData->deleteObject(pData, true);

    // Pull up any components below
    PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p2 != NULL)
    {
        if (p2->m_pPlayerCmb->getYPos() > rowY)
        {
            p2->m_pAvatarCmb->moveBy(0, -rowHeight);
            p2->m_pPlayerCmb->moveBy(0, -rowHeight);
            p2->m_pViewInfoBtn->moveBy(0, -rowHeight);
            p2->m_pColorBtn->moveBy(0, -rowHeight);
        }
        p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    m_pAllPlayersPanel->getDocument()->setHeight(5 + m_pAllPlayersData->size * rowHeight);
    checkStartEnable();
}

// -----------------------------------------------------------------
// Name : onOpenPlayer
// -----------------------------------------------------------------
void HostGameDlg::onOpenPlayer(char * sName, PlayerData * pData)
{
    pData->m_pAvatarCmb->setItem(-1);
    // If avatar or AI was selected, release it for other combos
    if (pData->m_Type == LocalPlayer && pData->m_pSelectedAvatar != NULL)
        releaseAvatar(pData);
    else if (pData->m_Type == AI && pData->m_pSelectedAI != NULL)
        releaseAI(pData);

    // Reset pData values
    pData->m_Type = LocalPlayer;
    pData->m_pSelectedAvatar = NULL;
    pData->m_pSelectedPlayer = NULL;
    pData->m_pSelectedAI = NULL;

    // Find player's profile
    Profile * pProfile = m_pLocalClient->getDataFactory()->getFirstProfile();
    while (pProfile != NULL)
    {
        if (strcmp(sName, pProfile->getName()) == 0)
        {
            // Found it; now build avatars list
            pData->m_pSelectedPlayer = pProfile;
            pData->m_pAvatarCmb->clearList();
            AvatarData * pFirstAvatar = NULL;
            AvatarData * pAvatar = (AvatarData*) pProfile->getAvatarsList()->getFirst(0);
            while (pAvatar != NULL)
            {
                char sAvId[NAME_MAX_CHARS+64];
                snprintf(sAvId, NAME_MAX_CHARS+64, "%s:%s", pAvatar->m_sEdition, pAvatar->m_sObjectId);
                guiButton * pBtn = pData->m_pAvatarCmb->addString(pAvatar->m_sCustomName, sAvId);
                pBtn->setAttachment(pAvatar);
                if (checkIfAvatarAvailable(pAvatar, pData))
                {
                    pBtn->setEnabled(true);
                    if (pFirstAvatar == NULL)
                    {
                        // Auto-select first available avatar
                        pFirstAvatar = pAvatar;
                        pData->m_pAvatarCmb->setItem(pData->m_pAvatarCmb->getItemsCount() - 1);
                    }
                }
                else
                    pBtn->setEnabled(false);
                pAvatar = (AvatarData*) pProfile->getAvatarsList()->getNext(0);
            }
            pData->m_pAvatarCmb->setEnabled(true);
            pData->m_pViewInfoBtn->setEnabled(true);
            pData->m_pColorBtn->setEnabled(true);
            if (pFirstAvatar == NULL)
            {
                onSelectRow(pData);
                checkStartEnable();
            }
            else
                onSelectAvatar(pFirstAvatar, pData);
            return;
        }
        pProfile = m_pLocalClient->getDataFactory()->getNextProfile();
    }
    // Shouldn't be here
    m_pLocalClient->getDebug()->notifyErrorMessage("Error: player not found");
}

// -----------------------------------------------------------------
// Name : onOpenAI
// -----------------------------------------------------------------
void HostGameDlg::onOpenAI(PlayerData * pData)
{
    pData->m_pAvatarCmb->setItem(-1);
    // If avatar or AI was selected, release it for other combos
    if (pData->m_Type == LocalPlayer && pData->m_pSelectedAvatar != NULL)
        releaseAvatar(pData);
    else if (pData->m_Type == AI && pData->m_pSelectedAI != NULL)
        releaseAI(pData);

    // Reset pData values
    pData->m_Type = AI;
    pData->m_pSelectedAvatar = NULL;
    pData->m_pSelectedPlayer = NULL;
    pData->m_pSelectedAI = NULL;

    // Add available ais
    pData->m_pAvatarCmb->clearList();
    AIData * pFirstAI = NULL;
    AIData * pAI = (AIData*) m_pAvailableAIList->getFirst(0);
    while (pAI != NULL)
    {
        char sId[NAME_MAX_CHARS+64];
        char sText[LABEL_MAX_CHARS];
        snprintf(sId, NAME_MAX_CHARS+64, "%s:%s", pAI->m_sEdition, pAI->m_sObjectId);
        pAI->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
        guiButton * pBtn = pData->m_pAvatarCmb->addString(sText, sId);
        pBtn->setAttachment(pAI);
        pBtn->setEnabled(true);
        if (pFirstAI == NULL)
        {
            pFirstAI = pAI;
            pData->m_pAvatarCmb->setItem(pData->m_pAvatarCmb->getItemsCount() - 1);
        }
        pAI = (AIData*) m_pAvailableAIList->getNext(0);
    }

    // Also add unavailable ai, as disabled items
    pAI = (AIData*) m_pSelectedAIList->getFirst(0);
    while (pAI != NULL)
    {
        char sId[NAME_MAX_CHARS+64];
        char sText[LABEL_MAX_CHARS];
        snprintf(sId, NAME_MAX_CHARS+64, "%s:%s", pAI->m_sEdition, pAI->m_sObjectId);
        pAI->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
        guiButton * pBtn = pData->m_pAvatarCmb->addString(sText, sId);
        pBtn->setAttachment(pAI);
        pBtn->setEnabled(false);
        pAI = (AIData*) m_pSelectedAIList->getNext(0);
    }

    pData->m_pAvatarCmb->setEnabled(true);
    pData->m_pViewInfoBtn->setEnabled(true);
    pData->m_pColorBtn->setEnabled(true);

    if (pFirstAI == NULL)
    {
        onSelectRow(pData);
        checkStartEnable();
    }
    else
        onSelectAI(pFirstAI, pData);
}

// -----------------------------------------------------------------
// Name : onOpenLAN
// -----------------------------------------------------------------
void HostGameDlg::onOpenLAN(PlayerData * pData)
{
    pData->m_pAvatarCmb->setItem(-1);
    // If avatar or AI was selected, release it for other combos
    if (pData->m_Type == LocalPlayer && pData->m_pSelectedAvatar != NULL)
        releaseAvatar(pData);
    else if (pData->m_Type == AI && pData->m_pSelectedAI != NULL)
        releaseAI(pData);

    // Reset pData values
    pData->m_Type = LAN;
    pData->m_pSelectedAvatar = NULL;
    pData->m_pSelectedPlayer = NULL;
    pData->m_pSelectedAI = NULL;

    pData->m_pAvatarCmb->clearList();
    pData->m_pAvatarCmb->setEnabled(false);
    pData->m_pViewInfoBtn->setEnabled(false);
    pData->m_pColorBtn->setEnabled(false);
    checkStartEnable();
}

// -----------------------------------------------------------------
// Name : onSelectAvatar
// -----------------------------------------------------------------
void HostGameDlg::onSelectAvatar(AvatarData * pAvatar, PlayerData * pData)
{
    assert(pAvatar != NULL);
    if (checkIfAvatarAvailable(pAvatar, pData))
    {
        char sNewId[NAME_MAX_CHARS+64];
        snprintf(sNewId, NAME_MAX_CHARS+64, "%s:%s", pAvatar->m_sEdition, pAvatar->m_sObjectId);
        // Disable new avatar in other combos
        PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
        while (p2 != NULL)
        {
            // Same player?
            if (p2 != pData && p2->m_Type == LocalPlayer && p2->m_pSelectedPlayer == pData->m_pSelectedPlayer)
                p2->m_pAvatarCmb->getItem(sNewId)->setEnabled(false);
            p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
        }
        pData->m_pSelectedAvatar = pAvatar;
        char sBuf[MAX_PATH];
        pAvatar->getBanner(sBuf, MAX_PATH);
        pData->m_pColorBtn->setNormalTexture(getDisplay()->getTextureEngine()->loadTexture(sBuf));
        onSelectRow(pData);

        // Check if number of equipped spells is ok
        guiComboBox * pCombo = (guiComboBox*) m_pServerOptionsPopup->getDocument()->getComponent("DeckSizeCombo");
        int iDeckSize = 10 * pCombo->getSelectedItemId();
        if (iDeckSize > 0)
        {
            Profile::SpellData * pSpellDesc = (Profile::SpellData*) pData->m_pSelectedPlayer->getSpellsList()->getFirst(0);
            int count = 0;
            while (pSpellDesc != NULL)
            {
                AvatarData * pOwner = pSpellDesc->m_pOwner;
                if (pOwner != NULL && strcmp(pAvatar->m_sEdition, pOwner->m_sEdition) == 0
                        && strcmp(pAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
                    count++;
                pSpellDesc = (Profile::SpellData*) pData->m_pSelectedPlayer->getSpellsList()->getNext(0);
            }
            if (count > iDeckSize)
            {
                char sText[LABEL_MAX_CHARS];
                guiPopup * pPopup = guiPopup::createOkAutoclosePopup(i18n->getText("WARNING_AVATAR_HAS_TOO_MUCH_SPELLS", sText, LABEL_MAX_CHARS), getDisplay());
                m_pLocalClient->getInterface()->registerFrame(pPopup);
                pPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - pPopup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - pPopup->getHeight()) / 2);
                pPopup->flash(1.0f);
            }
        }
    }
    else
    {
        // raise message popup: this Avatar is already selected in another row
        char sText[128];
        i18n->getText("CANNOT_SELECT_AVATAR_TWICE", sText, 128);
        guiPopup * popup = guiPopup::createTimedPopup(sText, 4, 200, getDisplay());
        m_pLocalClient->getInterface()->registerFrame(popup);
        popup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - popup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - popup->getHeight()) / 2);
        pData->m_pAvatarCmb->setItem(-1);
        pData->m_pSelectedAvatar = NULL;
    }
    checkStartEnable();
}

// -----------------------------------------------------------------
// Name : onSelectAI
// -----------------------------------------------------------------
void HostGameDlg::onSelectAI(AIData * pAI, PlayerData * pData)
{
    assert(pAI != NULL);
    if (m_pAvailableAIList->goTo(0, pAI))
    {
        m_pAvailableAIList->deleteCurrent(0, false);
        m_pSelectedAIList->addLast(pAI);
        char sNewId[NAME_MAX_CHARS+64];
        snprintf(sNewId, NAME_MAX_CHARS+64, "%s:%s", pAI->m_sEdition, pAI->m_sObjectId);
        // Disable new AI in other combos
        PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
        while (p2 != NULL)
        {
            // Other AI row?
            if (p2 != pData && p2->m_Type == AI)
                p2->m_pAvatarCmb->getItem(sNewId)->setEnabled(false);
            p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
        }
        pData->m_pSelectedAI = pAI;
        pData->m_pColorBtn->setNormalTexture(getDisplay()->getTextureEngine()->loadTexture("blason1"));
        onSelectRow(pData);

        // Check if number of equipped spells is ok
        guiComboBox * pCombo = (guiComboBox*) m_pServerOptionsPopup->getDocument()->getComponent("DeckSizeCombo");
        int iDeckSize = 10 * pCombo->getSelectedItemId();
        if (iDeckSize > 0)
        {
            int nbSpells = 0;
            SpellsPackContent * pPack = (SpellsPackContent*) pAI->m_pSpellsPacks->getFirst(0);
            while (pPack != NULL)
            {
                nbSpells += pPack->m_iNbSpells;
                pPack = (SpellsPackContent*) pAI->m_pSpellsPacks->getNext(0);
            }
            if (nbSpells > iDeckSize)
            {
                char sText[LABEL_MAX_CHARS];
                guiPopup * pPopup = guiPopup::createOkAutoclosePopup(i18n->getText("WARNING_AVATAR_HAS_TOO_MUCH_SPELLS", sText, LABEL_MAX_CHARS), getDisplay());
                m_pLocalClient->getInterface()->registerFrame(pPopup);
                pPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - pPopup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - pPopup->getHeight()) / 2);
                pPopup->flash(1.0f);
            }
        }
        checkStartEnable();
    }
    else
    {
        // raise message popup: this AI is already selected in another row
        char sText[128];
        i18n->getText("CANNOT_SELECT_AVATAR_TWICE", sText, 128);
        guiPopup * popup = guiPopup::createTimedPopup(sText, 4, 200, getDisplay());
        m_pLocalClient->getInterface()->registerFrame(popup);
        popup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - popup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - popup->getHeight()) / 2);
        pData->m_pAvatarCmb->setItem(-1);
        pData->m_pSelectedAI = NULL;
    }
}

// -----------------------------------------------------------------
// Name : releaseAvatar
// -----------------------------------------------------------------
void HostGameDlg::releaseAvatar(PlayerData * pData)
{
    assert(pData != NULL && pData->m_pSelectedAvatar != NULL);
    char sOldAvId[NAME_MAX_CHARS+64];
    snprintf(sOldAvId, NAME_MAX_CHARS+64, "%s:%s", pData->m_pSelectedAvatar->m_sEdition, pData->m_pSelectedAvatar->m_sObjectId);
    // Enable previous avatar in other combos
    PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p2 != NULL)
    {
        // Same player?
        if (p2 != pData && p2->m_Type == LocalPlayer && p2->m_pSelectedPlayer == pData->m_pSelectedPlayer)
            p2->m_pAvatarCmb->getItem(sOldAvId)->setEnabled(true);
        p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    pData->m_pSelectedAvatar = NULL;
}

// -----------------------------------------------------------------
// Name : releaseAI
// -----------------------------------------------------------------
void HostGameDlg::releaseAI(PlayerData * pData)
{
    assert(pData != NULL && pData->m_pSelectedAI != NULL);
    m_pSelectedAIList->deleteObject(pData->m_pSelectedAI, false);
    m_pAvailableAIList->addLast(pData->m_pSelectedAI);
    char sOldId[NAME_MAX_CHARS+64];
    snprintf(sOldId, NAME_MAX_CHARS+64, "%s:%s", pData->m_pSelectedAI->m_sEdition, pData->m_pSelectedAI->m_sObjectId);
    // Enable ai in other combos
    PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p2 != NULL)
    {
        // Same player?
        if (p2->m_Type == AI)
            p2->m_pAvatarCmb->getItem(sOldId)->setEnabled(true);
        p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    pData->m_pSelectedAI = NULL;
}

// -----------------------------------------------------------------
// Name : onSelectMap
// -----------------------------------------------------------------
void HostGameDlg::onSelectMap(char * sMapId)
{
    m_pMapReader->init(sMapId);
    MapReader::deleteMapParameters(m_pMapParameters);
    m_pMapReader->getMapParameters(m_pMapParameters, LABEL_MAX_CHARS);

    m_pMapOptionsPanel->getDocument()->deleteAllComponents();
    char sText[LABEL_MAX_CHARS];
    char sDefault[LABEL_MAX_CHARS];
    i18n->getText("DEFAULT", sDefault, LABEL_MAX_CHARS);
    int yPxl = SPACING;
    int xPxl = SPACING;
    int combox = SPACING;
    int topy = yPxl;
    guiComboBox * pDummyCombo = guiComboBox::createDefaultComboBox(" ", m_pLocalClient->getInterface(), m_pLocalClient->getDisplay());
    int lineHeight = pDummyCombo->getHeight() + SPACING;
    delete pDummyCombo;

    // Labels
    MapReader::MapParameters * pParam = (MapReader::MapParameters*) m_pMapParameters->getFirst(0);
    while (pParam != NULL)
    {
        guiLabel * pLbl = new guiLabel();
        pLbl->init(pParam->sLabel, H2_FONT, H2_COLOR, "", xPxl, yPxl, -1, -1, m_pLocalClient->getDisplay());
        m_pMapOptionsPanel->getDocument()->addComponent(pLbl);
        combox = max(combox, xPxl + pLbl->getWidth() + SPACING);
        yPxl += lineHeight;
        pParam = (MapReader::MapParameters*) m_pMapParameters->getNext(0);
    }

    // Combo boxes
    yPxl = topy;
    pParam = (MapReader::MapParameters*) m_pMapParameters->getFirst(0);
    while (pParam != NULL)
    {
        guiComboBox * pCombo = guiComboBox::createDefaultComboBox(pParam->sLabel, m_pLocalClient->getInterface(), m_pLocalClient->getDisplay());
        pCombo->setWidth(300);
        pCombo->moveTo(combox, yPxl);
        for (int i = 0; i < pParam->nbValues; i++)
        {
            if (i == pParam->defaultValueIndex)
            {
                snprintf(sText, LABEL_MAX_CHARS, "%s (%s)", pParam->pPossibleValueLabels[i], sDefault);
                pCombo->addString(sText, "");
            }
            else
                pCombo->addString(pParam->pPossibleValueLabels[i], "");
        }
        pCombo->setItem(pParam->defaultValueIndex);
        m_pMapOptionsPanel->getDocument()->addComponent(pCombo);
        yPxl += lineHeight;
        pParam = (MapReader::MapParameters*) m_pMapParameters->getNext(0);
    }
    m_pMapOptionsPanel->getDocument()->setHeight(yPxl);
}

// -----------------------------------------------------------------
// Name : checkIfAvatarAvailable
// -----------------------------------------------------------------
bool HostGameDlg::checkIfAvatarAvailable(AvatarData * pAvatar, PlayerData * pExceptHere)
{
    PlayerData * p2 = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p2 != NULL)
    {
        if (p2 != pExceptHere && p2->m_pSelectedAvatar == pAvatar && p2->m_pSelectedPlayer == pExceptHere->m_pSelectedPlayer)
            return false;
        p2 = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    return true;
}

// -----------------------------------------------------------------
// Name : onSelectRow
// -----------------------------------------------------------------
void HostGameDlg::onSelectRow(PlayerData * pData)
{
    m_pPlayerPanel->getDocument()->deleteAllComponents();
    m_pAvatarPanel->getDocument()->deleteAllComponents();

    if (pData->m_Type == LocalPlayer && pData->m_pSelectedPlayer != NULL)
    {
        // Set player info
        char sText[LABEL_MAX_CHARS] = "";
        char sBuf1[128];
        char sBuf2[128];

        // Write title (player name)
        int yPxl = 5;
        guiLabel * pLbl = new guiLabel();
        pLbl->init(pData->m_pSelectedPlayer->getName(), H2_FONT, H2_COLOR, "TitleLabe", 0, 0, 0, 0, getDisplay());
        pLbl->moveTo((m_pPlayerPanel->getInnerWidth() - pLbl->getWidth()) / 2, yPxl);
        m_pPlayerPanel->getDocument()->addComponent(pLbl);

        // Write other info
        yPxl += pLbl->getHeight() + 5;
        int cash = pData->m_pSelectedPlayer->getCash();
        i18n->getText("CASH_(d)", sBuf1, 128);
        snprintf(sBuf2, 128, sBuf1, cash);
        wsafecpy(sText, LABEL_MAX_CHARS, sBuf2);
        int won = pData->m_pSelectedPlayer->getNumberOfWonGames();
        int lost = pData->m_pSelectedPlayer->getNumberOfLostGames();
        i18n->getText("DISPUTED_GAMES_(d)_WON_(d)_LOST_(d)", sBuf1, 128);
        snprintf(sBuf2, 128, sBuf1, won+lost, won, lost);
        wsafecat(sText, LABEL_MAX_CHARS, "\n");
        wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
        if (won+lost > 0)
        {
            i18n->getText("PCT_VICTORIES_(d)", sBuf1, 128);
            snprintf(sBuf2, 128, sBuf1, (int) ((100*won)/(won+lost)));
            wsafecat(sText, LABEL_MAX_CHARS, "\n");
            wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
        }
        int navatars = pData->m_pSelectedPlayer->getAvatarsList()->size;
        i18n->getText("AVATARS_(d)", sBuf1, 128);
        snprintf(sBuf2, 128, sBuf1, navatars);
        wsafecat(sText, LABEL_MAX_CHARS, "\n");
        wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
        int nspells = pData->m_pSelectedPlayer->getSpellsList()->size;
        i18n->getText("SPELLS_(d)", sBuf1, 128);
        snprintf(sBuf2, 128, sBuf1, nspells);
        wsafecat(sText, LABEL_MAX_CHARS, "\n");
        wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
        pLbl = new guiLabel();
        pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", SPACING, yPxl, m_pPlayerPanel->getInnerWidth() - 2 * SPACING, 0, getDisplay());
        m_pPlayerPanel->getDocument()->addComponent(pLbl);

        m_pPlayerPanel->getDocument()->setHeight(pLbl->getHeight() + yPxl);

        if (pData->m_pSelectedAvatar != NULL)
        {
            // Set avatar info
            char sInfos[LABEL_MAX_CHARS];
            int iDocWidth = m_pAvatarPanel->getInnerWidth();
            int iImageSize = 64;

            pData->m_pSelectedAvatar->getInfos(sInfos, LABEL_MAX_CHARS, "\n", false, NULL, true, true, true, false);

            // Write title (Avatar name)
            yPxl = 5;
            guiLabel * pLbl = new guiLabel();
            pLbl->init(pData->m_pSelectedAvatar->m_sCustomName, H2_FONT, H2_COLOR, "TitleLabe", 0, 0, 0, 0, getDisplay());
            pLbl->moveTo((iDocWidth - pLbl->getWidth()) / 2, yPxl);
            m_pAvatarPanel->getDocument()->addComponent(pLbl);

            // Add image
            yPxl += pLbl->getHeight() + 5;
            int iTex = getDisplay()->getTextureEngine()->loadTexture(pData->m_pSelectedAvatar->m_sTextureFilename);
            guiImage * pImg = new guiImage();
            pImg->init(iTex, "Image", 5, yPxl, iImageSize, iImageSize, getDisplay());
            m_pAvatarPanel->getDocument()->addComponent(pImg);

            // Add label for characteristics
            pLbl = new guiLabel();
            pLbl->init(sInfos, TEXT_FONT, TEXT_COLOR, "CharacsLabe", 15 + iImageSize, yPxl, iDocWidth - 20 - iImageSize, 0, getDisplay());
            m_pAvatarPanel->getDocument()->addComponent(pLbl);

            // Count number of spells equipped
            int nbSpells = 0;
            Profile::SpellData * pObj2 = (Profile::SpellData*) pData->m_pSelectedPlayer->getSpellsList()->getFirst(0);
            while (pObj2 != NULL)
            {
                AvatarData * pOwner = pObj2->m_pOwner;
                if (pOwner != NULL && strcmp(pData->m_pSelectedAvatar->m_sEdition, pOwner->m_sEdition) == 0
                        && strcmp(pData->m_pSelectedAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
                    nbSpells++;
                pObj2 = (Profile::SpellData*) pData->m_pSelectedPlayer->getSpellsList()->getNext(0);
            }
            yPxl += 10 + max(pLbl->getHeight(), iImageSize);
            char sBuf[64];
            char sNbSpells[128];
            i18n->getText("SPELLS_(d)", sBuf, 64);
            snprintf(sNbSpells, 128, sBuf, nbSpells);
            pLbl = new guiLabel();
            pLbl->init(sNbSpells, TEXT_FONT, TEXT_COLOR, "NbSpells", 5, yPxl, iDocWidth - 10, 0, getDisplay());
            m_pAvatarPanel->getDocument()->addComponent(pLbl);

            // XP
            void * pArgs[2];
            int xp = pData->m_pSelectedAvatar->m_uXP;
            int next = pData->m_pSelectedAvatar->getNextLevelXP();
            pArgs[0] = &xp;
            pArgs[1] = &next;
            i18n->getText("TOTAL_XP(d1)_NEXT(d2)", sText, LABEL_MAX_CHARS, pArgs);
            yPxl += 3 + pLbl->getHeight();
            pLbl = new guiLabel();
            pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 5, yPxl, iDocWidth - 10, 0, getDisplay());
            m_pAvatarPanel->getDocument()->addComponent(pLbl);

            // Add description
            yPxl += 10 + pLbl->getHeight();
            pLbl = new guiLabel();
            pLbl->init(pData->m_pSelectedAvatar->m_sCustomDescription, TEXT_FONT, TEXT_COLOR, "DescLabe", 5, yPxl, iDocWidth - 10, 0, getDisplay());
            m_pAvatarPanel->getDocument()->addComponent(pLbl);

            m_pAvatarPanel->getDocument()->setHeight(pLbl->getHeight() + yPxl);
        }
    }
}

// -----------------------------------------------------------------
// Name : checkStartEnable
// -----------------------------------------------------------------
void HostGameDlg::checkStartEnable()
{
    int nblocal = 0;
    m_iNbRemoteClients = 0;
    PlayerData * p = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p != NULL)
    {
        if ((p->m_pSelectedPlayer != NULL && p->m_pSelectedAvatar != NULL) || p->m_pSelectedAI != NULL)
            nblocal++;
        p = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    m_bIsLocalClient = (nblocal > 0);
    getComponent("NextButton")->setEnabled(nblocal + m_iNbRemoteClients >= 2);
    char str[128];
    getComponent("NextButton")->setTooltipText(nblocal + m_iNbRemoteClients >= 2 ? "" : i18n->getText("NEED_TWO_PLAYERS", str, 128));
}

// -----------------------------------------------------------------
// Name : startGame
// -----------------------------------------------------------------
void HostGameDlg::startGame()
{
    m_fStartGameTimer = 0.5f;
    char sText[LABEL_MAX_CHARS];
    guiPopup * pPopup = guiPopup::createTimedPopup(i18n->getText("PLEASE_WAIT_WHILE_SERVER_STARTING", sText, LABEL_MAX_CHARS), -1, 300, getDisplay());
    pPopup->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pPopup->getWidth() / 2, 200);
    m_pLocalClient->getInterface()->registerFrame(pPopup);
    setEnabled(false);
}

// -----------------------------------------------------------------
// Name : _reallyStartGame
// -----------------------------------------------------------------
void HostGameDlg::_reallyStartGame()
{
    // Build client data
    int nbClients = m_iNbRemoteClients + (m_bIsLocalClient ? 1 : 0);
    ClientData * clients = new ClientData[nbClients];
    int iClient = 0;
    if (m_bIsLocalClient)
    {
        clients[iClient].bLocal = true;
        iClient++;
    }
    while (iClient < nbClients)
    {
        clients[iClient].bLocal = false;
        iClient++;
    }

    // Retrieve server parameters
    char sGameName[64];
    char sMapFile[MAX_PATH];

    guiEditBox * pEdit = (guiEditBox*) getComponent("GameName");
    wsafecpy(sGameName, 64, pEdit->getText());

    guiComboBox * pCombo = (guiComboBox*) m_pServerOptionsPopup->getDocument()->getComponent("TurnTimerCombo");
    int iValues[NB_TURN_TIMER_VALUES] = TURN_TIMER_VALUES;
    int iTurnTimer = iValues[pCombo->getSelectedItemId()];

    pCombo = (guiComboBox*) m_pServerOptionsPopup->getDocument()->getComponent("DeckSizeCombo");
    int iDeckSize = 10 * pCombo->getSelectedItemId();

    pCombo = (guiComboBox*) getComponent("MapTypeCombo");
    wsafecpy(sMapFile, MAX_PATH, pCombo->getSelectedItem()->getId());

    // Re-init map data
    m_pMapReader->init(sMapFile);
    MapReader::deleteMapParameters(m_pMapParameters);
    m_pMapReader->getMapParameters(m_pMapParameters, LABEL_MAX_CHARS);

    int * pCustomParams = NULL;
    if (m_pMapParameters->size > 0)
        pCustomParams = new int[m_pMapParameters->size];

    // Map custom parameters
    int i = 0;
    MapReader::MapParameters * pParam = (MapReader::MapParameters*) m_pMapParameters->getFirst(0);
    while (pParam != NULL)
    {
        pCombo = (guiComboBox*) m_pMapOptionsPanel->getDocument()->getComponent(pParam->sLabel);
        pCustomParams[i++] = pCombo->getSelectedItemId();
        pParam = (MapReader::MapParameters*) m_pMapParameters->getNext(0);
    }

    // Init map generator (we will nnot delete it here, as the pointer now belong to Server object)
    m_pMapReader->setMapParameters(pCustomParams, m_pMapParameters->size, m_pAllPlayersData->size - 1);
    delete[] pCustomParams;

    // Init server
    Server * pServer = m_pLocalClient->initServer(sGameName, nbClients, clients, m_pMapReader, iTurnTimer, iDeckSize);
    delete[] clients;
    if (pServer == NULL)
    {
        m_pLocalClient->getDebug()->notifyErrorMessage("Error: server could not be initialized.");
        return;
    }

    // Build players data
    ObjectList * pServerPlayers = pServer->getSolver()->getPlayersList();
    // Create neutral player
    char sName[NAME_MAX_CHARS];
    i18n->getText("NEUTRA", sName, NAME_MAX_CHARS);
    Player * pPlayer = new Player(0, 0, pServer->getSolver()->getGlobalSpellsPtr());
    wsafecpy(pPlayer->m_sProfileName, NAME_MAX_CHARS, sName);
    pPlayer->m_Color = rgb(0.5, 0.5, 0.5);
    wsafecpy(pPlayer->m_sBanner, 64, "blason1");
    pServer->getSolver()->setNeutralPlayer(pPlayer);
    // Human players
    int playerId = 1;
    PlayerData * p = (PlayerData*) m_pAllPlayersData->getFirst(0);
    while (p != NULL)
    {
        pPlayer = NULL;
        if (p->m_Type == AI && p->m_pSelectedAI != NULL)
        {
            // Create AI player object
            pPlayer = new Player(playerId, 0, pServer->getSolver()->getGlobalSpellsPtr());
            pServerPlayers->addLast(pPlayer);
            pPlayer->m_bIsAI = true;
            p->m_pSelectedAI->findLocalizedElement(pPlayer->m_sProfileName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            wsafecpy(pPlayer->m_sBanner, 64, "blason1");
            assert(p->m_iColor >= 0);
            pPlayer->m_Color = m_AllColors[p->m_iColor];
            // Set Avatar
            CoordsMap pos = m_pMapReader->getPlayerPosition(playerId-1);
            pServer->getSolver()->setInitialAvatar(p->m_pSelectedAI->createAvatar(m_pLocalClient), pPlayer, pos);
            // Add spells
            ObjectList * pSpellsList = new ObjectList(false);
            p->m_pSelectedAI->fillSpellsList(pSpellsList, m_pLocalClient);
            Spell * pSpell = (Spell*) pSpellsList->getFirst(0);
            while (pSpell != NULL)
            {
                pServer->getSolver()->addInitialPlayerSpell(pPlayer, pSpell->getObjectEdition(), pSpell->getObjectName());
                pSpell = (Spell*) pSpellsList->getNext(0);
            }
            delete pSpellsList;
            playerId++;
        }
        if (p->m_Type == LocalPlayer && p->m_pSelectedPlayer != NULL && p->m_pSelectedAvatar != NULL)
        {
            // Create player object
            pPlayer = new Player(playerId, 0, pServer->getSolver()->getGlobalSpellsPtr());
            pServerPlayers->addLast(pPlayer);
            wsafecpy(pPlayer->m_sProfileName, NAME_MAX_CHARS, p->m_pSelectedPlayer->getName());
            p->m_pSelectedAvatar->getBanner(pPlayer->m_sBanner, 64);
            assert(p->m_iColor >= 0);
            pPlayer->m_Color = m_AllColors[p->m_iColor];
            // Set Avatar
            CoordsMap pos = m_pMapReader->getPlayerPosition(playerId-1);
            pServer->getSolver()->setInitialAvatar(p->m_pSelectedAvatar->clone(m_pLocalClient), pPlayer, pos);
            // Add spells that are equipped
            Profile::SpellData * pSpellDesc = (Profile::SpellData*) p->m_pSelectedPlayer->getSpellsList()->getFirst(0);
            while (pSpellDesc != NULL)
            {
                AvatarData * pOwner = pSpellDesc->m_pOwner;
                if (pOwner != NULL && strcmp(p->m_pSelectedAvatar->m_sEdition, pOwner->m_sEdition) == 0
                        && strcmp(p->m_pSelectedAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
                    pServer->getSolver()->addInitialPlayerSpell(pPlayer, pSpellDesc->m_sEdition, pSpellDesc->m_sName);
                pSpellDesc = (Profile::SpellData*) p->m_pSelectedPlayer->getSpellsList()->getNext(0);
            }
            // Add equipped artifacts
            Artifact * pArtifact = (Artifact*) p->m_pSelectedPlayer->getArtifactsList()->getFirst(0);
            while (pArtifact != NULL)
            {
                AvatarData * pOwner = pArtifact->m_pOwner;
                if (pOwner != NULL && strcmp(p->m_pSelectedAvatar->m_sEdition, pOwner->m_sEdition) == 0
                        && strcmp(p->m_pSelectedAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
                {
                    Unit * pAvatarInGame = pPlayer->getAvatar();
                    assert(pAvatarInGame != NULL);
                    ArtifactEffect * pEffect = (ArtifactEffect*) pArtifact->getArtifactEffects()->getFirst(0);
                    while (pEffect != NULL)
                    {
                        switch (pEffect->getType())
                        {
                        case ARTIFACT_EFFECT_CHARAC:
                        {
                            bool bFound = true;
                            long val = pAvatarInGame->getValue(((ArtifactEffect_Charac*)pEffect)->m_sKey, false, &bFound);
                            if (bFound)
                                pAvatarInGame->setBaseValue(((ArtifactEffect_Charac*)pEffect)->m_sKey, max(0, val + ((ArtifactEffect_Charac*)pEffect)->m_iModifier));
                            else
                            {
                                char sError[1024];
                                snprintf(sError, 1024, "Warning: artifact %s tries to modify characteristic that doesn't exist (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Charac*)pEffect)->m_sKey);
                                m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                            }
                            break;
                        }
                        case ARTIFACT_EFFECT_SPELL:
                        {
                            Spell * pSpell = m_pLocalClient->getDataFactory()->findSpell(((ArtifactEffect_Spell*)pEffect)->m_sSpellEdition, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                            if (pSpell != NULL)
                                pServer->getSolver()->addInitialPlayerSpell(pPlayer, ((ArtifactEffect_Spell*)pEffect)->m_sSpellEdition, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                            else
                            {
                                char sError[1024];
                                snprintf(sError, 1024, "Warning: artifact %s tries to add spell that doesn't exist (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                                m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                            }
                            break;
                        }
                        case ARTIFACT_EFFECT_SKILL:
                        {
                            Skill * pSkill = new Skill(((ArtifactEffect_Skill*)pEffect)->m_sSkillEdition, ((ArtifactEffect_Skill*)pEffect)->m_sSkillName, ((ArtifactEffect_Skill*)pEffect)->m_sSkillParameters, pServer->getDebug());
                            if (pSkill != NULL && pSkill->isLoaded())
                                pAvatarInGame->addSkill(pSkill);
                            else
                            {
                                char sError[1024];
                                snprintf(sError, 1024, "Warning: artifact %s tries to add skill that doesn't exist or that can't be loaded (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Skill*)pEffect)->m_sSkillName);
                                m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                            }
                            break;
                        }
                        }
                        pEffect = (ArtifactEffect*) pArtifact->getArtifactEffects()->getNext(0);
                    }
                }
                pArtifact = (Artifact*) p->m_pSelectedPlayer->getArtifactsList()->getNext(0);
            }
            playerId++;
        }
        else
        {
        }
        p = (PlayerData*) m_pAllPlayersData->getNext(0);
    }
    pServer->onInitFinished();
}

// -----------------------------------------------------------------
// Name : getNextColor
// -----------------------------------------------------------------
int HostGameDlg::getNextColor(int iColor)
{
    iColor = (iColor + 1) % MAX_COLORS;
    int iFirstColor = iColor;
    while (true)
    {
        PlayerData * p = (PlayerData*) m_pAllPlayersData->getFirst(0);
        while (p != NULL)
        {
            if (iColor == p->m_iColor)
            {
                iColor = (iColor + 1) % MAX_COLORS;
                if (iColor == iFirstColor)
                    return -1;
                break;
            }
            p = (PlayerData*) m_pAllPlayersData->getNext(0);
            if (p == NULL)
                return iColor;
        }
    }
    return -1;
}

// -----------------------------------------------------------------
// Name : checkGameNameWarning
// -----------------------------------------------------------------
void HostGameDlg::checkGameNameWarning()
{
    // Check game name
    guiEditBox * pEdit = (guiEditBox*) getComponent("GameName");
    char sFilePath[MAX_PATH];
    snprintf(sFilePath, MAX_PATH, "%s%s.sav", SAVES_PATH, pEdit->getText());
    // Check if file exist
    FILE * pFile = NULL;
    if (0 == fopen_s(&pFile, sFilePath, "r"))
    {
        // Raise confirm popup
        char sWarning[128];
        setEnabled(false);
        m_pGameNameWarningPopup = guiPopup::createYesNoPopup(i18n->getText("GAME_NAME_ALREADY_EXIST", sWarning, 128), m_pLocalClient->getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pGameNameWarningPopup);
        m_pGameNameWarningPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pGameNameWarningPopup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pGameNameWarningPopup->getHeight()) / 2);
    }
    else
        startGame();
}

// -----------------------------------------------------------------
// Name : bringAbove
// TODO : remove that, enhance combo list display
// -----------------------------------------------------------------
void HostGameDlg::bringAbove(guiComponent * pCpnt)
{
    if (getComponentsList()->goTo(0, pCpnt))
        getComponentsList()->moveCurrentToEnd(0);
    else if (m_pAllPlayersPanel->getDocument()->getComponentsList()->goTo(0, pCpnt))
        m_pAllPlayersPanel->getDocument()->getComponentsList()->moveCurrentToEnd(0);
}
