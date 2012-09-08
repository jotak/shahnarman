#include "MapObjectDlg.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Gameboard/MapTile.h"
#include "../Gameboard/MapObject.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Building.h"
#include "../Gameboard/Skill.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiContainer.h"
#include "../GUIClasses/guiGauge.h"
#include "../GUIClasses/guiComboBox.h"
#include "../GUIClasses/guiPopup.h"
#include "InterfaceManager.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Ethnicity.h"
#include "../DeckData/UnitData.h"

#define SPACING           4
#define LIST_HEIGHT       64

#define BUILDABLE_COLOR   (rgba(1, 1, 1, 0.3f))
#define ICON_SIZE         32


// -----------------------------------------------------------------
// Name : MapObjectDlg
//  Constructor
// -----------------------------------------------------------------
MapObjectDlg::MapObjectDlg(LocalClient * pLocalClient, int iWidth, int iHeight) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  init("",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  // Objects list panel
  int yPxl = SPACING;
  m_pListPanel = guiContainer::createDefaultPanel(iWidth - 2 * SPACING, LIST_HEIGHT, "ListPane", pLocalClient->getDisplay());
  m_pListPanel->moveTo(SPACING, yPxl);
  addComponent(m_pListPanel);

  // Ok button
  char sText[LABEL_MAX_CHARS];
  i18n->getText1stUp("OK", sText, LABEL_MAX_CHARS);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "OkButton", pLocalClient->getDisplay());
  yPxl = iHeight - SPACING - pBtn->getHeight();
  pBtn->moveTo((iWidth - pBtn->getWidth()) / 2, yPxl);
  addComponent(pBtn);

  // Content panel
  yPxl = m_pListPanel->getYPos() + m_pListPanel->getHeight() + 2 * SPACING;
  m_pContentPanel = guiContainer::createDefaultPanel(iWidth - 2 * SPACING, pBtn->getYPos() - yPxl - 2 * SPACING, "ContentPane", pLocalClient->getDisplay());
  m_pContentPanel->moveTo(SPACING, yPxl);
  addComponent(m_pContentPanel);

  // Misc.
  m_pCurrentTown = NULL;
  m_pPointedBuilding = NULL;
  m_pBuildingPopup = NULL;
}

