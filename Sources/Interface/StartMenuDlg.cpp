#include "StartMenuDlg.h"
#include "../GUIClasses/guiButton.h"
#include "../GUIClasses/guiPopup.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Debug/DebugManager.h"
#include "InterfaceManager.h"
#include "SelectPlayerAvatarDlg.h"
#include "LoadGameDlg.h"
#include "HostGameDlg.h"
#include "OptionsDlg.h"

// -----------------------------------------------------------------
// Name : StartMenuDlg
//  Constructor
// -----------------------------------------------------------------
StartMenuDlg::StartMenuDlg(int iWidth, int iHeight, LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;

  init("Main menu",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, iWidth, iHeight, pLocalClient->getDisplay());

  int yPxl = 10;
  int butHeight = (iHeight - 2*yPxl) / 5;

  char str[64] = "";
  i18n->getText("BUILD_DECK", str, 64);
  guiButton * pBtn = new guiButton();
  pBtn->init(str, H1_FONT, H1_COLOR,
    -1,
    BCO_None,
    -1,
    BCO_Decal,
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:Transparent"),
    "BuildDeck", 0, yPxl, iWidth, butHeight, pLocalClient->getDisplay());
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText("PLAY", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId("PlayLAN");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText("PLAY_ONLINE", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId("PlayOnline");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText("OPTIONS", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId("Options");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText("QUIT", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId("Quit");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  m_pPlayLocalDlg = NULL;
}

// -----------------------------------------------------------------
// Name : ~StartMenuDlg
//  Destructor
// -----------------------------------------------------------------
StartMenuDlg::~StartMenuDlg()
{
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void StartMenuDlg::update(double delta)
{
  if (m_pPlayLocalDlg != NULL)
  {
    guiComponent * pCpnt = m_pPlayLocalDlg->getClickedComponent();
    if (pCpnt != NULL)
    {
      if (strcmp(pCpnt->getId(), "NewGame") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pPlayLocalDlg);
        m_pPlayLocalDlg = NULL;
        setEnabled(true);
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getHostGameDialog());
      }
      else if (strcmp(pCpnt->getId(), "JoinLAN") == 0)
      {
      }
      else if (strcmp(pCpnt->getId(), "LoadGame") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pPlayLocalDlg);
        m_pPlayLocalDlg = NULL;
        setEnabled(true);
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getLoadGameDialog());
      }
      else if (strcmp(pCpnt->getId(), "Back") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pPlayLocalDlg);
        m_pPlayLocalDlg = NULL;
        setEnabled(true);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool StartMenuDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "BuildDeck") == 0)
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getSelectPlayerDialog());
  else if (strcmp(pCpnt->getId(), "PlayLAN") == 0)
  {
    // Show popup
    m_pPlayLocalDlg = guiPopup::createEmptyPopup(getDisplay());
    m_pLocalClient->getInterface()->registerFrame(m_pPlayLocalDlg);
    guiDocument * pDoc = m_pPlayLocalDlg->getDocument();
    int docwidth = 200;

    // New game button
    int yPxl = 10;
    char sText[64];
    i18n->getText("NEW_GAME", sText, 64);
    guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "NewGame", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Join LAN button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText("JOIN_LAN", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, "JoinLAN", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Load game button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText("LOAD_GAME", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, "LoadGame", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Back button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText("BACK", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, "Back", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    pDoc->setDimensions(docwidth, yPxl + pBtn->getHeight() + 10);
    m_pPlayLocalDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - docwidth / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pDoc->getHeight() / 2);
    setEnabled(false);
  }
  else if (strcmp(pCpnt->getId(), "PlayOnline") == 0)
  {
  }
  else if (strcmp(pCpnt->getId(), "Options") == 0)
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getOptionsDialog());
  else if (strcmp(pCpnt->getId(), "Quit") == 0)
  {
    m_pLocalClient->requestExit();
    return false;
  }
  return true;
}
