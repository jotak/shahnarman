#include "PlayerSelectorDlg.h"
#include "InterfaceManager.h"
#include "../GUIClasses/guiFrame.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiContainer.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Players/Player.h"

#define SPACING           4

// -----------------------------------------------------------------
// Name : PlayerSelectorDlg
//  Constructor
// -----------------------------------------------------------------
PlayerSelectorDlg::PlayerSelectorDlg(LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pTarget = NULL;
  m_pCancelCallback = NULL;
  m_pSelectorImg = NULL;

  init("",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());
}

// -----------------------------------------------------------------
// Name : ~PlayerSelectorDlg
//  Destructor
// -----------------------------------------------------------------
PlayerSelectorDlg::~PlayerSelectorDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy PlayerSelectorDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy PlayerSelectorDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool PlayerSelectorDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
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
guiObject * PlayerSelectorDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
  m_pTarget = NULL;
  int xPxlRel = xPxl;
  int yPxlRel = yPxl;
  guiComponent * cpnt = getFirstComponent();
  while (cpnt != NULL)
  {
    if (cpnt->isAt(xPxlRel, yPxlRel))
    {
      if (strcmp(cpnt->getId(), "PlayerButton") == 0)
      {
        m_pTarget = cpnt;
        break;
      }
    }
    cpnt = getNextComponent();
  }
  return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void PlayerSelectorDlg::setTargetValid(bool bValid)
{
  m_pSelectorImg->setVisible(false);
  if (m_pTarget != NULL && bValid)
  {
    m_pSelectorImg->moveTo(m_pTarget->getXPos(), m_pTarget->getYPos());
    m_pSelectorImg->setVisible(true);
  }
}

// -----------------------------------------------------------------
// Name : hide
// -----------------------------------------------------------------
void PlayerSelectorDlg::hide()
{
  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  pFrm->setVisible(false);
}

// -----------------------------------------------------------------
// Name : showPlayers
// -----------------------------------------------------------------
void PlayerSelectorDlg::showPlayers(ObjectList * pPlayers, CLBK_ON_CANCEL * pCancelCallback)
{
  m_pCancelCallback = pCancelCallback;
  char sText[LABEL_MAX_CHARS];

  m_pTarget = NULL;
  int btnSize = 96;
  deleteAllComponents();
  m_pSelectorImg = new guiImage();
  int iTex = getDisplay()->getTextureEngine()->findTexture("interface:Selector");
  m_pSelectorImg->init(iTex, "SelectorImage", 0, 0, btnSize, btnSize, getDisplay());
  m_pSelectorImg->setVisible(false);
  addComponent(m_pSelectorImg);

  int xPxl = 0;
  int yPxl = 0;
  Player * pPlayer = (Player*) pPlayers->getFirst(0);
  while (pPlayer != NULL)
  {
    // Player icon
    guiImage * pImg = new guiImage();
    pImg->init(pPlayer->getAvatarTexture(), "PlayerButton", xPxl, yPxl, btnSize, btnSize, getDisplay());
    pImg->setTooltipText(pPlayer->getAvatarName());
    pImg->setAttachment(pPlayer);
    addComponent(pImg);

    pImg = new guiImage();
    pImg->init(pPlayer->m_iBannerTex, "PlayerButton", xPxl + btnSize - SMALL_ICON_SIZE, yPxl, SMALL_ICON_SIZE, SMALL_ICON_SIZE, getDisplay());
    pImg->setDiffuseColor(pPlayer->m_Color);
    pImg->setTooltipText(pPlayer->getAvatarName());
    pImg->setAttachment(pPlayer);
    addComponent(pImg);

    xPxl += btnSize + SPACING;
    if (xPxl + btnSize >= m_pLocalClient->getClientParameters()->screenXSize)
    {
      xPxl = 0;
      yPxl += btnSize + SPACING;
    }
    pPlayer = (Player*) pPlayers->getNext(0);
  }

  // Cancel button
  if (pCancelCallback != NULL)
    i18n->getText("CANCE", sText, LABEL_MAX_CHARS);
  else
    i18n->getText("CLOSE", sText, LABEL_MAX_CHARS);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "CancelButton", m_pLocalClient->getDisplay());
  pBtn->moveTo(xPxl / 2 - pBtn->getWidth() / 2, yPxl + btnSize + 2 * SPACING);
  addComponent(pBtn);

  setDimensions(xPxl, pBtn->getYPos() + pBtn->getHeight() + SPACING);

  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  m_pLocalClient->getInterface()->bringFrameAbove(pFrm);
  pFrm->setVisible(true);
  pFrm->updateSizeFit();
  pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pFrm->getHeight() / 2);
}