// -----------------------------------------------------------------
// Name : ~MapObjectDlg
//  Destructor
// -----------------------------------------------------------------
MapObjectDlg::~MapObjectDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy MapObjectDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy MapObjectDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void MapObjectDlg::update(double delta)
{
  guiDocument::update(delta);
  if (m_pBuildingPopup != NULL)
  {
    guiComponent * pCpnt = m_pBuildingPopup->getClickedComponent();
    if (pCpnt != NULL)
    {
      if (strcmp(pCpnt->getId(), "OkButton") == 0)
      {
        Building * pBuilding = (Building*) pCpnt->getAttachment();
        assert(pBuilding != NULL);
        // Set building being built
        m_pCurrentTown->setCurrentBuilding(pBuilding->getObjectName());
        // Update window components
        int prod = m_pCurrentTown->getProdPerTurn();
        int cost = pBuilding->getProductionCost();
        int stock = m_pCurrentTown->getProdInStock();
        guiGauge * pGauge = (guiGauge*) m_pContentPanel->getDocument()->getComponent("ProductionGauge");
        assert(pGauge != NULL);
        pGauge->setMax(cost);
        pGauge->setValue(stock);
        char sText[LABEL_MAX_CHARS];
        char sBuf1[LABEL_MAX_CHARS];
        char sBuf2[LABEL_MAX_CHARS];
        snprintf(sText, LABEL_MAX_CHARS, "%d / %d", stock, cost);
        pGauge->setTooltipText(sText);
        int remainingTurns = getRemainingTurns(cost, prod, stock);
        i18n->getText("2P", sBuf1, LABEL_MAX_CHARS);
        i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
        snprintf(sText, LABEL_MAX_CHARS, "%s%s%d %s", pBuilding->getLocalizedName(), sBuf1, remainingTurns, sBuf2);
        guiLabel * pLbl = (guiLabel*) m_pContentPanel->getDocument()->getComponent("ProductionGaugeLb");
        assert(pLbl != NULL);
        pLbl->setText(sText);
        pLbl->centerOnComponent(pGauge);
        pLbl->setTooltipText(pGauge->getTooltipText());
        // Delete frame
        m_pLocalClient->getInterface()->deleteFrame(m_pBuildingPopup);
        m_pBuildingPopup = NULL;
        setEnabled(true);
      }
      else if (strcmp(pCpnt->getId(), "CancelButton") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pBuildingPopup);
        m_pBuildingPopup = NULL;
        setEnabled(true);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool MapObjectDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "OkButton") == 0)
  {
    m_pLocalClient->getInterface()->hideMapObjectDialog();
    return false;
  }
  else if (strcmp(pCpnt->getId(), "ListButton") == 0)
  {
    guiComponent * pCpnt2 = m_pListPanel->getDocument()->getFirstComponent();
    while (pCpnt2 != NULL)
    {
      if (pCpnt2->getType() & GOTYPE_BUTTON)
      {
        guiToggleButton * pBtn = (guiToggleButton*) pCpnt2;
        if (pBtn == pCpnt)
        {
          pBtn->setClickState(true);
          loadObject((MapObject*) pBtn->getAttachment());
        }
        else
          pBtn->setClickState(false);
      }
      pCpnt2 = m_pListPanel->getDocument()->getNextComponent();
    }
    return false;
  }
  else if (strcmp(pCpnt->getId(), "Buildable") == 0)
  {
    if (pEvent->eEvent == Event_Down)
    {
      Building * pBuilding = (Building*) pCpnt->getAttachment();
      assert(pBuilding != NULL);
      raiseBuildingPopup(pBuilding);
    }
  }
  else if (strcmp(pCpnt->getId(), "UnitListButton") == 0)
  {
    Ethnicity::TownUnit * pUnit = (Ethnicity::TownUnit*) pCpnt->getAttachment();
    m_pCurrentTown->setCurrentUnit(pUnit);
    if (pUnit != NULL)
    {
      // Set selected unit to produce
      char sText[LABEL_MAX_CHARS];
      char sBuf1[LABEL_MAX_CHARS];
      char sBuf2[LABEL_MAX_CHARS];
      int cost = m_pCurrentTown->getUnitProdTime();
      int stock = m_pCurrentTown->getUnitProdInStock();
      guiGauge * pGauge = (guiGauge*) m_pContentPanel->getDocument()->getComponent("UnitProdGauge");
      assert(pGauge != NULL);
      pGauge->setMax(cost);
      pGauge->setValue(stock);
      snprintf(sText, LABEL_MAX_CHARS, "%d / %d", stock, cost);
      pGauge->setTooltipText(sText);
      // Find related UnitData
      UnitData * pUnitData = m_pLocalClient->getDataFactory()->getUnitData(m_pCurrentTown->getEthnicityEdition(), pUnit->m_sId);
      char sUnitName[NAME_MAX_CHARS];
      pUnitData->findLocalizedElement(sUnitName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
      i18n->getText("2P", sBuf1, LABEL_MAX_CHARS);
      i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
      snprintf(sText, LABEL_MAX_CHARS, "%s%s%d %s", sUnitName, sBuf1, cost - stock, sBuf2);
      guiLabel * pLbl = (guiLabel*) m_pContentPanel->getDocument()->getComponent("UnitProdGaugeLb");
      assert(pLbl != NULL);
      pLbl->setText(sText);
      pLbl->centerOnComponent(pGauge);
      pLbl->setTooltipText(pGauge->getTooltipText());
    }
    else
    {
      // Remove current selected unit
      char sText[LABEL_MAX_CHARS];
      guiGauge * pGauge = (guiGauge*) m_pContentPanel->getDocument()->getComponent("UnitProdGauge");
      assert(pGauge != NULL);
      pGauge->setMax(1);
      pGauge->setValue(0);
      pGauge->setTooltipText("");
      i18n->getText("PRODUCING_NOTHING", sText, LABEL_MAX_CHARS);
      guiLabel * pLbl = (guiLabel*) m_pContentPanel->getDocument()->getComponent("UnitProdGaugeLb");
      assert(pLbl != NULL);
      pLbl->setText(sText);
      pLbl->centerOnComponent(pGauge);
      pLbl->setTooltipText("");
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * MapObjectDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
  if (m_pPointedBuilding != NULL)
  {
    m_pPointedBuilding->setDiffuseColor(BUILDABLE_COLOR);
    m_pPointedBuilding = NULL;
  }
  if (m_pContentPanel->isDocumentAt(xPxl, yPxl))
  {
    int xPxlRel = xPxl - m_pContentPanel->getInnerXPos() - m_pContentPanel->getDocument()->getXPos();
    int yPxlRel = yPxl - m_pContentPanel->getInnerYPos() - m_pContentPanel->getDocument()->getYPos();
    guiComponent * cpnt = m_pContentPanel->getDocument()->getFirstComponent();
    while (cpnt != NULL)
    {
      if (cpnt->isAt(xPxlRel, yPxlRel) && strcmp(cpnt->getId(), "Buildable") == 0)
      {
        m_pPointedBuilding = cpnt;
        m_pPointedBuilding->setDiffuseColor(rgb(1, 1, 1));
        break;
      }
      cpnt = m_pContentPanel->getDocument()->getNextComponent();
    }
  }
  return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : setTile
// -----------------------------------------------------------------
void MapObjectDlg::setTile(MapTile * pTile, bool bLoadObject)
{
  // Delete previous list
  m_pListPanel->getDocument()->deleteAllComponents();

  // Fill new list
  int xPxl = 0;
  MapObject * pObj = pTile->getFirstMapObject();
  while (pObj != NULL)
  {
    guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(pObj->getTexture(), m_pListPanel->getInnerHeight(), "ListButton", getDisplay());
    pBtn->moveTo(xPxl, 0);
    pBtn->setAttachment(pObj);
    pBtn->setOwner(this);
    m_pListPanel->getDocument()->addComponent(pBtn);
    if (pObj->getType() & GOTYPE_UNIT)
    {
      // Add player banner
      int btnSize = pBtn->getWidth() / 3;
      Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(pObj->getOwner());
      assert(pPlayer != NULL);
      guiImage * pImg = new guiImage();
      pImg->init(pPlayer->m_iBannerTex, "", xPxl + 2*btnSize, 0, btnSize, btnSize, getDisplay());
      pImg->setDiffuseColor(pPlayer->m_Color);
      m_pListPanel->getDocument()->addComponent(pImg);
    }
    xPxl += m_pListPanel->getInnerHeight() + SPACING;
    pObj = pTile->getNextMapObject();
  }

  if (bLoadObject && m_pListPanel->getDocument()->getFirstComponent() != NULL)
  {
    ((guiToggleButton*) (m_pListPanel->getDocument()->getFirstComponent()))->setClickState(true);
    loadObject((MapObject*) m_pListPanel->getDocument()->getFirstComponent()->getAttachment());
  }
}

// -----------------------------------------------------------------
// Name : loadObject
// -----------------------------------------------------------------
void MapObjectDlg::loadObject(MapObject * pObj)
{
  m_pContentPanel->getDocument()->setEnabled(true);
  if ((pObj->getType() & GOTYPE_UNIT) || (pObj->getType() & GOTYPE_DEAD_UNIT))
    loadUnit((Unit*) pObj);
  else if (pObj->getType() & GOTYPE_TOWN)
  {
    loadTown((Town*) pObj);
    if (!m_pLocalClient->getPlayerManager()->isPlayerReady(pObj->getOwner()))
      m_pContentPanel->getDocument()->setEnabled(false);
  }
}

// -----------------------------------------------------------------
// Name : loadUnit
// -----------------------------------------------------------------
void MapObjectDlg::loadUnit(Unit * pUnit)
{
  // Delete previous components
  m_pContentPanel->getDocument()->deleteAllComponents();

  // Add new components
  char sText[LABEL_MAX_CHARS];
  char sBuf1[LABEL_MAX_CHARS];
  // Image
  guiImage * pImg = new guiImage();
  pImg->init(pUnit->getTexture(), "", SPACING, SPACING, 100, 100, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Name
  int xPxl = pImg->getXPos() + pImg->getWidth() + 4 * SPACING;
  int width = m_pContentPanel->getDocument()->getWidth() - xPxl - SPACING;
  int yPxl = SPACING;
  if (pUnit->getStatus() == US_Dead)
  {
    char sDead[16];
    i18n->getText("DEAD", sDead, 16);
    snprintf(sText, LABEL_MAX_CHARS, "%s (%s)", pUnit->getName(), sDead);
  }
  else
    wsafecpy(sText, LABEL_MAX_CHARS, pUnit->getName());
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H1_FONT, H1_COLOR, "", xPxl, yPxl, width, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Owner banner
  u8 owner = pUnit->getOwner();
  Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(owner);

  pImg = new guiImage();
  pImg->init(pPlayer->m_iBannerTex, "", pLbl->getXPos() - 36, yPxl+1, 32, 32, getDisplay());
  pImg->setDiffuseColor(pPlayer->m_Color);
  if (owner == 0)
    i18n->getText("NEUTRA", sText, LABEL_MAX_CHARS);
  else
  {
    i18n->getText("OWNED_BY_(s)", sBuf1, LABEL_MAX_CHARS);
    snprintf(sText, LABEL_MAX_CHARS, sBuf1, pPlayer->getAvatarName());
  }
  pImg->setTooltipText(sText);
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Caracs & description
  yPxl += pLbl->getHeight() + 2 * SPACING;
  pUnit->getInfo(sText, LABEL_MAX_CHARS, Dest_MapObjDialog);
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, width, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Skills label
  yPxl = max(yPxl + pLbl->getHeight(), pImg->getYPos() + pImg->getHeight()) + 2 * SPACING;
  i18n->getText("SKILLS_AND_ACTIVE_EFFECTS", sText, LABEL_MAX_CHARS);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, -1, -1, getDisplay());
  pLbl->moveTo(m_pContentPanel->getDocument()->getWidth() / 2 - pLbl->getWidth() / 2, yPxl);
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Skills panel
  yPxl += pLbl->getHeight() + SPACING;
  int panelWidth = m_pContentPanel->getDocument()->getWidth() - 2 * SPACING;
  int panelHeight = m_pContentPanel->getDocument()->getHeight() - yPxl - SPACING;
  guiContainer * pSkillsPanel = guiContainer::createDefaultPanel(panelWidth, panelHeight, "SkillsPane", getDisplay());
  pSkillsPanel->setWidthFitBehavior(FB_FitDocumentToFrameWhenSmaller);
  pSkillsPanel->moveTo(SPACING, yPxl);
  m_pContentPanel->getDocument()->addComponent(pSkillsPanel);

  // Fill Skills panel
  yPxl = 0;
  xPxl = 0;
  panelWidth = pSkillsPanel->getInnerWidth();
  LuaObject * pEffect = pUnit->getFirstEffect(0);
  while (pEffect != NULL)
  {
    snprintf(sText, LABEL_MAX_CHARS, "%s - %s", pEffect->getLocalizedName(), pEffect->getLocalizedDescription());
    guiImage * pImg = new guiImage();
    pImg->init(getDisplay()->getTextureEngine()->loadTexture(pEffect->getIconPath()), "", xPxl, yPxl, ICON_SIZE, ICON_SIZE, getDisplay());
    pImg->setTooltipText(sText);
    pSkillsPanel->getDocument()->addComponent(pImg);
    xPxl += pImg->getWidth() + SPACING;
    if (xPxl + pImg->getWidth() > panelWidth)
    {
      xPxl = 0;
      yPxl += pImg->getHeight() + SPACING;
    }
    pEffect = pUnit->getNextEffect(0);
  }

  ChildEffect * pChild = pUnit->getFirstChildEffect(0);
  while (pChild != NULL)
  {
    // Effect name
//    void * pArgs[2] = { pChild->sName, ((LuaObject*) pChild->getAttachment())->getLocalizedName() };
//    i18n->getText("%$1s_EFFECT_OF_%$2s", sText, LABEL_MAX_CHARS, pArgs);
    guiImage * pImg = new guiImage();
    pImg->init(getDisplay()->getTextureEngine()->loadTexture(pChild->sIcon), "", xPxl, yPxl, ICON_SIZE, ICON_SIZE, getDisplay());
    pImg->setTooltipText(pChild->sName);
    pSkillsPanel->getDocument()->addComponent(pImg);
    xPxl += pImg->getWidth() + SPACING;
    if (xPxl + pImg->getWidth() > panelWidth)
    {
      xPxl = 0;
      yPxl += pImg->getHeight() + SPACING;
    }
    pChild = pUnit->getNextChildEffect(0);
  }
  pSkillsPanel->getDocument()->setHeight(yPxl + ICON_SIZE);
}

// -----------------------------------------------------------------
// Name : loadTown
// -----------------------------------------------------------------
void MapObjectDlg::loadTown(Town * pTown)
{
  m_pCurrentTown = pTown;
  // Delete previous components
  m_pContentPanel->getDocument()->deleteAllComponents();

  // Add new components
  char sText[LABEL_MAX_CHARS];
  char sBuf1[LABEL_MAX_CHARS];
  char sBuf2[LABEL_MAX_CHARS];

  // Name and size
  int halfWidth = m_pContentPanel->getDocument()->getWidth() / 2;
  int yPxl = SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%s:%d", pTown->getName(), (int)pTown->getSize());
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H1_FONT, H1_COLOR, "", 0, 0, -1, -1, getDisplay());
  pLbl->moveTo(halfWidth - pLbl->getWidth() / 2, yPxl);
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Owner banner
  u8 owner = pTown->getOwner();
  Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(owner);
  int nextY;

  guiImage * pImg = new guiImage();
  int iTex = getDisplay()->getTextureEngine()->loadTexture(pPlayer->m_sBanner);
  pImg->init(iTex, "", pLbl->getXPos() - 36, yPxl+1, 32, 32, getDisplay());
  pImg->setDiffuseColor(pPlayer->m_Color);
  if (owner == 0)
    i18n->getText("NEUTRA", sText, LABEL_MAX_CHARS);
  else
  {
    i18n->getText("OWNED_BY_(s)", sBuf1, LABEL_MAX_CHARS);
    snprintf(sText, LABEL_MAX_CHARS, sBuf1, pPlayer->getAvatarName());
  }
  pImg->setTooltipText(sText);
  m_pContentPanel->getDocument()->addComponent(pImg);
  nextY = pImg->getYPos() + pImg->getHeight();

  // Size and people
  int xPxl = pLbl->getXPos() + pLbl->getWidth() + 2 * SPACING;
  Ethnicity * pEthn = m_pLocalClient->getDataFactory()->findEdition(pTown->getEthnicityEdition())->findEthnicity(pTown->getEthnicityId());
  pEthn->findLocalizedElement(sBuf1, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
  snprintf(sText, LABEL_MAX_CHARS, "(%s)", sBuf1);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 0, -1, -1, getDisplay());
  pLbl->moveTo(xPxl, yPxl + 10);
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Image
  yPxl = nextY + 2 * SPACING;
  pImg = new guiImage();
  pImg->init(pTown->getBigTexture(), "", 0, 0, -1, -1, getDisplay());
  xPxl = halfWidth - pImg->getWidth() / 2;
  pImg->moveTo(xPxl, yPxl);
  m_pContentPanel->getDocument()->addComponent(pImg);
  int lowparty = yPxl + pImg->getHeight() + 5 * SPACING;

  // Show buildings
  Building * pBuilding = pTown->getFirstBuilding(0);
  while (pBuilding != NULL)
  {
    pImg = NULL;
    if (pBuilding->isBuilt())
    {
      int itex = getDisplay()->getTextureEngine()->loadTexture(pBuilding->getIconPath());
      pImg = new guiImage();
      pImg->init(itex, "Built", xPxl + pBuilding->getXPos(), yPxl + pBuilding->getYPos(), -1, -1, getDisplay());
      pImg->setTooltipText(pBuilding->getLocalizedName());
      pImg->setAttachment(pBuilding);
      m_pContentPanel->getDocument()->addComponent(pImg);
    }
    else if (pTown->isBuildingAllowed(pBuilding))
    {
      int itex = getDisplay()->getTextureEngine()->loadTexture(pBuilding->getIconPath());
      pImg = new guiImage();
      pImg->init(itex, "Buildable", xPxl + pBuilding->getXPos(), yPxl + pBuilding->getYPos(), -1, -1, getDisplay());
      pImg->setDiffuseColor(BUILDABLE_COLOR);
      int cost = pBuilding->getProductionCost();
      int prod = pTown->getProdPerTurn();
      int remainingTurns = getRemainingTurns(cost, prod, 0);
      i18n->getText("2P", sBuf1, LABEL_MAX_CHARS);
      i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
      snprintf(sText, LABEL_MAX_CHARS, "%s%s%d %s", pBuilding->getLocalizedName(), sBuf1, remainingTurns, sBuf2);
      pImg->setTooltipText(sText);
      pImg->setCatchClicks(true);
      pImg->setOwner(this);
      pImg->setAttachment(pBuilding);
      m_pContentPanel->getDocument()->addComponent(pImg);
    }
    pBuilding = pTown->getNextBuilding(0);
  }

  // Stats area (1st column)
  yPxl = lowparty;
  xPxl = SPACING;
  // Food icon
  pImg = new guiImage();
  pImg->init(getDisplay()->getTextureEngine()->loadTexture("food"), "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Food text
  xPxl += pImg->getWidth() + SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%d", pTown->getFoodPerTurn());
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);
  int remainingTurns = getRemainingTurns(pTown->getMaxFoodStorage(), pTown->getFoodPerTurn(), pTown->getFoodInStock());
  void * p = &remainingTurns;
  i18n->getText("GROWS_IN_%$1d_TURNS", sText, LABEL_MAX_CHARS, &p);
  pImg->setTooltipText(sText);
  pLbl->setTooltipText(sText);

  // Happiness icon
  xPxl += pLbl->getWidth() + 5 * SPACING;
  pImg = new guiImage();
  pImg->init(getDisplay()->getTextureEngine()->loadTexture("happiness"), "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Happiness text
  int happiness = (int) pTown->getValue(STRING_HAPPINESS);
  xPxl += pImg->getWidth() + SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%d", happiness);
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);
  if (happiness < -7)
    i18n->getText("CITIZENS_ARE_VERY_UNHAPPY", sText, LABEL_MAX_CHARS);
  else if (happiness < -4)
    i18n->getText("CITIZENS_ARE_UNHAPPY", sText, LABEL_MAX_CHARS);
  else if (happiness < 0)
    i18n->getText("CITIZENS_ARE_A_BIT_UNHAPPY", sText, LABEL_MAX_CHARS);
  else if (happiness == 0)
    i18n->getText("CITIZENS_ARE_NEUTRA", sText, LABEL_MAX_CHARS);
  else if (happiness > 7)
    i18n->getText("CITIZENS_ARE_VERY_HAPPY", sText, LABEL_MAX_CHARS);
  else if (happiness > 4)
    i18n->getText("CITIZENS_ARE_HAPPY", sText, LABEL_MAX_CHARS);
  else
    i18n->getText("CITIZENS_ARE_A_BIT_HAPPY", sText, LABEL_MAX_CHARS);
  pImg->setTooltipText(sText);
  pLbl->setTooltipText(sText);

  // Fear icon
  xPxl += pLbl->getWidth() + 5 * SPACING;
  pImg = new guiImage();
  pImg->init(getDisplay()->getTextureEngine()->loadTexture("fear"), "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Fear text
  int fear = (int) pTown->getValue(STRING_FEAR);
  xPxl += pImg->getWidth() + SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%d", fear);
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);
  if (fear <= 0)
    i18n->getText("FEAR_DESCRIPTION_NULL", sText, LABEL_MAX_CHARS);
  else if (fear < 3)
    i18n->getText("FEAR_DESCRIPTION_LOW", sText, LABEL_MAX_CHARS);
  else if (fear < 7)
    i18n->getText("FEAR_DESCRIPTION_AVERAGE", sText, LABEL_MAX_CHARS);
  else
    i18n->getText("FEAR_DESCRIPTION_HIGH", sText, LABEL_MAX_CHARS);
  pImg->setTooltipText(sText);
  pLbl->setTooltipText(sText);

  // Food gauge
  //int gaugeX = xPxl + pLbl->getWidth() + 25;
  //guiGauge * pGauge = guiGauge::createDefaultGauge(storage, rgb(1, 1, 0.3f), 200, 16, "", getDisplay());
  //pGauge->setValue(stock);
  //pGauge->moveTo(gaugeX, yPxl);
  //m_pContentPanel->getDocument()->addComponent(pGauge);
  //snprintf(sText, LABEL_MAX_CHARS, "%d / %d", stock, storage);
  //pGauge->setTooltipText(sText);
  //i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
  //snprintf(sText, LABEL_MAX_CHARS, "%d %s", remainingTurns, sBuf2);
  //pLbl = new guiLabel();
  //pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 0, 0, -1, -1, getDisplay());
  //pLbl->centerOnComponent(pGauge);
  //pLbl->setTooltipText(pGauge->getTooltipText());
  //m_pContentPanel->getDocument()->addComponent(pLbl);

  // Production icon
  yPxl += pImg->getHeight() + 4 * SPACING;
  xPxl = SPACING;
  pImg = new guiImage();
  pImg->init(getDisplay()->getTextureEngine()->loadTexture("prod"), "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Production text
  int prod = pTown->getProdPerTurn();
  xPxl += pImg->getWidth() + SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%d", prod);
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Production gauge and label
  guiGauge * pGauge;
  int gaugeX = xPxl + pLbl->getWidth() + 25;
  pBuilding = pTown->getCurrentBuilding();
  if (pBuilding != NULL)
  {
    int cost = pBuilding->getProductionCost();
    int stock = pTown->getProdInStock();
    pGauge = guiGauge::createDefaultGauge(cost, rgb(0.3f, 0.3f, 1), 200, 16, "ProductionGauge", getDisplay());
    pGauge->setValue(stock);
    pGauge->moveTo(gaugeX, yPxl);
    m_pContentPanel->getDocument()->addComponent(pGauge);
    snprintf(sText, LABEL_MAX_CHARS, "%d / %d", stock, cost);
    pGauge->setTooltipText(sText);
    int remainingTurns = getRemainingTurns(cost, prod, stock);
    i18n->getText("2P", sBuf1, LABEL_MAX_CHARS);
    i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
    snprintf(sText, LABEL_MAX_CHARS, "%s%s%d %s", pBuilding->getLocalizedName(), sBuf1, remainingTurns, sBuf2);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "ProductionGaugeLb", 0, 0, -1, -1, getDisplay());
    pLbl->centerOnComponent(pGauge);
    pLbl->setTooltipText(pGauge->getTooltipText());
    m_pContentPanel->getDocument()->addComponent(pLbl);
  }
  else
  {
    pGauge = guiGauge::createDefaultGauge(1, rgb(0.3f, 0.3f, 1), 200, 16, "ProductionGauge", getDisplay());
    pGauge->setValue(0);
    pGauge->moveTo(gaugeX, yPxl);
    m_pContentPanel->getDocument()->addComponent(pGauge);
    i18n->getText("BUILDING_NOTHING", sText, LABEL_MAX_CHARS);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "ProductionGaugeLb", 0, 0, -1, -1, getDisplay());
    pLbl->centerOnComponent(pGauge);
    m_pContentPanel->getDocument()->addComponent(pLbl);
  }

  // Unit production icon
  yPxl += pImg->getHeight() + 4 * SPACING;
  xPxl = SPACING;
  pImg = new guiImage();
  pImg->init(getDisplay()->getTextureEngine()->loadTexture("unitprod"), "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pImg);

  // Unit production text
  xPxl += pImg->getWidth() + SPACING;
  snprintf(sText, LABEL_MAX_CHARS, "%d%%", pTown->getUnitProdBonus());
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Unit production gauge
  Ethnicity::TownUnit * pUnit = pTown->getCurrentUnit();
  if (pUnit != NULL)
  {
    int cost = pTown->getUnitProdTime();
    int stock = pTown->getUnitProdInStock();
    pGauge = guiGauge::createDefaultGauge(cost, rgb(1, 0.3f, 0.3f), 200, 16, "UnitProdGauge", getDisplay());
    pGauge->moveTo(gaugeX, yPxl);
    pGauge->setValue(stock);
    snprintf(sText, LABEL_MAX_CHARS, "%d / %d", stock, cost);
    pGauge->setTooltipText(sText);
    // Find related UnitData
    UnitData * pUnitData = m_pLocalClient->getDataFactory()->getUnitData(m_pCurrentTown->getEthnicityEdition(), pUnit->m_sId);
    char sUnitName[NAME_MAX_CHARS];
    pUnitData->findLocalizedElement(sUnitName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
    i18n->getText("2P", sBuf1, LABEL_MAX_CHARS);
    i18n->getText("TURNS", sBuf2, LABEL_MAX_CHARS);
    snprintf(sText, LABEL_MAX_CHARS, "%s%s%d %s", sUnitName, sBuf1, cost - stock, sBuf2);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "UnitProdGaugeLb", 0, 0, -1, -1, getDisplay());
    pLbl->centerOnComponent(pGauge);
    pLbl->setTooltipText(pGauge->getTooltipText());
    m_pContentPanel->getDocument()->addComponent(pLbl);
  }
  else
  {
    pGauge = guiGauge::createDefaultGauge(1, rgb(1, 0.3f, 0.3f), 200, 16, "UnitProdGauge", getDisplay());
    pGauge->moveTo(gaugeX, yPxl);
    pGauge->setValue(0);
    i18n->getText("PRODUCING_NOTHING", sText, LABEL_MAX_CHARS);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "UnitProdGaugeLb", 0, 0, -1, -1, getDisplay());
    pLbl->centerOnComponent(pGauge);
    m_pContentPanel->getDocument()->addComponent(pLbl);
  }
  m_pContentPanel->getDocument()->addComponent(pGauge);

  // Units area (2nd column)
  // Label "Produce unit"
  xPxl = m_pContentPanel->getInnerWidth() / 3 + SPACING;
  yPxl = lowparty;
  i18n->getText("PRODUCE_UNIT", sText, LABEL_MAX_CHARS);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Combo list, buildable units
  yPxl += pLbl->getHeight() + SPACING;
  guiComboBox * pCombo = guiComboBox::createDefaultComboBox("UnitsList", m_pLocalClient->getInterface(), getDisplay());
  pCombo->moveTo(xPxl, yPxl);
  pCombo->setOwner(this);
  i18n->getText("NONE(UNITS_LIST)", sText, LABEL_MAX_CHARS);
  pCombo->addString(sText, "UnitListButton");
  Ethnicity::TownUnit * pName = (Ethnicity::TownUnit*) pEthn->m_pBaseUnits->getFirst(0);
  while (pName != NULL)
  {
    UnitData * pUnit = m_pLocalClient->getDataFactory()->getUnitData(pTown->getEthnicityEdition(), pName->m_sId);
    assert(pUnit != NULL);
    pUnit->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
    guiButton * pComboBtn = pCombo->addString(sText, "UnitListButton");
    pComboBtn->setAttachment(pName);
    pName = (Ethnicity::TownUnit*) pEthn->m_pBaseUnits->getNext(0);
  }
  // Continue to fill combo, with extra units (added by buildings for instance)
  ObjectList * pList = pTown->getExtraBuildableUnit();
  pName = (Ethnicity::TownUnit*) pList->getFirst(0);
  while (pName != NULL)
  {
    UnitData * pUnit = m_pLocalClient->getDataFactory()->getUnitData(pTown->getEthnicityEdition(), pName->m_sId);
    assert(pUnit != NULL);
    pUnit->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
    guiButton * pComboBtn = pCombo->addString(sText, "UnitListButton");
    pComboBtn->setAttachment(pName);
    char sInfo[LABEL_MAX_CHARS] = "";
    pUnit->getInfos(sInfo, LABEL_MAX_CHARS, "\n", false);
    pComboBtn->setTooltipText(sInfo);
    pName = (Ethnicity::TownUnit*) pList->getNext(0);
  }
  m_pContentPanel->getDocument()->addComponent(pCombo);

//  int x3rdCol = xPxl = 2 * m_pContentPanel->getInnerWidth() / 3 + SPACING;
  xPxl = 2 * m_pContentPanel->getInnerWidth() / 3 + SPACING;
  yPxl = lowparty;

  // Active effects
  i18n->getText("ACTIVE_EFFECTS", sText, LABEL_MAX_CHARS);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pContentPanel->getDocument()->addComponent(pLbl);

  // Active effects panel
  yPxl += pLbl->getHeight() + SPACING;
  guiContainer * pPanel = guiContainer::createDefaultPanel(m_pContentPanel->getInnerWidth() / 3 - 2 * SPACING, m_pContentPanel->getInnerHeight() - yPxl - SPACING, "ActiveEffects", getDisplay());
  pPanel->moveTo(xPxl, yPxl);
  m_pContentPanel->getDocument()->addComponent(pPanel);

  // Fill effects panel
  yPxl = 0;
  xPxl = 0;
  int panelWidth = pPanel->getInnerWidth();
  LuaObject * pEffect = pTown->getFirstEffect(0);
  while (pEffect != NULL)
  {
    snprintf(sText, LABEL_MAX_CHARS, "%s - %s", pEffect->getLocalizedName(), pEffect->getLocalizedDescription());
    guiImage * pImg = new guiImage();
    pImg->init(getDisplay()->getTextureEngine()->loadTexture(pEffect->getIconPath()), "", xPxl, yPxl, ICON_SIZE, ICON_SIZE, getDisplay());
    pImg->setTooltipText(sText);
    pPanel->getDocument()->addComponent(pImg);
    xPxl += pImg->getWidth() + SPACING;
    if (xPxl + pImg->getWidth() > panelWidth)
    {
      xPxl = 0;
      yPxl += pImg->getHeight() + SPACING;
    }
    pEffect = pTown->getNextEffect(0);
  }

  ChildEffect * pChild = pTown->getFirstChildEffect(0);
  while (pChild != NULL)
  {
    // Effect name
//    void * pArgs[2] = { pChild->sName, ((LuaObject*) pChild->getAttachment())->getLocalizedName() };
//    i18n->getText("%$1s_EFFECT_OF_%$2s", sText, LABEL_MAX_CHARS, pArgs);
    guiImage * pImg = new guiImage();
    pImg->init(getDisplay()->getTextureEngine()->loadTexture(pChild->sIcon), "", xPxl, yPxl, ICON_SIZE, ICON_SIZE, getDisplay());
    pImg->setTooltipText(pChild->sName);
    pPanel->getDocument()->addComponent(pImg);
    xPxl += pImg->getWidth() + SPACING;
    if (xPxl + pImg->getWidth() > panelWidth)
    {
      xPxl = 0;
      yPxl += pImg->getHeight() + SPACING;
    }
    pChild = pTown->getNextChildEffect(0);
  }
  pPanel->getDocument()->setHeight(yPxl + ICON_SIZE);
  pPanel->updateSizeFit();

  // Heroe chances
  //yPxl += pImg->getHeight() + 4 * SPACING;
  //xPxl = x3rdCol;
  //i18n->getText("CHANCES_APPARITION_HEROE", sText, LABEL_MAX_CHARS);
  //pLbl = new guiLabel();
  //pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  //m_pContentPanel->getDocument()->addComponent(pLbl);

  //// Production gauge and label
  //yPxl += pLbl->getHeight() + 2 * SPACING;
  //int val = abs(pTown->getValue(STRING_HEROECHANCES));
  //pGauge = guiGauge::createDefaultGauge(1000, rgb(1.0f, 1.0f, 0), 200, 16, "HeroeGauge", getDisplay());
  //pGauge->moveTo(xPxl, yPxl);
  //pGauge->setValue(val);
  //m_pContentPanel->getDocument()->addComponent(pGauge);
  //snprintf(sText, LABEL_MAX_CHARS, "%d / %d", val, 1000);
  //pGauge->setTooltipText(sText);
}

// -----------------------------------------------------------------
// Name : raiseBuildingPopup
// -----------------------------------------------------------------
void MapObjectDlg::raiseBuildingPopup(Building * pBuilding)
{
  assert(m_pCurrentTown != NULL);

  // Raise building popup
  setEnabled(false);
  m_pBuildingPopup = guiPopup::createOkCancelPopup("", m_pLocalClient->getDisplay());

  // Make popup
  int xPxl = 0;
  int yPxl = 0;
  char sText[LABEL_MAX_CHARS];
  char sBuf1[LABEL_MAX_CHARS];
  char sBuf2[LABEL_MAX_CHARS];

  // Building texture
  int itex = getDisplay()->getTextureEngine()->loadTexture(pBuilding->getIconPath());
  guiImage * pImg = new guiImage();
  pImg->init(itex, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pImg);
  int imagebottom = yPxl + pImg->getHeight();

  // Buidling name
  int width = 4 * pImg->getWidth();
  xPxl += pImg->getWidth() + SPACING;
  guiLabel * pLbl = new guiLabel();
  pLbl->init(pBuilding->getLocalizedName(), H1_FONT, H1_COLOR, "", xPxl, yPxl, width - xPxl, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pLbl);

  // Cost
  yPxl += pLbl->getHeight() + 3 * SPACING;
  int cost = pBuilding->getProductionCost();
  int prod = m_pCurrentTown->getProdPerTurn();
  int remainingTurns = getRemainingTurns(cost, prod, 0);
  i18n->getText("COST", sBuf1, LABEL_MAX_CHARS);
  i18n->getText("2P", sBuf2, LABEL_MAX_CHARS);
  snprintf(sText, LABEL_MAX_CHARS, "%s%s%d ", sBuf1, sBuf2, cost);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pLbl);

  // Prod. icon
  xPxl += pLbl->getWidth();
  itex = getDisplay()->getTextureEngine()->loadTexture("prod");
  pImg = new guiImage();
  pImg->init(itex, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pImg);

  // Turns
  xPxl += pImg->getWidth() + SPACING;
  i18n->getText("TURNS", sBuf1, LABEL_MAX_CHARS);
  snprintf(sText, LABEL_MAX_CHARS, "     %d %s", remainingTurns, sBuf1);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", xPxl, yPxl, -1, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pLbl);

  // Description
  xPxl = SPACING;
  yPxl += pLbl->getHeight();
  yPxl = max(yPxl, imagebottom) + 2 * SPACING;
  pLbl = new guiLabel();
  pLbl->init(pBuilding->getLocalizedDescription(), H2_FONT, H2_COLOR, "", xPxl, yPxl, width - xPxl, -1, getDisplay());
  m_pBuildingPopup->getDocument()->addComponent(pLbl);

  // Move buttons
  yPxl += pLbl->getHeight() + 3 * SPACING;
  guiComponent * pCpnt = m_pBuildingPopup->getDocument()->getComponent("OkButton");
  pCpnt->moveTo(3 * width / 4 - pCpnt->getWidth() / 2, yPxl);
  pCpnt->setAttachment(pBuilding);  // remember which building it is for later
  pCpnt = m_pBuildingPopup->getDocument()->getComponent("CancelButton");
  pCpnt->moveTo(width / 4 - pCpnt->getWidth() / 2, yPxl);

  // Finalize
  m_pBuildingPopup->getDocument()->setDimensions(width, yPxl + pCpnt->getHeight() + SPACING);
  m_pLocalClient->getInterface()->registerFrame(m_pBuildingPopup);
  m_pBuildingPopup->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pBuildingPopup->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pBuildingPopup->getHeight()) / 2);
}

