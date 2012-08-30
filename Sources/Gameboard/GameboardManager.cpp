// -----------------------------------------------------------------
// GAMEBOARD MANAGER
// -----------------------------------------------------------------
#include "GameboardManager.h"
#include "../LocalClient.h"
#include "../Geometries/ModProgressiveScaling.h"
#include "../Geometries/ModProgressiveBlending.h"
#include "Unit.h"
#include "Town.h"
#include "GameboardInputs.h"
#include "../Geometries/GeometryQuads.h"
#include "../Geometries/GeometryText.h"
#include "../Interface/InterfaceManager.h"
#include "../Interface/ResolveDlg.h"
#include "../Debug/DebugManager.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Audio/AudioManager.h"

// -----------------------------------------------------------------
// Name : GameboardManager
// -----------------------------------------------------------------
GameboardManager::GameboardManager(LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_pInputs = new GameboardInputs(this, pLocalClient);
  m_pMagicCircleGeometry = m_pBattleIcon = m_pPathDot = NULL;
  m_Mode = GM_Orders;
  m_pBackgroundGeometry = NULL;
  m_pTerraIncognitaGeometry = NULL;
  m_pMagicCircles = new ObjectList(true);
  m_pHighlightedPlayerCircles = NULL;
}

// -----------------------------------------------------------------
// Name : ~GameboardManager
// -----------------------------------------------------------------
GameboardManager::~GameboardManager()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy GameboardManager\n");
#endif
  FREE(m_pPathDot);
  FREE(m_pBattleIcon);
  FREE(m_pMagicCircleGeometry);
  FREE(m_pBackgroundGeometry);
  FREE(m_pTerraIncognitaGeometry);
  FREE(m_pInputs);
  delete m_pMagicCircles;
#ifdef DBG_VERBOSE1
  printf("End destroy GameboardManager\n");
