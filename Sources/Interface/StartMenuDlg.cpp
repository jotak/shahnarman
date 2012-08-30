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

  init(L"Main menu",
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, iHeight, pLocalClient->getDisplay());

  int yPxl = 10;
  int butHeight = (iHeight - 2*yPxl) / 5;

  wchar_t str[64] = L"";
  i18n->getText(L"BUILD_DECK", str, 64);
  guiButton * pBtn = new guiButton();
  pBtn->init(str, H1_FONT, H1_COLOR,
    -1,
    BCO_None,
    -1,
    BCO_Decal,
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:Transparent"),
    L"BuildDeck", 0, yPxl, iWidth, butHeight, pLocalClient->getDisplay());
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText(L"PLAY", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId(L"PlayLAN");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText(L"PLAY_ONLINE", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId(L"PlayOnline");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText(L"OPTIONS", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId(L"Options");
  pBtn->moveTo(0, yPxl);
  addComponent(pBtn);

  yPxl += butHeight;
  i18n->getText(L"QUIT", str, 64);
  pBtn = (guiButton*) pBtn->clone();
  pBtn->setText(str);
  pBtn->setId(L"Quit");
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
      if (wcscmp(pCpnt->getId(), L"NewGame") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pPlayLocalDlg);
        m_pPlayLocalDlg = NULL;
        setEnabled(true);
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getHostGameDialog());
      }
      else if (wcscmp(pCpnt->getId(), L"JoinLAN") == 0)
      {
      }
      else if (wcscmp(pCpnt->getId(), L"LoadGame") == 0)
      {
        m_pLocalClient->getInterface()->deleteFrame(m_pPlayLocalDlg);
        m_pPlayLocalDlg = NULL;
        setEnabled(true);
        m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getLoadGameDialog());
      }
      else if (wcscmp(pCpnt->getId(), L"Back") == 0)
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
  if (wcscmp(pCpnt->getId(), L"BuildDeck") == 0)
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getSelectPlayerDialog());
  else if (wcscmp(pCpnt->getId(), L"PlayLAN") == 0)
  {
    // Show popup
    m_pPlayLocalDlg = guiPopup::createEmptyPopup(getDisplay());
    m_pLocalClient->getInterface()->registerFrame(m_pPlayLocalDlg);
    guiDocument * pDoc = m_pPlayLocalDlg->getDocument();
    int docwidth = 200;

    // New game button
    int yPxl = 10;
    wchar_t sText[64];
    i18n->getText(L"NEW_GAME", sText, 64);
    guiButton * pBtn = guiButton::createDefaultNormalButton(sText, L"NewGame", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Join LAN button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText(L"JOIN_LAN", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, L"JoinLAN", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Load game button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText(L"LOAD_GAME", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, L"LoadGame", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    // Back button
    yPxl += pBtn->getHeight() + 10;
    i18n->getText(L"BACK", sText, 64);
    pBtn = guiButton::createDefaultNormalButton(sText, L"Back", m_pLocalClient->getDisplay());
    pBtn->setWidth(docwidth-20);
    pBtn->moveTo(10, yPxl);
    pDoc->addComponent(pBtn);

    pDoc->setDimensions(docwidth, yPxl + pBtn->getHeight() + 10);
    m_pPlayLocalDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - docwidth / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pDoc->getHeight() / 2);
    setEnabled(false);
  }
  else if (wcscmp(pCpnt->getId(), L"PlayOnline") == 0)
  {
  }
  else if (wcscmp(pCpnt->getId(), L"Options") == 0)
    m_pLocalClient->getInterface()->setUniqueDialog(m_pLocalClient->getInterface()->getOptionsDialog());
  else if (wcscmp(pCpnt->getId(), L"Quit") == 0)
  {
    m_pLocalClient->requestExit();
    return false;
  }
  return true;
}
