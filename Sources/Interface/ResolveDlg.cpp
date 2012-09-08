#include "ResolveDlg.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../Data/LocalisationTool.h"
#include "../Data/LuaContext.h"
#include "../LocalClient.h"
#include "../Gameboard/Unit.h"
#include "../GUIClasses/guiContainer.h"
#include "../GUIClasses/guiImage.h"
#include "BattleSpellPopup.h"
#include "../Players/PlayerManager.h"
#include "../Players/Spell.h"
#include "InterfaceManager.h"
#include "SpellDlg.h"
#include "InfoboxDlg.h"
#include "LogDlg.h"
#include "../Server/Server.h"
#include "../Fx/FxManager.h"
#include "../Gameboard/GameboardManager.h"
#include "../Debug/DebugManager.h"

#define SPACING    4

// -----------------------------------------------------------------
// Name : ResolveDlg
//  Constructor
// -----------------------------------------------------------------
ResolveDlg::ResolveDlg(int iWidth, LocalClient * pLocalClient) : guiDocument()
{
    m_pLocalClient = pLocalClient;
    m_pAttackersPanel = NULL;
    m_pDefendersPanel = NULL;
    m_pBattlesListPanel = NULL;
    m_pBattleSelectorImg = m_pDefenderSelectorImg = m_pAttackerSelectorImg = NULL;
    m_pBattlesPositions = NULL;
    m_pPlayers = NULL;
    m_pSpellsPopup = NULL;
    m_pCastSpellForThisBattle = NULL;
    m_pSpellData = NULL;
    m_pTarget = NULL;
    m_iMaxHeight = 400;
    m_iTimerConcerned = 10;
    m_iTimerNotConcerned = 10;
    m_iSelectTexture = pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:Selector");
    m_iRangeAttackTex = pLocalClient->getDisplay()->getTextureEngine()->loadTexture("range_attack", false, 0, 24, 0, 24);
    m_pAllowedDefendersPerAttacker = new ObjectList(true);
    m_pAttacker = NULL;
    m_pAttackingUnit = NULL;
    m_pDefendingUnit = NULL;
    m_iNbBattles = 0;

    char sTitle[64];
    i18n->getText("RESOLVE", sTitle, 64);
    init(sTitle,
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, iWidth, 1, pLocalClient->getDisplay());

    m_iPlayerIt = pLocalClient->getPlayerManager()->getPlayersList()->getIterator();
    int yPos = initSimpleStatusScreen();
    int boxHeight = initChooseUnitsScreen(yPos);
    initChooseBattleScreen(yPos, boxHeight);

    u8 uNbAllPlayers = pLocalClient->getPlayerManager()->getPlayersCount();
    m_pCastSpellForThisBattle = new bool[uNbAllPlayers];
    for (int i = 0; i < uNbAllPlayers; i++)
        m_pCastSpellForThisBattle[i] = true;
}

// -----------------------------------------------------------------
// Name : ~ResolveDlg
//  Destructor
// -----------------------------------------------------------------
ResolveDlg::~ResolveDlg()
{
    if (m_pBattlesPositions != NULL)
        delete[] m_pBattlesPositions;
    if (m_pPlayers != NULL)
        delete[] m_pPlayers;
    if (m_pCastSpellForThisBattle != NULL)
        delete[] m_pCastSpellForThisBattle;
    FREE(m_pSpellData);
    delete m_pAllowedDefendersPerAttacker;
}

// -----------------------------------------------------------------
// Name : initSimpleStatusScreen
//  return Y position of next below element
// -----------------------------------------------------------------
int ResolveDlg::initSimpleStatusScreen()
{
    guiLabel * pLbl = new guiLabel();
    pLbl->init("...\n...", TEXT_FONT, TEXT_COLOR, "Status", 5, SPACING, getWidth() - 10, 0, getDisplay());
    addComponent(pLbl);
    return pLbl->getYPos() + pLbl->getHeight() + 2*SPACING;
}

// -----------------------------------------------------------------
// Name : initChooseBattleScreen
// -----------------------------------------------------------------
void ResolveDlg::initChooseBattleScreen(int yPos, int boxHeight)
{
    m_pBattlesListPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, boxHeight, "BattlesListPane", getDisplay());
    m_pBattlesListPanel->moveTo(SPACING, yPos);
    addComponent(m_pBattlesListPanel);

    char sText[64];
    yPos += m_pBattlesListPanel->getHeight() + 2 * SPACING;
    i18n->getText1stUp("IGNORE_BATTLES", sText, 64);
    guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "SkipButton", getDisplay());
    pBtn->autoPadWidth(6, 64);
    pBtn->moveTo(SPACING + 2, yPos);
    addComponent(pBtn);

    i18n->getText("SELECT_BATTLE", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, "SelectBattleButton", getDisplay());
    pBtn->autoPadWidth(6, 64);
    pBtn->moveTo(getWidth() - SPACING - pBtn->getWidth() - 2, yPos);
    addComponent(pBtn);

    m_pBattleSelectorImg = new guiImage();
    m_pBattleSelectorImg->init(m_iSelectTexture, "SelectorImage1", 0, 0, m_pBattlesListPanel->getInnerWidth(), 2 * SMALL_ICON_SIZE + SPACING, getDisplay());
    m_pBattlesListPanel->getDocument()->addComponent(m_pBattleSelectorImg);
}

