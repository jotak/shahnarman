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

  init(L"Select player and Avatar",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  m_pTextInput = NULL;
  m_pConfirmDelete = NULL;

  // Create top label "Choose player"
  int yPxl = 10;
  wchar_t str[LABEL_MAX_CHARS];
  guiLabel * pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"CHOOSE_PLAYER", str, LABEL_MAX_CHARS), H2_FONT, H2_COLOR, L"ChoosePlayerLabel", 0, 0, 0, 0, pLocalClient->getDisplay());
  pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, yPxl);
  addComponent(pLbl);

  // Create combo box for listing players
  yPxl += pLbl->getHeight() + SPACING;
  guiComboBox * pCombo = guiComboBox::createDefaultComboBox(L"ChoosePlayer", pLocalClient->getInterface(), pLocalClient->getDisplay());
  pCombo->moveTo(0, yPxl);
  addComponent(pCombo);
  setWidth(pCombo->getWidth());
  pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, pLbl->getYPos());

  // Shop button
  yPxl += pCombo->getHeight() + 2 * SPACING;
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"SHOP", str, LABEL_MAX_CHARS), L"ShopButton", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Create Shahmah button
  yPxl += pBtn->getHeight() + 2 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"CREATE_SHAHMAH", str, LABEL_MAX_CHARS), L"CreateShahmahButton", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Spells button
  yPxl += pBtn->getHeight() + 2 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"SPELLS", str, LABEL_MAX_CHARS), L"SpellsButton", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Equipments button
  yPxl += pBtn->getHeight() + 2 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"EQUIPMENTS", str, LABEL_MAX_CHARS), L"EquipButton", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Statistics button
  yPxl += pBtn->getHeight() + 2 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"STATISTICS", str, LABEL_MAX_CHARS), L"StatsButton", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Delete player button
  yPxl += pBtn->getHeight() + 2 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"DELETE_THIS_PLAYER", str, LABEL_MAX_CHARS), L"DeletePlayer", pLocalClient->getDisplay());
  pBtn->moveTo(0, yPxl);
  pBtn->setWidth(getWidth());
  addComponent(pBtn);

  // Main menu button
  yPxl += pBtn->getHeight() + 5 * SPACING;
  pBtn = guiButton::createDefaultNormalButton(i18n->getText(L"MAIN_MENU", str, LABEL_MAX_CHARS), L"BackButton", pLocalClient->getDisplay());
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
      if (wcscmp(pCpnt->getId(), L"OkButton") == 0)
      {
        wchar_t * sName = m_pTextInput->getEditBox()->getText();
        if (createPlayer(sName))
        {
          m_pLocalClient->getInterface()->deleteFrame(m_pTextInput);
          m_pTextInput = NULL;
          setEnabled(true);
        }
        else
        {
          m_pTextInput->getEditBox()->setText(L"Can't create player");
        }
      }
      else if (wcscmp(pCpnt->getId(), L"CancelButton") == 0)
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
      if (wcscmp(pCpnt->getId(), L"YesButton") == 0)
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
      else if (wcscmp(pCpnt->getId(), L"NoButton") == 0)
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
  if (wcscmp(pCpnt->getId(), L"LoadPlayer") == 0)
  {
    wchar_t * sText = ((guiButton*)pCpnt)->getText();
    if (wcscmp(sText, L"") != 0)
      loadPlayer(sText);
  }
  else if (wcscmp(pCpnt->getId(), L"NewPlayer") == 0)
  {
    setEnabled(false);
    unloadPlayer();
    wchar_t str[64];
    m_pTextInput = guiPopup::createTextInputPopup(i18n->getText(L"ENTER_PLAYER_NAME", str, 64), 1, false, 200, m_pLocalClient->getInput(), m_pLocalClient->getDisplay());
    m_pLocalClient->getInterface()->registerFrame(m_pTextInput);
    m_pTextInput->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pTextInput->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pTextInput->getHeight()) / 2);
  }
  else if (wcscmp(pCpnt->getId(), L"DeletePlayer") == 0)
  {
    // Raise confirm popup
    setEnabled(false);
    wchar_t str[128];
    m_pConfirmDelete = guiPopup::createYesNoPopup(i18n->getText(L"REALLY_DELETE_PLAYER", str, 128), m_pLocalClient->getDisplay());
    m_pLocalClient->getInterface()->registerFrame(m_pConfirmDelete);
    m_pConfirmDelete->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pConfirmDelete->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pConfirmDelete->getHeight()) / 2);
  }
  else if (wcscmp(pCpnt->getId(), L"BackButton") == 0)
  {
    unloadPlayer();
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
    return false;
  }
  else if (wcscmp(pCpnt->getId(), L"CreateShahmahButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getCreateAvatarDlg());
    return false;
  }
  else if (wcscmp(pCpnt->getId(), L"SpellsButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getBuildDeckDialog());
    return false;
  }
  else if (wcscmp(pCpnt->getId(), L"EquipButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getArtifactsEquipDialog());
    return false;
  }
  else if (wcscmp(pCpnt->getId(), L"ShopButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getShopDialog());
    return false;
  }
  return true;
}

// -----------------------------------------------------------------
// Name : loadPlayersList
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::loadPlayersList(wchar_t * sSelect)
{
  wchar_t str[64];
  guiComboBox * pCombo = (guiComboBox*) getComponent(L"ChoosePlayer");
  pCombo->clearList();

  int iSelect = -1;
  Profile * pProfile = m_pLocalClient->getDataFactory()->getFirstProfile();
  int count = 0;
  while (pProfile != NULL)
  {
    pCombo->addString(pProfile->getName(), L"LoadPlayer");
    if (sSelect != NULL && wcscmp(sSelect, pProfile->getName()) == 0)
      iSelect = count;
    count++;
    pProfile = m_pLocalClient->getDataFactory()->getNextProfile();
  }

  if (iSelect == -1 && count == 1)
    iSelect = 0;
  pCombo->addString(i18n->getText(L"CREATE_NEW", str, 64), L"NewPlayer");
  pCombo->setItem(iSelect);
}

// -----------------------------------------------------------------
// Name : createPlayer
// -----------------------------------------------------------------
bool SelectPlayerAvatarDlg::createPlayer(wchar_t * sName)
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

  ((guiComboBox*)getComponent(L"ChoosePlayer"))->setItem(-1);
  getComponent(L"DeletePlayer")->setEnabled(false);
  getComponent(L"SpellsButton")->setEnabled(false);
  getComponent(L"EquipButton")->setEnabled(false);
  getComponent(L"ShopButton")->setEnabled(false);
  getComponent(L"CreateShahmahButton")->setEnabled(false);
  getComponent(L"StatsButton")->setEnabled(false);
}

// -----------------------------------------------------------------
// Name : loadPlayer
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::loadPlayer(wchar_t * sName)
{
  m_pCurrentPlayer = m_pLocalClient->getDataFactory()->findProfile(sName);
  onPlayerDataChanged();
}

// -----------------------------------------------------------------
// Name : onPlayerDataChanged
// -----------------------------------------------------------------
void SelectPlayerAvatarDlg::onPlayerDataChanged()
{
  getComponent(L"SpellsButton")->setEnabled(m_pCurrentPlayer->getAvatarsList()->size > 0);
  getComponent(L"EquipButton")->setEnabled(m_pCurrentPlayer->getAvatarsList()->size > 0);
  getComponent(L"ShopButton")->setEnabled(true);
  getComponent(L"CreateShahmahButton")->setEnabled(true);
  getComponent(L"StatsButton")->setEnabled(true);
  getComponent(L"DeletePlayer")->setEnabled(true);
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool SelectPlayerAvatarDlg::onClickStart()
{
  if (m_pTextInput != NULL)
  {
    m_pTextInput->getDocument()->doClick(L"OkButton");
    return true;
  }
  if (getComponent(L"SpellsButton")->isEnabled())
  {
    doClick(L"SpellsButton");
    return true;
  }
  return false;
}
