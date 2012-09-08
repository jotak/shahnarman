#include "LevelUpDlg.h"
#include "InterfaceManager.h"
#include "BuildDeckDlg.h"
#include "../LocalClient.h"
#include "../GUIClasses/guiFrame.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiComboBox.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/AvatarData.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Ethnicity.h"
#include "../DeckData/Profile.h"

#define SPACING           5

// -----------------------------------------------------------------
// Name : LevelUpDlg
//  Constructor
// -----------------------------------------------------------------
LevelUpDlg::LevelUpDlg(LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_pCurrentAvatar = NULL;
    m_pCaller = NULL;
    m_pEditAvatarInfosPopup = NULL;
    m_pSelectedElement = NULL;
    m_iSpecialLevel = -1;

    int iWidth = 675; // 4 * (TREE_WIDTH + SPACING) + FIRSTCOLUMN_WIDTH + 2 * SPACING
    init(
        "LevelUpDoc",
        pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
        0, 0, iWidth, 1, pLocalClient->getDisplay());
}

// -----------------------------------------------------------------
// Name : ~LevelUpDlg
//  Destructor
// -----------------------------------------------------------------
LevelUpDlg::~LevelUpDlg()
{
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void LevelUpDlg::update(double delta)
{
    guiDocument::update(delta);
    if (m_pEditAvatarInfosPopup != NULL && *m_pEditAvatarInfosPopup != NULL)
        m_pLocalClient->getInterface()->getBuildDeckDialog()->updateEditAvatarInfosPopup();
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool LevelUpDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    if (strcmp(pCpnt->getId(), "OkButton") == 0)
    {
        // Update avatar progression
        assert(m_pCurrentAvatar != NULL);
        if (m_pSelectedElement != NULL)
        {
            int iTree = -1;
            // find selected tree
            for (int i = 0; i < NB_PROGRESSION_TREES; i++)
            {
                if (strcmp(m_pSelectedElement->m_pTree->m_sObjectId, m_pCurrentAvatar->m_pProgression[i].sTreeName) == 0)
                {
                    iTree = i;
                    break;
                }
            }
            assert(m_pCurrentAvatar->m_pOwner != NULL);
            m_pCurrentAvatar->m_pOwner->applyAvatarProgression(m_pCurrentAvatar, iTree, m_pSelectedElement);
            // Hide frame
            m_pLocalClient->getInterface()->setUniqueDialog(m_pCaller);
            return false;
        }
        else
        {
            // Should be opening a tree
            guiComboBox * pBox = (guiComboBox*) getComponent("OpenTreeCombo");
            assert(pBox != NULL);
            guiButton * pBtn = pBox->getSelectedItem();
            assert(pBtn != NULL);
            assert(m_pCurrentAvatar->m_pOwner != NULL);
            m_pCurrentAvatar->m_pOwner->openAvatarProgressionTree(m_pCurrentAvatar, (u8)m_iSpecialLevel, (ProgressionTree*) pBtn->getAttachment());
            // Hide frame
            m_pLocalClient->getInterface()->setUniqueDialog(m_pCaller);
            return false;
        }
    }
    else if (strcmp(pCpnt->getId(), "DoItLaterButton") == 0)
    {
        // Hide frame
        m_pLocalClient->getInterface()->setUniqueDialog(m_pCaller);
        return false;
    }
    else if (strcmp(pCpnt->getId(), "OpenTreeButton") == 0)
    {
        getComponent("OkButton")->setEnabled(true);
    }
    else if (strcmp(pCpnt->getId(), "ChoiceButton") == 0)
    {
        if (((guiToggleButton*)pCpnt)->getClickState())
        {
            m_pSelectedElement = (ProgressionElement*) pCpnt->getAttachment();
            assert(m_pSelectedElement != NULL);
            getComponent("OkButton")->setEnabled(true);
            guiComponent * pCpnt2 = getFirstComponent();
            while (pCpnt2 != NULL)
            {
                if (strcmp(pCpnt2->getId(), "ChoiceButton") == 0 && pCpnt2 != pCpnt && pCpnt2->isEnabled())
                    ((guiToggleButton*)pCpnt2)->setClickState(false);
                pCpnt2 = getNextComponent();
            }
        }
        else
        {
            m_pSelectedElement = NULL;
            getComponent("OkButton")->setEnabled(false);
        }
    }
    return true;
}

// -----------------------------------------------------------------
// Name : onHide
// -----------------------------------------------------------------
void LevelUpDlg::onHide()
{
    m_pCurrentAvatar = NULL;
}

// -----------------------------------------------------------------
// Name : getChosenElementAtLevel
// -----------------------------------------------------------------
ProgressionElement * LevelUpDlg::getChosenElementAtLevel(ProgressionTree * pTree, int iTree, int iLevel)
{
    assert(m_pCurrentAvatar != NULL);
    ProgressionElement * pElt = (ProgressionElement*) pTree->m_pElements[iLevel]->getFirst(0);
    while (pElt != NULL)
    {
        if (strcmp(m_pCurrentAvatar->m_pProgression[iTree].sElements[iLevel], pElt->m_sObjectId) == 0)
            return pElt;
        pElt = (ProgressionElement*) pTree->m_pElements[iLevel]->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : addChoiceButton
//  uState: 0 = not chosen, 1 = chosen, 2 = eligible
// -----------------------------------------------------------------
void LevelUpDlg::addChoiceButton(int xPxl, int yPxl, int btnSize, ProgressionElement * pElt, u8 uState)
{
    assert(m_pCurrentAvatar != NULL);
    char sText[LABEL_MAX_CHARS];
    s32 iSelTex = m_pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:Selector");
    // Choice
    int iTex = getDisplay()->getTextureEngine()->loadTexture(pElt->m_sTexture);
    guiToggleButton * pBtn = new guiToggleButton();
    pBtn->init("", TEXT_FONT, TEXT_COLOR, iSelTex, BCO_AddTex, -1, BCO_None, iTex, "ChoiceButton", xPxl, yPxl, btnSize, btnSize, getDisplay());
    pBtn->setAttachment(pElt);
    pBtn->setTooltipText(pElt->getDescription(sText, LABEL_MAX_CHARS, m_pLocalClient));
    if (uState != 2)
    {
        if (uState == 0)
            pBtn->setDiffuseColor(rgb(0.5f, 0.5f, 0.5f));
        else
            pBtn->setClickState(true);
        pBtn->setEnabled(false);
    }
    addComponent(pBtn);
}

// -----------------------------------------------------------------
// Name : setCurrentAvatar
// -----------------------------------------------------------------
void LevelUpDlg::setCurrentAvatar(AvatarData * pAvatar)
{
    m_pCurrentAvatar = pAvatar;
    short iSpecialLevels[MAX_LEVELS] = { 0, 1, -1, 2, -1, -1, 3, -1, -1, -1, -1, -1, -1, -1, -1 };
    deleteAllComponents();

    // Init position for components
    int iWidth = getWidth();
    int yPxl = 10;

    // Top label
    char sText[LABEL_MAX_CHARS];
    i18n->getText("LEVEL_UP", sText, LABEL_MAX_CHARS);
    guiLabel * pLbl = new guiLabel();
    pLbl->init(sText, H1_FONT, H1_COLOR, "TopLabe", 0, 0, iWidth - 10, 0, m_pLocalClient->getDisplay());
    pLbl->moveTo((iWidth - pLbl->getWidth()) / 2, yPxl);
    addComponent(pLbl);

    // Text
    yPxl += pLbl->getHeight() + 15;
    u16 uLevel = pAvatar->getRealLevel() + 1;
    assert(uLevel > 1);
    char sBuf[256];
    i18n->getText("(s)_LEVELED_UP_AND_IS_(d)", sBuf, 256);
    snprintf(sText, LABEL_MAX_CHARS, sBuf, pAvatar->m_sCustomName, (int)uLevel);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "CenterLabe", 0, 0, iWidth - 10, 0, m_pLocalClient->getDisplay());
    pLbl->moveTo(5, yPxl);
    addComponent(pLbl);

    Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(pAvatar->m_sEdition);
    assert(pEdition != NULL);
    m_iSpecialLevel = iSpecialLevels[uLevel-1];

    if (m_iSpecialLevel >= 0)
    {
        u8 uTreeType = -1;
        char sPhraseKey[128] = "";
        switch (m_iSpecialLevel)
        {
        case 1:
            uTreeType = PROGRESSION_MAGIC;
            wsafecpy(sPhraseKey, 128, "CHOOSE_MAGIC_PATH");
            break;
        case 2:
        case 3:
            uTreeType = PROGRESSION_TRAIT;
            wsafecpy(sPhraseKey, 128, "CHOOSE_CHARACTER_TRAIT");
            break;
        }
        assert(uTreeType >= 0);

        // Text
        i18n->getText(sPhraseKey, sText, LABEL_MAX_CHARS);
        yPxl += pLbl->getHeight() + 2 * SPACING;
        pLbl = new guiLabel();
        pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", SPACING, yPxl, iWidth - 2 * SPACING, 0, getDisplay());
        addComponent(pLbl);

        // Open tree combo
        yPxl += pLbl->getHeight() + SPACING;
        guiComboBox * pCombo = guiComboBox::createDefaultComboBox("OpenTreeCombo", m_pLocalClient->getInterface(), getDisplay());
        pCombo->moveTo(SPACING, yPxl);
        addComponent(pCombo);

        ObjectList * pTrees = new ObjectList(false);
        pEdition->getAllTreesByType(pTrees, uTreeType);
        ProgressionTree * pTree = (ProgressionTree*) pTrees->getFirst(0);
        while (pTree != NULL)
        {
            pTree->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            guiButton * pBtn = pCombo->addString(sText, "OpenTreeButton");
            pBtn->setAttachment(pTree);
            pTree->getDescription(sText, LABEL_MAX_CHARS, m_pLocalClient);
            pBtn->setTooltipText(sText);
            pTree = (ProgressionTree*) pTrees->getNext(0);
        }
        delete pTrees;
        yPxl += pCombo->getHeight() + SPACING;
    }
    else
    {
        int xTree = SPACING;
        int btnSize = 64;
        int treeWidth = 2 * btnSize + btnSize / 3;
        int treeTop = yPxl + pLbl->getHeight() + 2 * SPACING;

        // Trees
        for (int i = 0; i < NB_PROGRESSION_TREES; i++)
        {
            if (strcmp(pAvatar->m_pProgression[i].sTreeName, "") == 0)
                continue;

            ProgressionTree * pTree = pEdition->findProgressionTree(pAvatar->m_pProgression[i].sTreeName);
            assert(pTree != NULL);

            // Top label
            yPxl = treeTop;
            pTree->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            pLbl = new guiLabel();
            pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, 0, 0, getDisplay());
            pLbl->moveTo(xTree + treeWidth / 2 - pLbl->getWidth() / 2, yPxl);
            addComponent(pLbl);

            // Choices
            s32 iTexChoice = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("progression_choice");
            s32 iTexChoiceLeft = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("progression_choice_");
            s32 iTexChoiceRight = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("progression_choice_r");
            int iChoiceWidth = m_pLocalClient->getDisplay()->getTextureEngine()->getTexture(iTexChoice)->m_iWidth;  // image is centered on texture, so we take its width
            bool bLastChosenLeft = true;

            // branch image
            yPxl += pLbl->getHeight() + SPACING;
            guiImage * pImg = new guiImage();
            pImg->init(iTexChoice, "", xTree + treeWidth / 2 - iChoiceWidth / 2, yPxl, -1, -1, getDisplay());
            addComponent(pImg);

            // Get tree root choices
            yPxl += pImg->getHeight();
            int xPxl = xTree;
            bool bFirstCol = true;
            ProgressionElement * pChosen = getChosenElementAtLevel(pTree, i, 0);
            ProgressionElement * pElt = (ProgressionElement*) pTree->m_pElements[0]->getFirst(0);
            while (pElt != NULL)
            {
                addChoiceButton(xPxl, yPxl, btnSize, pElt, pChosen == NULL ? 2 : (pChosen == pElt ? 1 : 0));
                if (pChosen == pElt)
                    bLastChosenLeft = bFirstCol;
                if (bFirstCol)
                    xPxl += btnSize + btnSize / 3;
                else
                {
                    xPxl = xTree;
                    yPxl += btnSize + SPACING;
                }
                bFirstCol = !bFirstCol;
                pElt = (ProgressionElement*) pTree->m_pElements[0]->getNext(0);
            }
            if (!bFirstCol)
                yPxl += btnSize + SPACING;
            int iLevel = 1;
            while (pChosen != NULL)
            {
                // branch image
                pImg = new guiImage();
                pImg->init(bLastChosenLeft ? iTexChoiceLeft : iTexChoiceRight, "", xTree + treeWidth / 2 - iChoiceWidth / 2, yPxl, -1, -1, getDisplay());
                addComponent(pImg);
                yPxl += pImg->getHeight();
                xPxl = xTree;
                bFirstCol = true;

                ProgressionElement * pPrevChosen = pChosen;
                pChosen = getChosenElementAtLevel(pTree, i, iLevel++);
                pElt = (ProgressionElement*) pPrevChosen->m_pChildren->getFirst(0);
                while (pElt != NULL)
                {
                    addChoiceButton(xPxl, yPxl, btnSize, pElt, pChosen == NULL ? 2 : (pChosen == pElt ? 1 : 0));
                    if (pChosen == pElt)
                        bLastChosenLeft = bFirstCol;
                    if (bFirstCol)
                        xPxl += btnSize + btnSize / 3;
                    else
                    {
                        xPxl = xTree;
                        yPxl += btnSize + SPACING;
                    }
                    bFirstCol = !bFirstCol;
                    pElt = (ProgressionElement*) pPrevChosen->m_pChildren->getNext(0);
                }
                if (!bFirstCol)
                    yPxl += btnSize + SPACING;
            }
            xTree += treeWidth + SPACING;
        }
    }

    // Create button "Do it later"
    yPxl += SPACING;
    float fx = getWidth() / 8;
    guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText("DO_IT_LATER", sText, 256), "DoItLaterButton", m_pLocalClient->getDisplay());
    pBtn->moveTo((int)fx, yPxl);
    pBtn->setWidth(2*fx);
    pBtn->setTooltipText(i18n->getText("DO_IT_LATER_EXP", sText, 256));
    addComponent(pBtn);

    // Create button "Ok"
    pBtn = (guiButton*) pBtn->clone();
    pBtn->setText(i18n->getText1stUp("OK", sText, 256));
    pBtn->setId("OkButton");
    pBtn->moveBy((int)(4 * fx), 0);
    pBtn->setEnabled(false);
    addComponent(pBtn);

    // Size
    setHeight(yPxl + pBtn->getHeight() + SPACING);

    // Show frame
    m_pLocalClient->getInterface()->setUniqueDialog(this);
}

// -----------------------------------------------------------------
// Name : doAvatarLevelUp
// -----------------------------------------------------------------
void LevelUpDlg::doAvatarLevelUp(AvatarData * pAvatar, guiDocument * pCaller, bool bFirstCall)
{
    m_pCaller = pCaller;
    setCurrentAvatar(pAvatar);

    if (bFirstCall) // Edit avatar infos
        m_pEditAvatarInfosPopup = m_pLocalClient->getInterface()->getBuildDeckDialog()->showEditAvatarInfosPopup(pAvatar, this);
}
