#include "InfoboxDlg.h"
#include "InterfaceManager.h"
#include "SpellDlg.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiImage.h"
#include "../GUIClasses/guiLabel.h"
#include "../GUIClasses/guiFrame.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Gameboard/Unit.h"
#include "../Input/KeyboardInputEngine.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../Gameboard/GameboardManager.h"

#define ZONE_SPACING  8
#define SPACING       3

// -----------------------------------------------------------------
// Name : InfoboxDlg
//  Constructor
// -----------------------------------------------------------------
InfoboxDlg::InfoboxDlg(int iWidth, LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_pPlayersList = NULL;
  m_pInfoText = NULL;
  m_pMapTile = NULL;
  m_pSpellFrm = NULL;
  m_uDisplayFilter = m_uClickFilter = 0;
  //m_pPlayerStates = new int[m_pLocalClient->getPlayerManager()->getPlayersCount()];
  //for (u8 i = 0; i < m_pLocalClient->getPlayerManager()->getPlayersCount(); i++)
  //  m_pPlayerStates[i] = -1;
  //m_iLastTurnTimer = -1;
  //wsafecpy(m_sLastTurnTimer, 64, L"");

  m_iLittleSquareYDecal = 17;
  m_pLittleSquare = new guiImage();
  m_pLittleSquare->init(
    pLocalClient->getDisplay()->getTextureEngine()->loadTexture(L"littleSquare"),
    L"LittleSquare", 0, 0, -1, -1, m_pLocalClient->getDisplay());
  addComponent(m_pLittleSquare);

  wchar_t sTitle[64];
  i18n->getText(L"INFO", sTitle, 64);
  guiDocument::init(sTitle,
    pLocalClient->getDisplay()->getTextureEngine()->findTexture(L"interface:WinBg"),
    0, 0, iWidth, 1, pLocalClient->getDisplay());

  m_pInfoText = new guiLabel();
  m_pInfoText->init(L"", TEXT_FONT, TEXT_COLOR, L"InfoText", SPACING, SPACING, getWidth() - 10, 0, pLocalClient->getDisplay());
  addComponent(m_pInfoText);
  m_pPlayersList = new guiLabel();
  m_pPlayersList->init(L"", TEXT_FONT, TEXT_COLOR, L"PlayersList", 9, ZONE_SPACING + m_pInfoText->getYPos() + m_pInfoText->getHeight(), 0, 0, pLocalClient->getDisplay());
  addComponent(m_pPlayersList);

  // Stack data
  m_pTarget = NULL;
  m_pGroupInterface = NULL;
  m_pCurrentGroup = NULL;
  m_iStackHeight = 0;

  m_bIsSpellDlgSticked = false;
}

// -----------------------------------------------------------------
// Name : ~InfoboxDlg
//  Destructor
// -----------------------------------------------------------------
InfoboxDlg::~InfoboxDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy InfoboxDlg\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy InfoboxDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void InfoboxDlg::update(double delta)
{
  guiDocument::update(delta);
  if (!m_bEnabled)
    return;

  // check if SpellDlg is sticked
  if (m_pSpellFrm == NULL)
    m_pSpellFrm = m_pLocalClient->getInterface()->findFrameFromDoc(m_pLocalClient->getInterface()->getSpellDialog());
  assert(m_pSpellFrm != NULL);
  {
    bool isSticked = m_pSpellFrm->isSticked();
    if (isSticked && !m_bIsSpellDlgSticked)
    {
      m_bIsSpellDlgSticked = true;
      guiFrame * thisFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
      thisFrm->moveBy(-m_pSpellFrm->getWidth(), 0);
    }
    else if (!isSticked && m_bIsSpellDlgSticked)
    {
      m_bIsSpellDlgSticked = false;
      guiFrame * thisFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
      thisFrm->moveBy(m_pSpellFrm->getWidth(), 0);
    }
  }
}

