// -----------------------------------------------------------------
// INTERFACE MANAGER
// -----------------------------------------------------------------
#include "InterfaceManager.h"
#include "../LocalClient.h"
#include "../GUIClasses/guiTabbedFrame.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiPopup.h"
#include "../GUIClasses/guiComboBox.h"
#include "StartMenuDlg.h"
#include "SelectPlayerAvatarDlg.h"
#include "BuildDeckDlg.h"
#include "ArtifactsEquipDlg.h"
#include "ShopDlg.h"
#include "LoadGameDlg.h"
#include "HostGameDlg.h"
#include "LevelUpDlg.h"
#include "OptionsDlg.h"
#include "SpellDlg.h"
#include "LogDlg.h"
#include "InfoboxDlg.h"
#include "UnitOptionsDlg.h"
#include "ResolveDlg.h"
#include "MapObjectDlg.h"
#include "GameOverDlg.h"
#include "SpellsSelectorDlg.h"
#include "PlayerSelectorDlg.h"
#include "StatusDlg.h"
#include "CreateAvatarDlg.h"
#include "Tooltip.h"
#include "MoveOrAttackDlg.h"
#include "../Players/PlayerManager.h"
#include "../Players/Spell.h"
#include "../Input/InputEngine.h"
#include "../Players/Player.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/GameboardInputs.h"
#include "../Gameboard/GameboardManager.h"
#include "../Audio/AudioManager.h"
#include "../Debug/DebugManager.h"
#include "../Server/Server.h"

// -----------------------------------------------------------------
// Name : InterfaceManager
// -----------------------------------------------------------------
InterfaceManager::InterfaceManager(LocalClient * pLocalClient) : EventListener(7)
{
  m_pLocalClient = pLocalClient;
  m_pFrameList = new ObjectList(true);
  m_pTopDisplayList = new ObjectList(true);
  m_iFrmIt = m_pFrameList->getIterator();
  m_pMenuBgGeometry = NULL;
  m_pTargetOnClickClbk = NULL;
  m_pTargetOnMouseOverClbk = NULL;
  m_pTargetedObject = NULL;
  m_uIsLuaPlayerGO = 0;
  deleteAllFrames();
  guiContainer::initStatic();
}

// -----------------------------------------------------------------
// Name : ~InterfaceManager
// -----------------------------------------------------------------
InterfaceManager::~InterfaceManager()
{
  delete m_pFrameList;
  delete m_pTopDisplayList;
  FREE(m_pMenuBgGeometry);
  guiContainer::deleteStatic();
}

// -----------------------------------------------------------------
// Name : deleteAllFrames
// -----------------------------------------------------------------
void InterfaceManager::deleteAllFrames()
{
  m_pFrameList->deleteAll();
  m_pTopDisplayList->deleteAll();

  m_pStartMenuWindow = NULL;
  m_pSelectPlayerAvatarWnd = NULL;
  m_pBuildDeckWnd = NULL;
  m_pArtifactsEquipWnd = NULL;
  m_pShopWnd = NULL;
  m_pLoadGameWnd = NULL;
  m_pHostGameWnd = NULL;
  m_pLevelUpWnd = NULL;
  m_pCreateAvatarDlg = NULL;
  m_pOptionsWnd = NULL;
  m_pSpellWnd = NULL;
  m_pLogWnd = NULL;
  m_pInfoWnd = NULL;
  m_pUnitOptionsWnd = NULL;
  m_pResolveWnd = NULL;
  m_pGameOverWnd = NULL;
  m_pSpellsSelectorWnd = NULL;
  m_pPlayerSelectorWnd = NULL;
  m_pStatusWnd = NULL;
  m_pTooltip = NULL;
  m_pInGameMenu = NULL;
  m_fTooltipTime = 0;
  m_bTargetMode = false;
  m_pClickedObjects[0] = m_pClickedObjects[1] = NULL;
  m_pNextLocalPlayerDlg = NULL;
  m_pExtraMana = NULL;
  m_pPointedObject = NULL;
  m_pMoveOrAttackDlg = NULL;
  m_pMapObjectWnd = NULL;
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void InterfaceManager::Init()
{
  int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("menubg");
  QuadData quad(0, m_pLocalClient->getClientParameters()->screenXSize, 0, m_pLocalClient->getClientParameters()->screenYSize, iTex, m_pLocalClient->getDisplay());
  m_pMenuBgGeometry = new GeometryQuads(&quad, VB_Static);

  m_pLocalClient->getDisplay()->getTextureEngine()->loadComposedTexture("interface");
  InitMenu();
  m_pLocalClient->getInput()->addCursoredEventListener(this, m_pLocalClient->getDebug());
  m_pLocalClient->getInput()->pushUncursoredEventListener(this);
}

// -----------------------------------------------------------------
// Name : InitMenu
// -----------------------------------------------------------------
void InterfaceManager::InitMenu()
{
  m_pLocalClient->getAudio()->playMusic(MUSIC_INTRO);
  deleteAllFrames();

  // Init tooltips
  m_pTooltip = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
  m_pTooltip->removeEffect(POPUP_EFFECT_FOCUS);
  m_pTooltip->removeEffect(POPUP_EFFECT_INTRO);
  m_pTooltip->getDocument()->setEnabled(true);
  m_pTooltip->setVisible(false);
  m_pFrameList->addLast(m_pTooltip);

  // Create menu frame
  guiFrame * pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitDocumentToFrame, 600, 600, true, "StartMenu", m_pLocalClient->getDisplay());

  // Start menu dialog
  pFrame->setDimensions(350, 450);
  pFrame->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - 175, m_pLocalClient->getClientParameters()->screenYSize / 2 - 225);
  m_pStartMenuWindow = new StartMenuDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pStartMenuWindow);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("OptionsFrame");

  // Options dialog
  m_pOptionsWnd = new OptionsDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pOptionsWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  guiFrame * pFrame2 = guiFrame::createDefaultFrame(FB_FitFrameToDocument, FB_FitFrameToDocument, 1, 1, true, "SelectPlayerFrame", m_pLocalClient->getDisplay());

  // Select player and avatar dialog (build deck)
  m_pSelectPlayerAvatarWnd = new SelectPlayerAvatarDlg(m_pLocalClient);
  pFrame2->setDocument(m_pSelectPlayerAvatarWnd);
  m_pFrameList->addLast(pFrame2);
  pFrame2->setVisible(false);
  pFrame2->updateSizeFit();
  pFrame2->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrame2->getWidth() / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pFrame2->getHeight() / 2);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("BuildDeckFrame");
  pFrame->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - 300, m_pLocalClient->getClientParameters()->screenYSize / 2 - 300);
  pFrame->setDimensions(600, 600);

  // Build deck dialog
  m_pBuildDeckWnd = new BuildDeckDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pBuildDeckWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("ArtifactsEquipFrame");
  pFrame->setWidthFitBehavior(FB_FitFrameToDocument);

  // ArtifactsEquip dialog
  m_pArtifactsEquipWnd = new ArtifactsEquipDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pArtifactsEquipWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("ShopFrame");
  pFrame->setWidthFitBehavior(FB_FitDocumentToFrame);

  // Shop dialog
  m_pShopWnd = new ShopDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pShopWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("CreateShahmahFrame");

  // Create Shahmah dialog
  m_pCreateAvatarDlg = new CreateAvatarDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pCreateAvatarDlg);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("LoadGameFrame");

  // Load game dialog
  m_pLoadGameWnd = new LoadGameDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pLoadGameWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("HostGameFrame");
  pFrame->setHeight(m_pLocalClient->getClientParameters()->screenYSize - 20);
  pFrame->moveTo(pFrame->getXPos(), 10);

  // Host game dialog
  m_pHostGameWnd = new HostGameDlg(pFrame->getInnerWidth(), pFrame->getInnerHeight(), m_pLocalClient);
  pFrame->setDocument(m_pHostGameWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);

  // Clone frame
  pFrame = (guiFrame*) pFrame->clone();
  pFrame->setId("LevelUpFrame");
  pFrame->setDimensions(10, 10);
  pFrame->setWidthFitBehavior(FB_FitFrameToDocument);
  pFrame->setHeightFitBehavior(FB_FitFrameToDocument);

  // Level up dialog
  m_pLevelUpWnd = new LevelUpDlg(m_pLocalClient);
  pFrame->setDocument(m_pLevelUpWnd);
  m_pFrameList->addLast(pFrame);
  pFrame->setVisible(false);
  pFrame->moveTo((m_pLocalClient->getClientParameters()->screenXSize - m_pLevelUpWnd->getWidth()) / 2, 80);

  setUniqueDialog(m_pStartMenuWindow);
}

