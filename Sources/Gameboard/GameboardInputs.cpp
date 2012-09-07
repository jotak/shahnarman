// -----------------------------------------------------------------
// GAMEBOARD INPUTS
// -----------------------------------------------------------------
#include "GameboardInputs.h"
#include "GameboardManager.h"
#include "Unit.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../Fx/FxManager.h"
#include "../Input/InputEngine.h"
#include "../Interface/InterfaceManager.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Interface/InfoboxDlg.h"
#include "../GUIClasses/guiFrame.h"
#include "../GUIClasses/guiLabel.h"
#include "../Audio/AudioManager.h"

#define GAMEBOARD_INPUT_PRIORITY  5
#define CURSOR_BLINKING_PERIOD    0.75f

bool clbkAttackTarget_OnMouseOverInterface(BaseObject * pObj, u8 isLuaPlayerGO);
bool clbkAttackTarget_OnClickInterface(BaseObject * pObj, u8 isLuaPlayerGO);
LocalClient * gLocalClient = NULL;

// -----------------------------------------------------------------
// Name : GameboardInputs
// -----------------------------------------------------------------
GameboardInputs::GameboardInputs(GameboardManager * pGameboard, LocalClient * pLocalClient) : EventListener(GAMEBOARD_INPUT_PRIORITY)
{
  m_pGameboard = pGameboard;
  m_pLocalClient = pLocalClient;
  m_pSelectedMapObj = m_pPreSelectedMapObj = NULL;
  m_MouseMode = ModeNormal;
  m_bIsDraggingSelection = false;
  m_fMoveOrAttackColorTimer = -1;
  m_pCustomTargetOnMouseOverClbk = NULL;
  m_pCustomTargetOnClickClbk = NULL;
}

// -----------------------------------------------------------------
// Name : ~GameboardInputs
// -----------------------------------------------------------------
GameboardInputs::~GameboardInputs()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy GameboardInputs\n");
#endif
  InputEngine * pInputs = m_pLocalClient->getInput();
  if (pInputs != NULL)
  {
    pInputs->removeCursoredEventListener(this);
    EventListener * p = pInputs->popUncursoredEventListener();
    if (p != this && p != NULL)  // oops
      pInputs->pushUncursoredEventListener(p);
  }
