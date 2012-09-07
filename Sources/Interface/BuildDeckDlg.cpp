#include "BuildDeckDlg.h"
#include "ArtifactsEquipDlg.h"
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
#include "../Players/Spell.h"
#include "../Geometries/GeometryQuads.h"
#include "../Debug/DebugManager.h"

#define SPACING    4

// -----------------------------------------------------------------
// Name : BuildDeckDlg
//  Constructor
// -----------------------------------------------------------------
BuildDeckDlg::BuildDeckDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_uDraggingFrom = 0;
  m_pCurrentSpell = NULL;
  m_pCurrentPlayer = NULL;
  m_pCurrentAvatar = NULL;
  m_bEquipedDeployed = true;
  m_bNotEquipedDeployed = true;
  m_pEditAvatarInfosPopup = NULL;
  m_pCurrentEditingAvatar = NULL;
  m_pEditAvatarCaller = NULL;
  m_pConfirmDelete = NULL;

  init("Build deck",
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
  int listWidth = (getWidth() - buttonSize) / 2 - 2 * SPACING;

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

  // Create life filter button
  guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("mana_1000"),
    buttonSize, "LifeButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("LIFE", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create law filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("mana_0100"),
    buttonSize, "LawButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("LAW", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create death filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("mana_0010"),
    buttonSize, "DeathButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("DEATH", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create chaos filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("mana_0001"),
    buttonSize, "ChaosButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("CHAOS", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create battle filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("battle"),
    buttonSize, "BattleButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("BATTLE", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create adventure filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("adventure_filter"),
    buttonSize, "AdventureButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getTextUp("ADVENTURE", sWord, 64);
  i18n->getText("_(1s)_SPELLS_TYPE_(2s)_", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create avatar filter button
  pBtn = guiToggleButton::createDefaultTexturedToggleButton(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture("avatar_filter"),
    buttonSize, "AvatarButton", pLocalClient->getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setClickState(true);
  i18n->getText("_(s)_SPELLS_EQUIPPED_BY_ANOTHER", sTooltip, 128, pPhraseArgs);
  pBtn->setTooltipText(sTooltip);
  addComponent(pBtn);
  xPxl += buttonSize + SPACING;

  // Create button "equipments"
  int w = getWidth() / 3;
  pBtn2 = guiButton::createDefaultNormalButton(i18n->getText("EQUIPMENTS", str, 64), "EquipButton", pLocalClient->getDisplay());
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

  // Create label "Not equiped"
  yPxl += pBtn->getHeight() + 3 * SPACING;
  i18n->getText("NOT_EQUIPPED", str, 64);
  pLbl = new guiLabel();
  pLbl->init(str, H2_FONT, H2_COLOR, "NotEquippedLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
  pLbl->moveTo(SPACING, yPxl);
  addComponent(pLbl);

  // Create button "Not equiped: deploy"
  pBtn2 = guiButton::createDefaultNormalButton("-", "DeployNEqButton", pLocalClient->getDisplay());
  pBtn2->setOverOption(BCO_Scale);
  pBtn2->moveTo(listWidth - pBtn2->getWidth(), yPxl-4);
  pBtn2->setTooltipText(i18n->getText("SHOW_HIDE_ALL_IDENTICAL_SPELLS", sText, LABEL_MAX_CHARS));
  addComponent(pBtn2);

  // Create label "Equiped"
  i18n->getText("EQUIPPED", str, 64);
  pLbl = new guiLabel();
  pLbl->init(str, H2_FONT, H2_COLOR, "EquippedLabe", 0, 0, 0, 0, pLocalClient->getDisplay());
  pLbl->moveTo(getWidth() - SPACING - listWidth, yPxl);
  addComponent(pLbl);

  // Create button "Equiped: deploy"
  pBtn2 = guiButton::createDefaultNormalButton("-", "DeployEqButton", pLocalClient->getDisplay());
  pBtn2->setOverOption(BCO_Scale);
  pBtn2->moveTo(getWidth() - pBtn2->getWidth(), yPxl-4);
  pBtn2->setTooltipText(i18n->getText("SHOW_HIDE_ALL_IDENTICAL_SPELLS", sText, LABEL_MAX_CHARS));
  addComponent(pBtn2);

  KeyboardInputEngine * pKeyboardInput = pLocalClient->getInput()->hasKeyboard() ? (KeyboardInputEngine*) pLocalClient->getInput() : NULL;

  // Create list for spells not equipped
  int bottom = yPxl2 - SPACING;
  yPxl += pLbl->getHeight() + 2 * SPACING;
  m_pNotEquippedSpells = guiList::createDefaultList(listWidth, bottom - yPxl, "", pKeyboardInput, pLocalClient->getDisplay());
  m_pNotEquippedSpells->moveTo(SPACING, yPxl);
  addComponent(m_pNotEquippedSpells);

  // Create list for spells equipped
  m_pEquippedSpells = guiList::createDefaultList(listWidth, bottom - yPxl, "", pKeyboardInput, pLocalClient->getDisplay());
  m_pEquippedSpells->moveTo(getWidth() - SPACING - listWidth, yPxl);
  addComponent(m_pEquippedSpells);

  // Create button "to right"
  pBtn2 = guiButton::createDefaultNormalButton(">>", "ToRightButton", pLocalClient->getDisplay());
  pBtn2->setDimensions(buttonSize, buttonSize);
  yPxl = m_pEquippedSpells->getYPos() + m_pEquippedSpells->getHeight() / 2 - pBtn2->getHeight() - SPACING;
  pBtn2->moveTo((getWidth() - pBtn2->getWidth()) / 2, yPxl);
  addComponent(pBtn2);

  // Create button "to left"
  yPxl += pBtn2->getHeight() + 2 * SPACING;
  pBtn2 = guiButton::createDefaultNormalButton("<<", "ToLeftButton", pLocalClient->getDisplay());
  pBtn2->setDimensions(buttonSize, buttonSize);
  pBtn2->moveTo((getWidth() - pBtn2->getWidth()) / 2, yPxl);
  addComponent(pBtn2);

  // Drag image
  m_pDragImage = new guiImage();
  m_pDragImage->init(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("spellsdrag"), "", 0, 0, -1, -1, pLocalClient->getDisplay());
  m_pDragImage->setVisible(false);
  addComponent(m_pDragImage);
}

// -----------------------------------------------------------------
// Name : ~BuildDeckDlg
//  Destructor
// -----------------------------------------------------------------
BuildDeckDlg::~BuildDeckDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy BuildDeckDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy BuildDeckDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void BuildDeckDlg::update(double delta)
{
  guiDocument::update(delta);
  if (m_pEditAvatarInfosPopup != NULL)
    updateEditAvatarInfosPopup();

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

  guiComponent * pCpnt = NULL;
  bool bFocus = false;
  if (m_pEquippedSpells->hasFocus())
  {
    pCpnt = m_pEquippedSpells->getLastSelectedLabel();
    bFocus = true;
  }
  else if (m_pNotEquippedSpells->hasFocus())
  {
    pCpnt = m_pNotEquippedSpells->getLastSelectedLabel();
    bFocus = true;
  }
  if (bFocus)
    onSpellSelected(pCpnt == NULL ? NULL : pCpnt->getAttachment());

  switch (m_uDraggingFrom)
  {
  case 3: // un-equip spell(s)
    moveSelectedSpells(m_pEquippedSpells, m_pNotEquippedSpells);
    m_uDraggingFrom = 0;
    return;
  case 4: // equip spell(s)
    moveSelectedSpells(m_pNotEquippedSpells, m_pEquippedSpells);
    m_uDraggingFrom = 0;
    return;
    default:
    break;
  }
  switch (m_pEquippedSpells->getActionOnSelection())
  {
  case ButtonLeft:
    m_pEquippedSpells->onFocusLost();
    m_pNotEquippedSpells->setFocus();
    return;
  case ButtonStart:
    moveSelectedSpells(m_pEquippedSpells, m_pNotEquippedSpells);
    return;
    default:
    break;
  }
  switch (m_pNotEquippedSpells->getActionOnSelection())
  {
  case ButtonRight:
    m_pNotEquippedSpells->onFocusLost();
    m_pEquippedSpells->setFocus();
    return;
  case ButtonStart:
    moveSelectedSpells(m_pNotEquippedSpells, m_pEquippedSpells);
    return;
    default:
    break;
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool BuildDeckDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (pEvent->eEvent == Event_Drag && pEvent->eButton == Button1)
  {
    if (m_uDraggingFrom == 0)
    {
      if (pCpnt == m_pEquippedSpells)
        m_uDraggingFrom = 1;
      else if (pCpnt == m_pNotEquippedSpells)
        m_uDraggingFrom = 2;
      else
        m_uDraggingFrom = 0;
    }
    m_pDragImage->setVisible(m_uDraggingFrom > 0 && pEvent->dragDistance > 40);
    CoordsScreen csrel;
    if (m_uDraggingFrom == 1)
      csrel = CoordsScreen(m_pEquippedSpells->getXPos() + m_pEquippedSpells->getDocument()->getXPos(), m_pEquippedSpells->getYPos() + m_pEquippedSpells->getDocument()->getYPos());
    else if (m_uDraggingFrom == 2)
      csrel = CoordsScreen(m_pNotEquippedSpells->getXPos() + m_pNotEquippedSpells->getDocument()->getXPos(), m_pNotEquippedSpells->getYPos() + m_pNotEquippedSpells->getDocument()->getYPos());
    m_pDragImage->moveTo(csrel.x + pEvent->xPos - pEvent->xOffset, csrel.y + pEvent->yPos - pEvent->yOffset);
  }
  else if (pEvent->eEvent == Event_Up)
  {
    if (strcmp(pCpnt->getId(), "BackButton") == 0)
    {
      m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
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
    else if (strcmp(pCpnt->getId(), "LifeButton") == 0
      || strcmp(pCpnt->getId(), "LawButton") == 0
      || strcmp(pCpnt->getId(), "DeathButton") == 0
      || strcmp(pCpnt->getId(), "ChaosButton") == 0
      || strcmp(pCpnt->getId(), "BattleButton") == 0
      || strcmp(pCpnt->getId(), "AdventureButton") == 0
      || strcmp(pCpnt->getId(), "AvatarButton") == 0
      || strcmp(pCpnt->getId(), "EditionButton") == 0)
    {
      reloadSpells();
    }
    else if (strcmp(pCpnt->getId(), "ToRightButton") == 0)
      moveSelectedSpells(m_pNotEquippedSpells, m_pEquippedSpells);
    else if (strcmp(pCpnt->getId(), "ToLeftButton") == 0)
      moveSelectedSpells(m_pEquippedSpells, m_pNotEquippedSpells);
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
    else if (strcmp(pCpnt->getId(), "DeployEqButton") == 0)
    {
      m_bEquipedDeployed = !m_bEquipedDeployed;
      ((guiButton*)pCpnt)->setText(m_bEquipedDeployed ? "-" : "+");
      reloadSpells();
      m_pEquippedSpells->checkDocumentPosition();
    }
    else if (strcmp(pCpnt->getId(), "DeployNEqButton") == 0)
    {
      m_bNotEquipedDeployed = !m_bNotEquipedDeployed;
      ((guiButton*)pCpnt)->setText(m_bNotEquipedDeployed ? "-" : "+");
      reloadSpells();
      m_pNotEquippedSpells->checkDocumentPosition();
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
      case 1: // un-equip spell(s)
        {
          if (m_pNotEquippedSpells->isAt(pEvent->xPos + m_pEquippedSpells->getXPos() + m_pEquippedSpells->getDocument()->getXPos() - pEvent->xOffset, pEvent->yPos + m_pEquippedSpells->getYPos() + m_pEquippedSpells->getDocument()->getYPos() - pEvent->yOffset))
            m_uDraggingFrom = 3;
          else
            m_uDraggingFrom = 0;
          break;
        }
      case 2: // equip spell(s)
        {
          if (m_pEquippedSpells->isAt(pEvent->xPos + m_pNotEquippedSpells->getXPos() + m_pNotEquippedSpells->getDocument()->getXPos() - pEvent->xOffset, pEvent->yPos + m_pNotEquippedSpells->getYPos() + m_pNotEquippedSpells->getDocument()->getYPos() - pEvent->yOffset))
            m_uDraggingFrom = 4;
          else
            m_uDraggingFrom = 0;
          break;
        }
      }
      m_pDragImage->setVisible(false);
    }
  }
  else if (pEvent->eEvent == Event_Down)
  {
    if (strcmp(pCpnt->getId(), "TopLabel1") == 0)
    {
      assert(m_pCurrentAvatar != NULL);
      showEditAvatarInfosPopup(m_pCurrentAvatar, this);
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void BuildDeckDlg::onShow()
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
//    char sAvName[NAME_MAX_CHARS];
//    pAvatar->findLocalizedElement(sAvName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
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
void BuildDeckDlg::onHide()
{
  m_pObjectPopup->setVisible(false);
  m_pAvatarPopup->setVisible(false);
}

// -----------------------------------------------------------------
// Name : reloadSpells
// -----------------------------------------------------------------
void BuildDeckDlg::reloadSpells()
{
  if (m_pCurrentPlayer == NULL || m_pCurrentAvatar == NULL)
    return;

  char sId[CPNT_ID_MAX_CHARS];
  long_hash m_lEqLoaded;
  long_hash m_lNEqLoaded;
  int nbEquiped = 0;
  bool bMana[4];
  bMana[MANA_LIFE] = ((guiToggleButton*)getComponent("LifeButton"))->getClickState();
  bMana[MANA_LAW] = ((guiToggleButton*)getComponent("LawButton"))->getClickState();
  bMana[MANA_DEATH] = ((guiToggleButton*)getComponent("DeathButton"))->getClickState();
  bMana[MANA_CHAOS] = ((guiToggleButton*)getComponent("ChaosButton"))->getClickState();
  bool bBattle = ((guiToggleButton*)getComponent("BattleButton"))->getClickState();
  bool bAdventure = ((guiToggleButton*)getComponent("AdventureButton"))->getClickState();
  bool bAvatar = ((guiToggleButton*)getComponent("AvatarButton"))->getClickState();

  // Update spells lists
  m_pEquippedSpells->clear();
  m_pNotEquippedSpells->clear();
  Profile::SpellData * pObj = (Profile::SpellData*) m_pCurrentPlayer->getSpellsList()->getFirst(0);
  while (pObj != NULL)
  {
    Spell * pSpell = m_pLocalClient->getDataFactory()->findSpell(pObj->m_sEdition, pObj->m_sName);
    if (pSpell != NULL)
    {
      Mana mana = pSpell->getCost();
      bool bOk = true;
      for (int i = 0; i < 4; i++)
      {
        if (!bMana[i] && mana[i])
        {
          bOk = false;
          break;
        }
      }
      // Now check for edition filters
      if (bOk)
      {
        guiComponent * pCpnt = getFirstComponent();
        while (pCpnt != NULL)
        {
          if (strcmp(pCpnt->getId(), "EditionButton") == 0)
          {
            if (((guiToggleButton*)pCpnt)->getClickState() == false && strcmp(pSpell->getObjectEdition(), ((Edition*)(pCpnt->getAttachment()))->m_sObjectId) == 0)
            {
              bOk = false;
              break;
            }
          }
          pCpnt = getNextComponent();
        }
      }
      if (bOk && ((bBattle && pSpell->isAllowedInBattle()) || (bAdventure && !pSpell->isAllowedInBattle())))
      {
        guiList::guiListLabel * pLbl = NULL;
        AvatarData * pOwner = pObj->m_pOwner;
        pSpell->getUniqueId(sId, CPNT_ID_MAX_CHARS);

        if (pOwner == NULL) // not equipped
        {
          if (!m_bNotEquipedDeployed)
          {
            long_hash::iterator it = m_lNEqLoaded.find(sId);
            if (it == m_lNEqLoaded.end()) // Not found, it's the first one so add it
              m_lNEqLoaded[sId] = 1;
            else
            {
              m_lNEqLoaded[sId]++;
              bOk = false;  // Already in list and we don't want to have it twice => don't add it again
            }
          }
          if (bOk)
            pLbl = m_pNotEquippedSpells->addItem(pSpell->getLocalizedName(), sId);
        }
        else if (strcmp(m_pCurrentAvatar->m_sEdition, pOwner->m_sEdition) == 0  // equipped by current avatar
          && strcmp(m_pCurrentAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
        {
          nbEquiped++;
          if (!m_bEquipedDeployed)
          {
            long_hash::iterator it = m_lEqLoaded.find(sId);
            if (it == m_lEqLoaded.end()) // Not found, it's the first one so add it
              m_lEqLoaded[sId] = 1;
            else
            {
              m_lEqLoaded[sId]++;
              bOk = false;  // Already in list and we don't want to have it twice => don't add it again
            }
          }
          if (bOk)
            pLbl = m_pEquippedSpells->addItem(pSpell->getLocalizedName(), sId);
        }
        else if (bAvatar)   // equipped by another avatar (and we want to see it)
        {
          if (!m_bNotEquipedDeployed)
          {
            long_hash::iterator it = m_lNEqLoaded.find(sId);
            if (it == m_lNEqLoaded.end()) // Not found, it's the first one so add it
              m_lNEqLoaded[sId] = 1;
            else
            {
              m_lNEqLoaded[sId]++;
              bOk = false;  // Already in list and we don't want to have it twice => don't add it again
            }
          }
          if (bOk)
          {
            char sText[LABEL_MAX_CHARS];
//            char sAvatarName[NAME_MAX_CHARS];
//            pOwner->findLocalizedElement(sAvatarName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
            snprintf(sText, LABEL_MAX_CHARS, "%s (%s)", pSpell->getLocalizedName(), pOwner->m_sCustomName);
            pLbl = m_pNotEquippedSpells->addItem(sText, sId);
          }
        }
        if (pLbl != NULL)
        {
          pLbl->setAttachment(pObj);
          // Tooltip text
          char sTooltip[LABEL_MAX_CHARS];
          char sBuf[64];
          char sSep[8] = "";
          i18n->getText("COST", sBuf, 64);
          wsafecpy(sTooltip, LABEL_MAX_CHARS, sBuf);
          i18n->getText("2P", sBuf, 64);
          wsafecat(sTooltip, LABEL_MAX_CHARS, sBuf);
          // Mana images & numbers
          Mana mana = pSpell->getCost();
          char signs[4] = MANA_SIGNS;
          for (int i = 0; i < 4; i++)
          {
            if (mana[i] > 0)
            {
              // Number
              wsafecat(sTooltip, LABEL_MAX_CHARS, sSep);
              snprintf(sBuf, 64, "%c%d", signs[i], (int)mana[i]);
              wsafecat(sTooltip, LABEL_MAX_CHARS, sBuf);
              wsafecpy(sSep, 8, ", ");
            }
          }
          wsafecat(sTooltip, LABEL_MAX_CHARS, ". ");
          wsafecat(sTooltip, LABEL_MAX_CHARS, pSpell->getLocalizedDescription());
          m_pLocalClient->getDisplay()->getFontEngine()->putStringInBox(sTooltip, 300, m_aiAllFonts[(int)TEXT_FONT]);
          pLbl->setTooltipText(sTooltip);
        }
      }
    }
    else
    {
      char sError[128];
      snprintf(sError, 128, "Error: spell %s (edition %s) not found.", pObj->m_sName, pObj->m_sEdition);
      m_pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
    pObj = (Profile::SpellData*) m_pCurrentPlayer->getSpellsList()->getNext(0);
  }
  m_pEquippedSpells->sort();
  m_pNotEquippedSpells->sort();

  // If lists are not deployed, write items count
  if (!m_bEquipedDeployed)
  {
    char sText[LABEL_MAX_CHARS];
    guiList::guiListLabel * pItem = (guiList::guiListLabel*) m_pEquippedSpells->getDocument()->getFirstComponent();
    while (pItem != NULL)
    {
      snprintf(sText, LABEL_MAX_CHARS, "%s (%d)", pItem->getText(), (int) m_lEqLoaded[pItem->getId()]);
      pItem->setText(sText);
      pItem = (guiList::guiListLabel*) m_pEquippedSpells->getDocument()->getNextComponent();
    }
  }
  if (!m_bNotEquipedDeployed)
  {
    char sText[LABEL_MAX_CHARS];
    guiList::guiListLabel * pItem = (guiList::guiListLabel*) m_pNotEquippedSpells->getDocument()->getFirstComponent();
    while (pItem != NULL)
    {
      snprintf(sText, LABEL_MAX_CHARS, "%s (%d)", pItem->getText(), (int) m_lNEqLoaded[pItem->getId()]);
      pItem->setText(sText);
      pItem = (guiList::guiListLabel*) m_pNotEquippedSpells->getDocument()->getNextComponent();
    }
  }

  // Update equipped label (count spells)
  char sText[LABEL_MAX_CHARS];
  char sBuf1[LABEL_MAX_CHARS];
  char sBuf2[LABEL_MAX_CHARS];
  i18n->getText("EQUIPPED", sBuf1, LABEL_MAX_CHARS);
  i18n->getTextLow("SPELLS", sBuf2, LABEL_MAX_CHARS);
  snprintf(sText, LABEL_MAX_CHARS, "%s (%d %s)", sBuf1, nbEquiped, sBuf2);
  guiLabel * pLbl = (guiLabel*) getComponent("EquippedLabe");
  pLbl->setText(sText);
}

// -----------------------------------------------------------------
// Name : moveSelectedSpells
// -----------------------------------------------------------------
void BuildDeckDlg::moveSelectedSpells(guiList * pFrom, guiList * pTo)
{
  bool bChangeMade = false;
  bool bWasEquipped = (pFrom == m_pEquippedSpells);
  guiList::guiListLabel * pItem = pFrom->getFirstSelectedItem();
  while (pItem != NULL)
  {
    bChangeMade = true;
    // update profile
    Profile::SpellData * pObj = (Profile::SpellData*) pItem->getAttachment();
    if (bWasEquipped)
      pObj->m_pOwner = NULL;
    else
      pObj->m_pOwner = m_pCurrentPlayer->findAvatar(m_pCurrentAvatar->m_sEdition, m_pCurrentAvatar->m_sObjectId);
    pItem = pFrom->getNextSelectedItem();
  }
  if (bChangeMade)
  {
    m_pCurrentPlayer->save();
    reloadSpells();
    pFrom->checkDocumentPosition();
  }
}

// -----------------------------------------------------------------
// Name : onSpellSelected
// -----------------------------------------------------------------
void BuildDeckDlg::onSpellSelected(BaseObject * pObj)
{
  if (pObj == NULL)
  {
    if (m_pCurrentSpell != NULL)
    {
      m_pObjectPopup->getDocument()->deleteAllComponents();
      m_pCurrentSpell = NULL;
    }
    return;
  }

  if (m_pCurrentSpell != NULL && strcmp(m_pCurrentSpell->getObjectName(), ((Profile::SpellData*)pObj)->m_sName) == 0
    && strcmp(m_pCurrentSpell->getObjectEdition(), ((Profile::SpellData*)pObj)->m_sEdition) == 0)  // Same spell?
    return;

  m_pObjectPopup->getDocument()->deleteAllComponents();
  m_pCurrentSpell = (Spell*) m_pLocalClient->getDataFactory()->findSpell(((Profile::SpellData*)pObj)->m_sEdition, ((Profile::SpellData*)pObj)->m_sName);

  // Write title (spell name)
  int xPxl = SPACING;
  int yPxl = SPACING;
  guiLabel * pLbl = new guiLabel();
  pLbl->init(m_pCurrentSpell->getLocalizedName(), H2_FONT, H2_COLOR, "TitleLabe", xPxl, yPxl, m_pObjectPopup->getInnerWidth() - xPxl, 0, getDisplay());
  m_pObjectPopup->getDocument()->addComponent(pLbl);

  // Add image
  yPxl += pLbl->getHeight() + SPACING;
  int iTex = getDisplay()->getTextureEngine()->loadTexture(m_pCurrentSpell->getIconPath());
  guiImage * pImg = new guiImage();
  pImg->init(iTex, "Image", xPxl, yPxl, 64, 64, getDisplay());
  m_pObjectPopup->getDocument()->addComponent(pImg);

  // Mana images & numbers
  char str[8];
  xPxl += pImg->getWidth() + SPACING;
  Mana mana = m_pCurrentSpell->getCost();
  char signs[4] = MANA_SIGNS;
  for (int i = 0; i < 4; i++)
  {
    if (mana[i] > 0)
    {
      // Number
      snprintf(str, 8, "%c%d", signs[i], (int)mana[i]);
      pLbl = new guiLabel();
      pLbl->init(str, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, 0, 0, getDisplay());
      m_pObjectPopup->getDocument()->addComponent(pLbl);
      xPxl += pLbl->getWidth() + 2;
    }
  }

  // Write desciption
  xPxl = SPACING;
  yPxl = pImg->getYPos() + pImg->getHeight() + SPACING;
  pLbl = new guiLabel();
  pLbl->init(m_pCurrentSpell->getLocalizedDescription(), TEXT_FONT, TEXT_COLOR, "DescLabe", xPxl, yPxl, m_pObjectPopup->getInnerWidth() - xPxl, 0, getDisplay());
  m_pObjectPopup->getDocument()->addComponent(pLbl);
  if (m_pObjectPopup->getDocument()->getHeight() < pLbl->getYPos() + pLbl->getHeight())
    m_pObjectPopup->getDocument()->setHeight(pLbl->getYPos() + pLbl->getHeight());
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool BuildDeckDlg::onClickStart()
{
  doClick("BackButton");
  return true;
}

// -----------------------------------------------------------------
// Name : loadEditionButtons
// -----------------------------------------------------------------
void BuildDeckDlg::loadEditionButtons()
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
void BuildDeckDlg::onAvatarSelected()
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
//  m_pAvatarsPanel->moveTo(pLbl->getXPos() + pLbl->getWidth() + SPACING, m_pAvatarsPanel->getYPos());
//  m_pAvatarsPanel->setWidth(getWidth() - m_pAvatarsPanel->getXPos());

  reloadSpells();

  // Update avatar popup
  m_pAvatarPopup->getDocument()->deleteAllComponents();

//  char sName[NAME_MAX_CHARS] = "";
//  m_pCurrentAvatar->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
//  char sDescription[DESCRIPTION_MAX_CHARS] = "";
//  m_pCurrentAvatar->findLocalizedElement(sDescription, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
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

// -----------------------------------------------------------------
// Name : showEditAvatarInfosPopup
// -----------------------------------------------------------------
guiPopup ** BuildDeckDlg::showEditAvatarInfosPopup(AvatarData * pAvatar, guiDocument * pCaller)
{
  assert(m_pEditAvatarInfosPopup == NULL);
  m_pCurrentEditingAvatar = pAvatar;
  m_pEditAvatarCaller = pCaller;
  m_pEditAvatarInfosPopup = guiPopup::createEmptyPopup(getDisplay());
  m_pLocalClient->getInterface()->registerFrame(m_pEditAvatarInfosPopup);

  guiDocument * pDoc = m_pEditAvatarInfosPopup->getDocument();
  int docwidth = 400;

  // Avatar name label
  int yPxl = 5;
  char sText[NAME_MAX_CHARS];
  i18n->getText("ENTER_AVATAR_NAME", sText, NAME_MAX_CHARS);
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", 4, yPxl, docwidth-8, 0, getDisplay());
  pDoc->addComponent(pLbl);

  // Avatar name edit box
  yPxl += pLbl->getHeight() + 5;
  guiEditBox * pEdit = guiEditBox::createDefaultEditBox(1, false, docwidth-8, "AvatarName", (KeyboardInputEngine*)m_pLocalClient->getInput(), getDisplay());
  pEdit->moveTo(4, yPxl);
  pEdit->setText(pAvatar->m_sCustomName);
  pDoc->addComponent(pEdit);

  // Avatar description label
  yPxl += pEdit->getHeight() + 10;
  i18n->getText("ENTER_AVATAR_DESCRIPTION", sText, NAME_MAX_CHARS);
  pLbl = new guiLabel();
  pLbl->init(sText, H2_FONT, H2_COLOR, "", 4, yPxl, docwidth-8, 0, getDisplay());
  pDoc->addComponent(pLbl);

  // Avatar name edit box
  yPxl += pLbl->getHeight() + 5;
  pEdit = guiEditBox::createDefaultEditBox(16, true, docwidth-8, "AvatarDescription", (KeyboardInputEngine*)m_pLocalClient->getInput(), getDisplay());
  pEdit->moveTo(4, yPxl);
  pEdit->setText(pAvatar->m_sCustomDescription);
  pDoc->addComponent(pEdit);

  // Back button
  yPxl += pEdit->getHeight() + 10;
  i18n->getText("OK", sText, NAME_MAX_CHARS);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "OkButton", m_pLocalClient->getDisplay());
  pBtn->setWidth(docwidth-20);
  pBtn->moveTo(10, yPxl);
  pDoc->addComponent(pBtn);

  pDoc->setDimensions(docwidth, yPxl + pBtn->getHeight() + 10);
  m_pEditAvatarInfosPopup->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - docwidth / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pDoc->getHeight() / 2);
  m_pLocalClient->getInterface()->findFrameFromDoc(pCaller)->setEnabled(false);

  return &m_pEditAvatarInfosPopup;
}

// -----------------------------------------------------------------
// Name : updateEditAvatarInfosPopup
// -----------------------------------------------------------------
void BuildDeckDlg::updateEditAvatarInfosPopup()
{
  assert(m_pEditAvatarInfosPopup != NULL);
  guiComponent * pCpnt = m_pEditAvatarInfosPopup->getClickedComponent();
  if (pCpnt != NULL)
  {
    if (strcmp(pCpnt->getId(), "OkButton") == 0)
    {
      guiEditBox * pBox = (guiEditBox*) m_pEditAvatarInfosPopup->getDocument()->getComponent("AvatarName");
      assert(pBox != NULL);
      if (strcmp(pBox->getText(), "") == 0)
        m_pCurrentEditingAvatar->findLocalizedElement(m_pCurrentEditingAvatar->m_sCustomName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
      else
        wsafecpy(m_pCurrentEditingAvatar->m_sCustomName, NAME_MAX_CHARS, pBox->getText());
      pBox = (guiEditBox*) m_pEditAvatarInfosPopup->getDocument()->getComponent("AvatarDescription");
      assert(pBox != NULL);
      wsafecpy(m_pCurrentEditingAvatar->m_sCustomDescription, CUSTOMDESC_MAX_CHARS, pBox->getText());
      m_pLocalClient->getInterface()->deleteFrame(m_pEditAvatarInfosPopup);
      m_pEditAvatarInfosPopup = NULL;
      m_pEditAvatarCaller->setEnabled(true);
      Profile * pProfile = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
      pProfile->save();
    }
  }
}