// -----------------------------------------------------------------
// Name : InitGame
// -----------------------------------------------------------------
void InterfaceManager::InitGame()
{
  deleteAllFrames();

  // Init tooltips
  m_pTooltip = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
  m_pTooltip->removeEffect(POPUP_EFFECT_FOCUS);
  m_pTooltip->removeEffect(POPUP_EFFECT_INTRO);
  m_pTooltip->getDocument()->setEnabled(true);
  m_pTooltip->setVisible(false);
  m_pFrameList->addLast(m_pTooltip);

  // In game menu
  createInGameMenu();

  // Log window
  guiFrame * pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrameWhenSmaller, FB_FitFrameToDocumentWhenSmaller, 400, 250, true, "LogFrame", m_pLocalClient->getDisplay());
  pFrame->setMovable(false);
  pFrame->setRetractible(4);  // left side
  pFrame->moveTo(0, 0);
  m_pFrameList->addLast(pFrame);
  m_pLogWnd = new LogDlg(m_pLocalClient);
  pFrame->setDocument(m_pLogWnd);

  // Infos window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrameWhenSmaller, FB_FitFrameToDocument, 240, 300, true, "InfosFrame", m_pLocalClient->getDisplay());
  pFrame->setMovable(false);
  pFrame->moveTo(m_pLocalClient->getClientParameters()->screenXSize - 239, m_pLocalClient->getClientParameters()->screenYSize - 299);
  m_pFrameList->addLast(pFrame);
  m_pInfoWnd = new InfoboxDlg(pFrame->getInnerWidth(), m_pLocalClient);
  pFrame->setDocument(m_pInfoWnd);

  // Unit options window
  pFrame = guiFrame::createDefaultFrame(FB_FitFrameToDocument, FB_NoFit, 172, 64, true, "UnitOptionsFrame", m_pLocalClient->getDisplay());
  pFrame->setMovable(false);
  pFrame->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - 85, m_pLocalClient->getClientParameters()->screenYSize - 64);
  pFrame->setPositionType(FP_Fixed);
  m_pFrameList->addLast(pFrame);
  m_pUnitOptionsWnd = new UnitOptionsDlg(m_pLocalClient);
  pFrame->setDocument(m_pUnitOptionsWnd);

  // Spell window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitDocumentToFrameWhenSmaller, 80, m_pLocalClient->getClientParameters()->screenYSize, true, "SpellFrame", m_pLocalClient->getDisplay());
  pFrame->setMovable(false);
  pFrame->moveTo(m_pLocalClient->getClientParameters()->screenXSize - 79, 0);
  pFrame->setRetractible(2);  // right side
  m_pFrameList->addLast(pFrame);
  m_pSpellWnd = new SpellDlg(m_pLocalClient, m_pLocalClient->getPlayerManager()->getLocalPlayersCount(false) > 1);
  pFrame->setDocument(m_pSpellWnd);

  // Resolve window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitFrameToDocument, 400, 500, true, "ResolveFrame", m_pLocalClient->getDisplay());
  pFrame->setMovable(false);
  pFrame->moveTo(0, m_pLocalClient->getClientParameters()->screenYSize - 500);
  m_pFrameList->addLast(pFrame);
  m_pResolveWnd = new ResolveDlg(pFrame->getInnerWidth(), m_pLocalClient);
  pFrame->setDocument(m_pResolveWnd);

  // Map object window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitDocumentToFrame, m_pLocalClient->getClientParameters()->screenXSize - 80, m_pLocalClient->getClientParameters()->screenYSize - 80, false, "MapObjFrame", m_pLocalClient->getDisplay());
  m_pFrameList->addLast(pFrame);
  m_pMapObjectWnd = new MapObjectDlg(m_pLocalClient, pFrame->getInnerWidth(), pFrame->getInnerHeight());
  pFrame->setDocument(m_pMapObjectWnd);
  pFrame->moveTo(40, 40);

  // Game over window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitDocumentToFrame, m_pLocalClient->getClientParameters()->screenXSize - 80, m_pLocalClient->getClientParameters()->screenYSize - 80, false, "GameOverFrame", m_pLocalClient->getDisplay());
  m_pFrameList->addLast(pFrame);
  m_pGameOverWnd = new GameOverDlg(m_pLocalClient, pFrame->getInnerWidth(), pFrame->getInnerHeight());
  pFrame->setDocument(m_pGameOverWnd);
  pFrame->moveTo(40, 40);

  // Spells selector window
  pFrame = guiFrame::createDefaultFrame(FB_FitDocumentToFrame, FB_FitDocumentToFrame, m_pLocalClient->getClientParameters()->screenXSize - 80, m_pLocalClient->getClientParameters()->screenYSize - 80, false, "SpellsSelectorFrame", m_pLocalClient->getDisplay());
  m_pFrameList->addLast(pFrame);
  m_pSpellsSelectorWnd = new SpellsSelectorDlg(m_pLocalClient, pFrame->getInnerWidth(), pFrame->getInnerHeight());
  pFrame->setDocument(m_pSpellsSelectorWnd);
  pFrame->moveTo(40, 40);

  // Player selector window
  pFrame = guiFrame::createDefaultFrame(FB_FitFrameToDocument, FB_FitFrameToDocument, 1, 1, false, "PlayerSelectorFrame", m_pLocalClient->getDisplay());
  m_pFrameList->addLast(pFrame);
  m_pPlayerSelectorWnd = new PlayerSelectorDlg(m_pLocalClient);
  pFrame->setDocument(m_pPlayerSelectorWnd);

  // Player selector window
  pFrame = guiFrame::createDefaultFrame(FB_FitFrameToDocument, FB_FitFrameToDocument, 1, 1, false, "StatusFrame", m_pLocalClient->getDisplay());
  m_pFrameList->addLast(pFrame);
  m_pStatusWnd = new StatusDlg(m_pLocalClient);
  pFrame->setDocument(m_pStatusWnd);

  findFrame("UnitOptionsFrame")->setVisible(false);
  findFrame("ResolveFrame")->setVisible(false);
  findFrame("MapObjFrame")->setVisible(false);
  findFrame("GameOverFrame")->setVisible(false);
  findFrame("SpellsSelectorFrame")->setVisible(false);
  findFrame("PlayerSelectorFrame")->setVisible(false);
  findFrame("StatusFrame")->setVisible(false);
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void InterfaceManager::Update(double delta)
{
  // Update frames
  guiFrame * frm = (guiFrame*) m_pFrameList->getFirst(m_iFrmIt);
  while (frm != NULL)
  {
    frm->update(delta);
    if (frm->getDocument() == NULL)   // Dead frame
    {
      frm = deleteCurrentFrame(m_iFrmIt);
      continue;
    }
    frm = (guiFrame*) m_pFrameList->getNext(m_iFrmIt);
  }
  if (m_pNextLocalPlayerDlg != NULL && m_pNextLocalPlayerDlg->isVisible())
  {
    guiComponent * pCpnt = m_pNextLocalPlayerDlg->getClickedComponent();
    if (pCpnt != NULL)
      onClickNextLocalPlayer(pCpnt);
  }
  if (m_pInGameMenu != NULL && m_pInGameMenu->isVisible())
  {
    guiComponent * pCpnt = m_pInGameMenu->getClickedComponent();
    if (pCpnt != NULL)
      onClickInGameMenu(pCpnt);
  }
  if (m_pExtraMana != NULL && m_pExtraMana->isVisible())
  {
    guiComponent * pCpnt = m_pExtraMana->getClickedComponent();
    if (pCpnt != NULL)
      onClickExtraMana(pCpnt);
  }

  // Tooltip
  if (m_pPointedObject != NULL && m_fTooltipTime > 0)
  {
    m_fTooltipTime -= delta;
    if (m_fTooltipTime <= 0)
    {
      m_fTooltipTime = 0;
      char * sText = m_pPointedObject->getTooltipText();
      if (strcmp(sText, "") != 0)
      {
        m_pTooltip->getDocument()->deleteAllComponents();
        m_pTooltip->getDocument()->setDimensions(0, 0);
        getRichText(m_pTooltip->getDocument(), CoordsScreen(0, 0), sText);
        m_pTooltip->updateSizeFit();
        CoordsScreen cs = m_pLocalClient->getInput()->getCurrentCursorPosition();
        cs.y += 10;
        int width = m_pTooltip->getDocument()->getWidth() + 2;
        int height = m_pTooltip->getDocument()->getHeight() + 2;
        if (cs.x + width + 10 > m_pLocalClient->getClientParameters()->screenXSize)
          cs.x = m_pLocalClient->getClientParameters()->screenXSize - width - 10;
        if (cs.y + height + 10 > m_pLocalClient->getClientParameters()->screenYSize)
          cs.y = m_pLocalClient->getClientParameters()->screenYSize - height - 10;
        m_pTooltip->moveTo(cs.x, cs.y);
        m_pTooltip->setVisible(true);
        m_pFrameList->goTo(0, m_pTooltip);
        m_pFrameList->moveCurrentToEnd(0);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void InterfaceManager::Display()
{
  if (m_pLocalClient->getGameStep() == GS_InMenus)
    m_pMenuBgGeometry->display(CoordsScreen(0, 0, BOARDPLANE), F_RGBA_NULL);
  m_pTopDisplayList->deleteAll();
  guiFrame * frm = (guiFrame*) m_pFrameList->getFirst(m_iFrmIt);
  while (frm != NULL)
  {
    F_RGBA docColor = rgba(1,1,1,0.65f);
    if (frm != m_pTooltip)
      frm->displayAt(0, 0, rgba(1,1,1,1), docColor);
    frm = (guiFrame*) m_pFrameList->getNext(m_iFrmIt);
  }
  TopDisplayObject * obj = (TopDisplayObject*) m_pTopDisplayList->getFirst(0);
  while (obj != NULL)
  {
    obj->pObj->displayAt(obj->iX, obj->iY, obj->cpntColor, obj->docColor);
    obj = (TopDisplayObject*) m_pTopDisplayList->getNext(0);
  }
  if (m_pTooltip != NULL)
  {
    F_RGBA docColor = rgba(1,1,1,0.85f);
    m_pTooltip->displayAt(0, 0, rgba(1,1,1,1), docColor);
  }
}

// -----------------------------------------------------------------
// Name : onCatchButtonEvent
//  Called by Input Engine.
//  Must return true if event is consumed ; false to let the event be catched by other modules
//  Transfer the message to top frame under mouse
// -----------------------------------------------------------------
bool InterfaceManager::onCatchButtonEvent(ButtonAction * pEvent)
{
  if (pEvent->eButton == ButtonStart && pEvent->eEvent == Event_Down)
    return onClickStart();
  if (pEvent->eEvent == Event_Down)
  {
    if (m_bTargetMode && pEvent->eButton == Button1)
    {
      if (m_pTargetOnClickClbk(m_pTargetedObject, m_uIsLuaPlayerGO))
        return true;
    }

    // Reset clicked objects
    if (pEvent->eButton == Button1)
      m_pClickedObjects[0] = NULL;
    else if (pEvent->eButton == Button2)
      m_pClickedObjects[1] = NULL;

    int xoffset, yoffset;
    guiComponent * pObj1 = getObjectAt(pEvent->xPos - pEvent->xOffset, pEvent->yPos - pEvent->yOffset, &xoffset, &yoffset);
    pEvent->xOffset += xoffset;
    pEvent->yOffset += yoffset;
    if (pObj1 == NULL)
      return false; // not concerned
    if (!pObj1->isEnabled())
      return true;  // event is for us, but do nothing

    if ((pObj1->getType() & GOTYPE_FRAME) && pEvent->eButton == Button1)
      bringFrameAbove((guiFrame*)pObj1);

    // Send event
    guiObject * pObj = pObj1->onButtonEvent(pEvent);
    // Store object for dragging (only for main buttons)
    if (pEvent->eButton == Button1)
      m_pClickedObjects[0] = pObj;
    else if (pEvent->eButton == Button2)
      m_pClickedObjects[1] = pObj;
    // else, don't care about dragging, so just forget pObj.
    return true;
  }
  else
  {
    guiObject ** pObj = (pEvent->eButton == Button1) ? &(m_pClickedObjects[0]) : ((pEvent->eButton == Button2) ? &(m_pClickedObjects[1]) : NULL);
    if (pObj != NULL && *pObj != NULL)
    {
      *pObj = (*pObj)->onButtonEvent(pEvent);
      if (*pObj != NULL && ((*pObj)->getType() & GOTYPE_FRAME) && pEvent->eEvent == Event_Drag)
        ((guiFrame*) (*pObj))->checkPositionIfDragged(m_pLocalClient->getClientParameters());
    }
    return true;
  }
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
// -----------------------------------------------------------------
bool InterfaceManager::onCursorMoveEvent(int xPxl, int yPxl)
{
  if (m_bTargetMode)
    m_pTargetedObject = NULL;

  int xoffset, yoffset;
  guiComponent * pObj = getObjectAt(xPxl, yPxl, &xoffset, &yoffset);
  if (pObj == NULL)
  {
    if (m_pPointedObject != NULL)
    {
      m_pPointedObject->onCursorMoveOutEvent();
      m_pPointedObject = NULL;
      m_pTooltip->setVisible(false);
    }
    return false;
  }
  xPxl -= xoffset;
  yPxl -= yoffset;

  guiObject * pPointed = pObj->onCursorMoveEvent(xPxl, yPxl);
  if (pPointed == NULL)
  {
    if (m_pPointedObject != NULL)
    {
      m_pPointedObject->onCursorMoveOutEvent();
      m_pPointedObject = NULL;
      m_pTooltip->setVisible(false);
    }
  }
  else if (pPointed != m_pPointedObject)
  {
    if (m_pPointedObject != NULL)
      m_pPointedObject->onCursorMoveOutEvent();
    m_pTooltip->setVisible(false);
    m_pPointedObject = pPointed;
    if (m_fTooltipTime > 0)
      m_fTooltipTime = 0.5f;
    else
      m_fTooltipTime = 0.2f;
  }
  if (m_bTargetMode && (pObj->getType() & GOTYPE_FRAME))
  {
    m_pTargetedObject = ((guiFrame*)pObj)->getTargetedObject(&m_uIsLuaPlayerGO);
    if (m_pTargetedObject != NULL)
    {
      if (m_pTargetOnMouseOverClbk(m_pTargetedObject, m_uIsLuaPlayerGO))
        ((guiFrame*)pObj)->setTargetValid(true);
      else
      {
        ((guiFrame*)pObj)->setTargetValid(false);
        m_pTargetedObject = NULL;
      }
    }
    else
      ((guiFrame*)pObj)->setTargetValid(false);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : getObjectAt
// -----------------------------------------------------------------
guiComponent * InterfaceManager::getObjectAt(int xPxl, int yPxl, int * xoffset, int * yoffset)
{
  // First look at TopDisplayList
  TopDisplayObject * obj = (TopDisplayObject*) m_pTopDisplayList->getLast(0);
  while (obj != NULL)
  {
    if (obj->pObj->isAt(xPxl - obj->iX, yPxl - obj->iY))
    {
      *xoffset = obj->iX;
      *yoffset = obj->iY;
      return obj->pObj;
    }
    obj = (TopDisplayObject*) m_pTopDisplayList->getPrev(0);
  }
  // Then, frames
  *xoffset = 0;
  *yoffset = 0;
  guiFrame * frm = (guiFrame*) m_pFrameList->getLast(m_iFrmIt);
  while (frm != NULL)
  {
    if (frm != m_pTooltip && frm->isAt(xPxl, yPxl))
      return frm;
    frm = (guiFrame*) m_pFrameList->getPrev(m_iFrmIt);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findFrame
// -----------------------------------------------------------------
guiFrame * InterfaceManager::findFrame(const char * frmId)
{
  guiFrame * frm = (guiFrame*) m_pFrameList->getFirst(0);
  while (frm != NULL)
  {
    if (strcmp(frm->getId(), frmId) == 0)
      return frm;
    frm = (guiFrame*) m_pFrameList->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findFrameFromDoc
// -----------------------------------------------------------------
guiFrame * InterfaceManager::findFrameFromDoc(guiDocument * pDoc)
{
  guiFrame * frm = (guiFrame*) m_pFrameList->getFirst(0);
  while (frm != NULL)
  {
    if (frm->getDocument() == pDoc)
      return frm;
    frm = (guiFrame*) m_pFrameList->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : waitLocalPlayer
// -----------------------------------------------------------------
void InterfaceManager::waitLocalPlayer()
{
  // Remove move or attack popup if exist
  if (m_pMoveOrAttackDlg != NULL)
    m_pLocalClient->getGameboard()->getInputs()->onMoveOrAttackResponse(true, NULL);

  // Show popup "choose next player"
  m_pNextLocalPlayerDlg = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
  m_pFrameList->addLast(m_pNextLocalPlayerDlg);
  guiDocument * pDoc = m_pNextLocalPlayerDlg->getDocument();

  int width = 220;
  int nbLocalPlayersWaiting = 0;
  int yPxl = 0;
  Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
  while (pPlayer != NULL)
  {
    if (pPlayer->m_uClientId == m_pLocalClient->getClientId() && !pPlayer->m_bIsAI)
    {
      guiButton * pBtn = guiButton::createDefaultSmallButton(pPlayer->getAvatarName(), width, "", m_pLocalClient->getDisplay());
      pBtn->moveTo(0, yPxl);
      pBtn->setAttachment(pPlayer);
      if (pPlayer->getState() == waiting)
      {
        nbLocalPlayersWaiting++;
        pBtn->setEnabled(true);
      }
      else
        pBtn->setEnabled(false);
      pDoc->addComponent(pBtn);
      yPxl += pBtn->getHeight();
    }
    pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
  }

  char sText[256] = "";
  guiLabel * pLbl = new guiLabel();
  if (nbLocalPlayersWaiting == 0)
    i18n->getText("WAIT_WHILE_FINISHING_TURNS", sText, 256);
  else
    i18n->getText("CHOOSE_NEXT_PLAYER", sText, 256);
  pLbl->init(sText, H2_FONT, H2_COLOR, "", 0, 3, width-6, 0, m_pLocalClient->getDisplay());
  pLbl->setXPos(width / 2 - pLbl->getWidth() / 2);
  int yDecal = pLbl->getHeight() + 6;
  guiComponent * pCpnt = pDoc->getFirstComponent();
  while (pCpnt != NULL)
  {
    pCpnt->moveBy(0, yDecal);
    pCpnt = pDoc->getNextComponent();
  }
  pDoc->setDimensions(width, yDecal + yPxl);
  pDoc->addComponent(pLbl);
  m_pNextLocalPlayerDlg->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - width / 2, 80);
}

// -----------------------------------------------------------------
// Name : onClickNextLocalPlayer
// -----------------------------------------------------------------
void InterfaceManager::onClickNextLocalPlayer(guiComponent * pCpnt)
{
  // Get next player
  Player * pPlayer = (Player*) pCpnt->getAttachment();
  assert(pPlayer != NULL);
  assert(pPlayer->m_uClientId == m_pLocalClient->getClientId() && pPlayer->getState() == waiting && !pPlayer->m_bIsAI);
  deleteFrame(m_pNextLocalPlayerDlg);
  m_pNextLocalPlayerDlg = NULL;
  m_pLocalClient->startPlayerTurn(pPlayer);
}

// -----------------------------------------------------------------
// Name : enableNextPlayer
// -----------------------------------------------------------------
void InterfaceManager::enableNextPlayer(Player * pPlayer)
{
  m_pSpellWnd->updateContent(pPlayer);
  resetSharedPointers();
}

// -----------------------------------------------------------------
// Name : disableNoPlayer
// -----------------------------------------------------------------
void InterfaceManager::disableNoPlayer()
{
  m_pSpellWnd->updateContent(NULL);
  updateUnitOptionsDialog(NULL);
  resetSharedPointers();
}

// -----------------------------------------------------------------
// Name : updateUnitOptionsDialog
// -----------------------------------------------------------------
void InterfaceManager::updateUnitOptionsDialog(Unit * unit)
{
  if (m_pUnitOptionsWnd->getUnit() == unit && unit != NULL)
    m_pUnitOptionsWnd->updateOrder();
  else
  {
    m_pUnitOptionsWnd->setUnit(unit);
    findFrame("UnitOptionsFrame")->setVisible(unit != NULL);
  }
}

// -----------------------------------------------------------------
// Name : showStack
// -----------------------------------------------------------------
void InterfaceManager::showStack(MapTile * pTile, u32 uDisplayFilter, u32 uClickFilter, StackGroupInterface * pGroupInterface, GraphicObject * pDefaultSelectedObject)
{
//  findFrame("StackFrame")->setVisible(true);
  m_pInfoWnd->setMapTile(pTile, uDisplayFilter, uClickFilter, pGroupInterface, m_pLocalClient->getPlayerManager()->getActiveLocalPlayer(), pDefaultSelectedObject);
}

// -----------------------------------------------------------------
// Name : showMapObjectDialog
// -----------------------------------------------------------------
void InterfaceManager::showMapObjectDialog(MapTile * pTile)
{
  guiFrame * pFrm = findFrame("MapObjFrame");
  pFrm->setVisible(true);
  bringFrameAbove(pFrm);
  m_pMapObjectWnd->setTile(pTile);
}

// -----------------------------------------------------------------
// Name : hideMapObjectDialog
// -----------------------------------------------------------------
void InterfaceManager::hideMapObjectDialog()
{
  findFrame("MapObjFrame")->setVisible(false);
}

// -----------------------------------------------------------------
// Name : showResolveDialog
// -----------------------------------------------------------------
void InterfaceManager::showResolveDialog()
{
  if (m_pNextLocalPlayerDlg != NULL)
  {
    deleteFrame(m_pNextLocalPlayerDlg);
    m_pNextLocalPlayerDlg = NULL;
  }
  findFrame("ResolveFrame")->setVisible(true);
}

// -----------------------------------------------------------------
// Name : hideResolveDialog
// -----------------------------------------------------------------
void InterfaceManager::hideResolveDialog()
{
  findFrame("ResolveFrame")->setVisible(false);
}

// -----------------------------------------------------------------
// Name : showGameOverDialog
// -----------------------------------------------------------------
void InterfaceManager::showGameOverDialog(ObjectList * pWinners)
{
  m_pGameOverWnd->setWinners(pWinners);
  findFrame("GameOverFrame")->setVisible(true);
}

// -----------------------------------------------------------------
// Name : hideGameOverDialog
// -----------------------------------------------------------------
void InterfaceManager::hideGameOverDialog()
{
  findFrame("GameOverFrame")->setVisible(false);
}

extern bool clbkMoveOrAttack_OnMouseOverInterface(BaseObject * pObj, u8 isLua);
extern bool clbkMoveOrAttack_OnClickInterface(BaseObject * pObj, u8 isLua);
// -----------------------------------------------------------------
// Name : showMoveOrAttackDialog
// -----------------------------------------------------------------
void InterfaceManager::showMoveOrAttackDialog(Unit * pUnit, CoordsMap mapPos)
{
  assert(m_pMoveOrAttackDlg == NULL);
  m_pMoveOrAttackDlg = new MoveOrAttackDlg(m_pLocalClient, pUnit, mapPos);
  CoordsScreen cs = m_pLocalClient->getInput()->getCurrentCursorPosition();
  guiFrame * pFrm = guiFrame::createDefaultFrame(FB_FitFrameToDocument, FB_FitFrameToDocument, 1, 1, false, "MoveOrAttackFrame", m_pLocalClient->getDisplay());
  pFrm->moveTo(cs.x, cs.y);
  pFrm->setDocument(m_pMoveOrAttackDlg);
  registerFrame(pFrm);
  setTargetMode(clbkMoveOrAttack_OnMouseOverInterface, clbkMoveOrAttack_OnClickInterface);
//  setPrioritizedFrame(m_pMoveOrAttackPopup);
}
// -----------------------------------------------------------------
// Name : hideMoveOrAttackDialog
// -----------------------------------------------------------------
void InterfaceManager::hideMoveOrAttackDialog()
{
  if (m_pMoveOrAttackDlg != NULL)
  {
    deleteFrame(findFrameFromDoc(m_pMoveOrAttackDlg));
    m_pMoveOrAttackDlg = NULL;
  }
}

// -----------------------------------------------------------------
// Name : registerFrame
// -----------------------------------------------------------------
void InterfaceManager::registerFrame(guiFrame * pFrm)
{
  m_pFrameList->addLast(pFrm);
}

// -----------------------------------------------------------------
// Name : deleteFrame
// -----------------------------------------------------------------
void InterfaceManager::deleteFrame(guiFrame * pFrm)
{
  guiFrame * frm = (guiFrame*) m_pFrameList->getFirst(0);
  while (frm != NULL)
  {
    if (frm == pFrm)
    {
      m_pFrameList->deleteCurrent(0, true);
      resetSharedPointers();   // since the frame has probably been closed via its own button, we set ClickedObject to NULL by security. In the worst case it would just cancel a drag&drop or double-click.
      return;
    }
    frm = (guiFrame*) m_pFrameList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : deleteCurrentFrame
// -----------------------------------------------------------------
guiFrame * InterfaceManager::deleteCurrentFrame(int iterator)
{
  resetSharedPointers();   // since the frame has probably been closed via its own button, we set ClickedObject to NULL by security. In the worst case it would just cancel a drag&drop or double-click.
  return (guiFrame*) m_pFrameList->deleteCurrent(iterator, true);
}

// -----------------------------------------------------------------
// Name : resetSharedPointers
// -----------------------------------------------------------------
void InterfaceManager::resetSharedPointers(guiObject * pObj)
{
  if (pObj == NULL)
  {
    m_pClickedObjects[0] = m_pClickedObjects[1] = NULL;
    m_pPointedObject = NULL;
    m_pTooltip->setVisible(false);
    m_pTopDisplayList->deleteAll();
  }
  else
  {
    if (pObj == m_pClickedObjects[0])
      m_pClickedObjects[0] = NULL;
    if (pObj == m_pClickedObjects[1])
      m_pClickedObjects[1] = NULL;
    if (pObj == m_pPointedObject)
      m_pPointedObject = NULL;
  }
}

// -----------------------------------------------------------------
// Name : bringFrameAbove
// -----------------------------------------------------------------
void InterfaceManager::bringFrameAbove(guiFrame * pFrm)
{
  m_pFrameList->goTo(0, pFrm);
  m_pFrameList->moveCurrentToEnd(0);
  m_pFrameList->goTo(0, m_pTooltip);  // keep tooltips above, in any case
  m_pFrameList->moveCurrentToEnd(0);
}

// -----------------------------------------------------------------
// Name : setTargetMode
// -----------------------------------------------------------------
void InterfaceManager::setTargetMode(CLBK_INTERFACE_TARGET_ON_MOUSE_OVER * pCallback1, CLBK_INTERFACE_TARGET_ON_CLICK * pCallback2)
{
  m_bTargetMode = true;
  m_pTargetOnMouseOverClbk = pCallback1;
  m_pTargetOnClickClbk = pCallback2;
  m_pTargetedObject = NULL;
}

// -----------------------------------------------------------------
// Name : setUniqueDialog
// -----------------------------------------------------------------
void InterfaceManager::setUniqueDialog(guiDocument * pDoc)
{
  guiFrame * pFrm = (guiFrame*) m_pFrameList->getFirst(0);
  while (pFrm != NULL)
  {
    if (pFrm != m_pTooltip)
    {
      if (pFrm->getDocument() == pDoc)
      {
        pFrm->setEnabled(true);
        pFrm->getDocument()->setEnabled(true);
        pFrm->show();
      }
      else if (pFrm->isVisible())
        pFrm->hide();
    }
    pFrm = (guiFrame*) m_pFrameList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : topDisplay
// -----------------------------------------------------------------
void InterfaceManager::topDisplay(guiComponent * pObj, int iX, int iY, F_RGBA cpntColor, F_RGBA docColor)
{
  TopDisplayObject * obj = new TopDisplayObject();
  obj->pObj = pObj;
  obj->iX = iX;
  obj->iY = iY;
  obj->cpntColor = cpntColor;
  obj->docColor = docColor;
  m_pTopDisplayList->addLast(obj);
}

// -----------------------------------------------------------------
// Name : cancelTopDisplay
// -----------------------------------------------------------------
void InterfaceManager::cancelTopDisplay(guiComponent * pObj)
{
  TopDisplayObject * obj = (TopDisplayObject*) m_pTopDisplayList->getFirst(0);
  while (obj != NULL)
  {
    if (obj->pObj == pObj)
    {
      m_pTopDisplayList->deleteCurrent(0, true);
      return;
    }
    obj = (TopDisplayObject*) m_pTopDisplayList->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : onResize
// -----------------------------------------------------------------
void InterfaceManager::onResize(int oldw, int oldh)
{
  float fWCoef = (float) m_pLocalClient->getClientParameters()->screenXSize / (float) oldw;
  float fHCoef = (float) m_pLocalClient->getClientParameters()->screenYSize / (float) oldh;
  guiFrame * pFrm = (guiFrame*) m_pFrameList->getFirst(0);
  while (pFrm != NULL)
  {
    int xc = pFrm->getXPos() + pFrm->getWidth() / 2;
    int yc = pFrm->getYPos() + pFrm->getHeight() / 2;
    pFrm->moveBy((float)xc * (fWCoef - 1), (float)yc * (fHCoef - 1));
    // Check that it's not out of screen
    if (pFrm->getXPos() + pFrm->getWidth() < 5)
      pFrm->moveTo(5 - pFrm->getWidth(), pFrm->getYPos());
    else if (pFrm->getXPos() > m_pLocalClient->getClientParameters()->screenXSize - 5)
      pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize - 5, pFrm->getYPos());
    if (pFrm->getYPos() + pFrm->getHeight() < 5)
      pFrm->moveTo(pFrm->getXPos(), 5 - pFrm->getHeight());
    else if (pFrm->getYPos() > m_pLocalClient->getClientParameters()->screenYSize - 5)
      pFrm->moveTo(pFrm->getXPos(), m_pLocalClient->getClientParameters()->screenYSize - 5);
    pFrm = (guiFrame*) m_pFrameList->getNext(0);
  }
  FREE(m_pMenuBgGeometry);
  int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture("menubg");
  QuadData quad(0, m_pLocalClient->getClientParameters()->screenXSize, 0, m_pLocalClient->getClientParameters()->screenYSize, iTex, m_pLocalClient->getDisplay());
  m_pMenuBgGeometry = new GeometryQuads(&quad, VB_Static);
}

// -----------------------------------------------------------------
// Name : showInGameMenu
// -----------------------------------------------------------------
void InterfaceManager::showInGameMenu()
{
  m_pInGameMenu->setVisible(!m_pInGameMenu->isVisible());
}

// -----------------------------------------------------------------
// Name : createInGameMenu
// -----------------------------------------------------------------
void InterfaceManager::createInGameMenu()
{
  m_pInGameMenu = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
  registerFrame(m_pInGameMenu);
  guiDocument * pDoc = m_pInGameMenu->getDocument();
  int docwidth = 200;

  // Resume game button
  int yPxl = 10;
  char sText[64];
  i18n->getText("RESUME_GAME", sText, 64);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "Resume", m_pLocalClient->getDisplay());
  pBtn->setWidth(docwidth-20);
  pBtn->moveTo(10, yPxl);
  pDoc->addComponent(pBtn);

  // Save game button
  yPxl += pBtn->getHeight() + 10;
  i18n->getText("SAVE_GAME", sText, 64);
  pBtn = guiButton::createDefaultNormalButton(sText, "Save", m_pLocalClient->getDisplay());
  pBtn->setWidth(docwidth-20);
  pBtn->moveTo(10, yPxl);
  pBtn->setEnabled(m_pLocalClient->getServer() != NULL);
  pDoc->addComponent(pBtn);

  // Options button
  yPxl += pBtn->getHeight() + 10;
  i18n->getText("OPTIONS", sText, 64);
  pBtn = guiButton::createDefaultNormalButton(sText, "Options", m_pLocalClient->getDisplay());
  pBtn->setWidth(docwidth-20);
  pBtn->moveTo(10, yPxl);
  pDoc->addComponent(pBtn);

  // Quit button
  yPxl += pBtn->getHeight() + 10;
  i18n->getText("QUIT", sText, 64);
  pBtn = guiButton::createDefaultNormalButton(sText, "Quit", m_pLocalClient->getDisplay());
  pBtn->setWidth(docwidth-20);
  pBtn->moveTo(10, yPxl);
  pDoc->addComponent(pBtn);

  pDoc->setDimensions(docwidth, yPxl + pBtn->getHeight() + 10);
  m_pInGameMenu->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - docwidth / 2, m_pLocalClient->getClientParameters()->screenYSize / 2 - pDoc->getHeight() / 2);
  m_pInGameMenu->setVisible(false);
}

// -----------------------------------------------------------------
// Name : onClickInGameMenu
// -----------------------------------------------------------------
void InterfaceManager::onClickInGameMenu(guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "Resume") == 0)
    m_pInGameMenu->setVisible(false);
  else if (strcmp(pCpnt->getId(), "Save") == 0)
  {
    m_pInGameMenu->setVisible(false);
    Server * pServer = m_pLocalClient->getServer();
    assert(pServer != NULL);
    pServer->saveGame();
  }
  else if (strcmp(pCpnt->getId(), "Options") == 0)
  {
  }
  else if (strcmp(pCpnt->getId(), "Quit") == 0)
    m_pLocalClient->endGame();
}

// -----------------------------------------------------------------
// Name : onClickStart
// -----------------------------------------------------------------
bool InterfaceManager::onClickStart()
{
  if (m_pNextLocalPlayerDlg != NULL && m_pNextLocalPlayerDlg->isVisible())
  {
    Player * pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
      if (pPlayer->m_uClientId == m_pLocalClient->getClientId() && pPlayer->getState() == waiting && !pPlayer->m_bIsAI)
      {
        deleteFrame(m_pNextLocalPlayerDlg);
        m_pNextLocalPlayerDlg = NULL;
        m_pLocalClient->startPlayerTurn(pPlayer);
        return true;
      }
      pPlayer = (Player*) m_pLocalClient->getPlayerManager()->getPlayersList()->getNext(0);
    }
    return true;
  }
  if (m_pExtraMana != NULL && m_pExtraMana->isVisible())
  {
    m_pExtraMana->getDocument()->doClick("Ok");
    return true;
  }

  guiFrame * pFrm = (guiFrame*) m_pFrameList->getFirst(0);
  while (pFrm != NULL)
  {
    if (pFrm->isVisible())
    {
      if (pFrm->getDocument() == m_pResolveWnd)
        return m_pResolveWnd->onClickStart();
      if (pFrm->getDocument() == m_pMapObjectWnd)
        return m_pMapObjectWnd->onClickStart();
      if (pFrm->getDocument() == m_pSelectPlayerAvatarWnd)
        return m_pSelectPlayerAvatarWnd->onClickStart();
      if (pFrm->getDocument() == m_pBuildDeckWnd)
        return m_pBuildDeckWnd->onClickStart();
    }
    pFrm = (guiFrame*) m_pFrameList->getNext(0);
  }

  return false;
}

// -----------------------------------------------------------------
// Name : askForExtraMana
// -----------------------------------------------------------------
void InterfaceManager::askForExtraMana(char * sDescription, u16 mana, int min, int max, char * sCallback, u32 uSourceType)
{
  // Show popup "enter extra mana"
  m_pExtraMana = guiPopup::createEmptyPopup(m_pLocalClient->getDisplay());
  m_pFrameList->addLast(m_pExtraMana);
  guiDocument * pDoc = m_pExtraMana->getDocument();

  LuaObject * pLua = NULL;
  int yPxl = 5;
  int iWidth = 300;
  char sText[LABEL_MAX_CHARS];
  char sBuf[LABEL_MAX_CHARS];
  if (uSourceType == LUAOBJECT_SPELL)
  {
    pLua = m_pLocalClient->getPlayerManager()->getSpellBeingCast();
    i18n->getText("SPELL_(s)_ASK_EXTRA_MANA", sBuf, LABEL_MAX_CHARS);
  }
  else if (uSourceType == LUAOBJECT_SKILL)
  {
    pLua = (Skill*) m_pLocalClient->getPlayerManager()->getSkillBeingActivated()->getAttachment();
    i18n->getText("SKILL_(s)_ASK_EXTRA_MANA", sBuf, LABEL_MAX_CHARS);
  } // Note : this function should not be called by a building or a special tile, since it asks for extra mana in reaction to a spell casting or skill activation.
  assert(pLua != NULL);
  snprintf(sText, LABEL_MAX_CHARS, sBuf, pLua->getLocalizedName());
  if (sDescription[0] != '\0')
  {
    wsafecat(sText, LABEL_MAX_CHARS, " - ");
    wsafecat(sText, LABEL_MAX_CHARS, sDescription);
  }

  // Invisible information
  guiLabel * pLbl = new guiLabel();
  pLbl->init(sCallback, TEXT_FONT, TEXT_COLOR, "Hidden", uSourceType, 0, 0, 0, m_pLocalClient->getDisplay());
  pLbl->setVisible(false);
  pDoc->addComponent(pLbl);

  pLbl = new guiLabel();
  pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 5, yPxl, iWidth - 10, 0, m_pLocalClient->getDisplay());
  pDoc->addComponent(pLbl);
  yPxl += pLbl->getHeight() + 10;

  int nbCombos = 0;
  Mana remainingMana = getSpellDialog()->getRemainingMana();
  char signs[4] = MANA_SIGNS;
  char texts[4][LABEL_MAX_CHARS] = MANA_TEXTS;
  int _2powi = 1;
  for (int iMana = 0; iMana < 4; iMana++)
  {
    // combo?
    if ((mana & _2powi) && (min <= remainingMana[iMana]))
    {
      nbCombos++;
      i18n->getText(texts[iMana], sText, LABEL_MAX_CHARS);
      pLbl = new guiLabel();
      pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 5, yPxl, iWidth - 10, 0, m_pLocalClient->getDisplay());
      pDoc->addComponent(pLbl);
      yPxl += pLbl->getHeight();
      guiComboBox * pCombo = guiComboBox::createDefaultComboBox(texts[iMana], this, m_pLocalClient->getDisplay());
      pCombo->moveTo(5, yPxl);
      int max2 = (max < 0) ? max = remainingMana[iMana] : min(max, remainingMana[iMana]);
      char sMana[8];
      for (int i = min; i <= max2; i++)
      {
        snprintf(sMana, 8, "%d", i);
        snprintf(sText, LABEL_MAX_CHARS, "%c %d", signs[iMana], i);
        pCombo->addString(sText, sMana);
      }
      pCombo->setItem(0);
      pDoc->addComponent(pCombo);
      yPxl += pCombo->getHeight() + 10;
    }
    _2powi *= 2;
  }

  if (nbCombos == 0)
  {
    i18n->getText("NO_MANA_AVAILABLE", sText, LABEL_MAX_CHARS);
    pLbl = new guiLabel();
    pLbl->init(sText, TEXT_FONT, TEXT_COLOR, "", 5, yPxl, iWidth - 10, 0, m_pLocalClient->getDisplay());
    pDoc->addComponent(pLbl);
    yPxl += pLbl->getHeight() + 10;
  }

  // Buttons
  i18n->getText("OK", sText, LABEL_MAX_CHARS);
  guiButton * pBtn = guiButton::createDefaultNormalButton(sText, "Ok", m_pLocalClient->getDisplay());
  pBtn->moveTo(3 * iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);
  if (nbCombos == 0)
    pBtn->setEnabled(false);

  i18n->getText("CANCE", sText, LABEL_MAX_CHARS);
  pBtn = guiButton::createDefaultNormalButton(sText, "Cance", m_pLocalClient->getDisplay());
  pBtn->moveTo(iWidth / 4 - pBtn->getWidth() / 2, yPxl);
  pDoc->addComponent(pBtn);
  yPxl += pBtn->getHeight() + 5;

  pDoc->setDimensions(iWidth, yPxl);
  m_pExtraMana->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - m_pExtraMana->getWidth() / 2, 80);
}

// -----------------------------------------------------------------
// Name : onClickExtraMana
// -----------------------------------------------------------------
void InterfaceManager::onClickExtraMana(guiComponent * pCpnt)
{
  if (strcmp(pCpnt->getId(), "Ok") == 0)
  {
    char texts[4][LABEL_MAX_CHARS] = MANA_TEXTS;
    Mana amount;
    for (int i = 0; i < 4; i++)
    {
      guiComboBox * pCombo = (guiComboBox*) m_pExtraMana->getDocument()->getComponent(texts[i]);
      if (pCombo)
        amount.mana[i] = atoi(pCombo->getSelectedItem()->getId());
    }
    // No logic in what follows!
    guiLabel * pLbl = (guiLabel*) m_pExtraMana->getDocument()->getComponent("Hidden");
    m_pLocalClient->getPlayerManager()->addExtraMana(pLbl->getXPos(), true, pLbl->getText(), amount);
    //if (pLbl->getXPos() == LUAOBJECT_SPELL)
    //{
    //  Spell * pSpell = m_pLocalClient->getPlayerManager()->getSpellBeingCast();
    //  pSpell->callLuaFunction(pLbl->getText(), 0, "i", value);
    //}
    //else
    //{
    //  Skill * pSkill = (Skill*) m_pLocalClient->getPlayerManager()->getSkillBeingActivated()->getAttachment();
    //  pSkill->callLuaFunction(pLbl->getText(), 0, "i", value);
    //}
    deleteFrame(m_pExtraMana);
    m_pExtraMana = NULL;
  }
  else if (strcmp(pCpnt->getId(), "Cance") == 0)
  {
    // No logic in what follows!
    guiComponent * pLbl = m_pExtraMana->getDocument()->getComponent("Hidden");
    m_pLocalClient->getPlayerManager()->addExtraMana(pLbl->getXPos(), false, "", Mana());
    //if (pLbl->getXPos() == LUAOBJECT_SPELL)
    //  m_pLocalClient->getPlayerManager()->castSpellFinished(false, false);
    //else
    //  m_pLocalClient->getPlayerManager()->skillWasActivated(false, false);
    deleteFrame(m_pExtraMana);
    m_pExtraMana = NULL;
  }
}

// -----------------------------------------------------------------
// Name : getRichText
//  sSource is label, except when found:
//    #n is new line
//    #f is font, followed by font name (TEXT, H1, H2, DARK), until # is found
//    #i is image, followed by image file name, until # is found
// Example:
//    #fH1#Hello#n#ismiley_smile##n#fTEXT#Welcome to this game!
// -----------------------------------------------------------------
void InterfaceManager::getRichText(guiDocument * pDest, CoordsScreen offset, char * sSource)
{
  char sCurrentText[LABEL_MAX_CHARS] = "";
  int iSrcLen = strlen(sSource);
  int iSrc = 0;
  int iDst = 0;
  int iX = offset.x;
  int iY = offset.y;
  int maxX = iX;
  char curObject = '0';  // 0 for normal label
  guiComponent * pLastObj = NULL;
  FontId font = TEXT_FONT;
  F_RGBA color = TEXT_COLOR;
  for (iSrc = 0; iSrc < iSrcLen; iSrc++)
  {
    switch (curObject)
    {
    case '#':  // '#' (new object)
        curObject = sSource[iSrc];
        if (curObject == 'n')  // Start new normal object
        {
          if (iX > maxX)
            maxX = iX;
          iX = offset.x;
          iY += (pLastObj == NULL) ? 4 : pLastObj->getHeight() + 4;
          curObject = '0';
          iDst = 0;
        }
        break;
    case '0':   // normal label
      {
        if (sSource[iSrc] == '#')  // start new object
        {
          curObject = '#'; // new object (#)
          // Register previous object (normal label)
          sCurrentText[iDst] = '\0';
          if (iDst > 0)
          {
            guiLabel * pLbl = new guiLabel();
            pLbl->init(sCurrentText, font, color, "", iX, iY, 0, 0, m_pLocalClient->getDisplay());
            pDest->addComponent(pLbl);
            iX += pLbl->getWidth();
            pLastObj = pLbl;
          }
          iDst = 0;
        }
        else
          sCurrentText[iDst++] = sSource[iSrc];
        break;
      }
    case 'f':
      {
        if (sSource[iSrc] == '#')  // end FONT object
        {
          sCurrentText[iDst] = '\0';
          if (strcmp(sCurrentText, "H1") == 0)
          {
            font = H1_FONT;
            color = H1_COLOR;
          }
          else if (strcmp(sCurrentText, "H2") == 0)
          {
            font = H2_FONT;
            color = H2_COLOR;
          }
          else if (strcmp(sCurrentText, "DARK") == 0)
          {
            font = TEXT_FONT;
            color = TEXT_COLOR_DARK;
          }
          else
          {
            font = TEXT_FONT;
            color = TEXT_COLOR;
          }
          curObject = '0';
          iDst = 0;
        }
        else
          sCurrentText[iDst++] = sSource[iSrc];
        break;
      }
    case 'i':
      {
        if (sSource[iSrc] == '#')  // end IMAGE object
        {
          sCurrentText[iDst] = '\0';
          int iTex = m_pLocalClient->getDisplay()->getTextureEngine()->loadTexture(sCurrentText);
          if (iTex >= 0)
          {
            guiImage * pImg = new guiImage();
            pImg->init(iTex, "", iX, iY, -1, -1, m_pLocalClient->getDisplay());
            pDest->addComponent(pImg);
            iX += pImg->getWidth();
            pLastObj = pImg;
          }
          curObject = '0';
          iDst = 0;
        }
        else
          sCurrentText[iDst++] = sSource[iSrc];
        break;
      }
    }
  }

  // Ending normal label
  if (iDst > 0)
  {
    sCurrentText[iDst] = '\0';
    guiLabel * pLbl = new guiLabel();
    pLbl->init(sCurrentText, font, color, "", iX, iY, 0, 0, m_pLocalClient->getDisplay());
    pDest->addComponent(pLbl);
    iX += pLbl->getWidth();
    pLastObj = pLbl;
  }

  // Update document size
  if (iX > maxX)
    maxX = iX;
  if (pDest->getWidth() > maxX)
    maxX = pDest->getWidth();
  iY += (pLastObj == NULL) ? 4 : pLastObj->getHeight() + 4;
  if (pDest->getHeight() > iY)
    iY = pDest->getHeight();
  pDest->setDimensions(maxX, iY);
}
