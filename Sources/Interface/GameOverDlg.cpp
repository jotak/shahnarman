#include "GameOverDlg.h"
#include "InterfaceManager.h"
#include "../GUIClasses/guiContainer.h"
#include "../GUIClasses/guiButton.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Gameboard/Unit.h"
#include "../DeckData/AvatarData.h"

#define SPACING           4

// -----------------------------------------------------------------
// Name : GameOverDlg
//  Constructor
// -----------------------------------------------------------------
GameOverDlg::GameOverDlg(LocalClient * pLocalClient, int iWidth, int iHeight) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pGainsPanel = NULL;
  m_pStatsPanel = NULL;
  init("",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());

  // Other
  m_pWinners = NULL;
}

// -----------------------------------------------------------------
// Name : ~GameOverDlg
//  Destructor
// -----------------------------------------------------------------
GameOverDlg::~GameOverDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy GameOverDlg\n");
#endif
  FREE(m_pWinners);
#ifdef DBG_VERBOSE1
  printf("End destroy GameOverDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool GameOverDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "OkButton") == 0)
  {
    m_pLocalClient->endGame();
    return false;
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onShow
// -----------------------------------------------------------------
void GameOverDlg::onShow()
{
  deleteAllComponents();
  assert(m_pWinners != NULL);

  // Build main label string
  char sText[LABEL_MAX_CHARS];
  if (m_pWinners->size == 1)
    i18n->getText("VICTORY", sText, LABEL_MAX_CHARS);
  else
    i18n->getText("SHARED_VICTORY", sText, LABEL_MAX_CHARS);
  Player * pPlayer = (Player*) m_pWinners->getFirst(0);
  wsafecat(sText, LABEL_MAX_CHARS, pPlayer->getAvatarName());
  pPlayer = (Player*) m_pWinners->getNext(0);
  while (pPlayer != NULL)
  {
    wsafecat(sText, LABEL_MAX_CHARS, ", ");
    wsafecat(sText, LABEL_MAX_CHARS, pPlayer->getAvatarName());
    pPlayer = (Player*) m_pWinners->getNext(0);
  }
  wsafecat(sText, LABEL_MAX_CHARS, ".");

  // Create main label
  int yPxl = SPACING;
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, H1_FONT, H1_COLOR, "", 0, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->moveTo(getWidth() / 2 - pLbl->getWidth() / 2, yPxl);
  addComponent(pLbl);

  // Ok button
  i18n->getText1stUp("OK", sText, LABEL_MAX_CHARS);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "OkButton", m_pLocalClient->getDisplay());
  pBtn->moveTo((getWidth() - pBtn->getWidth()) / 2, getHeight() - SPACING - pBtn->getHeight());
  addComponent(pBtn);

  // Gains panel
  yPxl += pLbl->getHeight() + SPACING;
  int panelHeight = (getHeight() - yPxl - pBtn->getHeight() - 4 * SPACING) / 2;
  m_pGainsPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, panelHeight, "GainsPane", m_pLocalClient->getDisplay());
  m_pGainsPanel->setHeightFitBehavior(FB_FitDocumentToFrameWhenSmaller);
  m_pGainsPanel->moveTo(SPACING, yPxl);
  addComponent(m_pGainsPanel);

  // Stats panel
  yPxl += m_pGainsPanel->getHeight() + 2 * SPACING;
  m_pStatsPanel = guiContainer::createDefaultPanel(getWidth() - 2 * SPACING, panelHeight, "StatsPane", m_pLocalClient->getDisplay());
  m_pStatsPanel->setHeightFitBehavior(FB_FitDocumentToFrameWhenSmaller);
  m_pStatsPanel->moveTo(SPACING, yPxl);
  addComponent(m_pStatsPanel);

  // Fill gains panel
  yPxl = 0;
  ObjectList * pList = m_pLocalClient->getPlayerManager()->getPlayersList();
  pPlayer = (Player*) pList->getFirst(0);
  while (pPlayer != NULL)
  {
    yPxl += addPlayerGains(pPlayer, yPxl);
    pPlayer = (Player*) pList->getNext(0);
  }
  pList = m_pLocalClient->getPlayerManager()->getDeadPlayers();
  pPlayer = (Player*) pList->getFirst(0);
  while (pPlayer != NULL)
  {
    yPxl += addPlayerGains(pPlayer, yPxl);
    pPlayer = (Player*) pList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : addPlayerGains
// -----------------------------------------------------------------
int GameOverDlg::addPlayerGains(Player * pPlayer, int yPxl)
{
  // Add player image
  guiImage * pImg = new guiImage();
  pImg->init(pPlayer->getAvatarTexture(), "", 0, yPxl, 2 * SMALL_ICON_SIZE, 2 * SMALL_ICON_SIZE, m_pLocalClient->getDisplay());
  m_pGainsPanel->getDocument()->addComponent(pImg);

  // Show gained XP
  char sText[LABEL_MAX_CHARS];
  void * pArgs[2];
  int net = (int) pPlayer->m_uXPNet;
  int gross = (int) pPlayer->m_uXPGross;
  pArgs[0] = &net;
  pArgs[1] = &gross;
  i18n->getText("GAINED_XP(d1)_BEFORE(d2)", sText, LABEL_MAX_CHARS, pArgs);
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", pImg->getWidth() + SPACING, yPxl, 0, 0, m_pLocalClient->getDisplay());
  m_pGainsPanel->getDocument()->addComponent(pLbl);

  // Show total XP
  int yPxl2 = yPxl + pLbl->getHeight() + 4;
  int total = (int) pPlayer->m_pAvatarData->m_uXP;
  int next = (int) pPlayer->m_pAvatarData->getNextLevelXP();
  pArgs[0] = &total;
  pArgs[1] = &next;
  i18n->getText("TOTAL_XP(d1)_NEXT(d2)", sText, LABEL_MAX_CHARS, pArgs);
  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", pImg->getWidth() + SPACING, yPxl2, 0, 0, m_pLocalClient->getDisplay());
  m_pGainsPanel->getDocument()->addComponent(pLbl);

  return yPxl + pImg->getHeight() + SPACING;
}

// -----------------------------------------------------------------
// Name : setWinners
// -----------------------------------------------------------------
void GameOverDlg::setWinners(ObjectList * pWinners)
{
  FREE(m_pWinners);
  m_pWinners = pWinners;
}