// -----------------------------------------------------------------
// Name : updatePlayersState
// -----------------------------------------------------------------
void InfoboxDlg::updatePlayersState()
{
  wchar_t sPlayersList[LABEL_MAX_CHARS] = L"";
  Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
  int i = 0;
  while (pPlayer != NULL)
  {
    PlayerState state = pPlayer->getState();
    wsafecat(sPlayersList, LABEL_MAX_CHARS, pPlayer->getAvatarName());
    wchar_t sStatus[64] = L"";
    switch (state)
    {
    case waiting:
      i18n->getText(L"WAITING", sStatus, 64);
      break;
    case playing:
      i18n->getText(L"PLAYING", sStatus, 64);
      break;
    case finished:
      i18n->getText(L"FINISHED", sStatus, 64);
      break;
    case connection_error:
      i18n->getText(L"CONNECTION_PROBLEM", sStatus, 64);
      break;
    }
    wsafecat(sPlayersList, LABEL_MAX_CHARS, L" (");
    if (state == playing)
    {
      double fTimer = m_pLocalClient->getPlayerManager()->getTurnTimer();
      if (fTimer > 0)
      {
        int iTimer = (int) fTimer;
        wchar_t sTimer[64];
        swprintf_s(sTimer, 64, L"%d:%02d ", iTimer / 60, iTimer % 60);
        wsafecat(sPlayersList, LABEL_MAX_CHARS, sTimer);
      }
    }
    wsafecat(sPlayersList, LABEL_MAX_CHARS, sStatus);
    wsafecat(sPlayersList, LABEL_MAX_CHARS, L")\n");
    pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
    i++;
  }
  pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getDeadPlayers()->getFirst(0);
  while (pPlayer != NULL)
  {
    wsafecat(sPlayersList, LABEL_MAX_CHARS, pPlayer->getAvatarName());
    wchar_t sStatus[64] = L"";
    i18n->getText(L"DEAD", sStatus, 64);
    wsafecat(sPlayersList, LABEL_MAX_CHARS, L" (");
    wsafecat(sPlayersList, LABEL_MAX_CHARS, sStatus);
    wsafecat(sPlayersList, LABEL_MAX_CHARS, L")\n");
    pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getDeadPlayers()->getNext(0);
    i++;
  }
  m_pPlayersList->setText(sPlayersList);
  checkSize();

  if (m_pLittleSquare != NULL)
    m_pLittleSquare->setYPos(m_pPlayersList->getYPos() + 6 + m_iLittleSquareYDecal * m_pLocalClient->getPlayerManager()->getFirstResolutionIdx());
}

// -----------------------------------------------------------------
// Name : setInfoText
// -----------------------------------------------------------------
void InfoboxDlg::setInfoText(wchar_t * sText)
{
  if (wcscmp(sText, m_pInfoText->getText()) != 0)
  {
    int oldh = m_pInfoText->getHeight();
    m_pInfoText->setText(sText);
    int diff = m_pInfoText->getHeight() - oldh;
    m_pPlayersList->moveBy(0, diff);
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
      if (wcscmp(pCpnt->getId(), L"StackObject") == 0)
        pCpnt->moveBy(0, diff);
      pCpnt = getNextComponent();
    }
    checkSize();
  }
}