// -----------------------------------------------------------------
// Name : initChooseUnitsScreen
// -----------------------------------------------------------------
int ResolveDlg::initChooseUnitsScreen(int yPos)
{
    char sText[64];
    i18n->getText("ATTACKERS", sText, 64);
    guiLabel * pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "AttackersLabe", 0, yPos, 0, 0, getDisplay());
    pLbl->moveBy((getWidth()-pLbl->getWidth()) / 2, 0);
    addComponent(pLbl);

    int boxHeight = (m_iMaxHeight - 10*SPACING - 3 * pLbl->getHeight() - 26) / 2;
    yPos = pLbl->getYPos() + pLbl->getHeight() + SPACING;
    m_pAttackersPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, boxHeight, "AttackersPane", getDisplay());
    m_pAttackersPanel->moveTo(SPACING, yPos);
    addComponent(m_pAttackersPanel);

    yPos = m_pAttackersPanel->getYPos() + m_pAttackersPanel->getHeight() + 2 * SPACING;
    i18n->getText("DEFENDERS", sText, 64);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "DefendersLabe", 0, yPos, 0, 0, getDisplay());
    pLbl->moveBy((getWidth()-pLbl->getWidth()) / 2, 0);
    addComponent(pLbl);

    yPos = pLbl->getYPos() + pLbl->getHeight() + SPACING;
    m_pDefendersPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, boxHeight, "DefendersPane", getDisplay());
    m_pDefendersPanel->moveTo(SPACING, yPos);
    addComponent(m_pDefendersPanel);

    yPos = m_pDefendersPanel->getYPos() + m_pDefendersPanel->getHeight() + SPACING;
    i18n->getText1stUp("CANCEL", sText, 64);
    guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "CancelButton", getDisplay());
    pBtn->autoPadWidth(6, 64);
    pBtn->moveTo(SPACING + 2, yPos);
    addComponent(pBtn);

    i18n->getText("NEXT", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, "NextButton", getDisplay());
    pBtn->autoPadWidth(6, 64);
    pBtn->moveTo(getWidth() - SPACING - pBtn->getWidth() - 2, yPos);
    addComponent(pBtn);

    m_pAttackerSelectorImg = new guiImage();
    m_pAttackerSelectorImg->init(m_iSelectTexture, "SelectorImage2", 0, 0, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
    m_pAttackersPanel->getDocument()->addComponent(m_pAttackerSelectorImg);

    m_pDefenderSelectorImg = new guiImage();
    m_pDefenderSelectorImg->init(m_iSelectTexture, "SelectorImage3", 0, 0, m_pDefendersPanel->getInnerWidth(), 2 * SMALL_ICON_SIZE + SPACING, getDisplay());
    m_pDefendersPanel->getDocument()->addComponent(m_pDefenderSelectorImg);

    return boxHeight;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void ResolveDlg::update(double delta)
{
    guiDocument::update(delta);
    if (m_BattleStep == RBS_AskCastBattleSpells)
    {
        Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getCurrent(m_iPlayerIt);
        switch (m_pSpellsPopup->getResponse())
        {
        case BATTLESPELL_BUTTON_NEVERBATTLE:
            m_pCastSpellForThisBattle[pPlayer->m_uPlayerId-1] = false;
            // don't kreak, continue on BATTLESPELL_BUTTON_FINISHED
        case BATTLESPELL_BUTTON_FINISHED:
            m_pSpellData->addLong((long)pPlayer->m_uPlayerId);
            m_pLocalClient->getInterface()->getSpellDialog()->getCastSpellsData(m_pSpellData);
            m_pLocalClient->getInterface()->getSpellDialog()->updateContent(NULL);
            m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(m_iPlayerIt);
            askNextPlayerCastSpells();
            break;
        default:
            break;
        }
    }
}

// -----------------------------------------------------------------
// Name : setStatus
// -----------------------------------------------------------------
void ResolveDlg::setStatus(char * sStatus)
{
    guiLabel * pLbl = (guiLabel*) getComponent("Status");
    pLbl->setText(sStatus);
    setHeight(pLbl->getYPos() + pLbl->getHeight());
}

// -----------------------------------------------------------------
// Name : selectComponent
// -----------------------------------------------------------------
void ResolveDlg::selectComponent(guiImage * pSelector, guiComponent * pCpnt, guiContainer * pPanel, int iIndex)
{
    if (pCpnt != NULL)
    {
        pSelector->setDimensions(pCpnt->getWidth(), pCpnt->getHeight());
        pSelector->moveTo(pCpnt->getXPos(), pCpnt->getYPos());
        pSelector->setVisible(true);
    }
    else if (pPanel != NULL)
    {
        int iHeight = 2 * SMALL_ICON_SIZE + SPACING;
        pSelector->setDimensions(pPanel->getInnerWidth(), iHeight);
        pSelector->moveTo(0, iIndex * iHeight);
        pSelector->setVisible(true);
    }
    else
        pSelector->setVisible(false);
}

// -----------------------------------------------------------------
// Name : setSimpleStatusScreen
// -----------------------------------------------------------------
void ResolveDlg::setSimpleStatusScreen()
{
    int maxY = 0;
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
        pCpnt->setVisible(
            strcmp(pCpnt->getId(), "Status") == 0
        );
        if (pCpnt->isVisible())
            maxY = max(maxY, pCpnt->getYPos() + pCpnt->getHeight());
        pCpnt = getNextComponent();
    }
    setHeight(maxY + SPACING);
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->updateSizeFit();
    pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, 1 + m_pLocalClient->getClientParameters()->screenYSize - pFrm->getHeight());
}

// -----------------------------------------------------------------
// Name : setChooseBattleScreen
// -----------------------------------------------------------------
void ResolveDlg::setChooseBattleScreen()
{
    int maxY = 0;
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
        pCpnt->setVisible(
            strcmp(pCpnt->getId(), "Status") == 0
            || strcmp(pCpnt->getId(), "SelectBattleButton") == 0
            || strcmp(pCpnt->getId(), "SkipButton") == 0
            || strcmp(pCpnt->getId(), "BattlesListPane") == 0
        );
        if (strcmp(pCpnt->getId(), "SelectBattleButton") == 0)
            pCpnt->setEnabled(false);
        if (pCpnt->isVisible())
            maxY = max(maxY, pCpnt->getYPos() + pCpnt->getHeight());
        pCpnt = getNextComponent();
    }
    setHeight(maxY + SPACING);
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->updateSizeFit();
    pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, 1 + m_pLocalClient->getClientParameters()->screenYSize - pFrm->getHeight());
    m_iSelectedBattle = -1;
    selectComponent(m_pBattleSelectorImg, NULL);
}

