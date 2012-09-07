#include "LogDlg.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiButton.h"
#include "../GUIClasses/guiFrame.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/GameboardManager.h"
#include "../Interface/InterfaceManager.h"
#include "../Interface/MapObjectDlg.h"
#include "../Fx/FxManager.h"

#define SPACING   5
#define HSPACING  3

// -----------------------------------------------------------------
// Name : LogDlg
//  Constructor
// -----------------------------------------------------------------
LogDlg::LogDlg(LocalClient * pLocalClient) : guiDocument()
{
  m_pLocalClient = pLocalClient;
  m_uLogs = 0;
  m_pAllLogs = new ObjectList(true);
  m_pMapPosList = new ObjectList(true);

  char sTitle[64];
  i18n->getText("LOG", sTitle, 64);
  init(sTitle,
    pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
    0, 0, 1, 1, pLocalClient->getDisplay());
  m_bLastLogIsNewTurn = true;
}

// -----------------------------------------------------------------
// Name : ~LogDlg
//  Destructor
// -----------------------------------------------------------------
LogDlg::~LogDlg()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy LogDlg\n");
#endif
  delete m_pMapPosList;
  delete m_pAllLogs;
#ifdef DBG_VERBOSE1
  printf("End destroy LogDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool LogDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "MapObjScreen") == 0)
  {
    MapObject * pObj = (MapObject*) pCpnt->getAttachment();
    assert(pObj != NULL);
    MapTile * pTile = m_pLocalClient->getGameboard()->getMap()->getTileAt(pObj->getMapPos());
    assert(pTile != NULL);
    m_pLocalClient->getInterface()->showMapObjectDialog(pTile);
    MapObjectDlg * pDlg = m_pLocalClient->getInterface()->getMapObjectDialog();
    pDlg->setSelectedObject(pObj);
    m_pLocalClient->getFx()->zoomToMapPos(pObj->getMapPos());
  }
  else if (strcmp(pCpnt->getId(), "ZoomTo") == 0)
  {
    CoordsObject * pObj = (CoordsObject*) pCpnt->getAttachment();
    assert(pObj != NULL);
    m_pLocalClient->getFx()->zoomToMapPos(pObj->getCoordsMap());
  }
  return true;
}

// -----------------------------------------------------------------
// Name : log
// -----------------------------------------------------------------
void LogDlg::log(char * sText, u8 uLevel, u8 uAction, void * pParam)
{
  int xPxl = SPACING;
  int yPxl = SPACING;
  guiButton * pBtn = NULL;
  char sTooltip[LABEL_MAX_CHARS];
  switch (uAction)
  {
  case LOG_ACTION_ZOOMTO:
    {
      CoordsObject * pCoords = new CoordsObject(*(CoordsMap*) pParam);
      m_pMapPosList->addLast(pCoords);
      int iTex = getDisplay()->getTextureEngine()->loadTexture("eye", false, 0, 25, 0, 16);
      pBtn = guiButton::createDefaultImageButton(iTex, "ZoomTo", getDisplay());
      pBtn->moveTo(xPxl, yPxl);
      pBtn->setAttachment(pCoords);
      i18n->getText("SEE", sTooltip, LABEL_MAX_CHARS);
      pBtn->setTooltipText(sTooltip);
      break;
    }
  case LOG_ACTION_UNITSCREEN:
  case LOG_ACTION_TOWNSCREEN:
    {
      int iTex = getDisplay()->getTextureEngine()->loadTexture("eye", false, 0, 25, 0, 16);
      pBtn = guiButton::createDefaultImageButton(iTex, "MapObjScreen", getDisplay());
      pBtn->moveTo(xPxl, yPxl);
      BaseObject * pObj = (BaseObject*) pParam;
      pBtn->setAttachment(pObj);
      i18n->getText("SEE", sTooltip, LABEL_MAX_CHARS);
      pBtn->setTooltipText(sTooltip);
      break;
    }
  default:
    break;
  }

  xPxl += (pBtn == NULL) ? 0 : pBtn->getWidth() + SPACING;
  int wPxl = getWidth() - xPxl;
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", xPxl, yPxl, wPxl, 0, getDisplay());

  int lblHeight = pLbl->getHeight();
  guiLabel * pLbl2 = pLbl;
  guiComponent * cpnt = getFirstComponent();
  while (cpnt != NULL)
  {
    cpnt->moveBy(0, lblHeight + HSPACING);
    if (cpnt->getType() & GOTYPE_LABEL && cpnt->getYPos() > yPxl)
    {
      yPxl = cpnt->getYPos();
      pLbl2 = (guiLabel*) cpnt;
    }
    cpnt = getNextComponent();
  }
  addComponent(pLbl);
  if (pBtn != NULL)
    addComponent(pBtn);

  setHeight(yPxl + pLbl2->getHeight() + HSPACING);
  m_uLogs++;

  guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
  if (!pFrm->isSticked())
    pFrm->extract();
//    pFrm->flash(1.0f);

  if (uLevel > 0)
    m_pLocalClient->getFx()->showMessage(pLbl->getText());

  m_bLastLogIsNewTurn = false;
  m_pAllLogs->addLast(new LogItem(pLbl, pBtn, m_pLocalClient->getTurn()));
}

// -----------------------------------------------------------------
// Name : logNewTurn
// -----------------------------------------------------------------
void LogDlg::logNewTurn()
{
  if (!m_bLastLogIsNewTurn)
  {
    char sText[64];
    char sBuf[64];
    i18n->getText("NEW_TURN", sBuf, 64);
    snprintf(sText, 64, "** %s **", sBuf);
    log(sText);
    m_bLastLogIsNewTurn = true;
  }
  // Remove perimed items
  if (m_pLocalClient->getClientParameters()->iGameLogsLifetime >= 0)
  {
    bool bDeleted = false;
    LogItem * pItem = (LogItem*) m_pAllLogs->getFirst(0);
    while (pItem != NULL)
    {
      if (pItem->iTurn < m_pLocalClient->getTurn() - m_pLocalClient->getClientParameters()->iGameLogsLifetime)
      {
        getComponentsList()->deleteObject(pItem->pLbl, true);
        if (pItem->pBtn != NULL)
          getComponentsList()->deleteObject(pItem->pBtn, true);
        bDeleted = true;
      }
      pItem = (LogItem*) m_pAllLogs->getNext(0);
    }
    if (bDeleted)
    {
      int maxy = 0;
      guiComponent * pCpnt = getFirstComponent();
      while (pCpnt != NULL)
      {
        if (pCpnt->getYPos() + pCpnt->getHeight() + HSPACING > maxy)
          maxy = pCpnt->getYPos() + pCpnt->getHeight() + HSPACING;
        pCpnt = getNextComponent();
      }
      setHeight(maxy);
    }
  }
}