// -----------------------------------------------------------------
// Name : checkSize
// -----------------------------------------------------------------
void InfoboxDlg::checkSize()
{
  int iWidth = max(m_pPlayersList->getWidth() + m_pPlayersList->getXPos(), m_pInfoText->getWidth() + m_pInfoText->getXPos());
  iWidth += 5;

  int iHeight = m_pPlayersList->getHeight() + m_pPlayersList->getYPos();
  if (iHeight != getHeight())
  {
    guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
    assert(pFrm != NULL);
    pFrm->moveTo(pFrm->getXPos(), m_pLocalClient->getClientParameters()->screenYSize - (pFrm->getHeight()-getHeight()) - iHeight);
  }

  if (iWidth != getWidth() || iHeight != getHeight())
    setDimensions(iWidth, iHeight);
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool InfoboxDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (wcscmp(pCpnt->getId(), L"StackObject") == 0 && m_pGroupInterface != NULL && pCpnt->getAttachment() != NULL && (pCpnt->getType() & GOTYPE_BUTTON))
  {
    if ((m_pLocalClient->getInput()->hasKeyboard() && !((KeyboardInputEngine*)m_pLocalClient->getInput())->isCtrlPressed() && m_pCurrentGroup != NULL) ||
      (!m_pLocalClient->getInput()->hasKeyboard() && pEvent->eButton == Button1))
    {
      m_pCurrentGroup = m_pGroupInterface->resetCurrentGroup(pCpnt->getAttachment(), m_pCurrentGroup);
      ((guiToggleButton*)pCpnt)->setClickState(true);
    }
    m_pCurrentGroup = m_pGroupInterface->onClickOnGroupItem(pCpnt->getAttachment(), ((guiToggleButton*)pCpnt)->getClickState(), m_pCurrentGroup);
    // Now click or unclick every component depending on wether they're in the group
    pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
      if (pCpnt->getType() & GOTYPE_BUTTON)
      {
        if (wcscmp(pCpnt->getId(), L"StackObject") == 0 && pCpnt->getAttachment() == NULL || m_pCurrentGroup == NULL)
          ((guiToggleButton*)pCpnt)->setClickState(false);
        else
          ((guiToggleButton*)pCpnt)->setClickState(m_pCurrentGroup->goTo(0, pCpnt->getAttachment()));
      }
      pCpnt = getNextComponent();
    }
    // finally, update gameboard
    if (m_pCurrentGroup != NULL)
      m_pLocalClient->getGameboard()->selectMapObject((MapObject*) m_pCurrentGroup->getFirst(0), false);
    else
      m_pLocalClient->getGameboard()->selectMapObject(NULL, false);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
guiObject * InfoboxDlg::onCursorMoveEvent(int xPxl, int yPxl)
{
  m_pTarget = NULL;

  if (m_pPlayersList->isAt(xPxl, yPxl))
  {
    int dy = yPxl - m_pPlayersList->getYPos();
    int iPlayer = dy / getDisplay()->getFontEngine()->getFontHeight(m_aiAllFonts[(int)m_pPlayersList->getFontId()]);
    if (iPlayer < m_pLocalClient->getPlayerManager()->getPlayersCount())
    {
      Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->goTo(0, iPlayer);
      assert(pPlayer != NULL);
      wchar_t sBuf[LABEL_MAX_CHARS] = L"";
      setInfoText(pPlayer->getInfo(sBuf, LABEL_MAX_CHARS));
    }
  }
  else
  {
    guiComponent * cpnt = getFirstComponent();
    while (cpnt != NULL)
    {
      if (cpnt->isAt(xPxl, yPxl))
      {
        if (wcscmp(cpnt->getId(), L"StackObject") == 0)
        {
          BaseObject * pAttachment = cpnt->getAttachment();
          if (pAttachment != NULL)
          {
            m_pTarget = cpnt;
            wchar_t sBuf[LABEL_MAX_CHARS] = L"";
            if (((GraphicObject*)pAttachment)->getType() & GOTYPE_MAPTILE)
              setInfoText(((MapTile*)pAttachment)->getInfo(sBuf, LABEL_MAX_CHARS));
            else
              setInfoText(((MapObject*)pAttachment)->getInfo(sBuf, LABEL_MAX_CHARS, Dest_InfoDialog));
          }
        }
        break;
      }
      cpnt = getNextComponent();
    }
  }
  return guiDocument::onCursorMoveEvent(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : setTargetValid
// -----------------------------------------------------------------
void InfoboxDlg::setTargetValid(bool bValid)
{
  guiComponent * pOther = getFirstComponent();
  while (pOther != NULL)
  {
    if (pOther != m_pTarget && pOther->getType() & GOTYPE_BUTTON)
      ((guiToggleButton*)pOther)->setClickState(false);
    pOther = getNextComponent();
  }
  if (m_pTarget != NULL)
  {
    if (m_pTarget->getType() & GOTYPE_BUTTON)
      ((guiToggleButton*)m_pTarget)->setClickState(bValid);
  }
}

// -----------------------------------------------------------------
// Name : setMapTile
// -----------------------------------------------------------------
void InfoboxDlg::setMapTile(MapTile * pTile, u32 uDisplayFilter, u32 uClickFilter, StackGroupInterface * pGroupInterface, Player * pCurrentPlayer, GraphicObject * pDefaultSelectedObject)
{
  m_pMapTile = pTile;
  m_uDisplayFilter = uDisplayFilter;
  m_uClickFilter = uClickFilter;
  m_pGroupInterface = pGroupInterface;
  m_pCurrentGroup = NULL;

  guiComponent * pCpnt = getFirstComponent();
  while (pCpnt != NULL)
  {
    if (wcscmp(pCpnt->getId(), L"StackObject") == 0)
      pCpnt = deleteCurrentComponent(true);
    else
      pCpnt = getNextComponent();
  }
  m_pFocusedComponent = NULL;
  if (m_pMapTile == NULL)
    return;

  // First add all clickable objects
  int yStart = m_pInfoText->getYPos() + m_pInfoText->getHeight() + ZONE_SPACING;
  int yDecal = addObjects(yStart, pCurrentPlayer, true);
  // Then, non-clickable objects
  yDecal = addObjects(yDecal, pCurrentPlayer, false);
  m_iStackHeight = yDecal - yStart;
  m_pPlayersList->setYPos(2*ZONE_SPACING + m_iStackHeight + m_pInfoText->getYPos() + m_pInfoText->getHeight());
  checkSize();

  // Default selected object ?
  if (pDefaultSelectedObject != NULL)
  {
    guiComponent * pCpnt = getFirstComponent();
    while (pCpnt != NULL)
    {
      if ((pCpnt->getType() & GOTYPE_BUTTON) && pCpnt->getAttachment() == pDefaultSelectedObject)
      {
        ((guiToggleButton*)pCpnt)->setClickState(true);
        ButtonAction pEvent;
        pEvent.eButton = Button1;
        pEvent.eEvent = Event_Down;
        onButtonEvent(&pEvent, pCpnt);
        break;
      }
      pCpnt = getNextComponent();
    }
  }
}

// -----------------------------------------------------------------
// Name : addObjects
// -----------------------------------------------------------------
int InfoboxDlg::addObjects(int yPxl, Player * pCurrentPlayer, bool bButtons)
{
  int xPxl = SMALL_ICON_SPACING;
  yPxl += SMALL_ICON_SPACING;

  u32 uFilter = bButtons ? m_uClickFilter : m_uDisplayFilter;
  if (uFilter & GOTYPE_MAPTILE)
  {
    // First, display terrain
    bool bOk;
    if (bButtons)
      bOk = addButton(m_pMapTile, xPxl, yPxl, SMALL_ICON_SIZE);
    else
      bOk = addImage(m_pMapTile, xPxl, yPxl, SMALL_ICON_SIZE);
    if (bOk)
      xPxl += SMALL_ICON_SIZE + SMALL_ICON_SPACING;
  }

  bool bOwned = ((uFilter & GOTYPE_OWNED_OBJECT) != 0);
  // Then, display all map objects
  MapObject * mapObj = m_pMapTile->getFirstMapObject(uFilter);
  while (mapObj != NULL)
  {
    if (!bOwned || (pCurrentPlayer != NULL && mapObj->getOwner() == pCurrentPlayer->m_uPlayerId))
    {
      if (xPxl + SMALL_ICON_SIZE >= m_iWidth)
      {
        xPxl = SMALL_ICON_SPACING;
        yPxl += SMALL_ICON_SIZE + SMALL_ICON_SPACING;
      }
      bool bOk;
      if (bButtons)
        bOk = addButton(mapObj, xPxl, yPxl, SMALL_ICON_SIZE);
      else
        bOk = addImage(mapObj, xPxl, yPxl, SMALL_ICON_SIZE);
      if (bOk)
        xPxl += SMALL_ICON_SIZE + SMALL_ICON_SPACING;
    }
    mapObj = m_pMapTile->getNextMapObject(uFilter);
  }
  return yPxl + SMALL_ICON_SIZE + SMALL_ICON_SPACING;
}

// -----------------------------------------------------------------
// Name : addButton
// -----------------------------------------------------------------
bool InfoboxDlg::addButton(GraphicObject * pObj, int xPxl, int yPxl, int iSize)
{
  // First, check that the object isn't attached yet
  guiComponent * pCpnt = getFirstComponent();
  while (pCpnt != NULL)
  {
    if (pCpnt->getAttachment() == pObj)
      return false;
    pCpnt = getNextComponent();
  }
  // Create button
  int itex = (pObj->getType() & GOTYPE_MAPTILE) ? ((MapTile*)pObj)->getTexture() : ((MapObject*)pObj)->getTexture();
  guiToggleButton * pBtn = guiToggleButton::createDefaultTexturedToggleButton(itex, iSize, L"StackObject", getDisplay());
  pBtn->moveTo(xPxl, yPxl);
  pBtn->setAttachment(pObj);
  pBtn->setCatchButton2Events(!m_pLocalClient->getInput()->hasKeyboard());
  addComponent(pBtn);
  if (pObj->getType() & GOTYPE_UNIT)
  {
    // Add player banner
    Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(((MapObject*)pObj)->getOwner());
    assert(pPlayer != NULL);
    guiImage * pImg = new guiImage();
    pImg->init(pPlayer->m_iBannerTex, L"StackObject", xPxl + 2 * iSize / 3, yPxl, iSize / 3, iSize / 3, getDisplay());
    pImg->setDiffuseColor(pPlayer->m_Color);
    addComponent(pImg);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : addImage
// -----------------------------------------------------------------
bool InfoboxDlg::addImage(GraphicObject * pObj, int xPxl, int yPxl, int iSize)
{
  // First, check that the object doesn't isn't attached yet
  guiComponent * pCpnt = getFirstComponent();
  while (pCpnt != NULL)
  {
    if (pCpnt->getAttachment() == pObj)
      return false;
    pCpnt = getNextComponent();
  }
  // Create image
  int itex = (pObj->getType() & GOTYPE_MAPTILE) ? ((MapTile*)pObj)->getTexture() : ((MapObject*)pObj)->getTexture();
  guiImage * pImage = new guiImage();
  pImage->init(itex, L"StackObject", xPxl, yPxl, iSize, iSize, getDisplay());
  pImage->setAttachment(pObj);
  addComponent(pImage);
  if (pObj->getType() & GOTYPE_UNIT)
  {
    // Add player banner
    Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(((MapObject*)pObj)->getOwner());
    assert(pPlayer != NULL);
    guiImage * pImg = new guiImage();
    pImg->init(pPlayer->m_iBannerTex, L"StackObject", xPxl + 2 * iSize / 3, yPxl, iSize / 3, iSize / 3, getDisplay());
    pImg->setDiffuseColor(pPlayer->m_Color);
    addComponent(pImg);
  }
  return true;
}