#ifdef DBG_VERBOSE1
  printf("End destroy GameboardInputs\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void GameboardInputs::init()
{
  m_pSelectedMapObj = m_pPreSelectedMapObj = NULL;
  m_MouseMode = ModeNormal;
  m_bIsDraggingSelection = false;
  m_PreviousPointedTile.x = m_PreviousPointedTile.y = -1;
  m_pLocalClient->getInput()->addCursoredEventListener(this, m_pLocalClient->getDebug());
  m_pLocalClient->getInput()->pushUncursoredEventListener(this);
  m_fMoveOrAttackColorTimer = -1;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void GameboardInputs::update(double delta)
{
  if (m_fMoveOrAttackColorTimer > CURSOR_BLINKING_PERIOD / 2)
  {
    m_fMoveOrAttackColorTimer = max(m_fMoveOrAttackColorTimer - delta, 0.01f);
    if (m_fMoveOrAttackColorTimer <= CURSOR_BLINKING_PERIOD / 2)
      m_pGameboard->getMapCursor(MAPCURSOR_MOVETARGET)->setColor(rgb(1, 0, 0));
  }
  else if (m_fMoveOrAttackColorTimer > 0)
  {
    m_fMoveOrAttackColorTimer -= delta;
    if (m_fMoveOrAttackColorTimer <= 0)
    {
      m_pGameboard->getMapCursor(MAPCURSOR_MOVETARGET)->setColor(rgb(0, 1, 0));
      m_fMoveOrAttackColorTimer = CURSOR_BLINKING_PERIOD;
    }
  }
}

// -----------------------------------------------------------------
// Name : setMouseMode
// -----------------------------------------------------------------
void GameboardInputs::setMouseMode(GameboardMouseMode mouseMode, CLBK_CUSTOM_TARGET_ON_MOUSE_OVER * pCallback1, CLBK_CUSTOM_TARGET_ON_CLICK * pCallback2)
{
  m_MouseMode = mouseMode;
  m_DragNextTurnPosition.x = m_DragToPosition.x = -1;
//  m_pLocalClient->getInterface()->catchMouseOverEvents(mouseMode != ModeMoveToTarget);
  if (mouseMode == ModeSelectCustomTarget)
  {
    assert(pCallback1 != NULL && pCallback2 != NULL);
    m_pCustomTargetOnMouseOverClbk = pCallback1;
    m_pCustomTargetOnClickClbk = pCallback2;
  }
  else if (mouseMode == ModeAttackTarget)
  {
    gLocalClient = m_pLocalClient;  // for callbacks
    m_pLocalClient->getInterface()->setTargetMode(clbkAttackTarget_OnMouseOverInterface, clbkAttackTarget_OnClickInterface);
  }
}

// -----------------------------------------------------------------
// Name : onButton1Down
//  If mouse mode is ModeAttackTarget or ModeMoveToTarget, this function will
//  confirm/cancel the order assigned to the selected unit.
//  Else in normal mode, it will select the pointed object and return it to InputManager.
// -----------------------------------------------------------------
bool GameboardInputs::onButton1Down(int xPxl, int yPxl)
{
  m_bIsDraggingSelection = false;
  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(CoordsScreen(xPxl, yPxl));
  if (m_MouseMode == ModeSelectCustomTarget)
  {
    m_pCustomTargetOnClickClbk(mapPos);
    return false;
  }
  if (!m_pGameboard->getMap()->isInBounds(mapPos))
  {
    if (m_MouseMode == ModeAttackTarget || m_MouseMode == ModeMoveToTarget)
    {
      setMouseMode(ModeNormal);
      Unit * unit = getActiveSelectedUnit();
      if (unit != NULL)
      {
        m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
        m_pGameboard->updateUnitOrder(unit);
      }
    }
    else
      m_pGameboard->selectMapObject(NULL);
    return false;
  }

  if (m_MouseMode == ModeAttackTarget)
  {
    Unit * unit = getActiveSelectedUnit();
    switch (m_pGameboard->setUnitAttackOrder(unit, mapPos))
    {
    case 0: // invalid attack
      setMouseMode(ModeNormal);
      m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
      m_pGameboard->updateUnitOrder(unit);
      m_pLocalClient->getInterface()->unsetTargetMode();
      break;
    case 1: // valid attack
      setMouseMode(ModeNormal);
      m_pLocalClient->getInterface()->unsetTargetMode();
      break;
    case 2: // valid attack but still need to choose target (mutliple targets)
      // pop StackDlg up
      m_pLocalClient->getInterface()->showStack(m_pGameboard->getMap()->getTileAt(mapPos), GOTYPE_MAPTILE | GOTYPE_MAPOBJECT, GOTYPE_UNIT | GOTYPE_FOE_OBJECT, NULL, NULL);
      guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(m_pLocalClient->getInterface()->getInfoDialog());
      pFrm->flash(1.0f);
      m_pLocalClient->getInterface()->bringFrameAbove(pFrm);
      break;
    }
    return false;
  }

  if (m_MouseMode == ModeMoveToTarget)
  {
    setMouseMode(ModeNormal);
    Unit * unit = getActiveSelectedUnit();
    if (unit != NULL)
    {
      if (!m_pGameboard->isCurrentTargetValid())
      {
        m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
        m_pGameboard->updateUnitOrder(unit);
      }
    }
    return false;
  }

  m_pPreSelectedMapObj = m_pGameboard->getMap()->getFirstObjectAt(mapPos);
  if (m_pPreSelectedMapObj != NULL && (m_pPreSelectedMapObj->getType() & GOTYPE_UNIT))
    m_pGameboard->getMap()->saveCurrentGroupOrder((Unit*)m_pPreSelectedMapObj);
  return (m_pPreSelectedMapObj != NULL);
}

// -----------------------------------------------------------------
// Name : onButton1Drag
//  If the already selected object is not the dragging object,
//  it generates a click on the dragging object in order to select or deselect.
//  Else, if the object is a unit owned by active player, it drags it.
// -----------------------------------------------------------------
bool GameboardInputs::onButton1Drag(int xPxl, int yPxl)
{
  assert(m_pPreSelectedMapObj != NULL);
  if (m_pPreSelectedMapObj != m_pSelectedMapObj)
    m_pGameboard->selectMapObject(m_pPreSelectedMapObj);

  // Get position on map
  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(CoordsScreen(xPxl, yPxl));
  // Select
  u32 type = m_pSelectedMapObj->getType();
  if (type & GOTYPE_UNIT)
  {
    Unit * unit = (Unit*) m_pSelectedMapObj;
    if (m_pLocalClient->getPlayerManager()->isPlayerReady(unit->getOwner()))
    {
      // If we were not already dragging and we're still on the unit's tile, do not calculate anything
      if (!m_bIsDraggingSelection && mapPos == unit->getMapPos())
        return true;
      // If we were already dragging and we're still on the same tile, do not calculate anything again
      if (m_bIsDraggingSelection && m_DragToPosition == mapPos)
        return true;
      // Call setUnitMoveOrder to know if the move is valid
      m_DragToPosition = mapPos;
      m_bIsDraggingSelection = true;
      m_pGameboard->setUnitMoveOrder(unit, mapPos);
      bool bMultiTarget;
      if (NULL != m_pGameboard->getFoeAt(mapPos, unit->getOwner(), &bMultiTarget))
        m_fMoveOrAttackColorTimer = CURSOR_BLINKING_PERIOD;
      else
        m_fMoveOrAttackColorTimer = -1;
    }
  }
  return true;
}

// -----------------------------------------------------------------
// Name : onButton1Up
// -----------------------------------------------------------------
bool GameboardInputs::onButton1Up(int xPxl, int yPxl)
{
  if (!m_bIsDraggingSelection)    // No drag so no drop ; if it's a click, it will generate the click event
    return true;
  m_bIsDraggingSelection = false;

  Unit * unit = getActiveSelectedUnit();
  if (unit != NULL)
  {
    if (!m_pGameboard->isCurrentTargetValid())
    {
      m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
      m_pGameboard->updateUnitOrder(unit);
    }
    else if (m_fMoveOrAttackColorTimer > 0) // Raise "move or attack" popup
    {
      gLocalClient = m_pLocalClient;  // for callbacks
      m_pLocalClient->getInterface()->showMoveOrAttackDialog(unit, m_DragToPosition);
    }
  }
  return false; // We've finished a drag ; don't wait for any subsequent event
}

// -----------------------------------------------------------------
// Name : onButton1Click
// -----------------------------------------------------------------
bool GameboardInputs::onButton1Click(int xPxl, int yPxl)
{
  // Get position on map
  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(CoordsScreen(xPxl, yPxl));
  if (!m_pGameboard->getMap()->isInBounds(mapPos))
    return false;
  // Get first unit owned by active player ; if none, first object
  MapObject * mapObj = m_pGameboard->getMap()->getFirstObjectAt(mapPos);
  while (mapObj != NULL)
  {
    if ((mapObj->getType() & GOTYPE_UNIT) && m_pLocalClient->getPlayerManager()->isPlayerReady(mapObj->getOwner()))
    {
      m_pGameboard->selectMapObject(mapObj);
      return true;
    }
    mapObj = m_pGameboard->getMap()->getNextObjectAt(mapPos);
  }
  // No owned unit ; select first object
  mapObj = m_pGameboard->getMap()->getFirstObjectAt(mapPos);
  m_pGameboard->selectMapObject(mapObj);
  return true;
}

// -----------------------------------------------------------------
// Name : onButton1DoubleClick
// -----------------------------------------------------------------
bool GameboardInputs::onButton1DoubleClick(int xPxl, int yPxl)
{
  // Get position on map
  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(CoordsScreen(xPxl, yPxl));
  if (m_pGameboard->getMap()->isInBounds(mapPos))
  {
    AudioManager::getInstance()->playSound(SOUND_MAP_CLICK);
    m_pLocalClient->getInterface()->showMapObjectDialog(m_pGameboard->getMap()->getTileAt(mapPos));
  }
  return false;
  //Unit * unit = getActiveSelectedUnit();
  //if (unit != NULL)
  //{
  //  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(CoordsScreen(xPxl, yPxl));
  //  if (m_pGameboard->getMap()->isInBounds(mapPos))
  //  {
  //    switch (m_pGameboard->setUnitAttackOrder(unit, mapPos))
  //    {
  //    case 0: // invalid attack
  //      break;
  //    case 1: // valid attack
  //      return false;
  //    case 2: // valid attack but still need to choose target (mutliple targets)
  //      setMouseMode(ModeAttackTarget);
  //      // pop StackDlg up
  //      m_pLocalClient->getInterface()->showStack(m_pGameboard->getMap()->getTileAt(mapPos), GOTYPE_MAPTILE | GOTYPE_MAPOBJECT, GOTYPE_UNIT | GOTYPE_FOE_OBJECT, NULL, NULL);
  //      guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(m_pLocalClient->getInterface()->getStackDialog());
  //      pFrm->flash(1.0f);
  //      m_pLocalClient->getInterface()->bringFrameAbove(pFrm);
  //      return false;
  //    }
  //  }
  //  m_pGameboard->getMap()->retrievePreviousGroupOrder(unit);
  //  m_pGameboard->updateUnitOrder(unit);
  //}
//  return onButton1Click(xPxl, yPxl);
}

// -----------------------------------------------------------------
// Name : onCatchButtonEvent
// -----------------------------------------------------------------
bool GameboardInputs::onCatchButtonEvent(ButtonAction * pEvent)
{
  if (pEvent->eButton == Button2)
  {
    if (pEvent->eEvent == Event_Down)
      return true;
    if (pEvent->eEvent == Event_Drag)
    {
      Coords3D p3d = m_pLocalClient->getDisplay()->get3DDistance(CoordsScreen(pEvent->xPos - pEvent->xPosInit, pEvent->yPos - pEvent->yPosInit, BOARDPLANE), DMS_3D);
      m_pLocalClient->getDisplay()->moveCameraBy(p3d);
      m_pLocalClient->getFx()->dragInertness(p3d);
      return true;
    }
    return false;
  }
  else if (pEvent->eButton == Button1)
  {
    switch (pEvent->eEvent)
    {
    case Event_Down:
      return onButton1Down(pEvent->xPos, pEvent->yPos);
    case Event_Drag:
      return onButton1Drag(pEvent->xPos, pEvent->yPos);
    case Event_Up:
      return onButton1Up(pEvent->xPos, pEvent->yPos);
    case Event_Click:
      return onButton1Click(pEvent->xPos, pEvent->yPos);
    case Event_DoubleClick:
      return onButton1DoubleClick(pEvent->xPos, pEvent->yPos);
    default:
    break;
    }
  }
  else if (pEvent->eButton == ButtonStart && pEvent->eEvent == Event_Down)
  {
    // First see if interface catches then click
    // Else try to end current player turn
    if (!m_pLocalClient->getInterface()->onClickStart())
      m_pLocalClient->getPlayerManager()->requestEndPlayerOrders();
  }
  else if (pEvent->eButton == ButtonBack && pEvent->eEvent == Event_Down)
    m_pLocalClient->getInterface()->showInGameMenu();
  else if (pEvent->eButton == ButtonZ && (pEvent->eEvent == Event_Down || pEvent->eEvent == Event_DoubleClick))
    m_pLocalClient->getFx()->zoom(true);
  else if (pEvent->eButton == ButtonX && (pEvent->eEvent == Event_Down || pEvent->eEvent == Event_DoubleClick))
    m_pLocalClient->getFx()->zoom(false);
  return true;
}

// -----------------------------------------------------------------
// Name : onCursorMoveEvent
//  If MouseMode is ModeMoveToTarget or ModeAttackTarget, get pointer position and update unit order
// -----------------------------------------------------------------
bool GameboardInputs::onCursorMoveEvent(int xPxl, int yPxl)
{
  CoordsScreen pointer = m_pLocalClient->getInput()->getCurrentCursorPosition();
  CoordsMap mapPos = m_pLocalClient->getDisplay()->getMapCoords(pointer);

  // Nothing to do if we're still pointing the same tile
  if (mapPos == m_PreviousPointedTile)
    return true;
  m_PreviousPointedTile = mapPos;

  if (m_MouseMode == ModeMoveToTarget || m_MouseMode == ModeAttackTarget)
  {
    Unit * unit = getActiveSelectedUnit();
    assert(unit != NULL);
    if (m_DragToPosition != mapPos)
    {
      if (m_MouseMode == ModeMoveToTarget)
        m_pGameboard->setUnitMoveOrder(unit, mapPos);
      else
        m_pGameboard->setUnitAttackOrder(unit, mapPos);
      m_DragToPosition = mapPos;
    }
  }
  else if (m_MouseMode == ModeSelectCustomTarget)
  {
    if (m_DragToPosition != mapPos)
    {
      m_pCustomTargetOnMouseOverClbk(mapPos);
      m_DragToPosition = mapPos;
    }
  }

  if (m_pGameboard->getMap()->isInBounds(mapPos))
  {
    // Tile info
    char sText[LABEL_MAX_CHARS] = "";
    MapTile * pTile = m_pGameboard->getMap()->getTileAt(mapPos);
    pTile->getInfo(sText, LABEL_MAX_CHARS);

    // Influence info
    char sInfluence[128] = "";
    Player * pPlayer = m_pLocalClient->getPlayerManager()->findPlayer(pTile->m_uInfluence);
    assert(pPlayer != NULL);
    void * p = pPlayer->getAvatarName();
    i18n->getText("HIGHEST_INFLUENCE_%$1s", sInfluence, 128, &p);
    wsafecat(sText, LABEL_MAX_CHARS, "\n");
    wsafecat(sText, LABEL_MAX_CHARS, sInfluence);

    // Magic circles info
    char sMagicCircle[128] = "";
    ObjectList * pMagicCircles = m_pGameboard->getMagicCircles();
    CoordsObject * pCoords = (CoordsObject*) pMagicCircles->getFirst(0);
    while (pCoords != NULL)
    {
      if (pCoords->getCoordsMap() == mapPos)
      {
        Player * pPlayer = (Player*) (pCoords->getAttachment());
        assert(pPlayer != NULL);
        p = pPlayer->getAvatarName();
        i18n->getText("MAGIC_CIRCLE_(%$1s)", sMagicCircle, 128, &p);
        wsafecat(sText, LABEL_MAX_CHARS, "\n");
        wsafecat(sText, LABEL_MAX_CHARS, sMagicCircle);
      }
      pCoords = (CoordsObject*) pMagicCircles->getNext(0);
    }

    // Objects infos
    int iObjects = m_pGameboard->getMap()->getObjectsCountAt(mapPos);
    if (iObjects == 1)
    {
      MapObject * mapObj = m_pGameboard->getMap()->getFirstObjectAt(mapPos);
      assert(mapObj != NULL);
      char sBuf1[LABEL_MAX_CHARS] = "";
      wsafecat(sText, LABEL_MAX_CHARS, "\n");
      wsafecat(sText, LABEL_MAX_CHARS, mapObj->getInfo(sBuf1, LABEL_MAX_CHARS, Dest_InfoDialog));
    }
    else
    {
      MapObject * mapObj = m_pGameboard->getMap()->getFirstObjectAt(mapPos);
      while (mapObj != NULL)
      {
        char sBuf1[LABEL_MAX_CHARS] = "";
        wsafecat(sText, LABEL_MAX_CHARS, "\n");
        wsafecat(sText, LABEL_MAX_CHARS, mapObj->getInfo(sBuf1, LABEL_MAX_CHARS, Dest_ShortInfoDialog));
        mapObj = m_pGameboard->getMap()->getNextObjectAt(mapPos);
      }
    }
    m_pLocalClient->getInterface()->getInfoDialog()->setInfoText(sText);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : getActiveSelectedUnit
// -----------------------------------------------------------------
Unit * GameboardInputs::getActiveSelectedUnit()
{
  Unit * pActiveUnit = NULL;
  if (m_pSelectedMapObj != NULL)
  {
    if (m_pSelectedMapObj->getType() & GOTYPE_UNIT)
    {
      pActiveUnit = (Unit*)m_pSelectedMapObj;
      if (! m_pLocalClient->getPlayerManager()->isPlayerReady(pActiveUnit->getOwner()))
        pActiveUnit = NULL;
    }
  }
  return pActiveUnit;
}

// -----------------------------------------------------------------
// Name : onMoveOrAttackResponse
// -----------------------------------------------------------------
void GameboardInputs::onMoveOrAttackResponse(bool bCancel, Unit * pTarget)
{
  setMouseMode(ModeNormal);
  Unit * unit = getActiveSelectedUnit();
  if (unit != NULL)
  {
    if (bCancel)
    {
      m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
      m_pGameboard->updateUnitOrder(unit);
    }
    else if (pTarget != NULL)  // attack target
    {
      if (!m_pGameboard->setCurrentUnitAttackOrder(pTarget))
      {
        m_pGameboard->getMap()->retrievePreviousGroupOrder(unit, m_pLocalClient->getInterface()->getUnitOptionsDialog());
        m_pGameboard->updateUnitOrder(unit);
      }
    }
    else // move to position
      m_pGameboard->updateUnitOrder(unit);
  }
  m_fMoveOrAttackColorTimer = -1;
  m_pLocalClient->getInterface()->hideMoveOrAttackDialog();
  m_pLocalClient->getInterface()->unsetTargetMode();
}


// callbacks
bool clbkAttackTarget_OnMouseOverInterface(BaseObject * pObj, u8 isLuaPlayerGO)
{
  if (isLuaPlayerGO != 3)
    return false;
  return (((GraphicObject*)pObj)->getType() & GOTYPE_UNIT) && gLocalClient->getGameboard()->setCurrentUnitAttackOrder((Unit*)pObj);
}

bool clbkAttackTarget_OnClickInterface(BaseObject * pObj, u8 isLuaPlayerGO)
{
  if (pObj == NULL || isLuaPlayerGO != 3)
    return false;
  gLocalClient->getGameboard()->getInputs()->setMouseMode(ModeNormal);
  Unit * unit = gLocalClient->getGameboard()->getInputs()->getActiveSelectedUnit();
  if (unit != NULL)
  {
    if (!((((GraphicObject*)pObj)->getType() & GOTYPE_UNIT) && gLocalClient->getGameboard()->setCurrentUnitAttackOrder((Unit*)pObj)))
    {
      gLocalClient->getGameboard()->getMap()->retrievePreviousGroupOrder(unit, gLocalClient->getInterface()->getUnitOptionsDialog());
      gLocalClient->getGameboard()->updateUnitOrder(unit);
    }
  }
  gLocalClient->getInterface()->unsetTargetMode();
  return true;
}

bool clbkMoveOrAttack_OnMouseOverInterface(BaseObject * pObj, u8 isLua)
{
  return true;
}

bool clbkMoveOrAttack_OnClickInterface(BaseObject * pObj, u8 isLua)
{
  if (isLua == 3)
    gLocalClient->getGameboard()->getInputs()->onMoveOrAttackResponse(pObj == NULL, (pObj != NULL && (((GraphicObject*)pObj)->getType() & GOTYPE_UNIT)) ? (Unit*)pObj : NULL);
  return true;
}
