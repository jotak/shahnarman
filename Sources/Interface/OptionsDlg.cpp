#include "OptionsDlg.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiPopup.h"
#include "../GUIClasses/guiEditBox.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../Debug/DebugManager.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "../DeckData/Edition.h"
#include "../Audio/AudioManager.h"

#define CONFIRM_VIDEO_MODE_TIMER    5.0f

// -----------------------------------------------------------------
// Name : OptionsDlg
//  Constructor
// -----------------------------------------------------------------
OptionsDlg::OptionsDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_iSelectedLanguage = 0;

    init("Options",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, iWidth, iHeight, pLocalClient->getDisplay());

    int yPxl = 10;
    int butHeight = (iHeight - 2*yPxl) / 5;

    char str[64] = "";
    i18n->getText("GAME_OPTIONS", str, 64);
    guiButton * pBtn = new guiButton();
    pBtn->init(str, H1_FONT, H1_COLOR,
               -1, BCO_None,
               -1, BCO_Decal,
               pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:Transparent"),
               "GameOptions", 0, yPxl, iWidth, butHeight, pLocalClient->getDisplay());
    addComponent(pBtn);

    yPxl += butHeight;
    i18n->getText("VIDEO_OPTIONS", str, 64);
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(str);
    pBtn->setId("VideoOptions");
    pBtn->moveTo(0, yPxl);
    addComponent(pBtn);

    yPxl += butHeight;
    i18n->getText("AUDIO_OPTIONS", str, 64);
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(str);
    pBtn->setId("AudioOptions");
    pBtn->moveTo(0, yPxl);
    addComponent(pBtn);

    yPxl += 2*butHeight;
    i18n->getText("BACK", str, 64);
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(str);
    pBtn->setId("BackButton");
    pBtn->moveTo(0, yPxl);
    addComponent(pBtn);

    m_pGameOptionsDlg = NULL;
    m_pVideoOptionsDlg = NULL;
    m_pAudioOptionsDlg = NULL;
    m_pConfirmVideoModeDlg = NULL;
    m_fConfirmVideoModeTimer = 0.0f;
}