// -----------------------------------------------------------------
// Name : setChooseUnitsScreen
// -----------------------------------------------------------------
void ResolveDlg::setChooseUnitsScreen()
{
    int maxY = 0;
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
        pCpnt->setVisible(
            strcmp(pCpnt->getId(), "Status") == 0
            || strcmp(pCpnt->getId(), "AttackersLabe") == 0
            || strcmp(pCpnt->getId(), "DefendersLabe") == 0
            || (m_BattleStep == RBS_ChooseUnits && strcmp(pCpnt->getId(), "CancelButton") == 0)
            || (m_BattleStep != RBS_AskCastBattleSpells && strcmp(pCpnt->getId(), "NextButton") == 0)
            || strcmp(pCpnt->getId(), "AttackersPane") == 0
            || strcmp(pCpnt->getId(), "DefendersPane") == 0
        );
        if (strcmp(pCpnt->getId(), "NextButton") == 0)
        {
            if (m_BattleStep == RBS_ChooseUnits && m_pAttackingUnit != NULL && m_pDefendingUnit != NULL)
                pCpnt->setEnabled(true);
            else
                pCpnt->setEnabled(false);
        }
        if (pCpnt->isVisible())
            maxY = max(maxY, pCpnt->getYPos() + pCpnt->getHeight());
        pCpnt = getNextComponent();
    }
    setHeight(maxY + SPACING);
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->updateSizeFit();
    pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, 1 + m_pLocalClient->getClientParameters()->screenYSize - pFrm->getHeight());

    if (m_BattleStep == RBS_AskCastBattleSpells)
    {
        // Move m_pDefenderSelectorImg to selected unit
        guiComponent * cpnt = m_pDefendersPanel->getDocument()->getFirstComponent();
        while (cpnt != NULL)
        {
            BaseObject * pAttachment = cpnt->getAttachment();
            if (pAttachment != NULL && pAttachment == m_pDefendingUnit)
            {
                selectComponent(m_pDefenderSelectorImg, cpnt);
                return;
            }
            cpnt = m_pDefendersPanel->getDocument()->getNextComponent();
        }
        assert(false); // we must not be here : there must be a valid m_pSelectedDefendingUnit
    }
}

// -----------------------------------------------------------------
// Name : setCurrentBattleScreen
// -----------------------------------------------------------------
void ResolveDlg::setCurrentBattleScreen()
{
    int maxY = 0;
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
        pCpnt->setVisible(
            strcmp(pCpnt->getId(), "Status") == 0
            || strcmp(pCpnt->getId(), "AttackersLabe") == 0
            || strcmp(pCpnt->getId(), "DefendersLabe") == 0
            || (m_BattleStep == RBS_ChooseUnits && strcmp(pCpnt->getId(), "CancelButton") == 0)
            || (m_BattleStep != RBS_AskCastBattleSpells && strcmp(pCpnt->getId(), "NextButton") == 0)
            || strcmp(pCpnt->getId(), "AttackersPane") == 0
            || strcmp(pCpnt->getId(), "DefendersPane") == 0
        );
        if (strcmp(pCpnt->getId(), "NextButton") == 0)
        {
            if (m_BattleStep == RBS_ChooseUnits && m_pAttackingUnit != NULL && m_pDefendingUnit != NULL)
                pCpnt->setEnabled(true);
            else
                pCpnt->setEnabled(false);
        }
        if (pCpnt->isVisible())
            maxY = max(maxY, pCpnt->getYPos() + pCpnt->getHeight());
        pCpnt = getNextComponent();
    }
    setHeight(maxY + SPACING);
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->updateSizeFit();
    pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, 1 + m_pLocalClient->getClientParameters()->screenYSize - pFrm->getHeight());

    m_pAttackerSelectorImg = (guiImage*) m_pAttackerSelectorImg->clone();
    m_pAttackersPanel->getDocument()->deleteAllComponents();
    m_pAttackersPanel->getDocument()->addComponent(m_pAttackerSelectorImg);
    m_pDefenderSelectorImg = (guiImage*) m_pDefenderSelectorImg->clone();
    m_pDefendersPanel->getDocument()->deleteAllComponents();
    m_pDefendersPanel->getDocument()->addComponent(m_pDefenderSelectorImg);

    guiImage * pImg = addUnitToFlowPanel(m_pAttackersPanel, (Unit*) m_pAttackingUnit->getFirst(0), m_pAttackingUnit->getCurrentType(0) == 1 ? m_iRangeAttackTex : -1);
    pImg->setAttachment(m_pAttackingUnit);
    selectComponent(m_pAttackerSelectorImg, pImg);

    pImg = addUnitToFlowPanel(m_pDefendersPanel, m_pDefendingUnit);
    pImg->setAttachment(m_pDefendingUnit);
    selectComponent(m_pDefenderSelectorImg, pImg);
}

