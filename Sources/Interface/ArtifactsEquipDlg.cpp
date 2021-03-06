#include "ArtifactsEquipDlg.h"
#include "BuildDeckDlg.h"
#include "ShopDlg.h"
#include "SelectPlayerAvatarDlg.h"
#include "StartMenuDlg.h"
#include "InterfaceManager.h"
#include "LevelUpDlg.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/AvatarData.h"
#include "../DeckData/Profile.h"
#include "../DeckData/Edition.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiPopup.h"
#include "../Players/Artifact.h"
#include "../Debug/DebugManager.h"

#define SPACING    4
//                                HEAD    BODY    LHAND   RHAND   FOOT
#define ARTIFACT_SCREEN_POS_X   { 17,     411,    456,    49,     437};
#define ARTIFACT_SCREEN_POS_Y   { 18,     28,     228,    450,    542};

// -----------------------------------------------------------------
// Name : ArtifactsEquipDlg
//  Constructor
// -----------------------------------------------------------------
ArtifactsEquipDlg::ArtifactsEquipDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_uDraggingFrom = 0;
    m_pCurrentArtifact = NULL;
    m_pCurrentPlayer = NULL;
    m_pCurrentAvatar = NULL;
    m_pEditAvatarInfosPopup = NULL;
    m_pCurrentEditingAvatar = NULL;
    m_pEditAvatarCaller = NULL;
    m_pConfirmDelete = NULL;

    init("ArtifactsEquipDlg",
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, iWidth, iHeight, m_pLocalClient->getDisplay());

    // Object popup
    m_pObjectPopup = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
    m_pObjectPopup->setWidthFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pObjectPopup->setHeightFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pObjectPopup->setDimensions(200, 250);
    m_pObjectPopup->moveTo(0, 0);
    m_pObjectPopup->setVisible(false);
    m_pLocalClient->getInterface()->registerFrame(m_pObjectPopup);

    // Avatar popup
    m_pAvatarPopup = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
    m_pAvatarPopup->setWidthFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pAvatarPopup->setHeightFitBehavior(FB_FitDocumentToFrameWhenSmaller);
    m_pAvatarPopup->setDimensions(200, 250);
    m_pAvatarPopup->moveTo(m_pLocalClient->getClientParameters()->screenXSize - 200, 0);
    m_pAvatarPopup->setVisible(false);
    m_pLocalClient->getInterface()->registerFrame(m_pAvatarPopup);

    // Banner icon
    int yPxl = 0;
    char sText[LABEL_MAX_CHARS];
    int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("blason1");
    guiButton * pBtn1 = guiButton::createDefaultImageButton(iTex, "Banner", getDisplay());
    pBtn1->moveTo(0, yPxl);
    pBtn1->setTooltipText(i18n->getText("CLICK_TO_CHANGE_BANNER", sText, LABEL_MAX_CHARS));
    addComponent(pBtn1);
    int bannerWidth = pBtn1->getWidth();

    // Level up?
    pBtn1 = guiButton::createDefaultWhiteButton("", 32, 32, "LevelUp", getDisplay());
    pBtn1->moveTo(iWidth - 32, yPxl);
    addComponent(pBtn1);
    pBtn1->attachImage(getDisplay()->getTextureEngine()->loadTexture("levelup"));
    i18n->getText("LEVEL_UP", sText, LABEL_MAX_CHARS);
    pBtn1->setTooltipText(sText);

    // Create top label 2
    guiLabel * pLbl = new guiLabel();
    pLbl->init("_", H2_FONT, H2_COLOR, "TopLabel2", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo(0, 7);
    addComponent(pLbl);

    // Avatars list panel
    m_pAvatarsPanel = guiContainer::createDefaultPanel(iWidth - 2 * (bannerWidth + SPACING), 36, "AvatarsPane", pLocalClient->getDisplay());
    m_pAvatarsPanel->moveTo(bannerWidth + SPACING, yPxl);
    addComponent(m_pAvatarsPanel);

    // Create top label 1
    yPxl += m_pAvatarsPanel->getHeight() + 2 * SPACING;
    pLbl = new guiLabel();
    pLbl->init("_", H1_FONT, H1_COLOR, "TopLabel1", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->setTooltipText(i18n->getText("CLICK_TO_EDIT_NAME_DESCR", sText, LABEL_MAX_CHARS));
    pLbl->setCatchClicks(true);
    pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, yPxl);
    addComponent(pLbl);

    // Button delete
    iTex = pLocalClient->getDisplay()->getTextureEngine()->loadTexture("delete");
    guiButton * pBtn2 = guiButton::createDefaultImageButton(iTex, "DeleteAvatar", pLocalClient->getDisplay());
    pBtn2->moveTo(pLbl->getXPos() + pLbl->getWidth() + 2*SPACING, yPxl + 3);
    pBtn2->setTooltipText(i18n->getText("DELETE_AVATAR", sText, LABEL_MAX_CHARS));
    addComponent(pBtn2);

    int buttonSize = 32;

    // Create label "Filters"
    yPxl += pLbl->getHeight() + 23;
    char str[64];
    i18n->getText("FILTERS", str, 64);
    pLbl = new guiLabel();
    pLbl->init(str, H2_FONT, H2_COLOR, "FiltersLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    addComponent(pLbl);
    int xPxl = SPACING;
    pLbl->moveTo(xPxl, yPxl);

    // Tooltip texts for following buttons
    char sTooltip[128];
    char sInclude[64];
    char sWord[64];
    void * pPhraseArgs[2];
    pPhraseArgs[0] = sInclude;
    pPhraseArgs[1] = sWord;
    i18n->getText("INCLUDE", sInclude, 64);
    xPxl = pLbl->getWidth() + 2 * SPACING;
    yPxl -= 8;

    // Create head filter button
    guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(
                                 pLocalClient->getDisplay()->getTextureEngine()->loadTexture("artifact_head"),
                                 buttonSize, "HeadButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getTextUp("HEAD", sWord, 64);
    i18n->getText("%$1s_ARTIFACTS_FOR_%$2s", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create body filter button
    pBtn = guiToggleButton::createDefaultTexturedToggleButton(
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("artifact_body"),
               buttonSize, "BodyButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getTextUp("BODY", sWord, 64);
    i18n->getText("%$1s_ARTIFACTS_FOR_%$2s", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create left hand filter button
    pBtn = guiToggleButton::createDefaultTexturedToggleButton(
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("artifact_left_hand"),
               buttonSize, "LHandButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getTextUp("LEFT_HAND", sWord, 64);
    i18n->getText("%$1s_ARTIFACTS_FOR_%$2s", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create right hand filter button
    pBtn = guiToggleButton::createDefaultTexturedToggleButton(
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("artifact_right_hand"),
               buttonSize, "RHandButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getTextUp("RIGHT_HAND", sWord, 64);
    i18n->getText("%$1s_ARTIFACTS_FOR_%$2s", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create foot filter button
    pBtn = guiToggleButton::createDefaultTexturedToggleButton(
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("artifact_foot"),
               buttonSize, "FootButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getTextUp("FOOT", sWord, 64);
    i18n->getText("%$1s_ARTIFACTS_FOR_%$2s", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create avatar filter button
    pBtn = guiToggleButton::createDefaultTexturedToggleButton(
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("avatar_filter"),
               buttonSize, "AvatarButton", pLocalClient->getDisplay());
    pBtn->moveTo(xPxl, yPxl);
    pBtn->setClickState(true);
    i18n->getText("%$1s_ARTIFACTS_EQUIPPED_BY_ANOTHER", sTooltip, 128, pPhraseArgs);
    pBtn->setTooltipText(sTooltip);
    addComponent(pBtn);
    xPxl += buttonSize + SPACING;

    // Create button "spells"
    int w = getWidth() / 3;
    pBtn2 = guiButton::createDefaultNormalButton(i18n->getText("SPELLS", str, 64), "SpellsButton", pLocalClient->getDisplay());
    int yPxl2 = getHeight() - pBtn2->getHeight() - SPACING;
    pBtn2->setWidth(w);
    pBtn2->moveTo(0, yPxl2);
    addComponent(pBtn2);

    // Create button "shop"
    pBtn2 = guiButton::createDefaultNormalButton(i18n->getText("SHOP", str, 64), "ShopButton", pLocalClient->getDisplay());
    pBtn2->setWidth(w);
    pBtn2->moveTo(w+1, yPxl2);
    addComponent(pBtn2);

    // Create button "back"
    pBtn2 = guiButton::createDefaultNormalButton(i18n->getText("MAIN_MENU", str, 64), "BackButton", pLocalClient->getDisplay());
    pBtn2->setWidth(w);
    pBtn2->moveTo(2*w+1, yPxl2);
    addComponent(pBtn2);

    int panelWidth = getWidth() / 2 - 2 * SPACING;

    // Create label "Not equiped"
    yPxl += pBtn->getHeight() + 3 * SPACING;
    i18n->getText("NOT_EQUIPPED", str, 64);
    pLbl = new guiLabel();
    pLbl->init(str, H2_FONT, H2_COLOR, "NotEquippedLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo(SPACING, yPxl);
    addComponent(pLbl);

    // Create label "Equiped"
    i18n->getText("EQUIPPED", str, 64);
    pLbl = new guiLabel();
    pLbl->init(str, H2_FONT, H2_COLOR, "EquippedLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
    pLbl->moveTo(SPACING, yPxl);  // Will x-align later
    addComponent(pLbl);

    KeyboardInputEngine * pKeyboardInput = pLocalClient->getInput()->hasKeyboard() ? (KeyboardInputEngine*) pLocalClient->getInput() : NULL;

    // Create panel for artifacts not equipped
    int bottom = yPxl2 - SPACING;
    yPxl += pLbl->getHeight() + 2 * SPACING;
    m_pArtifactsPool = guiContainer::createDefaultPanel(panelWidth, bottom - yPxl, "", pLocalClient->getDisplay());
    m_pArtifactsPool->moveTo(SPACING, yPxl);
    addComponent(m_pArtifactsPool);

    // Create image for equipped artifacts
    xPxl = m_pArtifactsPool->getXPos() + m_pArtifactsPool->getWidth() + 2 * SPACING;
    iTex = pLocalClient->getDisplay()->getTextureEngine()->loadTexture("Artifacts_Bg");
    Texture * pTex = pLocalClient->getDisplay()->getTextureEngine()->getTexture(iTex);
    assert(pTex != NULL);
    double ratio = ((double)(bottom - yPxl)) / (double)(pTex->m_iHeight);
    int width = (int) (ratio * (double)(pTex->m_iWidth));
    m_pPlayerBgImage = new guiImage();
    m_pPlayerBgImage->init(iTex, "AvatarImage", xPxl, yPxl, width, bottom - yPxl, pLocalClient->getDisplay());
    addComponent(m_pPlayerBgImage);
    pLbl->moveTo(xPxl, pLbl->getYPos());  // Move last label (equipped) to align it
    if (xPxl + width > getWidth())
        setWidth(xPxl + width);

    // Highlight image
    m_pHighlightImage = new guiImage();
    m_pHighlightImage->init(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("EmptyWhiteRoundedSquare"), "", 0, 0, (int) (ratio * 128.0f), (int) (ratio * 128.0f), pLocalClient->getDisplay());
    m_pHighlightImage->setDiffuseColor(rgba(1, 1, 1, 0.2f));
    m_pHighlightImage->setVisible(false);
    addComponent(m_pHighlightImage);

    // Artifact images
    int pX[5] = ARTIFACT_SCREEN_POS_X;
    int pY[5] = ARTIFACT_SCREEN_POS_Y;
    for (int i = 0; i < 5; i++)
    {
        m_pArtifactsImages[i] = new guiImage();
        // Dummy texture (it will change)
        m_pArtifactsImages[i]->init(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("spellsdrag"), "EquippedArtifact", m_pPlayerBgImage->getXPos() + (int) (ratio * pX[i]), m_pPlayerBgImage->getYPos() + (int) (ratio * pY[i]), (int) (ratio * 128.0f), (int) (ratio * 128.0f), pLocalClient->getDisplay());
        m_pArtifactsImages[i]->setCatchClicks(true);
        m_pArtifactsImages[i]->setOwner(this);
        m_pArtifactsImages[i]->setVisible(false);
        addComponent(m_pArtifactsImages[i]);
    }

    // Drag image
    m_pDragImage = new guiImage();
    // Dummy texture (it will change)
    m_pDragImage->init(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("spellsdrag"), "", 0, 0, -1, -1, pLocalClient->getDisplay());
    m_pDragImage->setVisible(false);
    addComponent(m_pDragImage);
}

// -----------------------------------------------------------------
// Name : ~ArtifactsEquipDlg
//  Destructor
// -----------------------------------------------------------------
ArtifactsEquipDlg::~ArtifactsEquipDlg()
{
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void ArtifactsEquipDlg::update(double delta)
{
    guiDocument::update(delta);
    if (m_pEditAvatarInfosPopup != NULL && *m_pEditAvatarInfosPopup != NULL)
        m_pLocalClient->getInterface()->getBuildDeckDialog()->updateEditAvatarInfosPopup();

    if (m_pConfirmDelete != NULL)
    {
        guiComponent * pCpnt = m_pConfirmDelete->getClickedComponent();
        if (pCpnt != NULL)
        {
            if (strcmp(pCpnt->getId(), "YesButton") == 0)
            {
                // Remove frame
                m_pLocalClient->getInterface()->deleteFrame(m_pConfirmDelete);
                m_pConfirmDelete = NULL;
                setEnabled(true);

                // Delete avatar
                assert(m_pCurrentAvatar != NULL);
                m_pCurrentPlayer->deleteAvatar(m_pCurrentAvatar);
                if (m_pCurrentPlayer->getAvatarsList()->size > 0)
                    onShow(); // reload content, another avatar will be selected
                else
                    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getSelectPlayerDialog());
            }
            else if (strcmp(pCpnt->getId(), "NoButton") == 0)
            {
                m_pLocalClient->getInterface()->deleteFrame(m_pConfirmDelete);
                m_pConfirmDelete = NULL;
                setEnabled(true);
            }
        }
    }

    switch (m_uDraggingFrom)
    {
    case 3: // un-equip spell(s)
    {
        assert(m_pCurrentAvatar != NULL);
        assert(m_pCurrentArtifact != NULL);
        m_pCurrentArtifact->m_pOwner = NULL;
        m_pCurrentAvatar->m_pEquippedArtifacts[m_pCurrentArtifact->getPosition()] = NULL; // remove artifact
        m_pCurrentPlayer->save();
        reloadArtifacts();
        m_uDraggingFrom = 0;
        return;
    }
    case 4: // equip spell(s)
    {
        assert(m_pCurrentAvatar != NULL);
        assert(m_pCurrentArtifact != NULL);
        if (m_pCurrentAvatar->m_pEquippedArtifacts[m_pCurrentArtifact->getPosition()] != NULL)
            m_pCurrentAvatar->m_pEquippedArtifacts[m_pCurrentArtifact->getPosition()]->m_pOwner = NULL; // remove previous artifact
        m_pCurrentAvatar->m_pEquippedArtifacts[m_pCurrentArtifact->getPosition()] = m_pCurrentArtifact;
        m_pCurrentArtifact->m_pOwner = m_pCurrentPlayer->findAvatar(m_pCurrentAvatar->m_sEdition, m_pCurrentAvatar->m_sObjectId);;
        m_pCurrentPlayer->save();
        reloadArtifacts();
        m_uDraggingFrom = 0;
        return;
    }
    }
    //switch (m_pEquippedSpells->getActionOnSelection())
    //{
    //case ButtonLeft:
    //  m_pEquippedSpells->onFocusLost();
    //  m_pNotEquippedSpells->setFocus();
    //  return;
    //case ButtonStart:
    //  moveSelectedSpells(m_pEquippedSpells, m_pNotEquippedSpells);
    //  return;
    //}
    //switch (m_pNotEquippedSpells->getActionOnSelection())
    //{
    //case ButtonRight:
    //  m_pNotEquippedSpells->onFocusLost();
    //  m_pEquippedSpells->setFocus();
    //  return;
    //case ButtonStart:
    //  moveSelectedSpells(m_pNotEquippedSpells, m_pEquippedSpells);
    //  return;
    //}
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool ArtifactsEquipDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (pEvent->eEvent == Event_Drag && pEvent->eButton == Button1)
    {
        if (m_uDraggingFrom == 0)
        {
            if (strcmp(pCpnt->getId(), "EquippedArtifact") == 0)
            {
                int iTex = ((guiImage*)pCpnt)->getImageTexture();
                m_pDragImage->setImageTexture(iTex);
                m_pHighlightImage->setXPos(m_pArtifactsImages[((Artifact*)(pCpnt->getAttachment()))->getPosition()]->getXPos());
                m_pHighlightImage->setYPos(m_pArtifactsImages[((Artifact*)(pCpnt->getAttachment()))->getPosition()]->getYPos());
                m_uDraggingFrom = 1;
            }
            else if (strcmp(pCpnt->getId(), "ArtifactPoolImage") == 0)
            {
                int iTex = ((guiImage*)pCpnt)->getImageTexture();
                m_pDragImage->setImageTexture(iTex);
                m_pHighlightImage->setXPos(m_pArtifactsImages[((Artifact*)(pCpnt->getAttachment()))->getPosition()]->getXPos());
                m_pHighlightImage->setYPos(m_pArtifactsImages[((Artifact*)(pCpnt->getAttachment()))->getPosition()]->getYPos());
                m_uDraggingFrom = 2;
            }
        }
        m_pDragImage->setVisible(m_uDraggingFrom > 0 && pEvent->dragDistance > 40);
        m_pHighlightImage->setVisible(m_pDragImage->isVisible());
        CoordsScreen csrel;
        if (m_uDraggingFrom == 1)
            csrel = CoordsScreen(0, 0);
        else if (m_uDraggingFrom == 2)
            csrel = CoordsScreen(m_pArtifactsPool->getXPos() + m_pArtifactsPool->getDocument()->getXPos(), m_pArtifactsPool->getYPos() + m_pArtifactsPool->getDocument()->getYPos());
        m_pDragImage->moveTo(csrel.x + pEvent->xPos - pEvent->xOffset, csrel.y + pEvent->yPos - pEvent->yOffset);
    }
    else if (pEvent->eEvent == Event_Up)
    {
        if (strcmp(pCpnt->getId(), "BackButton") == 0)
        {
            m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
            return false;
        }
        else if (strcmp(pCpnt->getId(), "SpellsButton") == 0)
        {
            m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getBuildDeckDialog());
            return false;
        }
        else if (strcmp(pCpnt->getId(), "ShopButton") == 0)
        {
            m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getShopDialog());
            return false;
        }
        else if (strcmp(pCpnt->getId(), "LevelUp") == 0)
        {
            m_pLocalClient->getInterface()->getLevelUpDialog()->doAvatarLevelUp(m_pCurrentAvatar, this, false);
            return false;
        }
        else if (strcmp(pCpnt->getId(), "Banner") == 0)
        {
            m_pCurrentAvatar->m_uBanner = (m_pCurrentAvatar->m_uBanner + 1) % NB_BANNERS;
            guiButton * pBtn = (guiButton*) pCpnt;
            assert(pBtn != NULL);
            char sBanner[MAX_PATH] = "";
            m_pCurrentAvatar->getBanner(sBanner, MAX_PATH);
            pBtn->setNormalTexture(getDisplay()->getTextureEngine()->loadTexture(sBanner));
            m_pCurrentPlayer->save();
        }
        else if (strcmp(pCpnt->getId(), "HeadButton") == 0
                 || strcmp(pCpnt->getId(), "BodyButton") == 0
                 || strcmp(pCpnt->getId(), "LHandButton") == 0
                 || strcmp(pCpnt->getId(), "RHandButton") == 0
                 || strcmp(pCpnt->getId(), "FootButton") == 0
                 || strcmp(pCpnt->getId(), "AvatarButton") == 0
                 || strcmp(pCpnt->getId(), "EditionButton") == 0)
        {
            reloadArtifacts();
        }
        else if (strcmp(pCpnt->getId(), "AvatarListButton") == 0)
        {
            guiComponent * pCpnt2 = m_pAvatarsPanel->getDocument()->getFirstComponent();
            while (pCpnt2 != NULL)
            {
                if (pCpnt2 != pCpnt)
                    ((guiToggleButton*)pCpnt2)->setClickState(false);
                else
                    ((guiToggleButton*)pCpnt2)->setClickState(true);
                pCpnt2 = m_pAvatarsPanel->getDocument()->getNextComponent();
            }
            m_pCurrentAvatar = (AvatarData*) pCpnt->getAttachment();
            assert(m_pCurrentAvatar != NULL);
            onAvatarSelected();
        }
        else if (strcmp(pCpnt->getId(), "DeleteAvatar") == 0)
        {
            // Raise confirm popup
            setEnabled(false);
            char str[128];
            m_pConfirmDelete = guiPopup::createYesNoPopup(i18n->getText("REALLY_DELETE_AVATAR", str, 128), m_pLocalClient->getDisplay());
            m_pLocalClient->getInterface()->registerFrame(m_pConfirmDelete);
            m_pConfirmDelete->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pConfirmDelete->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pConfirmDelete->getHeight()) / 2);
        }
        else
        {
            switch (m_uDraggingFrom)
            {
            case 1: // un-equip artifact
            {
                if (m_pArtifactsPool->isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset))
                    m_uDraggingFrom = 3;
                else
                    m_uDraggingFrom = 0;
                break;
            }
            case 2: // equip artifact
            {
                if (m_pPlayerBgImage->isAt(pEvent->xPos + m_pArtifactsPool->getXPos() + m_pArtifactsPool->getDocument()->getXPos() - pEvent->xOffset, pEvent->yPos + m_pArtifactsPool->getYPos() + m_pArtifactsPool->getDocument()->getYPos() - pEvent->yOffset))
                    m_uDraggingFrom = 4;
                else
                    m_uDraggingFrom = 0;
                break;
            }
            }
            m_pDragImage->setVisible(false);
            m_pHighlightImage->setVisible(false);
        }
    }
    else if (pEvent->eEvent == Event_Down)
    {
        if (strcmp(pCpnt->getId(), "TopLabel1") == 0)
        {
            assert(m_pCurrentAvatar != NULL);
            m_pEditAvatarInfosPopup = m_pLocalClient->getInterface()->getBuildDeckDialog()->showEditAvatarInfosPopup(m_pCurrentAvatar, this);
        }
        else if (strcmp(pCpnt->getId(), "EquippedArtifact") == 0)
            onArtifactSelected(pCpnt->getAttachment());
        else if (strcmp(pCpnt->getId(), "ArtifactPoolImage") == 0)
            onArtifactSelected(pCpnt->getAttachment());
    }
    return true;
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void ArtifactsEquipDlg::onShow()
{
    loadEditionButtons();
    m_pCurrentPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();

    // Fill avatars combo list
    assert(m_pCurrentPlayer != NULL);
    m_pCurrentAvatar = NULL;
    m_pAvatarsPanel->getDocument()->deleteAllComponents();
    int xPxl = 0;
    int buttonSize = m_pAvatarsPanel->getInnerHeight();
    AvatarData * pAvatar = (AvatarData*) m_pCurrentPlayer->getAvatarsList()->getFirst(0);
    while (pAvatar != NULL)
    {
        int iTex = getDisplay()->getTextureEngine()->loadTexture(pAvatar->m_sTextureFilename);
        guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(iTex, buttonSize, "AvatarListButton", getDisplay());
        pBtn->moveTo(xPxl, 0);
        pBtn->setAttachment(pAvatar);
        pBtn->setTooltipText(pAvatar->m_sCustomName);
        pBtn->setOwner(this);
        m_pAvatarsPanel->getDocument()->addComponent(pBtn);
        if (m_pCurrentAvatar == NULL)
        {
            m_pCurrentAvatar = pAvatar;
            pBtn->setClickState(true);
        }
        xPxl += buttonSize + 2;
        pAvatar = (AvatarData*) m_pCurrentPlayer->getAvatarsList()->getNext(0);
    }

    assert(m_pCurrentAvatar != NULL);
    onAvatarSelected();

    m_pObjectPopup->setVisible(true);
    m_pAvatarPopup->setVisible(true);
}

// -----------------------------------------------------------------
// Name : onHide
// -----------------------------------------------------------------
void ArtifactsEquipDlg::onHide()
{
    m_pObjectPopup->setVisible(false);
    m_pAvatarPopup->setVisible(false);
}

// -----------------------------------------------------------------
// Name : reloadArtifacts
// -----------------------------------------------------------------
void ArtifactsEquipDlg::reloadArtifacts()
{
    if (m_pCurrentPlayer == NULL || m_pCurrentAvatar == NULL)
        return;

    bool bPosition[5];
    bPosition[ARTIFACT_POSITION_HEAD] = ((guiToggleButton*)getComponent("HeadButton"))->getClickState();
    bPosition[ARTIFACT_POSITION_BODY] = ((guiToggleButton*)getComponent("BodyButton"))->getClickState();
    bPosition[ARTIFACT_POSITION_LHAND] = ((guiToggleButton*)getComponent("LHandButton"))->getClickState();
    bPosition[ARTIFACT_POSITION_RHAND] = ((guiToggleButton*)getComponent("RHandButton"))->getClickState();
    bPosition[ARTIFACT_POSITION_FOOT] = ((guiToggleButton*)getComponent("FootButton"))->getClickState();
    bool bAvatar = ((guiToggleButton*)getComponent("AvatarButton"))->getClickState();

    // Update spells lists
    m_pArtifactsPool->getDocument()->deleteAllComponents();
    for (int i = 0; i < 5; i++)
        m_pArtifactsImages[i]->setVisible(false);
    int xPxl = 0;
    int yPxl = 0;
    int btnSize = 64;
    m_pArtifactsPool->getDocument()->setHeight(yPxl + btnSize + SPACING);
    Artifact * pArtifact = (Artifact*) m_pCurrentPlayer->getArtifactsList()->getFirst(0);
    while (pArtifact != NULL)
    {
        bool bOk = bPosition[pArtifact->getPosition()];
        // Now check for edition filters
        if (bOk)
        {
            guiComponent * pCpnt = getFirstComponent();
            while (pCpnt != NULL)
            {
                if (strcmp(pCpnt->getId(), "EditionButton") == 0)
                {
                    if (((guiToggleButton*)pCpnt)->getClickState() == false && strcmp(pArtifact->getEdition(), ((Edition*)(pCpnt->getAttachment()))->m_sObjectId) == 0)
                    {
                        bOk = false;
                        break;
                    }
                }
                pCpnt = getNextComponent();
            }
        }
        if (bOk)
        {
            AvatarData * pOwner = pArtifact->m_pOwner;
            char sText[LABEL_MAX_CHARS];
            pArtifact->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            if (pOwner != NULL && strcmp(m_pCurrentAvatar->m_sEdition, pOwner->m_sEdition) == 0  // equipped by current avatar
                    && strcmp(m_pCurrentAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
            {
                int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(pArtifact->getTexture());
                guiImage * pImg = m_pArtifactsImages[pArtifact->getPosition()];
                pImg->setImageTexture(iTex);
                pImg->setAttachment(pArtifact);
                pImg->setTooltipText(sText);
                pImg->setVisible(true);
                m_pCurrentAvatar->m_pEquippedArtifacts[pArtifact->getPosition()] = pArtifact;
            }
            else
            {
                guiImage * pImg = NULL;
                int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(pArtifact->getTexture());
                if (pOwner == NULL) // not equipped
                {
                    pImg = new guiImage();
                }
                else if (bAvatar)   // equipped by another avatar (and we want to see it)
                {
                    char sText2[LABEL_MAX_CHARS];
                    snprintf(sText2, LABEL_MAX_CHARS, "%s (%s)", sText, pOwner->m_sCustomName);
                    wsafecpy(sText, LABEL_MAX_CHARS, sText2);
                    pImg = new guiImage();
                    pImg->setDiffuseColor(rgba(0.7f, 0.7f, 0.7f, 1));
                }
                if (pImg != NULL)
                {
                    pImg->init(iTex, "ArtifactPoolImage", xPxl, yPxl, btnSize, btnSize, getDisplay());
                    m_pArtifactsPool->getDocument()->addComponent(pImg);
                    pImg->setAttachment(pArtifact);
                    pImg->setTooltipText(sText);
                    pImg->setCatchClicks(true);
                    pImg->setOwner(this);
                    xPxl += btnSize + SPACING;
                    if (xPxl + btnSize > m_pArtifactsPool->getInnerWidth())
                    {
                        xPxl = 0;
                        yPxl += btnSize + SPACING;
                        m_pArtifactsPool->getDocument()->setHeight(yPxl + btnSize + SPACING);
                    }
                }
            }
        }
        pArtifact = (Artifact*) m_pCurrentPlayer->getArtifactsList()->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : onArtifactSelected
// -----------------------------------------------------------------
void ArtifactsEquipDlg::onArtifactSelected(BaseObject * pObj)
{
    if (pObj == NULL)
    {
        if (m_pCurrentArtifact != NULL)
        {
            m_pObjectPopup->getDocument()->deleteAllComponents();
            m_pCurrentArtifact = NULL;
        }
        return;
    }

    if (m_pCurrentArtifact == (Artifact*) pObj)
        return;

    m_pObjectPopup->getDocument()->deleteAllComponents();
    m_pCurrentArtifact = (Artifact*) pObj;

    // Write title (artifact name)
    int xPxl = SPACING;
    int yPxl = SPACING;
    char sText[LABEL_MAX_CHARS];
    m_pCurrentArtifact->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
    guiLabel * pLbl = new guiLabel();
    pLbl->init(sText, H2_FONT, H2_COLOR, "TitleLabe", xPxl, yPxl, m_pObjectPopup->getInnerWidth() - xPxl, 0, getDisplay());
    m_pObjectPopup->getDocument()->addComponent(pLbl);

    // Add image
    yPxl += pLbl->getHeight() + SPACING;
    int iTex = getDisplay()->getTextureEngine()->loadTexture(m_pCurrentArtifact->getTexture());
    guiImage * pImg = new guiImage();
    pImg->init(iTex, "Image", xPxl, yPxl, 64, 64, getDisplay());
    m_pObjectPopup->getDocument()->addComponent(pImg);

    // Write desciption
    xPxl = SPACING;
    yPxl = pImg->getYPos() + pImg->getHeight() + SPACING;
    m_pCurrentArtifact->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "DescLabe", xPxl, yPxl, m_pObjectPopup->getInnerWidth() - xPxl, 0, getDisplay());
    m_pObjectPopup->getDocument()->addComponent(pLbl);
    if (m_pObjectPopup->getDocument()->getHeight() < pLbl->getYPos() + pLbl->getHeight())
        m_pObjectPopup->getDocument()->setHeight(pLbl->getYPos() + pLbl->getHeight());
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool ArtifactsEquipDlg::onClickStart()
{
    doClick("BackButton");
    return true;
}

// -----------------------------------------------------------------
// Name : loadEditionButtons
// -----------------------------------------------------------------
void ArtifactsEquipDlg::loadEditionButtons()
{
    // Delete any existing edition button
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
        if (strcmp(pCpnt->getId(), "EditionButton") == 0)
            pCpnt = deleteCurrentComponent(true);
        else
            pCpnt = getNextComponent();
    }

    pCpnt = getComponent("AvatarButton");
    assert(pCpnt != NULL);

    // Tooltip texts for following buttons
    char sTooltip[128];
    char sInclude[64];
    char sWord[64];
    void * pPhraseArgs[2] = { sInclude, sWord };
    i18n->getText("INCLUDE", sInclude, 64);
    int xPxl = pCpnt->getXPos() + pCpnt->getWidth() + SPACING;
    int yPxl = pCpnt->getYPos();
    int buttonSize = pCpnt->getHeight();

    Edition * pEd = m_pLocalClient->getDataFactory()->getFirstEdition();
    while (pEd != NULL)
    {
        char sFile[MAX_PATH];
        snprintf(sFile, MAX_PATH, "%s/logo", pEd->m_sObjectId);
        // Create edition filter button
        guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(
                                     m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(sFile),
                                     buttonSize, "EditionButton", m_pLocalClient->getDisplay());
        pBtn->moveTo(xPxl, yPxl);
        pBtn->setClickState(true);
        pEd->findLocalizedElement(sWord, 64, i18n->getCurrentLanguageName(), "name");
        i18n->getText("%$1s_SPELLS_FROM_EDITION_%$2s", sTooltip, 128, pPhraseArgs);
        pBtn->setTooltipText(sTooltip);
        pBtn->setAttachment(pEd);
        addComponent(pBtn);
        xPxl += buttonSize + SPACING;
        pEd = m_pLocalClient->getDataFactory()->getNextEdition();
    }
}

// -----------------------------------------------------------------
// Name : onAvatarSelected
// -----------------------------------------------------------------
void ArtifactsEquipDlg::onAvatarSelected()
{
    getComponent("LevelUp")->setVisible(m_pCurrentAvatar->isLevelUp());
    guiButton * pBtn = (guiButton*) getComponent("Banner");
    assert(pBtn != NULL);
    char sBanner[MAX_PATH] = "";
    m_pCurrentAvatar->getBanner(sBanner, MAX_PATH);
    pBtn->setNormalTexture(getDisplay()->getTextureEngine()->loadTexture(sBanner));

    // Update top label 1
    guiLabel * pLbl = (guiLabel*) getComponent("TopLabel1");
    char str[NAME_MAX_CHARS];
    pLbl->setText(m_pCurrentAvatar->m_sCustomName);
    pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, pLbl->getYPos());

    // Update delete avatar button
    guiComponent * pCpnt = getComponent("DeleteAvatar");
    pCpnt->moveTo(pLbl->getXPos() + pLbl->getWidth() + 2*SPACING, pLbl->getYPos() + 3);

    // Update top label 2
    pLbl = (guiLabel*) getComponent("TopLabel2");
    char sbuf[NAME_MAX_CHARS];
    i18n->getText("%s_AVATARS", sbuf, NAME_MAX_CHARS);
    snprintf(str, NAME_MAX_CHARS, sbuf, m_pCurrentPlayer->getName());
    pLbl->setText(str);
    pLbl->moveTo(getWidth() / 2 - pLbl->getWidth() / 2, pLbl->getYPos());

    reloadArtifacts();

    // Update avatar popup
    m_pAvatarPopup->getDocument()->deleteAllComponents();

    char sInfos[LABEL_MAX_CHARS];
    m_pCurrentAvatar->getInfos(sInfos, LABEL_MAX_CHARS, "\n", false, NULL, true, true, true, false);

    int iDocWidth = m_pAvatarPopup->getInnerWidth();
    int iImageSize = 64;

    // Write title (Avatar name)
    int yPxl = 5;
    pLbl = new guiLabel();
    pLbl->init(m_pCurrentAvatar->m_sCustomName, H2_FONT, H2_COLOR, "TitleLabe", 0, 0, 0, 0, getDisplay());
    pLbl->moveTo((iDocWidth - pLbl->getWidth()) / 2, yPxl);
    m_pAvatarPopup->getDocument()->addComponent(pLbl);

    // Add image
    yPxl += pLbl->getHeight() + 5;
    int iTex = getDisplay()->getTextureEngine()->loadTexture(m_pCurrentAvatar->m_sTextureFilename);
    guiImage * pImg = new guiImage();
    pImg->init(iTex, "Image", 5, yPxl, iImageSize, iImageSize, getDisplay());
    m_pAvatarPopup->getDocument()->addComponent(pImg);

    // Add label for characteristics
    pLbl = new guiLabel();
    pLbl->init(sInfos, TEXT_FONT, TEXT_COLOR, "CharacsLabe", 15 + iImageSize, yPxl, iDocWidth - 20 - iImageSize, 0, getDisplay());
    m_pAvatarPopup->getDocument()->addComponent(pLbl);

    char sText[LABEL_MAX_CHARS];
    // Count number of spells equipped
    int nbSpells = 0;
    Profile::SpellData * pObj2 = (Profile::SpellData*) m_pCurrentPlayer->getSpellsList()->getFirst(0);
    while (pObj2 != NULL)
    {
        AvatarData * pOwner = pObj2->m_pOwner;
        if (pOwner != NULL && strcmp(m_pCurrentAvatar->m_sEdition, pOwner->m_sEdition) == 0
                && strcmp(m_pCurrentAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
            nbSpells++;
        pObj2 = (Profile::SpellData*) m_pCurrentPlayer->getSpellsList()->getNext(0);
    }
    yPxl += 5 + max(pLbl->getHeight(), iImageSize);
    char sBuf[64];
    char sNbSpells[128];
    i18n->getText("SPELLS_(d)", sBuf, 64);
    snprintf(sNbSpells, 128, sBuf, nbSpells);
    pLbl = new guiLabel();
    pLbl->init(sNbSpells, TEXT_FONT, TEXT_COLOR, "NbSpells", 5, yPxl, iDocWidth - 10, 0, getDisplay());
    m_pAvatarPopup->getDocument()->addComponent(pLbl);

    // XP
    void * pArgs[2];
    int xp = m_pCurrentAvatar->m_uXP;
    int next = m_pCurrentAvatar->getNextLevelXP();
    pArgs[0] = &xp;
    pArgs[1] = &next;
    i18n->getText("TOTAL_XP(d1)_NEXT(d2)", sText, LABEL_MAX_CHARS, pArgs);
    yPxl += 3 + pLbl->getHeight();
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 5, yPxl, iDocWidth - 10, 0, getDisplay());
    m_pAvatarPopup->getDocument()->addComponent(pLbl);

    // Add description
    yPxl += 10 + pLbl->getHeight();
    pLbl = new guiLabel();
    pLbl->init(m_pCurrentAvatar->m_sCustomDescription, TEXT_FONT, TEXT_COLOR, "DescLabe", 5, yPxl, iDocWidth - 10, 0, getDisplay());
    m_pAvatarPopup->getDocument()->addComponent(pLbl);
    m_pAvatarPopup->getDocument()->setHeight(yPxl + pLbl->getHeight());
}
