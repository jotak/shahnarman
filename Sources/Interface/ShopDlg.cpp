#include "ShopDlg.h"
#include "InterfaceManager.h"
#include "StartMenuDlg.h"
#include "SelectPlayerAvatarDlg.h"
#include "BuildDeckDlg.h"
#include "ArtifactsEquipDlg.h"
#include "../GUIClasses/guiPopup.h"
#include "../GUIClasses/guiSmartSlider.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../DeckData/ShopItem.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Profile.h"
#include "../DeckData/Edition.h"
#include "../Players/Spell.h"

#define SPACING    4

// -----------------------------------------------------------------
// Name : ShopDlg
//  Constructor
// -----------------------------------------------------------------
ShopDlg::ShopDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pBuyingShopItem = NULL;

  init(L"Shop",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, iHeight, pLocalClient->getDisplay());

  m_pShopItemDlg = NULL;

  // Create top label "Shop"
  int yPxl = SPACING;
  wchar_t sText[LABEL_MAX_CHARS];
  guiLabel * pLbl = new guiLabel();
  pLbl->init(i18n->getText(L"SHOP", sText, LABEL_MAX_CHARS), H1_FONT, H1_COLOR, L"ShopLabel", 0, 0, 0, 0, pLocalClient->getDisplay());
  pLbl->moveTo((getWidth() - pLbl->getWidth()) / 2, yPxl);
  addComponent(pLbl);

  // Create top label player name
  yPxl += pLbl->getHeight() + SPACING;
  pLbl = new guiLabel();
  pLbl->init(L"_", H2_FONT, H2_COLOR, L"PlayerLabel", 0, 0, 0, 0, pLocalClient->getDisplay());
  pLbl->moveTo(getWidth() / 2, yPxl);
  addComponent(pLbl);

  // Create player info panel
  yPxl += pLbl->getHeight() + 2 * SPACING;
  m_pPlayerPanel = guiContainer::createDefaultPanel(getWidth() - 2*SPACING, 150, L"PlayerPanel", pLocalClient->getDisplay());
  m_pPlayerPanel->moveTo(SPACING, yPxl);
  addComponent(m_pPlayerPanel);

  // Create player info label for 1st column
  int iLabelBoxWidth = (m_pPlayerPanel->getWidth() / 3) - 6 * SPACING;
  int xPxl = SPACING;
  pLbl = new guiLabel();
  pLbl->init(L"", TEXT_FONT, TEXT_COLOR, L"LabelCol1", xPxl, SPACING, iLabelBoxWidth, 0, pLocalClient->getDisplay());
  m_pPlayerPanel->getDocument()->addComponent(pLbl);

  // Create player info label for 2nd column
  xPxl += iLabelBoxWidth + 2 * SPACING;
  pLbl = new guiLabel();
  pLbl->init(L"", TEXT_FONT, TEXT_COLOR, L"LabelCol2", xPxl, SPACING, iLabelBoxWidth, 0, pLocalClient->getDisplay());
  m_pPlayerPanel->getDocument()->addComponent(pLbl);

  // Create player info label for 3rd column
  xPxl += iLabelBoxWidth + 2 * SPACING;
  pLbl = new guiLabel();
  pLbl->init(L"", TEXT_FONT, TEXT_COLOR, L"LabelCol3", xPxl, SPACING, iLabelBoxWidth, 0, pLocalClient->getDisplay());
  m_pPlayerPanel->getDocument()->addComponent(pLbl);

  // Create button "spells"
  int w = getWidth() / 3;
  guiButton * pBtn2 = guiButton::createDefaultNormalButton(i18n->getText(L"SPELLS", sText, 64), L"SpellsButton", pLocalClient->getDisplay());
  int yPxl2 = getHeight() - pBtn2->getHeight() - SPACING;
  pBtn2->setWidth(w);
  pBtn2->moveTo(0, yPxl2);
  pBtn2->setEnabled(false);
  addComponent(pBtn2);

  // Create button "equipments"
  pBtn2 = guiButton::createDefaultNormalButton(i18n->getText(L"EQUIPMENTS", sText, 64), L"EquipButton", pLocalClient->getDisplay());
  pBtn2->setWidth(w);
  pBtn2->moveTo(w+1, yPxl2);
  pBtn2->setEnabled(false);
  addComponent(pBtn2);

  // Create button "back"
  pBtn2 = guiButton::createDefaultNormalButton(i18n->getText(L"MAIN_MENU", sText, 64), L"BackButton", pLocalClient->getDisplay());
  pBtn2->setWidth(w);
  pBtn2->moveTo(2*w+1, yPxl2);
  addComponent(pBtn2);

  // Create shop panel
  yPxl += m_pPlayerPanel->getHeight() + 2 * SPACING;
  m_pShopPanel = guiContainer::createDefaultPanel(getWidth() - 2*SPACING, yPxl2 - SPACING - yPxl, L"PlayerPanel", pLocalClient->getDisplay());
  m_pShopPanel->moveTo(SPACING, yPxl);
  addComponent(m_pShopPanel);
}