// -----------------------------------------------------------------
// Name : ~OptionsDlg
//  Destructor
// -----------------------------------------------------------------
OptionsDlg::~OptionsDlg()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy OptionsDlg\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy OptionsDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void OptionsDlg::update(double delta)
{
    if (m_pGameOptionsDlg != NULL)
    {
        guiComponent * pCpnt = m_pGameOptionsDlg->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "OkButton") == 0)
            {
                setEnabled(true);
                onAcceptGameParameters();
                m_pLocalClient->getInterface()->deleteFrame(m_pGameOptionsDlg);
                m_pGameOptionsDlg = NULL;
            }
            else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
            {
                setEnabled(true);
                m_pLocalClient->getInterface()->deleteFrame(m_pGameOptionsDlg);
                m_pGameOptionsDlg = NULL;
            }
            else  // Language combo box button?
            {
                for (int i = 0; i < m_pLocalClient->getClientParameters()->iNbLanguages; i++)
                {
                    if (strcmp(m_pLocalClient->getClientParameters()->sLanguages[i], pCpnt->getId()) == 0)
                    {
                        m_iSelectedLanguage = i;
                        break;
                    }
                }
            }
        }
    }
    if (m_pVideoOptionsDlg != NULL)
    {
        guiComponent * pCpnt = m_pVideoOptionsDlg->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "OkButton") == 0)
            {
                setEnabled(true);
                onAcceptVideoParameters();
                m_pLocalClient->getInterface()->deleteFrame(m_pVideoOptionsDlg);
                m_pVideoOptionsDlg = NULL;
            }
            else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
            {
                setEnabled(true);
                m_pLocalClient->getInterface()->deleteFrame(m_pVideoOptionsDlg);
                m_pVideoOptionsDlg = NULL;
            }
        }
    }
    if (m_pAudioOptionsDlg != NULL)
    {
        guiComponent * pCpnt = m_pAudioOptionsDlg->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "OkButton") == 0)
            {
                setEnabled(true);
                onAcceptAudioParameters();
                m_pLocalClient->getInterface()->deleteFrame(m_pAudioOptionsDlg);
                m_pAudioOptionsDlg = NULL;
            }
            else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
            {
                setEnabled(true);
                m_pLocalClient->getInterface()->deleteFrame(m_pAudioOptionsDlg);
                m_pAudioOptionsDlg = NULL;
            }
        }
    }
    if (m_pConfirmVideoModeDlg != NULL)
    {
        m_fConfirmVideoModeTimer -= delta;
        // Update timer label
        char sText[LABEL_MAX_CHARS];
        char sBuf[LABEL_MAX_CHARS];
        i18n->getText("CLOSES_IN_(d)_SEC", sBuf, LABEL_MAX_CHARS);
        snprintf(sText, LABEL_MAX_CHARS, sBuf, (int) m_fConfirmVideoModeTimer);
        guiLabel * pLbl = (guiLabel*) m_pConfirmVideoModeDlg->getDocument()->getComponent("TimerLabe");
        assert(pLbl != NULL);
        pLbl->setText(sText);

        // Check clicked component
        guiComponent * pCpnt = m_pConfirmVideoModeDlg->getClickedComponent();
        if (pCpnt != NULL && strcmp(pCpnt->getId(), "OkButton") == 0)
        {
            setEnabled(true);
            m_pLocalClient->getInterface()->deleteFrame(m_pConfirmVideoModeDlg);
            m_pConfirmVideoModeDlg = NULL;
        }
        else if ((pCpnt != NULL && strcmp(pCpnt->getId(), "CancelButton") == 0) || m_fConfirmVideoModeTimer <= 0.0f)
        {
            // Timeout or cancel: go in windowed mode for security
            setEnabled(true);
            m_pLocalClient->getInterface()->deleteFrame(m_pConfirmVideoModeDlg);
            m_pConfirmVideoModeDlg = NULL;
            m_pLocalClient->getClientParameters()->fullscreen = false;
            m_pLocalClient->getClientParameters()->saveParameters(m_pLocalClient->getDebug());
            extern void restartGlut();
            restartGlut();
        }
    }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool OptionsDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (strcmp(pCpnt->getId(), "GameOptions") == 0)
    {
        // Show popup
        m_pGameOptionsDlg = guiPopup::createOkCancelPopup("", getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pGameOptionsDlg);
        guiDocument * pDoc = m_pGameOptionsDlg->getDocument();
        int iWidth = 300;
        int yPxl = 4;
        char sText[LABEL_MAX_CHARS] = "";
        guiLabel * pLbl = new guiLabel();
        i18n->getText("LANGUAGE", sText, LABEL_MAX_CHARS);
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 4, yPxl + 5, 0, 0, getDisplay());
        pDoc->addComponent(pLbl);
        guiComboBox * pCombo = guiComboBox::createDefaultComboBox("Language", m_pLocalClient->getInterface(), getDisplay());
        pCombo->moveTo(pLbl->getXPos() + pLbl->getWidth() + 4, yPxl);
        pDoc->addComponent(pCombo);
        // Read available languages
        for (int i = 0; i < m_pLocalClient->getClientParameters()->iNbLanguages; i++)
            pCombo->addString(m_pLocalClient->getClientParameters()->sLanguages[i], m_pLocalClient->getClientParameters()->sLanguages[i]);
        m_iSelectedLanguage = m_pLocalClient->getClientParameters()->language;
        pCombo->setItem(m_iSelectedLanguage);
        // Active editions label
        yPxl += pCombo->getHeight() + 8;
        i18n->getText("AVAILABLE_EDITIONS", sText, LABEL_MAX_CHARS);
        pLbl = new guiLabel();
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, 0, 0, getDisplay());
        pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
        pDoc->addComponent(pLbl);
        // Panel
        yPxl += pLbl->getHeight() + 4;
        guiContainer * pPanel = guiContainer::createDefaultPanel(iWidth - 8, 250, "Pane", getDisplay());
        pPanel->moveTo(4, yPxl);
        pDoc->addComponent(pPanel);
        yPxl = 4;
        int maxWidth = 0;
        Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
        while (pEdition != NULL)
        {
            guiButton * pBtn = guiToggleButton::createDefaultCheckBox("CheckBox", getDisplay());
            pBtn->moveTo(4, yPxl);
            pBtn->setAttachment(pEdition);
            pPanel->getDocument()->addComponent(pBtn);
            char sEditionName[NAME_MAX_CHARS];
            pEdition->findLocalizedElement(sEditionName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            snprintf(sText, LABEL_MAX_CHARS, "%s\n  (md5: %s)", sEditionName, pEdition->getChecksum());
            pLbl = new guiLabel();
            pLbl->init(sText, H2_FONT, H2_COLOR, "", 30, yPxl, 0, 0, getDisplay());
            pPanel->getDocument()->addComponent(pLbl);
            pLbl->setCatchClicks(true);
            pLbl->setComponentOwner(pBtn);
            ((guiToggleButton*)pBtn)->setClickState(pEdition->isActive());
            if (pLbl->getWidth() + pLbl->getXPos() > maxWidth)
                maxWidth = pLbl->getWidth() + pLbl->getXPos();
            yPxl += pLbl->getHeight() + 4;
            pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
        }
        if (maxWidth > pPanel->getDocument()->getWidth())
            pPanel->getDocument()->setWidth(maxWidth);
        // Keep logs for how many turns?
        yPxl = pPanel->getYPos() + pPanel->getHeight() + 4;
        pLbl = new guiLabel();
        pLbl->init(i18n->getText("KEEP_LOGS", sText, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, "", 4, yPxl, 0, 0, getDisplay());
        pDoc->addComponent(pLbl);
        // Keep logs combo
        yPxl += pLbl->getHeight() + 4;
        pCombo = guiComboBox::createDefaultComboBox("KeepLogsCombo", m_pLocalClient->getInterface(), getDisplay());
        pCombo->moveTo(10, yPxl);
        pDoc->addComponent(pCombo);
        // Fill combo
        pCombo->addString(i18n->getText("UNLIMITED(KEEP_LOGS)", sText, LABEL_MAX_CHARS), "KeepLogsComboItem");
        void * pArgs[1];
        for (int i = 10; i <= 50; i+=10)
        {
            pArgs[0] = &i;
            i18n->getText("%$1d_TURNS(KEEP_LOGS)", sText, LABEL_MAX_CHARS, pArgs);
            pCombo->addString(sText, "KeepLogsComboItem");
        }
        if (m_pLocalClient->getClientParameters()->iGameLogsLifetime < 0)
            pCombo->setItem(0);
        else
            pCombo->setItem(m_pLocalClient->getClientParameters()->iGameLogsLifetime / 10);
        // Ok/Cancel buttons
        yPxl += pCombo->getHeight() + 8;
        guiButton * pBtn = (guiButton*) pDoc->getComponent("OkButton");
        pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
        pBtn = (guiButton*) pDoc->getComponent("CancelButton");
        pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);

        pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 4);
        m_pGameOptionsDlg->updateSizeFit();
        m_pGameOptionsDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - m_pGameOptionsDlg->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - m_pGameOptionsDlg->getHeight() / 2);
        setEnabled(false);
    }
    else if (strcmp(pCpnt->getId(), "VideoOptions") == 0)
    {
        // Show popup
        m_pVideoOptionsDlg = guiPopup::createOkCancelPopup("", getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pVideoOptionsDlg);
        guiDocument * pDoc = m_pVideoOptionsDlg->getDocument();
        int iWidth = 300;
        int yPxl = 5;
        guiButton * pBtn = guiToggleButton::createDefaultCheckBox("FullscreenBtn", getDisplay());
        pBtn->moveTo(4, yPxl);
        pDoc->addComponent(pBtn);
        yPxl += 2;
        char sText[LABEL_MAX_CHARS] = "";
        i18n->getText("FULLSCREEN", sText, LABEL_MAX_CHARS);
        guiLabel * pLbl = new guiLabel();
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 30, yPxl, 0, 0, getDisplay());
        pDoc->addComponent(pLbl);
        pLbl->setCatchClicks(true);
        pLbl->setComponentOwner(pBtn);
        ((guiToggleButton*)pBtn)->setClickState(m_pLocalClient->getClientParameters()->fullscreen);
        yPxl += pLbl->getHeight() + 6;
        i18n->getText("SCREEN_RESOLUTION", sText, LABEL_MAX_CHARS);
        pLbl = new guiLabel();
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, iWidth - 8, 0, getDisplay());
        pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
        pDoc->addComponent(pLbl);
        yPxl += pLbl->getHeight() + 5;
        guiComboBox * pBox = guiComboBox::createDefaultComboBox("ResolutionCombo", m_pLocalClient->getInterface(), getDisplay());
        pBox->moveTo(4, yPxl);
        pDoc->addComponent(pBox);
        yPxl += pBox->getHeight() + 8;
        // Fill combo
        CoordsScreen pRes[100];
        int pBpp[100];
        int nbResults = getAvailableDisplayModes(pRes, pBpp, 100);
        for (int i = 0; i < nbResults; i++)
        {
            char sText[LABEL_MAX_CHARS];
            snprintf(sText, LABEL_MAX_CHARS, "%dx%d:%d", pRes[i].x, pRes[i].y, pBpp[i]);
            pBox->addString(sText, sText);
            if (strcmp(sText, m_pLocalClient->getClientParameters()->sGameModeString) == 0)
                pBox->setItem(i);
        }

        pBtn = (guiButton*) pDoc->getComponent("OkButton");
        pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
        pBtn = (guiButton*) pDoc->getComponent("CancelButton");
        pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
        pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 4);
        m_pVideoOptionsDlg->updateSizeFit();
        m_pVideoOptionsDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - m_pVideoOptionsDlg->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - m_pVideoOptionsDlg->getHeight() / 2);
        setEnabled(false);
    }
    else if (strcmp(pCpnt->getId(), "AudioOptions") == 0)
    {
        // Show popup
        m_pAudioOptionsDlg = guiPopup::createOkCancelPopup("", getDisplay());
        m_pLocalClient->getInterface()->registerFrame(m_pAudioOptionsDlg);
        guiDocument * pDoc = m_pAudioOptionsDlg->getDocument();
        int iWidth = 300;
        int yPxl = 5;
        // Sound
        char sText[LABEL_MAX_CHARS] = "";
        i18n->getText("SOUND_VOLUME", sText, LABEL_MAX_CHARS);
        guiLabel * pLbl = new guiLabel();
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 4, yPxl, iWidth - 8, 0, getDisplay());
        pDoc->addComponent(pLbl);
        guiComboBox * pBox = guiComboBox::createDefaultComboBox("SoundCombo", m_pLocalClient->getInterface(), getDisplay());
        pBox->setWidth(100);
        pBox->moveTo(pLbl->getXPos() + pLbl->getWidth() + 4, yPxl);
        pDoc->addComponent(pBox);
        // Fill combo
        for (int i = 0; i <= 10; i++)
        {
            if (i == 0)
                i18n->getText("0_MUTE", sText, LABEL_MAX_CHARS);
            else
                snprintf(sText, LABEL_MAX_CHARS, "%d", i);
            pBox->addString(sText, "");
            if (m_pLocalClient->getClientParameters()->iSoundVolume == i)
                pBox->setItem(i);
        }

        // Music
        yPxl += pBox->getHeight() + 8;
        i18n->getText("MUSIC_VOLUME", sText, LABEL_MAX_CHARS);
        pLbl = new guiLabel();
        pLbl->init(sText, H2_FONT, H2_COLOR, "", 4, yPxl, iWidth - 8, 0, getDisplay());
        pDoc->addComponent(pLbl);
        pBox = guiComboBox::createDefaultComboBox("MusicCombo", m_pLocalClient->getInterface(), getDisplay());
        pBox->setWidth(100);
        pBox->moveTo(pLbl->getXPos() + pLbl->getWidth() + 4, yPxl);
        pDoc->addComponent(pBox);
        // Fill combo
        for (int i = 0; i <= 10; i++)
        {
            if (i == 0)
                i18n->getText("0_MUTE", sText, LABEL_MAX_CHARS);
            else
                snprintf(sText, LABEL_MAX_CHARS, "%d", i);
            pBox->addString(sText, "");
            if (m_pLocalClient->getClientParameters()->iMusicVolume == i)
                pBox->setItem(i);
        }

        yPxl += pBox->getHeight() + 8;
        guiButton * pBtn = (guiButton*) pDoc->getComponent("OkButton");
        pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
        pBtn = (guiButton*) pDoc->getComponent("CancelButton");
        pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
        pDoc->setDimensions(iWidth, yPxl + pBtn->getHeight() + 4);
        m_pAudioOptionsDlg->updateSizeFit();
        m_pAudioOptionsDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - m_pAudioOptionsDlg->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - m_pAudioOptionsDlg->getHeight() / 2);
        setEnabled(false);
    }
    else if (strcmp(pCpnt->getId(), "BackButton") == 0)
    {
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
        return false;
    }
    return true;
}