#endif
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void GameboardManager::Init()
{
  DisplayEngine * pDisplay = m_pLocalClient->getDisplay();
  m_Map.initGraphics(pDisplay);
  m_pInputs->init();

  // Init map cursors
  for (int i = 0; i < NB_MAPCURSORS; i++)
    m_AllCursors[i].init(m_pLocalClient->getDisplay());
  m_AllCursors[MAPCURSOR_SELECTION].setColor(rgb(0.4f, 0.7f, 1.0f));
  m_AllCursors[MAPCURSOR_MOVETURNPOS].setColor(rgb(1.0f, 1.0f, 0.0f));
  m_AllCursors[MAPCURSOR_SPECIALTARGET].setColor(rgb(0.9f, 0.0f, 0.9f));
  m_AllCursors[MAPCURSOR_WRONGACTION].setColor(rgb(0.5f, 0.5f, 0.5f));

  // Prepare other graphics
  // texture must be loaded with special parameters (wrap: repeat), so load it manually
  int iTex = pDisplay->getTextureEngine()->loadTexture(L"darker_map");
  QuadData bgquad(0, m_pLocalClient->getClientParameters()->screenXSize, 0, m_pLocalClient->getClientParameters()->screenYSize, iTex, pDisplay);
  // modify quad so that texture is repeated
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(iTex);
  bgquad.m_fUEnd = ((double)m_pLocalClient->getClientParameters()->screenXSize) / ((double)pTex->m_iWidth);
  bgquad.m_fVEnd = ((double)m_pLocalClient->getClientParameters()->screenYSize) / ((double)pTex->m_iHeight);
  m_pBackgroundGeometry = new GeometryQuads(&bgquad, VB_Static);
  QuadData quad0(0, 170, 0, 86, L"terra_incognita", pDisplay);
  m_pTerraIncognitaGeometry = new GeometryQuads(&quad0, VB_Static);
  QuadData quad1(0.0f, 0.2f, 0.0f, 0.2f, L"path_dot", pDisplay);
  m_pPathDot = new GeometryQuads(&quad1, VB_Static);
  QuadData quad2(0.0f, 2.0f, 0.0f, 2.0f, L"battle", pDisplay);
  m_pBattleIcon = new GeometryQuads(&quad2, VB_Static);
  m_pBattleIcon->bindModifier(new ModProgressiveScaling(0, 0.8f, 1.0f, 1.0f, -0.3f, 0.5f, 0.5f, PSB_ForthAndBack));
  QuadData quad3(0.0f, 1.0f, 0.0f, 1.0f, L"magic_circle", pDisplay);
  m_pMagicCircleGeometry = new GeometryQuads(&quad3, VB_Static);
  GeometryModifier * pMod = new ModProgressiveScaling(0, 0.8f, 1.0f, 1.0f, -0.3f, 0.5f, 0.5f, PSB_ForthAndBack);
  pMod->setActive(false);
  m_pMagicCircleGeometry->bindModifier(pMod);
  m_pMagicCircleGeometry->bindModifier(new ModProgressiveBlending(1, rgba(1, 1, 1, 1), rgba(0.5f, 0.5f, 0.5f, 0.5f), 2.0f));
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void GameboardManager::Update(double delta)
{
  m_Map.update(delta);
  m_pInputs->update(delta);

  // Update map cursors
  for (int i = 0; i < NB_MAPCURSORS; i++)
    m_AllCursors[i].update(delta);

  if (m_Mode == GM_SelectBattle)
    m_pBattleIcon->update(delta);

  m_pMagicCircleGeometry->update(delta);
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void GameboardManager::Display()
{
  m_Map.display();

  // Magic circles
  CoordsObject * pCoords = (CoordsObject*) m_pMagicCircles->getFirst(0);
  while (pCoords != NULL)
  {
    Coords3D pos3d = m_pLocalClient->getDisplay()->get3DCoords(pCoords->getCoordsMap(), BOARDPLANE-0.006f);
    Player * pPlayer = (Player*) (pCoords->getAttachment());
    assert(pPlayer != NULL);
    if (m_pHighlightedPlayerCircles == NULL || m_pHighlightedPlayerCircles == pPlayer)
      m_pMagicCircleGeometry->display(pos3d, pPlayer->m_Color);
    pCoords = (CoordsObject*) m_pMagicCircles->getNext(0);
  }

  // Display map cursors
  for (int i = 0; i < NB_MAPCURSORS; i++)
    m_AllCursors[i].display();

  m_Map.displayObjects(m_pLocalClient->getPlayerManager());

  if (m_Mode == GM_SelectBattle)
  {
//    m_pLocalClient->getDisplay()->enableBlending();
    for (int i = 0; i < m_pLocalClient->getInterface()->getResolveDialog()->getNumberOfBattles(); i++)
    {
      Coords3D pos3d = m_pLocalClient->getDisplay()->get3DCoords(m_pLocalClient->getInterface()->getResolveDialog()->getBattlePosition(i), BOARDPLANE-0.006f) - Coords3D(0.5f, 0.5f, 0.0f);
      m_pBattleIcon->display(pos3d, rgba(1.0f, 1.0f, 1.0f, 0.7f));
    }
//    m_pLocalClient->getDisplay()->disableBlending();
  }
  else
  {
    Unit * unit = m_pInputs->getActiveSelectedUnit();
    if (unit != NULL)
    {
      if (m_AllCursors[MAPCURSOR_MOVETARGET].isEnabled())
      {
        ObjectList * pPath = unit->getPath();
        if (pPath->size > 2)
        {
          int i = 1;
          Unit::AStarStep * pStep = (Unit::AStarStep*) pPath->goTo(0, i);
          do {
//            Coords3D pos3d = m_pLocalClient->getDisplay()->get3DCoords(pStep->pos, FARPLANE-EPSILON) + Coords3D(0.4f, 0.4f, 0.0f);
            Coords3D pos3d = m_pLocalClient->getDisplay()->get3DCoords(pStep->pos, BOARDPLANE) + Coords3D(0.4f, 0.4f, 0.0f);
            m_pPathDot->display(pos3d, F_RGBA_NULL);
            pStep = (Unit::AStarStep*) pPath->getNext(0);
            i++;
          } while (i < pPath->size - 1);
        }
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : displayBackground
// -----------------------------------------------------------------
void GameboardManager::displayBackground()
{
  //CoordsScreen coords = CoordsScreen(0, 0, FARPLANE);
  //m_pBackgroundGeometry->display(coords, F_RGBA_NULL);
  //int width = m_pLocalClient->getClientParameters()->screenXSize;
  //int height = m_pLocalClient->getClientParameters()->screenYSize;
  //int texwidth = m_pLocalClient->getDisplay()->getTextureEngine()->getTexture(m_pTerraIncognitaGeometry->getTexture())->m_iWidth;
  //int texheight = m_pLocalClient->getDisplay()->getTextureEngine()->getTexture(m_pTerraIncognitaGeometry->getTexture())->m_iHeight;
  //coords.x = 0;
  //coords.y = height / 2 - texheight / 2;
  //m_pTerraIncognitaGeometry->display(coords, F_RGBA_NULL);
  //coords.x = width - texwidth;
  //m_pTerraIncognitaGeometry->display(coords, F_RGBA_NULL);
  //coords.x = width / 2 - texwidth / 2;
  //coords.y = 0;
  //m_pTerraIncognitaGeometry->display(coords, F_RGBA_NULL);
  //coords.y = height - texheight;
  //m_pTerraIncognitaGeometry->display(coords, F_RGBA_NULL);
}

// -----------------------------------------------------------------
// Name : getFoeAt
// -----------------------------------------------------------------
Unit * GameboardManager::getFoeAt(CoordsMap position, u8 unitOwnerId, bool * bMultipleTargets)
{
  *bMultipleTargets = false;
  if (!m_Map.isInBounds(position))
    return NULL;
  // check if multiple targets
  Unit * pTarget = NULL;
  MapObject * mapObj = m_Map.getFirstObjectAt(position);
  while (mapObj != NULL)
  {
    if ((mapObj->getType() & GOTYPE_UNIT) && mapObj->getOwner() != unitOwnerId)
    {
      if (pTarget != NULL)
      {
        *bMultipleTargets = true;
        return pTarget;
      }
      else
        pTarget = (Unit*) mapObj;
    }
    mapObj = m_Map.getNextObjectAt(position);
  }
  return pTarget;
}

// -----------------------------------------------------------------
// Name : selectMapObject
// -----------------------------------------------------------------
void GameboardManager::selectMapObject(MapObject * mapObj, bool bUpdateStack)
{
  AudioManager::getInstance()->playSound(SOUND_CLICK);
  m_pInputs->setSelectedMapObject(mapObj);
  if (mapObj != NULL)
  {
    m_Map.bringAbove(mapObj);
    m_AllCursors[MAPCURSOR_SELECTION].setEnabled(true);
    m_AllCursors[MAPCURSOR_SELECTION].moveTo(mapObj->getMapPos());
    if (bUpdateStack)
      m_pLocalClient->getInterface()->showStack(m_Map.getTileAt(mapObj->getMapPos()), GOTYPE_MAPTILE | GOTYPE_MAPOBJECT, GOTYPE_UNIT | GOTYPE_OWNED_OBJECT, &m_Map, mapObj);
  }
  else
  {
    m_AllCursors[MAPCURSOR_SELECTION].setEnabled(false);
  }

  Unit * pUnit = m_pInputs->getActiveSelectedUnit();
  m_pLocalClient->getInterface()->updateUnitOptionsDialog(pUnit);
  updateUnitTargetCursors(pUnit);
}

// -----------------------------------------------------------------
// Name : setUnitMoveOrder
//  returns true if the move is valid
// -----------------------------------------------------------------
bool GameboardManager::setUnitMoveOrder(Unit * pUnit, CoordsMap mapPos)
{
  bool bMoveValid = getMap()->setMoveGroupOrder(pUnit, mapPos, m_pLocalClient->getInterface()->getUnitOptionsDialog());
  m_pLocalClient->getInterface()->updateUnitOptionsDialog(pUnit);
  if (bMoveValid)
  {
    updateUnitTargetCursors(pUnit);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(false);
  }
  else
  {
    updateUnitTargetCursors(NULL);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(true);
    m_AllCursors[MAPCURSOR_WRONGACTION].moveTo(mapPos);
  }
  return bMoveValid;
}

// -----------------------------------------------------------------
// Name : setUnitAttackOrder
//  return 0 is not valid, 1 if valid, 2 if valid but multiple possible targets
// -----------------------------------------------------------------
char GameboardManager::setUnitAttackOrder(Unit * pUnit, CoordsMap mapPos)
{
  bool bMultiTarget;
  Unit * pTarget = getFoeAt(mapPos, pUnit->getOwner(), &bMultiTarget);
  char iMoveValid = getMap()->setAttackGroupOrder(pUnit, pTarget, m_pLocalClient->getInterface()->getUnitOptionsDialog()) ? 1 : 0;
  m_pLocalClient->getInterface()->updateUnitOptionsDialog(pUnit);
  if (iMoveValid)
  {
    updateUnitTargetCursors(pUnit);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(false);
    // is there several possible targets on that tile?
    if (bMultiTarget)
      iMoveValid = 2;
  }
  else
  {
    updateUnitTargetCursors(NULL);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(true);
    m_AllCursors[MAPCURSOR_WRONGACTION].moveTo(mapPos);
  }
  return iMoveValid;
}

// -----------------------------------------------------------------
// Name : setCurrentUnitAttackOrder
// -----------------------------------------------------------------
bool GameboardManager::setCurrentUnitAttackOrder(Unit * pTarget)
{
  Unit * pUnit = m_pInputs->getActiveSelectedUnit();
  assert(pUnit != NULL);
  bool bMoveValid = pTarget->getOwner() != pUnit->getOwner() && getMap()->setAttackGroupOrder(pUnit, pTarget, m_pLocalClient->getInterface()->getUnitOptionsDialog());
  m_pLocalClient->getInterface()->updateUnitOptionsDialog(pUnit);
  if (bMoveValid)
  {
    updateUnitTargetCursors(pUnit);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(false);
  }
  else
  {
    updateUnitTargetCursors(NULL);
    m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(true);
    m_AllCursors[MAPCURSOR_WRONGACTION].moveTo(pTarget->getMapPos());
  }
  return bMoveValid;
}

// -----------------------------------------------------------------
// Name : updateUnitOrder
// -----------------------------------------------------------------
void GameboardManager::updateUnitOrder(Unit * pUnit)
{
  updateUnitTargetCursors(pUnit);
  m_pLocalClient->getInterface()->updateUnitOptionsDialog(pUnit);
  m_AllCursors[MAPCURSOR_WRONGACTION].setEnabled(false);
}

// -----------------------------------------------------------------
// Name : updateUnitTargetCursors
// -----------------------------------------------------------------
void GameboardManager::updateUnitTargetCursors(Unit * pUnit)
{
  if (pUnit == NULL || pUnit->getPath()->size == 0)
  {
    m_AllCursors[MAPCURSOR_MOVETARGET].setEnabled(false);
    m_AllCursors[MAPCURSOR_MOVETURNPOS].setEnabled(false);
    return;
  }
  UnitOrder order = pUnit->getOrder();
  if (order == OrderMove || order == OrderAttack)
  {
    CoordsMap mpDest = pUnit->getDestination();
    CoordsMap mpTurn = pUnit->getPathTurnPosition();
    m_AllCursors[MAPCURSOR_MOVETARGET].setEnabled(true);
    m_AllCursors[MAPCURSOR_MOVETARGET].moveTo(mpDest);
    m_AllCursors[MAPCURSOR_MOVETARGET].setColor((order == OrderMove) ? rgb(0.0f ,1.0f, 0.0f) : rgb(1.0f ,0.0f, 0.0f));
    if (mpDest != mpTurn)
    {
      m_AllCursors[MAPCURSOR_MOVETURNPOS].setEnabled(true);
      m_AllCursors[MAPCURSOR_MOVETURNPOS].moveTo(mpTurn);
    }
    else
      m_AllCursors[MAPCURSOR_MOVETURNPOS].setEnabled(false);
  }
  else
  {
    m_AllCursors[MAPCURSOR_MOVETARGET].setEnabled(false);
    m_AllCursors[MAPCURSOR_MOVETURNPOS].setEnabled(false);
  }
}

// -----------------------------------------------------------------
// Name : disableNoPlayer
// -----------------------------------------------------------------
void GameboardManager::disableNoPlayer()
{
  m_pInputs->setSelectedMapObject(NULL);
  for (int i = 0; i < NB_MAPCURSORS; i++)
    m_AllCursors[i].setEnabled(false);
}

// -----------------------------------------------------------------
// Name : enableNextPlayer
// -----------------------------------------------------------------
void GameboardManager::enableNextPlayer(Player * pPlayer)
{
  // Loop through all map tiles and check number of allies and foes on each tile
  for (int x = 0; x < getMap()->getWidth(); x++)
  {
    for (int y = 0; y < getMap()->getHeight(); y++)
    {
      MapTile * pTile = getMap()->getTileAt(CoordsMap(x, y));
      FREE(pTile->m_pNbAlliesGeo);
      FREE(pTile->m_pNbFoesGeo);
      int allies = 0;
      int foes = 0;
      MapObject * pMapObj = pTile->getFirstMapObject(GOTYPE_UNIT);
      while (pMapObj != NULL)
      {
        if (pMapObj->getOwner() == pPlayer->m_uPlayerId)
          allies++;
        else
          foes++;
        pMapObj = pTile->getNextMapObject(GOTYPE_UNIT);
      }
      wchar_t sNb[8];
      int iFont = m_pLocalClient->getDisplay()->getFontEngine()->registerFont(L"Arabolical_32", m_pLocalClient->getDisplay()->getTextureEngine());
      if (allies > 1)
      {
        swprintf_s(sNb, 8, L"%d", allies);
        pTile->m_pNbAlliesGeo = new GeometryText(sNb, iFont, 0.3f, VB_Static, m_pLocalClient->getDisplay());
      }
      if (foes > 0 && allies + foes > 1)
      {
        swprintf_s(sNb, 8, L"%d", foes);
        pTile->m_pNbFoesGeo = new GeometryText(sNb, iFont, 0.3f, VB_Static, m_pLocalClient->getDisplay());
      }
      // Again, loop into map objects to put owned units first
      pMapObj = (MapObject*) pTile->m_pMapObjects->getLast(0);
      MapObject * pFirstMoved = NULL;
      while (pMapObj != NULL)
      {
        if ((pMapObj->getType() & GOTYPE_UNIT) && pMapObj->getOwner() == pPlayer->m_uPlayerId)
        {
          if (pFirstMoved == pMapObj)
            break;
          if (pFirstMoved == NULL)
            pFirstMoved = pMapObj;
          pTile->m_pMapObjects->moveCurrentToBegin(0);
          pMapObj = (MapObject*) pTile->m_pMapObjects->getLast(0);
        }
        pMapObj = (MapObject*) pTile->m_pMapObjects->getPrev(0);
      }
    }
  }
}

// -----------------------------------------------------------------
// Name : setBattleMode
// -----------------------------------------------------------------
void GameboardManager::setBattleMode()
{
  m_Mode = GM_SelectBattle;
}

// -----------------------------------------------------------------
// Name : unsetBattleMode
// -----------------------------------------------------------------
void GameboardManager::unsetBattleMode()
{
  m_Mode = GM_Orders;
}

// -----------------------------------------------------------------
// Name : updateTownsData
// -----------------------------------------------------------------
void GameboardManager::updateTownsData(NetworkData * pData)
{
  while (pData->dataYetToRead() > 0)
  {
    u32 uTownId = (u32) pData->readLong();
    Town * pTown = m_Map.findTown(uTownId);
    if (pTown == NULL)
      m_pLocalClient->getDebug()->notifyErrorMessage(L"Error at updateTownsData: town id not found.");
    else
      pTown->deserializeForUpdate(pData);
  }
}

// -----------------------------------------------------------------
// Name : updateTilesInfluence
// -----------------------------------------------------------------
void GameboardManager::updateTilesInfluence(NetworkData * pData)
{
  for (int x = 0; x < getMap()->getWidth(); x++)
  {
    for (int y = 0; y < getMap()->getHeight(); y++)
    {
      MapTile * pTile = getMap()->getTileAt(CoordsMap(x, y));
      pTile->m_uInfluence = (u8) pData->readLong();
    }
  }
}

// -----------------------------------------------------------------
// Name : unsetTargetMode
// -----------------------------------------------------------------
void GameboardManager::unsetTargetMode()
{
  getMapCursor(MAPCURSOR_WRONGACTION)->setEnabled(false);
  getMapCursor(MAPCURSOR_SPECIALTARGET)->setEnabled(false);
  getInputs()->setMouseMode(ModeNormal);
  m_pLocalClient->getInterface()->unsetTargetMode();
  highlightMagicCirclesForPlayer(NULL);
}

// -----------------------------------------------------------------
// Name : highlightMagicCirclesForPlayer
// -----------------------------------------------------------------
void GameboardManager::highlightMagicCirclesForPlayer(Player * pPlayer)
{
  m_pHighlightedPlayerCircles = pPlayer;
  GeometryModifier * pMod = m_pMagicCircleGeometry->getModifier(0);
  assert(pMod != NULL);
  if (pPlayer != NULL)
    pMod->setActive(true);
  else
    pMod->setActive(false);
}