// -----------------------------------------------------------------
// Name : ~ShopDlg
//  Destructor
// -----------------------------------------------------------------
ShopDlg::~ShopDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy ShopDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy ShopDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void ShopDlg::update(double delta)
{
  guiDocument::update(delta);
  if (m_pShopItemDlg != NULL)
  {
    guiComponent * pCpnt = m_pShopItemDlg->getClickedComponent();
    if (pCpnt != NULL)
    {
      if (wcscmp(pCpnt->getId(), L"BuyButton") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pShopItemDlg);
        m_pShopItemDlg = NULL;
        setEnabled(true);
        Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
        assert(pPlayer != NULL);
        pPlayer->buyItem(m_pBuyingShopItem);
        reloadContent();
      }
      else if (wcscmp(pCpnt->getId(), L"CancelButton") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pShopItemDlg);
        m_pShopItemDlg = NULL;
        setEnabled(true);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void ShopDlg::onShow()
{
  reloadContent();
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool ShopDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (wcscmp(pCpnt->getId(), L"ShopSlider") == 0)
  {
    guiSliderItem * pItem = ((guiSmartSlider*)pCpnt)->getSelectedItem();
    if (pItem != NULL)
      showShopDialog((ShopItem*)pItem);
  }
  else if (wcscmp(pCpnt->getId(), L"BackButton") == 0)
  {
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getStartMenuDialog());
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
  return true;
}

// -----------------------------------------------------------------
// Name : reloadContent
// -----------------------------------------------------------------
void ShopDlg::reloadContent()
{
  Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
  assert(pPlayer != NULL);

  wchar_t sText[LABEL_MAX_CHARS] = L"";
  wchar_t sBuf1[LABEL_MAX_CHARS];
  wchar_t sBuf2[LABEL_MAX_CHARS];
  wchar_t sBuf3[LABEL_MAX_CHARS];

  guiLabel * pLbl = (guiLabel*) getComponent(L"PlayerLabel");
  pLbl->setText(pPlayer->getName());

  getComponent(L"SpellsButton")->setEnabled(pPlayer->getAvatarsList()->size > 0);
  getComponent(L"EquipButton")->setEnabled(pPlayer->getAvatarsList()->size > 0);

  // Label col 1
  int cash = pPlayer->getCash();
  i18n->getText(L"CASH_(d)", sBuf1, LABEL_MAX_CHARS);
  swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, cash);
  wsafecpy(sText, LABEL_MAX_CHARS, sBuf2);
  int won = pPlayer->getNumberOfWonGames();
  int lost = pPlayer->getNumberOfLostGames();
  i18n->getText(L"DISPUTED_GAMES_(d)_WON_(d)_LOST_(d)", sBuf1, LABEL_MAX_CHARS);
  swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, won+lost, won, lost);
  wsafecat(sText, LABEL_MAX_CHARS, L"\n");
  wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
  if (won+lost > 0)
  {
    i18n->getText(L"PCT_VICTORIES_(d)", sBuf1, LABEL_MAX_CHARS);
    swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, (int) ((100*won)/(won+lost)));
    wsafecat(sText, LABEL_MAX_CHARS, L"\n");
    wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
  }
  ((guiLabel*)(m_pPlayerPanel->getDocument()->getComponent(L"LabelCol1")))->setText(sText);

  // Label col 2
  int navatars = pPlayer->getAvatarsList()->size;
  i18n->getText(L"AVATARS_(d)", sBuf1, LABEL_MAX_CHARS);
  swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, navatars);
  wsafecpy(sText, LABEL_MAX_CHARS, sBuf2);
  ((guiLabel*)(m_pPlayerPanel->getDocument()->getComponent(L"LabelCol2")))->setText(sText);

  // Label col 3
  int nLife = 0;
  int nLaw = 0;
  int nDeath = 0;
  int nChaos = 0;
  int nActiveSpells = 0;
  Profile::SpellData * pSpellDesc = (Profile::SpellData*) pPlayer->getSpellsList()->getFirst(0);
  while (pSpellDesc != NULL)
  {
    Spell * pSpell = m_pLocalClient->getDataFactory()->findSpell(pSpellDesc->m_sEdition, pSpellDesc->m_sName);
    if (pSpell != NULL) // NULL spell probably means that its edition was desactivated
    {
      nActiveSpells++;
      Mana mana = pSpell->getCost();
      if (mana[MANA_LIFE] > 0)
        nLife++;
      if (mana[MANA_LAW] > 0)
        nLaw++;
      if (mana[MANA_DEATH] > 0)
        nDeath++;
      if (mana[MANA_CHAOS] > 0)
        nChaos++;
    }
    pSpellDesc = (Profile::SpellData*) pPlayer->getSpellsList()->getNext(0);
  }
  i18n->getText(L"SPELLS_(d)", sBuf1, LABEL_MAX_CHARS);
  swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, nActiveSpells);
  wsafecpy(sText, LABEL_MAX_CHARS, sBuf2);
  i18n->getText(L"2P", sBuf2, LABEL_MAX_CHARS); // ":"
  i18n->getText1stUp(L"LIFE", sBuf3, LABEL_MAX_CHARS);
  swprintf(sBuf1, LABEL_MAX_CHARS, L"\n- %s%s%d", sBuf3, sBuf2, nLife);
  wsafecat(sText, LABEL_MAX_CHARS, sBuf1);
  i18n->getText1stUp(L"LAW", sBuf3, LABEL_MAX_CHARS);
  swprintf(sBuf1, LABEL_MAX_CHARS, L"\n- %s%s%d", sBuf3, sBuf2, nLaw);
  wsafecat(sText, LABEL_MAX_CHARS, sBuf1);
  i18n->getText1stUp(L"DEATH", sBuf3, LABEL_MAX_CHARS);
  swprintf(sBuf1, LABEL_MAX_CHARS, L"\n- %s%s%d", sBuf3, sBuf2, nDeath);
  wsafecat(sText, LABEL_MAX_CHARS, sBuf1);
  i18n->getText1stUp(L"CHAOS", sBuf3, LABEL_MAX_CHARS);
  swprintf(sBuf1, LABEL_MAX_CHARS, L"\n- %s%s%d\n", sBuf3, sBuf2, nChaos);
  wsafecat(sText, LABEL_MAX_CHARS, sBuf1);
  i18n->getText(L"TOTAL_SPELLS_(d)", sBuf1, LABEL_MAX_CHARS);
  swprintf(sBuf2, LABEL_MAX_CHARS, sBuf1, pPlayer->getSpellsList()->size);
  wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
  ((guiLabel*)(m_pPlayerPanel->getDocument()->getComponent(L"LabelCol3")))->setText(sText);

  int yPxl = SPACING;
  m_pShopPanel->getDocument()->deleteAllComponents();
  Edition * pEdition = m_pLocalClient->getDataFactory()->getFirstEdition();
  while (pEdition != NULL)
  {
    if (pEdition->isActive())
    {
      // Edition label
      pEdition->findLocalizedElement(sText, LABEL_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
      guiLabel * pLbl = new guiLabel();
      pLbl->init(sText, H2_FONT, H2_COLOR, L"EditionLabel", 0, 0, 0, 0, getDisplay());
      pLbl->moveTo(m_pShopPanel->getInnerWidth() / 2 - pLbl->getWidth() / 2, yPxl);
      m_pShopPanel->getDocument()->addComponent(pLbl);
      yPxl += pLbl->getHeight() + SPACING;

      // Create slider bar with shopping items
      guiSmartSlider * pSlider = new guiSmartSlider();
      pSlider->init(64, SPACING, TEXT_FONT, TEXT_COLOR, L"ShopSlider", 0, yPxl, m_pShopPanel->getInnerWidth(), 0, getDisplay());
      pEdition->addShopItems(pPlayer, pSlider, m_pLocalClient->getDebug());
      pSlider->loadGeometry();
      pSlider->setOwner(this);
      m_pShopPanel->getDocument()->addComponent(pSlider);
      yPxl += pSlider->getHeight() + 2 * SPACING;
    }
    pEdition = m_pLocalClient->getDataFactory()->getNextEdition();
  }
  m_pShopPanel->getDocument()->setHeight(yPxl);
}

// -----------------------------------------------------------------
// Name : showShopDialog
// -----------------------------------------------------------------
void ShopDlg::showShopDialog(ShopItem * pItem)
{
  Profile * pPlayer = m_pLocalClient->getInterface()->getSelectPlayerDialog()->getCurrentPlayer();
  assert(pPlayer != NULL);
  setEnabled(false);
  m_pShopItemDlg = pItem->createPopup(pPlayer->getCash(), m_pLocalClient->getDisplay());
  m_pLocalClient->getInterface()->registerFrame(m_pShopItemDlg);
  m_pShopItemDlg->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pShopItemDlg->getWidth()) / 2, (m_pLocalClient->getClientParameters()->screenYSize - m_pShopItemDlg->getHeight()) / 2);
  m_pBuyingShopItem = pItem;
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool ShopDlg::onClickStart()
{
  if (m_pShopItemDlg != NULL)
  {
    m_pShopItemDlg->getDocument()->doClick(L"BuyButton");
    return true;
  }
  return false;
}
