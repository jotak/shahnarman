#include "SelectPlayerAvatarDlg.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "BuildDeckDlg.h"
#include "ArtifactsEquipDlg.h"
#include "CreateAvatarDlg.h"
#include "ShopDlg.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiPopup.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Profile.h"

#define SPACING    4

// -----------------------------------------------------------------
// Name : SelectPlayerAvatarDlg
//  Constructor
// -----------------------------------------------------------------
SelectPlayerAvatarDlg::SelectPlayerAvatarDlg(LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_pCurrentPlayer = NULL;

    init("Select player and Avatar",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, 1, 1, pLocalClient->getDisplay());

    m_pTextInput = NULL;
    m_pConfirmDelete = NULL;

    // Create top label "Choose player"
    int yPxl = 10;
    char str[LABEL_MAX_CHARS];
    guiLabel * pLbl = new guiLabel();
    pLbl->init(i18n->getText("CHOOSE_PLAYER", str, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, "ChoosePlayerLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, yPxl);
    addComponent(pLbl);

    // Create combo box for listing players
    yPxl += pLbl->getHeight() + SPACING;
    guiComboBox * pCombo = guiComboBox::createDefaultComboBox("ChoosePlayer", pLocalClient->getInterface(), pLocalClient->getDisplay());
    pCombo->moveTo(0, yPxl);
    addComponent(pCombo);
    setWidth(pCombo->getWidth());
    pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, pLbl->getYPos());

    // Shop button
    yPxl += pCombo->getHeight() + 2 * SPACING;
    guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText("SHOP", str, LABEL_MAX_CHARS), "ShopButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Create Shahmah button
    yPxl += pBtn->getHeight() + 2 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("CREATE_SHAHMAH", str, LABEL_MAX_CHARS), "CreateShahmahButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Spells button
    yPxl += pBtn->getHeight() + 2 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("SPELLS", str, LABEL_MAX_CHARS), "SpellsButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Equipments button
    yPxl += pBtn->getHeight() + 2 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("EQUIPMENTS", str, LABEL_MAX_CHARS), "EquipButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Statistics button
    yPxl += pBtn->getHeight() + 2 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("STATISTICS", str, LABEL_MAX_CHARS), "StatsButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Delete player button
    yPxl += pBtn->getHeight() + 2 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("DELETE_THIS_PLAYER", str, LABEL_MAX_CHARS), "DeletePlayer", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    // Main menu button
    yPxl += pBtn->getHeight() + 5 * SPACING;
    pBtn = guiButton::createDefaultNormalButton(i18n->getText("MAIN_MENU", str, LABEL_MAX_CHARS), "BackButton", pLocalClient->getDisplay());
    pBtn->moveTo(0, yPxl);
    pBtn->setWidth(getWidth());
    addComponent(pBtn);

    yPxl += pBtn->getHeight() + SPACING;
    setHeight(yPxl);

    loadPlayersList();
    unloadPlayer();
}

// -----------------------------------------------------------------
// Name : ~SelectPlayerAvatarDlg
//  Destructor
// -----------------------------------------------------------------
SelectPlayerAvatarDlg::~SelectPlayerAvatarDlg()
{
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::update(double delta)
{
    guiDocument::update(delta);
    if (m_pTextInput != NULL)
    {
        guiComponent * pCpnt = m_pTextInput->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "OkButton") == 0)
            {
                char * sName = m_pTextInput->getEditBox()->getText();
                if (createPlayer(sName))
                {
                    m_pLocalClient->getInterface()->deleteFrame(m_pTextInput);
                    m_pTextInput = NULL;
                    setEnabled(true);
                }
                else
                {
                    m_pTextInput->getEditBox()->setText("Can't create player");
                }
            }
            else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
            {
                m_pLocalClient->getInterface()->deleteFrame(m_pTextInput);
                m_pTextInput = NULL;
                setEnabled(true);
            }
        }
    }
    if (m_pConfirmDelete != NULL)
    {
        guiComponent * pCpnt = m_pConfirmDelete->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "YesButton") == 0)
            {
                // Delete player
                assert(m_pCurrentPlayer != NULL);
                m_pCurrentPlayer->deleteProfile();
                m_pLocalClient->getDataFactory()->onProfileDeleted(m_pCurrentPlayer);
                unloadPlayer();
                loadPlayersList();

                // Remove frame
                m_pLocalClient->getInterface()->deleteFrame(m_pConfirmDelete);
                m_pConfirmDelete = NULL;
                setEnabled(true);
            }
            else if (strcmp(pCpnt->getId(), "NoButton") == 0)
            {
                m_pLocalClient->getInterface()->deleteFrame(m_pConfirmDelete);
                m_pConfirmDelete = NULL;
                setEnabled(true);
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::onShow()
{
    if (m_pCurrentPlayer != NULL)
        onPlayerDataChanged();
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool SelectPlayerAvatarDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (strcmp(pCpnt->getId(), "LoadPlayer") == 0)
    {
        char * sText = ((guiButton*)pCpnt)->getText();
        if (strcmp(sText, "") != 0)
            loadPlayer(sText);
    }
    else if (strcmp(pCpnt->getId(), "NewPlayer") == 0)
    {
        setEnabled(false);
        unloadPlayer();
        char str[64];
        m_pTextInput = guiPopup::createTextInputPopup(i18n->getText("ENTER_PLAYER_NAME", str, 64), 1, false, 200, m_pLocalClient->getInput(), m_pLocalClient->getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pTextInput);
        m_pTextInput->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pTextInput->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pTextInput->getHeight()) / 2);
    }
    else if (strcmp(pCpnt->getId(), "DeletePlayer") == 0)
    {
        // Raise confirm popup
        setEnabled(false);
        char str[128];
        m_pConfirmDelete = guiPopup::createYesNoPopup(i18n->getText("REALLY_DELETE_PLAYER", str, 128), m_pLocalClient->getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pConfirmDelete);
        m_pConfirmDelete->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pConfirmDelete->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pConfirmDelete->getHeight()) / 2);
    }
    else if (strcmp(pCpnt->getId(), "BackButton") == 0)
    {
        unloadPlayer();
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "CreateShahmahButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getCreateAvatarDlg());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "SpellsButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getBuildDeckDialog());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "EquipButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getArtifactsEquipDialog());
        return false;
    }
    else if (strcmp(pCpnt->getId(), "ShopButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getShopDialog());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------
// Name : loadPlayersList
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::loadPlayersList(char * sSelect)
{
    char str[64];
    guiComboBox * pCombo = (guiComboBox*) getComponent("ChoosePlayer");
    pCombo->clearList();

    int iSelect = -1;
    Profile * pProfile = m_pLocalClient->getDataFactory()->getFirstProfile();
    int count = 0;
    while (pProfile != NULL)
    {
        pCombo->addString(pProfile->getName(), "LoadPlayer");
        if (sSelect != NULL && strcmp(sSelect, pProfile->getName()) == 0)
            iSelect = count;
        count++;
        pProfile = m_pLocalClient->getDataFactory()->getNextProfile();
    }

    if (iSelect == -1 && count == 1)
        iSelect = 0;
    pCombo->addString(i18n->getText("CREATE_NEW", str, 64), "NewPlayer");
    pCombo->setItem(iSelect);
}

// -----------------------------------------------------------------
// Name : createPlayer
// -----------------------------------------------------------------
bool SelectPlayerAvatarDlg::createPlayer(char * sName)
{
    Profile * pPlayer = new Profile(m_pLocalClient);
    if (pPlayer->create(sName))
    {
        m_pCurrentPlayer = pPlayer;
        m_pLocalClient->getDataFactory()->onProfileAdded(pPlayer);
        loadPlayersList(sName);
        onPlayerDataChanged();
        return true;
    }
    delete pPlayer;
    return false;
}

// -----------------------------------------------------------------
// Name : unloadPlayer
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::unloadPlayer()
{
    m_pCurrentPlayer = NULL;

    ((guiComboBox*)getComponent("ChoosePlayer"))->setItem(-1);
    getComponent("DeletePlayer")->setEnabled(false);
    getComponent("SpellsButton")->setEnabled(false);
    getComponent("EquipButton")->setEnabled(false);
    getComponent("ShopButton")->setEnabled(false);
    getComponent("CreateShahmahButton")->setEnabled(false);
    getComponent("StatsButton")->setEnabled(false);
}

// -----------------------------------------------------------------
// Name : loadPlayer
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::loadPlayer(char * sName)
{
    m_pCurrentPlayer = m_pLocalClient->getDataFactory()->findProfile(sName);
    onPlayerDataChanged();
}

// -----------------------------------------------------------------
// Name : onPlayerDataChanged
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::onPlayerDataChanged()
{
    getComponent("SpellsButton")->setEnabled(m_pCurrentPlayer->getAvatarsList()->size > 0);
    getComponent("EquipButton")->setEnabled(m_pCurrentPlayer->getAvatarsList()->size > 0);
    getComponent("ShopButton")->setEnabled(true);
    getComponent("CreateShahmahButton")->setEnabled(true);
    getComponent("StatsButton")->setEnabled(true);
    getComponent("DeletePlayer")->setEnabled(true);
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool SelectPlayerAvatarDlg::onClickStart()
{
    if (m_pTextInput != NULL)
    {
        m_pTextInput->getDocument()->doClick("OkButton");
        return true;
    }
    if (getComponent("SpellsButton")->isEnabled())
    {
        doClick("SpellsButton");
        return true;
    }
    return false;
}