// -----------------------------------------------------------------
// Name : onAcceptGameParameters
// -----------------------------------------------------------------
void OptionsDlg::onAcceptGameParameters()
{
    // Write active editions file
    char sFile[MAX_PATH];
    FILE * pFile = NULL;
    snprintf(sFile, MAX_PATH, "%sactive.txt", EDITIONS_PATH);
    if (0 != fopen_s(&pFile, sFile, "w"))
    {
        m_pLocalClient->getDebug()->notifyErrorMessage("Error: can't open active.txt file for writing.");
        return;
    }

    // Loop through buttons
    guiContainer * pPanel = (guiContainer*) m_pGameOptionsDlg->getDocument()->getComponent("Pane");
    assert(pPanel != NULL);
    guiComponent * pCpnt = pPanel->getDocument()->getFirstComponent();
    while (pCpnt != NULL)
    {
        if (strcmp(pCpnt->getId(), "CheckBox") == 0)
        {
            Edition * pEdition = (Edition*) pCpnt->getAttachment();
            assert(pEdition != NULL);
            pEdition->deactivate();
            if (((guiToggleButton*)pCpnt)->getClickState())
            {
//        m_pLocalClient->getClientParameters()->resetLocale(m_pLocalClient->getDebug());
                fputs(pEdition->m_sObjectId, pFile);
                fputs("\n", pFile);
                pEdition->activate(m_pLocalClient->getDebug());
            }
        }
        pCpnt = pPanel->getDocument()->getNextComponent();
    }
    fclose(pFile);

    // get language
    if (m_pLocalClient->getClientParameters()->language != m_iSelectedLanguage)
    {
        m_pLocalClient->getClientParameters()->language = m_iSelectedLanguage;
        char sText[LABEL_MAX_CHARS];
        i18n->getText("LANGUAGE_CHANGE_ON_RESTART", sText, LABEL_MAX_CHARS);
        guiPopup * pPopup = guiPopup::createTimedPopup(sText, 5.0f, 250, getDisplay());
        m_pLocalClient->getInterface()->registerFrame(pPopup);
        pPopup->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pPopup->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pPopup->getHeight() / 2);
    }

    // Get keep logs parameter
    guiComboBox * pCombo = (guiComboBox*) m_pGameOptionsDlg->getDocument()->getComponent("KeepLogsCombo");
    int id = pCombo->getSelectedItemId();
    if (id == 0)
        m_pLocalClient->getClientParameters()->iGameLogsLifetime = -1;
    else
        m_pLocalClient->getClientParameters()->iGameLogsLifetime = id * 10;
}

