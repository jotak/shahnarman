#include "SpellsSelectorDlg.h"
#include "InterfaceManager.h"
#include "../GUIClasses/guiFrame.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiContainer.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Gameboard/MapObject.h"
#include "../Players/Spell.h"
#include "../Players/Player.h"
#include "../Gameboard/Unit.h"

#define SPACING           4

// -----------------------------------------------------------------
// Name : SpellsSelectorDlg
//  Constructor
// -----------------------------------------------------------------
SpellsSelectorDlg::SpellsSelectorDlg(LocalClient * pLocalClient, int iWidth, int iHeight) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pTarget = NULL;
  m_pCancelCallback = NULL;

  init("",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  // Cancel button
  char sText[LABEL_MAX_CHARS];
  guiButton * pBtn = guiButton::createDefaultNormalButton(i18n->getText("CLOSE", sText, LABEL_MAX_CHARS), "CancelButton", m_pLocalClient->getDisplay());
  pBtn->moveTo(iWidth / 2 - pBtn->getWidth() / 2, iHeight - pBtn->getHeight() - SPACING);
  addComponent(pBtn);

  // Create container
  m_pSpellsContainer = guiContainer::createDefaultPanel(iWidth - 2 * SPACING, iHeight - pBtn->getHeight() - 3 * SPACING, "", m_pLocalClient->getDisplay());
  m_pSpellsContainer->moveTo(SPACING, SPACING);
  addComponent(m_pSpellsContainer);
}