// -----------------------------------------------------------------
// Name : setSelectedObject
// -----------------------------------------------------------------
void MapObjectDlg::setSelectedObject(MapObject * pObj)
{
  setTile(pObj->getMap()->getTileAt(pObj->getMapPos()), false);
  guiComponent * pCpnt = m_pListPanel->getDocument()->getFirstComponent();
  while (pCpnt != NULL)
  {
    if (strcmp(pCpnt->getId(), "ListButton") == 0)
    {
      guiToggleButton * pBtn = (guiToggleButton*) pCpnt;
      MapObject * pOther = (MapObject*) pBtn->getAttachment();
      pBtn->setClickState(pOther == pObj);
    }
    pCpnt = m_pListPanel->getDocument()->getNextComponent();
  }
  loadObject(pObj);
}

// -----------------------------------------------------------------
// Name : getRemainingTurns
// -----------------------------------------------------------------
int MapObjectDlg::getRemainingTurns(int cost, int delta, int stock)
{
  if (delta == 0)
    return 999;
  else if (delta > 0)
  {
    int remainingTurns = (cost - stock) / delta;
    return (remainingTurns * delta < cost - stock) ? remainingTurns + 1 : remainingTurns;
  }
  else
  {
    int remainingTurns = (cost - stock) / -delta;
    return (remainingTurns * -delta < cost - stock) ? remainingTurns + 1 : remainingTurns;
  }
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool MapObjectDlg::onClickStart()
{
  doClick("OkButton");
  return true;
}