// -----------------------------------------------------------------
// Name : onAcceptVideoParameters
// -----------------------------------------------------------------
void OptionsDlg::onAcceptVideoParameters()
{
    guiToggleButton * pButton = (guiToggleButton*) m_pVideoOptionsDlg->getDocument()->getComponent("FullscreenBtn");
    assert(pButton != NULL);
    m_pLocalClient->getClientParameters()->fullscreen = pButton->getClickState();
    guiComboBox * pCombo = (guiComboBox*) m_pVideoOptionsDlg->getDocument()->getComponent("ResolutionCombo");
    assert(pCombo != NULL);
    wsafecpy(m_pLocalClient->getClientParameters()->sGameModeString, 64, pCombo->getSelectedItem()->getId());
    m_pLocalClient->getClientParameters()->saveParameters(m_pLocalClient->getDebug());
    extern void restartGlut();
    restartGlut();
    if (m_pLocalClient->getClientParameters()->fullscreen)
    {
        // Show confirm popup
        m_fConfirmVideoModeTimer = 15.0f;
        char sText[LABEL_MAX_CHARS];
        i18n->getText("CONFIRM_RESOLUTION", sText, LABEL_MAX_CHARS);
        m_pConfirmVideoModeDlg = guiPopup::createOkCancelPopup(sText, getDisplay());
        char sBuf[LABEL_MAX_CHARS];
        i18n->getText("CLOSES_IN_(d)_SEC", sBuf, LABEL_MAX_CHARS);
        snprintf(sText, LABEL_MAX_CHARS, sBuf, (int) m_fConfirmVideoModeTimer);
        guiLabel * pLbl = new guiLabel();
        pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "TimerLabe", 10, 80, 200, 0, getDisplay());
        m_pConfirmVideoModeDlg->getDocument()->addComponent(pLbl);
        m_pLocalClient->getInterface()->registerFrame(m_pConfirmVideoModeDlg);
        m_pConfirmVideoModeDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - m_pConfirmVideoModeDlg->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - m_pConfirmVideoModeDlg->getHeight() / 2);
        setEnabled(false);
    }
}

// -----------------------------------------------------------------
// Name : onAcceptAudioParameters
// -----------------------------------------------------------------
void OptionsDlg::onAcceptAudioParameters()
{
    guiComboBox * pCombo = (guiComboBox*) m_pAudioOptionsDlg->getDocument()->getComponent("SoundCombo");
    assert(pCombo != NULL);
    m_pLocalClient->getClientParameters()->iSoundVolume = pCombo->getSelectedItemId();
    pCombo = (guiComboBox*) m_pAudioOptionsDlg->getDocument()->getComponent("MusicCombo");
    assert(pCombo != NULL);
    m_pLocalClient->getClientParameters()->iMusicVolume = pCombo->getSelectedItemId();
    m_pLocalClient->getClientParameters()->saveParameters(m_pLocalClient->getDebug());
    AudioManager::getInstance()->updateVolume();
}