// -----------------------------------------------------------------
// Name : ~SpellsSelectorDlg
//  Destructor
// -----------------------------------------------------------------
SpellsSelectorDlg::~SpellsSelectorDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy SpellsSelectorDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy SpellsSelectorDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool SpellsSelectorDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "CancelButton") == 0)
  {
    if (m_pCancelCallback != NULL)
      m_pCancelCallback();
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    pFrm->setVisible(false);
    return false;
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * SpellsSelectorDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
  m_pTarget = NULL;
  int xPxlRel = xPxl - m_pSpellsContainer->getInnerXPos() - m_pSpellsContainer->getDocument()->getXPos();
  int yPxlRel = yPxl - m_pSpellsContainer->getInnerYPos() - m_pSpellsContainer->getDocument()->getYPos();
  guiComponent * cpnt = m_pSpellsContainer->getDocument()->getFirstComponent();
  while (cpnt != NULL)
  {
    if (cpnt->isAt(xPxlRel, yPxlRel))
    {
      if (strcmp(cpnt->getId(), "SpellButton") == 0)
      {
        m_pTarget = cpnt;
        break;
      }
      else if (strcmp(cpnt->getId(), "SpellLabe") == 0)
      {
        m_pTarget = (guiComponent*) cpnt->getAttachment();
        break;
      }
    }
    cpnt = m_pSpellsContainer->getDocument()->getNextComponent();
  }
  return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void SpellsSelectorDlg::setTargetValid(bool bValid)
{
  guiComponent * pOther = m_pSpellsContainer->getDocument()->getFirstComponent();
  while (pOther != NULL)
  {
    if (pOther != m_pTarget && pOther->getType() & GOTYPE_BUTTON)
      ((guiToggleButton*)pOther)->setClickState(false);
    pOther = m_pSpellsContainer->getDocument()->getNextComponent();
  }
  if (m_pTarget != NULL)
  {
    if (m_pTarget->getType() & GOTYPE_BUTTON)
      ((guiToggleButton*)m_pTarget)->setClickState(bValid);
  }
}

// -----------------------------------------------------------------
// Name : hide
// -----------------------------------------------------------------
void SpellsSelectorDlg::hide()
{
  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  pFrm->setVisible(false);
}

// -----------------------------------------------------------------
// Name : showActiveSpellsOnTile
// -----------------------------------------------------------------
void SpellsSelectorDlg::showActiveSpellsOnTile(MapTile * pTile)
{
  m_pSpellsContainer->getDocument()->deleteAllComponents();
  m_pTarget = NULL;

  char sText[LABEL_MAX_CHARS];
  int yPxl = 0;
  MapObject * pObj = pTile->getFirstMapObject();
  while (pObj != NULL)
  {
    // MapObject icon
    guiImage * pImg = new guiImage();
    pImg->init(pObj->getTexture(), "", 0, yPxl, 2 * SMALL_ICON_SIZE, 2 * SMALL_ICON_SIZE, getDisplay());
    pImg->setTooltipText(pObj->getInfo(sText, LABEL_MAX_CHARS, Dest_ShortInfoDialog));
    m_pSpellsContainer->getDocument()->addComponent(pImg);

    // Attached spells
    int switchY = 0;
    int xPxl[2];
    xPxl[0] = xPxl[1] = pImg->getWidth() + 2 * SPACING;
    LuaObject * pLua = pObj->getFirstEffect(0);
    while (pLua != NULL)
    {
      if (pLua->getType() == LUAOBJECT_SPELL)
      {
        // Spell icon
        int iTex = getDisplay()->getTextureEngine()->loadTexture(((Spell*)pLua)->getIconPath());
        guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(iTex, SMALL_ICON_SIZE, "SpellButton", getDisplay());
        pBtn->moveTo(xPxl[switchY], yPxl + switchY * SMALL_ICON_SIZE);
        pBtn->setAttachment(pLua);
        pBtn->setTooltipText(((Spell*)pLua)->getInfo(sText, LABEL_MAX_CHARS));
        m_pSpellsContainer->getDocument()->addComponent(pBtn);
        xPxl[switchY] += pBtn->getWidth() + SPACING;

        // Spell label
        guiLabel * pLbl = new guiLabel();
        pLbl->init(((Spell*)pLua)->getLocalizedName(), TEXT_FONT, TEXT_COLOR, "SpellLabe", xPxl[switchY], yPxl + switchY * SMALL_ICON_SIZE, -1, -1, getDisplay());
        pLbl->setAttachment(pBtn);
        pLbl->setTooltipText(sText);
        m_pSpellsContainer->getDocument()->addComponent(pLbl);
        xPxl[switchY] += pLbl->getWidth() + 3 * SPACING;

        switchY = 1 - switchY;
      }
      pLua = pObj->getNextEffect(0);
    }
    int width = max(xPxl[0], xPxl[1]);
    if (width > m_pSpellsContainer->getDocument()->getWidth())
      m_pSpellsContainer->getDocument()->setWidth(width);

    yPxl += pImg->getHeight() + SPACING;
    pObj = pTile->getNextMapObject();
  }

  m_pSpellsContainer->getDocument()->setHeight(yPxl);

  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  m_pLocalClient->getInterface()->bringFrameAbove(pFrm);
  pFrm->setVisible(true);
}

// -----------------------------------------------------------------
// Name : showPlayersDeck
//  iSrc : 0 = active, 1 = hand, 2 = deck, 3 = discard, 4 = active & global
// -----------------------------------------------------------------
void SpellsSelectorDlg::showPlayersSpells(ObjectList * pPlayers, int iSrc, CLBK_ON_CANCEL * pCancelCallback)
{
  m_pCancelCallback = pCancelCallback;
  char sText[LABEL_MAX_CHARS];
  guiButton * pBtn = (guiButton*) getComponent("CancelButton");
  assert(pBtn != NULL);
  if (pCancelCallback != NULL)
    i18n->getText("CANCE", sText, LABEL_MAX_CHARS);
  else
    i18n->getText("CLOSE", sText, LABEL_MAX_CHARS);
  pBtn->setText(sText);

  bool bGlobal = false;
  if (iSrc == 4)
  {
    bGlobal = true;
    iSrc = 0;
  }
  m_pTarget = NULL;
  m_pSpellsContainer->getDocument()->deleteAllComponents();
  int iDocWidth = m_pSpellsContainer->getInnerWidth();
  m_pSpellsContainer->getDocument()->setWidth(iDocWidth);

  int yPxl = 0;
  Player * pPlayer = (Player*) pPlayers->getFirst(0);
  while (pPlayer != NULL)
  {
    // Player icon
    guiImage * pImg = new guiImage();
    pImg->init(pPlayer->getAvatarTexture(), "", 0, yPxl, 2 * SMALL_ICON_SIZE, 2 * SMALL_ICON_SIZE, getDisplay());
    m_pSpellsContainer->getDocument()->addComponent(pImg);

    // Player name
    guiLabel * pLbl = new guiLabel();
    pLbl->init(pPlayer->getAvatarName(), H1_FONT, H1_COLOR, "", 2 * SMALL_ICON_SIZE + SPACING, yPxl, -1, -1, getDisplay());
    m_pSpellsContainer->getDocument()->addComponent(pLbl);
    yPxl += pImg->getHeight() + SPACING;

    // Player's spells
    int xPxl = 0;
    ObjectList * pList = iSrc == 0 ? pPlayer->m_pActiveSpells : iSrc == 1 ? pPlayer->m_pHand : iSrc == 2 ? pPlayer->m_pDeck : pPlayer->m_pDiscard;
    Spell * pSpell = (Spell*) pList->getFirst(0);
    while (pSpell != NULL)
    {
      if (!bGlobal || pSpell->isGlobal())
      {
        // Spell icon
        int iTex = getDisplay()->getTextureEngine()->loadTexture(pSpell->getIconPath());
        guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(iTex, SMALL_ICON_SIZE, "SpellButton", getDisplay());
        pBtn->moveTo(xPxl, yPxl);
        pBtn->setAttachment(pSpell);
        pBtn->setTooltipText(pSpell->getInfo(sText, LABEL_MAX_CHARS));
        m_pSpellsContainer->getDocument()->addComponent(pBtn);
        xPxl += pBtn->getWidth() + SPACING;
        if (xPxl + SMALL_ICON_SIZE > iDocWidth)
        {
          xPxl = 0;
          yPxl += pBtn->getHeight() + SPACING;
        }
      }
      pSpell = (Spell*) pList->getNext(0);
    }
    yPxl += SMALL_ICON_SIZE + 3 * SPACING;
    pPlayer = (Player*) pPlayers->getNext(0);
  }

  m_pSpellsContainer->getDocument()->setHeight(yPxl);

  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  m_pLocalClient->getInterface()->bringFrameAbove(pFrm);
  pFrm->setVisible(true);
}
