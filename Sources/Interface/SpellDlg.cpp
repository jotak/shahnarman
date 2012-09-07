#include "SpellDlg.h"
#include "SpellsSelectorDlg.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Players/Player.h"
#include "../Geometries/GeometryQuads.h"
#include "../GUIClasses/guiImage.h"
#include "../Players/PlayerManager.h"
#include "../Interface/InterfaceManager.h"
#include "../Interface/InfoboxDlg.h"
#include "../Players/Spell.h"
#include "../Gameboard/GameboardManager.h"

#define SPACING   4

// -----------------------------------------------------------------
// Name : SpellDlg
//  Constructor
// -----------------------------------------------------------------
SpellDlg::SpellDlg(LocalClient * pLocalClient, bool bHideSpells) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pCurrentPlayer = NULL;
  m_pCanceledSpell = NULL;
  m_bOnlyInstants = false;
  m_bCantCastSpells = false;

  wchar_t sTitle[64];
  i18n->getText(L"SPELLS", sTitle, 64);
  init(sTitle,
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());
}

// -----------------------------------------------------------------
// Name : ~SpellDlg
//  Destructor
// -----------------------------------------------------------------
SpellDlg::~SpellDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy SpellDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy SpellDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool SpellDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (wcscmp(pCpnt->getId(), L"ActiveSpellsButton") == 0)
  {
    m_pLocalClient->getInterface()->getSpellsSelectorDialog()->showPlayersSpells(m_pLocalClient->getPlayerManager()->getPlayersList(), 0, NULL);
  }
  else if (wcscmp(pCpnt->getId(), L"GameMenuButton") == 0)
  {
    m_pLocalClient->getInterface()->showInGameMenu();
  }
  else if (wcscmp(pCpnt->getId(), L"EoTButton") == 0)
  {
    m_pLocalClient->getPlayerManager()->requestEndPlayerOrders();
    return false; // don't try to send data because the button will be deleted
  }
  else if (wcscmp(pCpnt->getId(), L"SpellButton") == 0)
  {
    if (((guiToggleButton*)pCpnt)->getClickState())
    {
      assert(m_pCurrentPlayer != NULL);
      // Make sure the player still has enough mana
      Spell * pSpell = (Spell*)pCpnt->getAttachment();
      if (takeMana(pSpell->getCost()))
        m_pLocalClient->getPlayerManager()->castSpell(m_pCurrentPlayer, pSpell);
      else
        ((guiToggleButton*)pCpnt)->setClickState(false);
    }
    else
    {
      m_pCanceledSpell = (Spell*)pCpnt->getAttachment();
      extern void clbkSelectTarget_cancelSelection(u32, int);
      clbkSelectTarget_cancelSelection(LUAOBJECT_SPELL, 0);
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : updateContent
// -----------------------------------------------------------------
void SpellDlg::updateContent(Player * pPlayer, bool bOnlyInstants, bool bCantCastSpells)
{
  deleteAllComponents();
  m_pCurrentPlayer = pPlayer;
  m_bOnlyInstants = bOnlyInstants;
  m_bCantCastSpells = bCantCastSpells;

  int iWidth = getWidth();
  wchar_t sText[LABEL_MAX_CHARS];
  // Active spells button
  int iTex = getDisplay()->getTextureEngine()->loadTexture(L"active_spells");
  guiButton * pBtn1 = guiButton::createDefaultImageButton(iTex, L"ActiveSpellsButton", getDisplay());
  pBtn1->setOverOption(BCO_Decal);
  pBtn1->moveTo(0, 0);
  pBtn1->setTooltipText(i18n->getText(L"SHOW_ACTIVE_SPELLS", sText, LABEL_MAX_CHARS));
  addComponent(pBtn1);

  // Game menu button
  iTex = getDisplay()->getTextureEngine()->loadTexture(L"game_menu");
  pBtn1 = guiButton::createDefaultImageButton(iTex, L"GameMenuButton", getDisplay());
  pBtn1->setOverOption(BCO_Decal);
  pBtn1->moveTo(iWidth / 2, 0);
  pBtn1->setTooltipText(i18n->getText(L"SHOW_GAME_MENU", sText, LABEL_MAX_CHARS));
  addComponent(pBtn1);

  if (pPlayer == NULL)
    return;

  // Mana
  m_RemainingMana = pPlayer->getMana() - pPlayer->m_SpentMana;
  int xPxl = iWidth / 4;
  int yPxl = pBtn1->getHeight() + 2 * SPACING;
  wchar_t str[8];
  guiLabel * pLbl = new guiLabel();
  swprintf(str, 8, L"%c %d", MANA_LIFE_CHAR, (int) m_RemainingMana[MANA_LIFE]);
  pLbl->init(str, TEXT_FONT, TEXT_COLOR, L"LifeLabel", 0, 0, 0, 0, getDisplay());
  pLbl->moveTo(xPxl - pLbl->getWidth() / 2, yPxl);
  pLbl->setTooltipText(i18n->getText(L"LIFE", sText, LABEL_MAX_CHARS));
  addComponent(pLbl);

  pLbl = new guiLabel();
  swprintf(str, 8, L"%c %d", MANA_LAW_CHAR, (int) m_RemainingMana[MANA_LAW]);
  pLbl->init(str, TEXT_FONT, TEXT_COLOR, L"LawLabel", 0, 0, 0, 0, getDisplay());
  pLbl->moveTo(3 * xPxl - pLbl->getWidth() / 2, yPxl);
  pLbl->setTooltipText(i18n->getText(L"LAW", sText, LABEL_MAX_CHARS));
  addComponent(pLbl);

  yPxl += pLbl->getHeight() + SPACING;
  pLbl = new guiLabel();
  swprintf(str, 8, L"%c %d", MANA_DEATH_CHAR, (int) m_RemainingMana[MANA_DEATH]);
  pLbl->init(str, TEXT_FONT, TEXT_COLOR, L"DeathLabel", 0, 0, 0, 0, getDisplay());
  pLbl->moveTo(xPxl - pLbl->getWidth() / 2, yPxl);
  pLbl->setTooltipText(i18n->getText(L"DEATH", sText, LABEL_MAX_CHARS));
  addComponent(pLbl);

  pLbl = new guiLabel();
  swprintf(str, 8, L"%c %d", MANA_CHAOS_CHAR, (int) m_RemainingMana[MANA_CHAOS]);
  pLbl->init(str, TEXT_FONT, TEXT_COLOR, L"ChaosLabel", 0, 0, 0, 0, getDisplay());
  pLbl->moveTo(3 * xPxl - pLbl->getWidth() / 2, yPxl);
  pLbl->setTooltipText(i18n->getText(L"CHAOS", sText, LABEL_MAX_CHARS));
  addComponent(pLbl);

  int buttonSize = iWidth - 2 * SPACING;
  yPxl = pLbl->getYPos() + pLbl->getHeight() + 3 * SPACING;
  Spell * pSpell = (Spell*) pPlayer->m_pHand->getFirst(0);
  while (pSpell != NULL)
  {
    int iTexId = getDisplay()->getTextureEngine()->loadTexture(pSpell->getIconPath());
    guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(iTexId, buttonSize, L"SpellButton", getDisplay());
    pBtn->setOverOption(BCO_Decal);
    pBtn->moveTo(SPACING, yPxl);
    pBtn->setAttachment(pSpell);
    pSpell->setAttachment(pBtn);
    wchar_t sMana[32] = L"";
    swprintf(sText, LABEL_MAX_CHARS, L"#fH2#%s  #fTEXT#%s#n#i%s##n%s",
            pSpell->getLocalizedName(),
            pSpell->getManaText(sMana, 32),
            pSpell->getIconPath(),
            pSpell->getLocalizedDescription());
    if (wcscmp(pSpell->getTargetInfo(), L"") != 0)
    {
      wchar_t sBuf1[256] = L"";
      wchar_t sBuf2[256] = L"";
      i18n->getText(L"CAST_ON_(s)", sBuf1, 256);
      swprintf(sBuf2, 256, sBuf1, pSpell->getTargetInfo());
      wsafecat(sText, LABEL_MAX_CHARS, L"\n");
      wsafecat(sText, LABEL_MAX_CHARS, sBuf2);
    }
    pBtn->setTooltipText(sText);
    addComponent(pBtn);
    if (!(pSpell->getCost() <= m_RemainingMana) || (bOnlyInstants && !pSpell->isAllowedInBattle()) || bCantCastSpells)
      pBtn->setEnabled(false);
    yPxl += buttonSize + SPACING;
    pSpell = (Spell*) pPlayer->m_pHand->getNext(0);
  }

  // Game menu button
  iTex = getDisplay()->getTextureEngine()->loadTexture(L"EoT");
  pBtn1 = guiButton::createDefaultImageButton(iTex, L"EoTButton", getDisplay());
  pBtn1->setDimensions(buttonSize, buttonSize);
  pBtn1->setOverOption(BCO_Decal);
  pBtn1->moveTo(0, getHeight() - buttonSize);
  pBtn1->setTooltipText(i18n->getText(L"END_OF_TURN", sText, LABEL_MAX_CHARS));
  addComponent(pBtn1);
}

// -----------------------------------------------------------------
// Name : updateManaLabels
// -----------------------------------------------------------------
void SpellDlg::updateManaLabels()
{
  int xPxl = getWidth() / 4;
  wchar_t str[4];
  swprintf(str, 4, L"%c %d", MANA_LIFE_CHAR, (int) m_RemainingMana[MANA_LIFE]);
  guiLabel * pLbl = (guiLabel*) getComponent(L"LifeLabel");
  pLbl->setText(str);
  pLbl->moveTo(xPxl - pLbl->getWidth() / 2, pLbl->getYPos());
  swprintf(str, 4, L"%c %d", MANA_LAW_CHAR, (int) m_RemainingMana[MANA_LAW]);
  pLbl = (guiLabel*) getComponent(L"LawLabel");
  pLbl->setText(str);
  pLbl->moveTo(3 * xPxl - pLbl->getWidth() / 2, pLbl->getYPos());
  swprintf(str, 4, L"%c %d", MANA_DEATH_CHAR, (int) m_RemainingMana[MANA_DEATH]);
  pLbl = (guiLabel*) getComponent(L"DeathLabel");
  pLbl->setText(str);
  pLbl->moveTo(xPxl - pLbl->getWidth() / 2, pLbl->getYPos());
  swprintf(str, 4, L"%c %d", MANA_CHAOS_CHAR, (int) m_RemainingMana[MANA_CHAOS]);
  pLbl = (guiLabel*) getComponent(L"ChaosLabel");
  pLbl->setText(str);
  pLbl->moveTo(3 * xPxl - pLbl->getWidth() / 2, pLbl->getYPos());

  // Also enable or disable (un)available spells due to mana change
  if (m_pCurrentPlayer != NULL)
  {
    Spell * pSpell = (Spell*) m_pCurrentPlayer->m_pHand->getFirst(0);
    while (pSpell != NULL)
    {
      guiToggleButton * pBtn = (guiToggleButton*) pSpell->getAttachment();
      assert(pBtn != NULL);
      if (pBtn->getClickState() == false
          && (!(pSpell->getCost() <= m_RemainingMana)
          || (m_bOnlyInstants && !pSpell->isAllowedInBattle()) || m_bCantCastSpells))
        pBtn->setEnabled(false);
      else
        pBtn->setEnabled(true);
      pSpell = (Spell*) m_pCurrentPlayer->m_pHand->getNext(0);
    }
  }
}

// -----------------------------------------------------------------
// Name : cancelCastSpell
// -----------------------------------------------------------------
void SpellDlg::cancelCastSpell(Spell * pSpell)
{
  if (pSpell == NULL)
    pSpell = m_pCanceledSpell;
  assert(pSpell != NULL);
  guiToggleButton * pBtn = (guiToggleButton*) pSpell->getAttachment();
  assert(pBtn != NULL);
  pBtn->setClickState(false);
  restituteMana(pSpell->getCost() + pSpell->getExtraMana());
  pSpell->resetResolveParameters();
}

// -----------------------------------------------------------------
// Name : updateSpellInformation
// -----------------------------------------------------------------
void SpellDlg::updateSpellInformation(Spell * pSpell)
{
  if (pSpell == NULL)
  {
    pSpell = m_pCanceledSpell;
    m_pCanceledSpell = NULL;
  }
  assert(pSpell != NULL);
  wchar_t sText[LABEL_MAX_CHARS];
  guiToggleButton * pBtn = (guiToggleButton*) pSpell->getAttachment();
  assert(pBtn != NULL);
  pBtn->setTooltipText(pSpell->getInfo(sText, LABEL_MAX_CHARS));
}

// -----------------------------------------------------------------
// Name : getCastSpellsData
// -----------------------------------------------------------------
void SpellDlg::getCastSpellsData(NetworkData * pData)
{
  pData->addLong((long)getNumberOfClickedSpells());
  guiComponent * pCpnt = getFirstComponent();
  while (pCpnt != NULL)
  {
    if (wcscmp(pCpnt->getId(), L"SpellButton") == 0)
    {
      if (((guiToggleButton*)pCpnt)->getClickState())
      {
        Spell * pSpell = (Spell*)pCpnt->getAttachment();
        pData->addLong((long) pSpell->getInstanceId());
        pData->addString(pSpell->getResolveParameters());
      }
    }
    pCpnt = getNextComponent();
  }
}

// -----------------------------------------------------------------
// Name : getNumberOfClickedSpells
// -----------------------------------------------------------------
int SpellDlg::getNumberOfClickedSpells()
{
  int nbSpells = 0;
  guiComponent * pCpnt = getFirstComponent();
  while (pCpnt != NULL)
  {
    if (wcscmp(pCpnt->getId(), L"SpellButton") == 0)
    {
      if (((guiToggleButton*)pCpnt)->getClickState())
        nbSpells++;
    }
    pCpnt = getNextComponent();
  }
  return nbSpells;
}

// -----------------------------------------------------------------
// Name : takeMana
// -----------------------------------------------------------------
bool SpellDlg::takeMana(Mana mana)
{
  if (m_pCurrentPlayer != NULL && mana <= m_RemainingMana)
  {
    m_RemainingMana -= mana;
    updateManaLabels();
    return true;
  }
  return false;
}

// -----------------------------------------------------------------
// Name : restituteMana
// -----------------------------------------------------------------
void SpellDlg::restituteMana(Mana mana)
{
  if (m_pCurrentPlayer != NULL)
  {
    m_RemainingMana += mana;
    updateManaLabels();
  }
}

// -----------------------------------------------------------------
// Name : disableEOT
// -----------------------------------------------------------------
void SpellDlg::disableEOT(bool bDisabled)
{
  guiComponent * pCpnt = getComponent(L"EoTButton");
  assert(pCpnt != NULL);
  pCpnt->setEnabled(!bDisabled);
}