// -----------------------------------------------------------------
// Name : updateChooseBattlePanel
//  In this panel, the player can choose any battle on screen where he's involved in
// -----------------------------------------------------------------
void ResolveDlg::updateChooseBattlePanel(NetworkData * pData)
{
    if (m_pBattlesPositions != NULL)
        delete[] m_pBattlesPositions;
    m_pBattleSelectorImg = (guiImage*) m_pBattleSelectorImg->clone();
    m_pBattlesListPanel->getDocument()->deleteAllComponents();
    m_pBattlesListPanel->getDocument()->addComponent(m_pBattleSelectorImg);
    m_pBattlesListPanel->getDocument()->setDimensions(40, 40);

    m_iNbBattles = (int) (pData->dataYetToRead() / (2 * sizeof(long)));
    assert(m_iNbBattles > 0);
    m_pBattlesPositions = new CoordsMap[m_iNbBattles];
    int yPxl = 0;
    for (int i = 0; i < m_iNbBattles; i++)
    {
        m_pBattlesPositions[i].x = (int) pData->readLong();
        m_pBattlesPositions[i].y = (int) pData->readLong();

        guiImage * pMainImg = new guiImage();
        char sId[8] = "0";
        snprintf(sId, 8, "%d", i);
        pMainImg->init(m_pLocalClient->getGameboard()->getBattleTexture(), sId, 0, yPxl, 2 * SMALL_ICON_SIZE + SPACING, 2 * SMALL_ICON_SIZE + SPACING, getDisplay());
        m_pBattlesListPanel->getDocument()->addComponent(pMainImg);

        int xPxlLow = 2 * SMALL_ICON_SIZE + 2 * SPACING;
        int xPxlHigh = 2 * SMALL_ICON_SIZE + 2 * SPACING;
        MapTile * pTile = m_pLocalClient->getGameboard()->getMap()->getTileAt(m_pBattlesPositions[i]);
        MapObject * mapObj = (MapObject*) pTile->m_pMapObjects->getFirst(0);
        while (mapObj != NULL)
        {
            if (mapObj->getType() & GOTYPE_UNIT)
            {
                if (((Unit*)mapObj)->getOwner() == m_pAttacker->m_uPlayerId)
                {
                    guiImage * pImage = new guiImage();
                    pImage->init(mapObj->getTexture(), "", xPxlHigh, yPxl, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
                    pImage->setAttachment(mapObj);
                    m_pBattlesListPanel->getDocument()->addComponent(pImage);
                    xPxlHigh += SMALL_ICON_SIZE + SPACING;
                    if (m_pBattlesListPanel->getDocument()->getWidth() < xPxlHigh)
                        m_pBattlesListPanel->getDocument()->setWidth(xPxlHigh);
                }
                else
                {
                    guiImage * pImage = new guiImage();
                    pImage->init(mapObj->getTexture(), "", xPxlLow, yPxl + SMALL_ICON_SIZE + SPACING, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
                    pImage->setAttachment(mapObj);
                    m_pBattlesListPanel->getDocument()->addComponent(pImage);
                    xPxlLow += SMALL_ICON_SIZE + SPACING;
                    if (m_pBattlesListPanel->getDocument()->getWidth() < xPxlLow)
                        m_pBattlesListPanel->getDocument()->setWidth(xPxlLow);
                }
            }
            mapObj = (MapObject*) pTile->m_pMapObjects->getNext(0);
        }

        yPxl += pMainImg->getHeight() + SPACING;
        if (m_pBattlesListPanel->getDocument()->getHeight() < yPxl)
            m_pBattlesListPanel->getDocument()->setHeight(yPxl);
    }
}

// -----------------------------------------------------------------
// Name : updateChooseUnitsPanel
//  After choosing a battle, the player can choose his attacking unit and the defending unit.
// -----------------------------------------------------------------
void ResolveDlg::updateChooseUnitsPanel(NetworkData * pData)
{
    if (m_pPlayers != NULL)
        delete[] m_pPlayers;
    // clear panels
    m_pAttackerSelectorImg = (guiImage*) m_pAttackerSelectorImg->clone();
    m_pAttackersPanel->getDocument()->deleteAllComponents();
    m_pAttackersPanel->getDocument()->addComponent(m_pAttackerSelectorImg);
    m_pAttackersPanel->getDocument()->setDimensions(40, 40);
    m_pDefenderSelectorImg = (guiImage*) m_pDefenderSelectorImg->clone();
    m_pDefendersPanel->getDocument()->deleteAllComponents();
    m_pDefendersPanel->getDocument()->addComponent(m_pDefenderSelectorImg);
    m_pDefendersPanel->getDocument()->setDimensions(40, 40);
    // hide selectors
    m_pAttackingUnit = NULL;
    selectComponent(m_pAttackerSelectorImg, NULL);
    selectComponent(m_pDefenderSelectorImg, NULL);
    bool bFirstAtt = true;
    bool bFirstDef = true;

    // set up temporary players data
    m_iNbPlayers = 0;
    u8 uNbAllPlayers = m_pLocalClient->getPlayerManager()->getPlayersCount() + 1; // include neutral player
    int * pAllPlayers = new int[uNbAllPlayers];
    for (int i = 0; i < uNbAllPlayers; i++)
        pAllPlayers[i] = -1;
    CoordsScreen * pPlayerNextUnitPosition = new CoordsScreen[uNbAllPlayers];
    ObjectList allDefenders(false);
    m_pAllowedDefendersPerAttacker->deleteAll();

    // start reading data
    while (pData->dataYetToRead() > 0)
    {
        // First we get the attacker data
        u8 uPlayerId = (u8) pData->readLong();
        u32 uUnitId = (u32) pData->readLong();
        bool isRange = (pData->readLong() == 1);
        Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(uPlayerId);
        assert(pPlayer != NULL);
        Unit * pUnit = pPlayer->findUnit(uUnitId);
        assert(pUnit != NULL);
        MetaObjectList * pList = new MetaObjectList(false);
        m_pAllowedDefendersPerAttacker->addLast(pList);
        pList->addLast(pUnit, isRange ? 1 : 0);

        guiImage * pImg = addUnitToFlowPanel(m_pAttackersPanel, pUnit, isRange ? m_iRangeAttackTex : -1);
        pImg->setAttachment(pList);
        if (bFirstAtt)
        {
            m_pAttackingUnit = pList;
            selectComponent(m_pAttackerSelectorImg, pImg);
        }

        // Then, a list of valid defenders for this attacker
        int nbDefs = pData->readLong();
        for (int i = 0; i < nbDefs; i++)
        {
            uPlayerId = (u8) pData->readLong();
            uUnitId = (u32) pData->readLong();
            pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(uPlayerId);
            assert(pPlayer != NULL);
            pUnit = pPlayer->findUnit(uUnitId);
            assert(pUnit != NULL);

            // Add it as a valid defender
            pList->addLast(pUnit);

            // Check if it was already displayed
            if (!allDefenders.goTo(0, pUnit))
            {
                allDefenders.addLast(pUnit);
                if (pAllPlayers[uPlayerId] == -1)
                {
                    // New defender player: add his avatar's icon to panel
                    pAllPlayers[uPlayerId] = m_iNbPlayers++;
                    pPlayerNextUnitPosition[uPlayerId].x = 2 * (SMALL_ICON_SIZE + SPACING);
                    pPlayerNextUnitPosition[uPlayerId].y = pAllPlayers[uPlayerId] * 2 * (SMALL_ICON_SIZE + SPACING);
                    pPlayerNextUnitPosition[uPlayerId].z = 0; // "z" coord is used as boolean to know if we're on top line or bottom line
                    guiImage * pAvatarImg = new guiImage();
                    pAvatarImg->init(pPlayer->m_iBannerTex, "", 0, pPlayerNextUnitPosition[uPlayerId].y, 2 * SMALL_ICON_SIZE + SPACING, 2 * SMALL_ICON_SIZE + SPACING, getDisplay());
                    pAvatarImg->setDiffuseColor(pPlayer->m_Color);
                    m_pDefendersPanel->getDocument()->addComponent(pAvatarImg);
                    if (m_pDefendersPanel->getDocument()->getHeight() < m_iNbPlayers * 2 * (SMALL_ICON_SIZE + SPACING))
                        m_pDefendersPanel->getDocument()->setHeight(m_iNbPlayers * 2 * (SMALL_ICON_SIZE + SPACING));
                }
                guiImage * pImage = new guiImage();
                pImage->init(pUnit->getTexture(), "", pPlayerNextUnitPosition[uPlayerId].x, pPlayerNextUnitPosition[uPlayerId].y, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
                pImage->setAttachment(pUnit);
                if (!bFirstAtt)
                    pImage->setEnabled(false);
                m_pDefendersPanel->getDocument()->addComponent(pImage);
                if (bFirstDef)
                {
                    m_pDefendingUnit = pUnit;
                    selectComponent(m_pDefenderSelectorImg, pImage);
                    bFirstDef = false;
                }
                if (pPlayerNextUnitPosition[uPlayerId].z == 0)
                {
                    pPlayerNextUnitPosition[uPlayerId].z = 1;
                    pPlayerNextUnitPosition[uPlayerId].y += SMALL_ICON_SIZE + SPACING;
                }
                else
                {
                    pPlayerNextUnitPosition[uPlayerId].z = 0;
                    pPlayerNextUnitPosition[uPlayerId].x += SMALL_ICON_SIZE + SPACING;
                    pPlayerNextUnitPosition[uPlayerId].y -= SMALL_ICON_SIZE + SPACING;
                    if (m_pDefendersPanel->getDocument()->getWidth() < pPlayerNextUnitPosition[uPlayerId].x)
                        m_pDefendersPanel->getDocument()->setWidth(pPlayerNextUnitPosition[uPlayerId].x);
                }
            }
        }
        bFirstAtt = false;
    }

    m_pPlayers = new u8[m_iNbPlayers];
    for (int i = 0; i < uNbAllPlayers; i++)
    {
        if (pAllPlayers[i] >= 0)
            m_pPlayers[pAllPlayers[i]] = i;
    }
    delete[] pAllPlayers;
}

// -----------------------------------------------------------------
// Name : addUnitToFlowPanel
// -----------------------------------------------------------------
guiImage * ResolveDlg::addUnitToFlowPanel(guiContainer * pPanel, Unit * pUnit, int iOverTex)
{
    // Count buttons
    int nbButtons = 0;
    guiComponent * pCpnt = (guiComponent*) pPanel->getDocument()->getComponentsList()->getFirst(0);
    while (pCpnt != NULL)
    {
        if (strcmp(pCpnt->getId(), "UnitButton") == 0)
            nbButtons++;
        pCpnt = (guiComponent*) pPanel->getDocument()->getComponentsList()->getNext(0);
    }
    int nbButtonsPerLine = pPanel->getWidth() / (SMALL_ICON_SIZE + SPACING);
    int xPxl = SPACING + (nbButtons % nbButtonsPerLine) * (SPACING + SMALL_ICON_SIZE);
    int yPxl = SPACING + (nbButtons / nbButtonsPerLine) * (SPACING + SMALL_ICON_SIZE);
    guiImage * pImage = new guiImage();
    pImage->init(pUnit->getTexture(), "UnitButton", xPxl, yPxl, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
    pImage->setAttachment(pUnit);
    pPanel->getDocument()->addComponent(pImage);
    char sText[256];
    if (iOverTex >= 0)
    {
        guiImage * pImage2 = new guiImage();
        pImage2->init(iOverTex, "RangeAttackIcon", xPxl + SMALL_ICON_SIZE - 16, yPxl, 16, 16, getDisplay());
        pImage2->setAttachment(pImage);
        pPanel->getDocument()->addComponent(pImage2);
        pImage2->setTooltipText(i18n->getText("RANGE_ATTACK", sText, 256));
        pImage->setTooltipText(sText);
    }
    else
        pImage->setTooltipText(i18n->getText("MELEE_ATTACK", sText, 256));
    return pImage;
}

// -----------------------------------------------------------------
// Name : askNextPlayerCastSpells
// -----------------------------------------------------------------
void ResolveDlg::askNextPlayerCastSpells()
{
    if (m_pSpellsPopup != NULL)
    {
        m_pLocalClient->getInterface()->deleteFrame(m_pSpellsPopup);
        m_pSpellsPopup = NULL;
    }

    Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getCurrent(m_iPlayerIt);
    while (pPlayer != NULL)
    {
        if (pPlayer->m_uClientId == m_pLocalClient->getClientId() && m_pCastSpellForThisBattle[pPlayer->m_uPlayerId-1] && !pPlayer->m_bIsAI)
        {
            // Ask this player to cast spells
            int iTimer = m_iTimerNotConcerned;
            if (pPlayer == m_pAttacker || (m_pDefendingUnit != NULL && pPlayer->m_uPlayerId == m_pDefendingUnit->getOwner()))
                iTimer = m_iTimerConcerned;
            m_pSpellsPopup = new BattleSpellPopup(pPlayer, iTimer, getDisplay());
            m_pSpellsPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pSpellsPopup->getWidth()) / 2, 0);
            m_pLocalClient->getInterface()->registerFrame(m_pSpellsPopup);
            // Enable spell dialog
            m_pLocalClient->getInterface()->getSpellDialog()->updateContent(pPlayer, true);
            return;
        }
        pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(m_iPlayerIt);
    }
    // No player left : send collected data to server
    m_pLocalClient->sendMessage(m_pSpellData);
    delete m_pSpellData;
    m_pSpellData = NULL;
    char sStatus[128];
    i18n->getText("WAIT_SPELLS", sStatus, 128);
    setStatus(sStatus);
    setSimpleStatusScreen();
    m_BattleStep = RBS_Passive;
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
guiObject * ResolveDlg::onButtonEvent(ButtonAction * pEvent)
{
    if (m_pBattlesListPanel->isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset))
    {
        int docYPxl = pEvent->yPos - pEvent->yOffset - m_pBattlesListPanel->getInnerYPos() - m_pBattlesListPanel->getDocument()->getYPos();
        int h = 2 * (SMALL_ICON_SIZE + SPACING);
        m_iSelectedBattle = docYPxl / h;
        if (m_iSelectedBattle >= m_iNbBattles || h - docYPxl % h < SPACING)
        {
            m_iSelectedBattle = -1;
            selectComponent(m_pBattleSelectorImg, NULL);
            getComponent("SelectBattleButton")->setEnabled(false);
        }
        else
        {
            selectComponent(m_pBattleSelectorImg, NULL, m_pBattlesListPanel, m_iSelectedBattle);
            getComponent("SelectBattleButton")->setEnabled(true);
            m_pLocalClient->getFx()->zoomToMapPos(m_pBattlesPositions[m_iSelectedBattle]);
        }
    }
    else if (m_pAttackersPanel->isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset) && m_BattleStep == RBS_ChooseUnits)
    {
        guiObject * pResult = guiDocument::onButtonEvent(pEvent);
        guiComponent * pClicked = m_pAttackersPanel->getDocument()->getFocusedComponent();
        if (pClicked != NULL)
        {
            if (pClicked != (guiComponent*) m_pAttackerSelectorImg)
            {
                if (strcmp(pClicked->getId(), "RangeAttackIcon") == 0)
                {
                    pClicked = (guiComponent*) pClicked->getAttachment();
                    assert(pClicked != NULL);
                }
                m_pAttackingUnit = (MetaObjectList*) pClicked->getAttachment();
                selectComponent(m_pAttackerSelectorImg, pClicked);
                // Enable or disable defenders
                guiComponent * pCpnt = m_pDefendersPanel->getDocument()->getFirstComponent();
                while (pCpnt != NULL)
                {
                    if (strcmp(pCpnt->getId(), "UnitButton") == 0)
                    {
                        Unit * pDef = (Unit*) pCpnt->getAttachment();
                        assert(pDef != NULL);
                        pCpnt->setEnabled(m_pAttackingUnit->goTo(0, pDef));
                    }
                    pCpnt = m_pDefendersPanel->getDocument()->getNextComponent();
                }
                getComponent("NextButton")->setEnabled(m_pDefenderSelectorImg->isVisible());
            }
        }
        else
        {
            m_pAttackingUnit = NULL;
            selectComponent(m_pAttackerSelectorImg, NULL);
            getComponent("NextButton")->setEnabled(false);
        }
        return pResult;
    }
    else if (m_pDefendersPanel->isAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset) && m_BattleStep == RBS_ChooseUnits)
    {
        // Set defender (player)
        // Range attack => the attacker directly selects the defending unit
        guiObject * pResult = guiDocument::onButtonEvent(pEvent);
        guiComponent * pClicked = m_pDefendersPanel->getDocument()->getFocusedComponent();
        if (pClicked != NULL)
        {
            if (pClicked != (guiComponent*) m_pDefenderSelectorImg && pClicked->isEnabled())
            {
                m_pDefendingUnit = (Unit*) pClicked->getAttachment();
                if (m_pDefendingUnit != NULL)
                {
                    selectComponent(m_pDefenderSelectorImg, pClicked);
                    getComponent("NextButton")->setEnabled(true);
                }
                else
                    pClicked = NULL;
            }
        }
        if (pClicked == NULL)
        {
            m_pDefendingUnit = NULL;
            selectComponent(m_pDefenderSelectorImg, NULL);
            getComponent("NextButton")->setEnabled(false);
        }
        return pResult;
    }
    return guiDocument::onButtonEvent(pEvent);
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * ResolveDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
    m_pTarget = NULL;
    if (m_pAttackersPanel->isDocumentAt(xPxl, yPxl))
        mouseOverPanel(m_pAttackersPanel, xPxl, yPxl);
    else if (m_pDefendersPanel->isDocumentAt(xPxl, yPxl))
        mouseOverPanel(m_pDefendersPanel, xPxl, yPxl);
    else if (m_pBattlesListPanel->isDocumentAt(xPxl, yPxl))
        mouseOverPanel(m_pBattlesListPanel, xPxl, yPxl);
    return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : mouseOverPanel
// -----------------------------------------------------------------
void ResolveDlg::mouseOverPanel(guiContainer * pPanel, int xPxl, int yPxl)
{
    int xPxlRel = xPxl - pPanel->getInnerXPos() - pPanel->getDocument()->getXPos();
    int yPxlRel = yPxl - pPanel->getInnerYPos() - pPanel->getDocument()->getYPos();
    guiComponent * cpnt = pPanel->getDocument()->getFirstComponent();
    while (cpnt != NULL)
    {
        if (cpnt->isAt(xPxlRel, yPxlRel))
        {
            BaseObject * pAttachment = cpnt->getAttachment();
            if (pPanel == m_pAttackersPanel && pAttachment != NULL)
                pAttachment = ((MetaObjectList*)pAttachment)->getFirst(0);
            if (pAttachment != NULL)
            {
                m_pTarget = cpnt;
                char sBuf[LABEL_MAX_CHARS] = "";
                m_pLocalClient->getInterface()->getInfoDialog()->setInfoText(((MapObject*)pAttachment)->getInfo(sBuf, LABEL_MAX_CHARS, Dest_InfoDialog));
                break;
            }
        }
        cpnt = pPanel->getDocument()->getNextComponent();
    }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool ResolveDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    switch (m_BattleStep)
    {
    case RBS_Init:
    case RBS_Passive:
    case RBS_AskCastBattleSpells:
    case RBS_SelectSpellTarget:
        break;
    case RBS_ChooseBattle:
    {
        if (strcmp(pCpnt->getId(), "SelectBattleButton") == 0)
        {
            assert(m_iSelectedBattle >= 0);
            NetworkData msg(NETWORKMSG_RESOLVE_SELECT_BATTLE);
            msg.addLong((long)m_pBattlesPositions[m_iSelectedBattle].x);
            msg.addLong((long)m_pBattlesPositions[m_iSelectedBattle].y);
            m_pLocalClient->sendMessage(&msg);
        }
        else if (strcmp(pCpnt->getId(), "SkipButton") == 0)
        {
            m_iNbBattles = 0;
            NetworkData msg(NETWORKMSG_RESOLVE_DIALOG_NO_MORE_BATTLE);
            m_pLocalClient->sendMessage(&msg);
        }
        break;
    }
    case RBS_ChooseUnits:
    {
        if (strcmp(pCpnt->getId(), "NextButton") == 0)
        {
            assert(m_pAttackingUnit != NULL);
            assert(m_pDefendingUnit != NULL);
            Unit * pAttacker = (Unit*) m_pAttackingUnit->getFirst(0);
            assert(pAttacker != NULL);
            NetworkData msg(NETWORKMSG_RESOLVE_UNITS_CHOSEN);
            msg.addLong((long)pAttacker->getId());
            msg.addLong((long)m_pAttackingUnit->getCurrentType(0)); // 1 for range, else melee
            msg.addLong((long)m_pDefendingUnit->getOwner());
            msg.addLong((long)m_pDefendingUnit->getId());
            m_pLocalClient->sendMessage(&msg);
        }
        else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
        {
            char sBuf1[512] = "";
            char sBuf2[512] = "";
            i18n->getText("(s)_CHOOSE_BATTLE", sBuf1, 512);
            assert(m_pAttacker != NULL);
            snprintf(sBuf2, 512, sBuf1, m_pAttacker->getAvatarName());
            setStatus(sBuf2);
            setChooseBattleScreen();
            m_BattleStep = RBS_ChooseBattle;
        }
        break;
    }
    }
    return true;
}

// -----------------------------------------------------------------
// Name : onMessage
// -----------------------------------------------------------------
void ResolveDlg::onMessage(int iMessage, NetworkData * pData)
{
    switch (iMessage)
    {
    case NETWORKMSG_PROCESS_AI:
    {
        char sBuf1[512] = "";
        char sBuf2[512] = "temp";
        void * p = &sBuf2;
        i18n->getText("PROCESSING_AI_%$1s", sBuf1, 512, &p);
        setStatus(sBuf1);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_RESOLVE_NEUTRAL_AI:
    {
        char sBuf1[512] = "";
        i18n->getText("PROCESSING_NEUTRAL_AI", sBuf1, 512);
        setStatus(sBuf1);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_RESOLVING_SPELL_ORDERS:
    {
        char sBuf1[512] = "";
        char sBuf2[512] = "";
        i18n->getText("RESOLVING_SPELLS_(s)", sBuf1, 512);
        Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(pPlayer != NULL);
        snprintf(sBuf2, 512, sBuf1, pPlayer->getAvatarName());
        setStatus(sBuf2);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_RESOLVING_MOVE_ORDERS:
    {
        char sBuf1[512] = "";
        char sBuf2[512] = "";
        i18n->getText("RESOLVING_MOVES_(s)", sBuf1, 512);
        Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(pPlayer != NULL);
        snprintf(sBuf2, 512, sBuf1, pPlayer->getAvatarName());
        setStatus(sBuf2);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_RESOLVE_SELECT_BATTLE:
    {
        u8 uNbAllPlayers = m_pLocalClient->getPlayerManager()->getPlayersCount();
        for (int i = 0; i < uNbAllPlayers; i++)
            m_pCastSpellForThisBattle[i] = true;
        char sBuf1[512] = "";
        char sBuf2[512] = "";
        i18n->getText("(s)_CHOOSE_BATTLE", sBuf1, 512);
        m_pAttacker = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(m_pAttacker != NULL);
        snprintf(sBuf2, 512, sBuf1, m_pAttacker->getAvatarName());
        setStatus(sBuf2);
        updateChooseBattlePanel(pData);
        setChooseBattleScreen();
        m_BattleStep = RBS_ChooseBattle;
        break;
    }
    case NETWORKMSG_RESOLVE_OTHER_PLAYER_SELECTS_BATTLE:
    {
        u8 uNbAllPlayers = m_pLocalClient->getPlayerManager()->getPlayersCount();
        for (int i = 0; i < uNbAllPlayers; i++)
            m_pCastSpellForThisBattle[i] = true;
        char sBuf1[512] = "";
        char sBuf2[512] = "";
        i18n->getText("WAIT_(s)_CHOOSING_BATTLES", sBuf1, 512);
        m_pAttacker = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(m_pAttacker != NULL);
        snprintf(sBuf2, 512, sBuf1, m_pAttacker->getAvatarName());
        setStatus(sBuf2);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_SET_RESOLVE_DIALOG_UNITS:
    {
        char sBuf1[512] = "";
        char sBuf2[512] = "";
        i18n->getText("(s)_SELECT_ATTACKER_AND_TARGET", sBuf1, 512);
        assert(m_pAttacker != NULL);
        snprintf(sBuf2, 512, sBuf1, m_pAttacker->getAvatarName());
        setStatus(sBuf2);
        updateChooseUnitsPanel(pData);
        m_BattleStep = RBS_ChooseUnits;
        setChooseUnitsScreen();
        break;
    }
    case NETWORKMSG_RESOLVE_START_CAST_BATTLE_SPELL:
    {
        m_pAllowedDefendersPerAttacker->deleteAll();
        MetaObjectList * pList = new MetaObjectList(false);
        m_pAllowedDefendersPerAttacker->addLast(pList);

        m_pAttacker = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(m_pAttacker != NULL);
        Unit * pUnit = m_pAttacker->findUnit(pData->readLong());
        assert(pUnit != NULL);
        bool isRange = (pData->readLong() == 1);
        m_pAttackingUnit = pList;
        m_pAttackingUnit->addLast(pUnit, isRange ? 1 : 0);
        Player * pDef = m_pLocalClient->getPlayerManager()->findPlayer((u8)pData->readLong());
        assert(pDef != NULL);
        m_pDefendingUnit = pDef->findUnit(pData->readLong());
        assert(m_pDefendingUnit != NULL);

        char sBuf[512] = "";
        void * pPhraseArgs[2];
        pPhraseArgs[0] = m_pAttacker->getAvatarName();
        pPhraseArgs[1] = pDef->getAvatarName();
        i18n->getText("(1s)_IS_ATTACKING_(2s)_CAN_CAST_SPELLS", sBuf, 512, pPhraseArgs);
        setStatus(sBuf);
        // Loop through all local players that haven't answered yet "i don't want to cast any spell on this battle"
        // Ask them, one after another, if they cast spell
        // Once asked to everyone, send all spells to server
        m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(m_iPlayerIt);
        m_BattleStep = RBS_AskCastBattleSpells;
        setCurrentBattleScreen();
        FREE(m_pSpellData);
        m_pSpellData = new NetworkData(NETWORKMSG_CAST_BATTLE_SPELLS);
        askNextPlayerCastSpells();
        break;
    }
    case NETWORKMSG_RESOLVE_START_CAST_POST_BATTLE_SPELL:
    {
        char sBuf1[512] = "";
        i18n->getText("NO_MORE_BATTLES_CAN_CAST_SPELLS", sBuf1, 512);
        setStatus(sBuf1);
        m_iNbBattles = 0;
        m_pAttacker = NULL;
        m_pAttackingUnit = NULL;
        m_pDefendingUnit = NULL;
        // Loop through all local players that haven't answered yet "i don't want to cast any spell on this battle"
        // Ask them, one after another, if they cast spell
        // Once asked to everyone, send all spells to server
        m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(m_iPlayerIt);
        m_BattleStep = RBS_AskCastBattleSpells;
        setSimpleStatusScreen();
        FREE(m_pSpellData);
        m_pSpellData = new NetworkData(NETWORKMSG_CAST_POST_BATTLE_SPELLS);
        askNextPlayerCastSpells();
        break;
    }
    case NETWORKMSG_RESOLVE_DIALOG_UPDATE_TOWNS:
    {
        char sBuf[512] = "";
        i18n->getText("UPDATING_TOWNS", sBuf, 512);
        setStatus(sBuf);
        setSimpleStatusScreen();
        m_BattleStep = RBS_Passive;
        break;
    }
    case NETWORKMSG_RESOLVE_NEED_SELECT_TARGET:
    {
        LuaContext context;
        if (!context.deserialize(pData, m_pLocalClient->getPlayerManager(), m_pLocalClient->getGameboard()->getMap()))
            break;
        extern u8 g_uLuaSelectTargetType;
        extern u32 g_uLuaSelectConstraints;
        g_uLuaSelectTargetType = (u8) pData->readLong();
        g_uLuaSelectConstraints = (u32) pData->readLong();
        pData->readString(m_sSelectTargetLuaCallback);

        char sStatus[512] = "";
        switch (context.pLua->getType())
        {
        case LUAOBJECT_SPELL:
        {
            assert(context.pPlayer->m_uClientId == m_pLocalClient->getClientId()); // must have been sent to right client
            void * pPhraseArgs[2];
            pPhraseArgs[0] = context.pPlayer->getAvatarName();
            pPhraseArgs[1] = context.pLua->getLocalizedName();
            i18n->getText("(1s)_CAST_(2s)_SELECT_TARGET", sStatus, 512, pPhraseArgs);
            // Set current spell and caster
            m_pLocalClient->getPlayerManager()->setSpellBeingCastOnResolve((Spell*) (context.pLua));
            m_pLocalClient->getInterface()->getSpellDialog()->updateContent(context.pPlayer, false, true);
            break;
        }
        case LUAOBJECT_SKILL:
        {
            assert(context.pPlayer->m_uClientId == m_pLocalClient->getClientId()); // must have been sent to right client
            void * pPhraseArgs[2];
            pPhraseArgs[0] = context.pPlayer->getAvatarName();
            pPhraseArgs[1] = context.pLua->getLocalizedName();
            i18n->getText("(1s)_ACTIVATE_(2s)_SELECT_TARGET", sStatus, 512, pPhraseArgs);
            // Set current spell and caster
            m_pLocalClient->getPlayerManager()->setSkillBeingActivatedOnResolve(context.pUnit, context.pLua->getChildEffect(context.pLua->getCurrentEffect()));
            m_pLocalClient->getInterface()->getSpellDialog()->updateContent(context.pPlayer, false, true);
            break;
        }
        default:  // no "resolve" function for buildings and special tiles
            return;
        }
        setStatus(sStatus);
        setSimpleStatusScreen();
        m_BattleStep = RBS_SelectSpellTarget;
        extern int LUA_selectTarget(const char*, bool);
        LUA_selectTarget("", true);
        break;
    }
    }
}

// -----------------------------------------------------------------
// Name : resolveTargetSelectionFinished
// -----------------------------------------------------------------
void ResolveDlg::resolveTargetSelectionFinished(bool bCanceled, LuaObject * pLua, ChildEffect * pChild, Unit * pUnit)
{
    u32 uType = pLua->getType();
    NetworkData msg(NETWORKMSG_RESOLVE_TARGET_SELECTED);
    msg.addLong(pChild == NULL ? 0 : 1);
    msg.addLong(uType); // To say if it's a spell or a skill
    msg.addLong(bCanceled ? 1 : 0);
    if (!bCanceled)
    {
        msg.addString(m_sSelectTargetLuaCallback);
        if (pChild == NULL) // the main object is cast
        {
            if (pLua->getType() == LUAOBJECT_SPELL)
                msg.addString(((Spell*)pLua)->getResolveParameters());
        }
        else  // Child object is activated
        {
            msg.addString(pChild->sResolveParams);
        }
    }
    m_pLocalClient->sendMessage(&msg);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void ResolveDlg::setTargetValid(bool bValid)
{
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool ResolveDlg::onClickStart()
{
    if (m_pSpellsPopup != NULL && m_pSpellsPopup->isVisible())
    {
        m_pSpellsPopup->setReponse(BATTLESPELL_BUTTON_FINISHED);
        return true;
    }
    return false;
}
