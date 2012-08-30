#include "Town.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Ethnicity.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../Server/TurnSolver.h"
#include "../Server/Server.h"
#include "../Players/Player.h"
#include "../Players/PlayerManager.h"
#include "../Interface/LogDlg.h"
#include "../GUIClasses/guiLabel.h"
#include "../LocalClient.h"
#include "Building.h"
#include "Unit.h"

#define BIG_PICTURE_WIDTH   700
#define BIG_PICTURE_HEIGHT  250

// -----------------------------------------------------------------
// Name : Town
//  Constructor
// -----------------------------------------------------------------
Town::Town(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects) : MapObject(mapPos, pMap, pGlobalEffects, L"")
{
  m_pLocalClient = NULL;
  m_pCurrentBuildingUnit = NULL;
  m_pBuildings = new ObjectList(true);
  m_pExtraBuildableUnits = new ObjectList(true);
  m_pHeroes = new ObjectList(false);
  wsafecpy(m_sCurrentBuilding, NAME_MAX_CHARS, L"");
  registerValue(STRING_GROWTH, 0);
  registerValue(STRING_PRODUCTIVITY, 0);
  registerValue(STRING_UNITPROD, 0);
  registerValue(STRING_HAPPINESS, 0);
  registerValue(STRING_FEAR, 0);
  registerValue(STRING_RADIUS, 0);
  registerValue(STRING_HEROECHANCES, 0);

  m_uTownId = 0;
  m_iFoodPerTurn = 0;
  m_uFoodInStock = 0;
  m_uProdPerTurn = 0;
  m_uProdInStock = 0;
  m_uUnitProdInStock = 0;
  m_uSize = 0;
}

// -----------------------------------------------------------------
// Name : ~Town
//  Destructor
// -----------------------------------------------------------------
Town::~Town()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Town\n");
#endif
  delete m_pBuildings;
  delete m_pExtraBuildableUnits;
  FREE(m_pCurrentBuildingUnit);
  delete m_pHeroes;
#ifdef DBG_VERBOSE1
  printf("End destroy Town\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Town::init(u32 uTownId, u8 uSize, Ethnicity * pEthn, LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_uTownId = uTownId;
  wchar_t * pName = pEthn->getRandomTownName();
  if (pName != NULL)
    wsafecpy(m_sName, NAME_MAX_CHARS, pName);
  else
    wsafecpy(m_sName, NAME_MAX_CHARS, L"NULL");
  wsafecpy(m_sEthnicityEdition, NAME_MAX_CHARS, pEthn->m_sEdition);
  wsafecpy(m_sEthnicityId, NAME_MAX_CHARS, pEthn->m_sObjectId);
  for (int i = 0; i < 5; i++)
    wsafecpy(m_sTextures[i], MAX_PATH, pEthn->m_sTownTextures[i]);
  wsafecpy(m_sBigPicture, MAX_PATH, pEthn->m_sTownBigPict);
  m_uSize = uSize;
  setBaseValue(STRING_GROWTH, pEthn->m_uTownsGrowth);
  setBaseValue(STRING_PRODUCTIVITY, pEthn->m_uTownsProductivity);
  setBaseValue(STRING_UNITPROD, pEthn->m_uTownsUnitProd);
  setBaseValue(STRING_HAPPINESS, pEthn->m_iTownsHappinessBonus);
  setBaseValue(STRING_FEAR, max(0, pEthn->m_iTownsFearBonus));
  setBaseValue(STRING_RADIUS, m_uSize + pEthn->m_iTownsRadiusBonus);
  setBaseValue(STRING_HEROECHANCES, 0);
  m_iFoodPerTurn = 0;
  m_uFoodInStock = 0;
  m_uProdPerTurn = 0;
  m_uProdInStock = 0;
  m_uUnitProdInStock = 0;
  m_pCurrentBuildingUnit = NULL;

  // Buildings
  wsafecpy(m_sCurrentBuilding, NAME_MAX_CHARS, L"");
  Ethnicity::BuildingFile * pBuildFile = (Ethnicity::BuildingFile*) pEthn->m_pBuildingFiles->getFirst(0);
  while (pBuildFile != NULL)
  {
    Building * pBuilding = new Building(pBuildFile->m_iX, pBuildFile->m_iY, m_sEthnicityEdition, pBuildFile->m_sId, pLocalClient->getDebug());
    m_pBuildings->addLast(pBuilding);
    pBuildFile = (Ethnicity::BuildingFile*) pEthn->m_pBuildingFiles->getNext(0);
  }

  updateIdentifiers();
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Town::serialize(NetworkData * pData)
{
  pData->addLong(m_uTownId);
  pData->addString(m_sName);
  pData->addString(m_sEthnicityEdition);
  pData->addString(m_sEthnicityId);
  pData->addLong(m_uOwner);
  pData->addLong(m_uSize);
  LuaTargetable::serializeValues(pData);
  pData->addLong(m_iFoodPerTurn);
  pData->addLong(m_uFoodInStock);
  pData->addLong(m_uProdPerTurn);
  pData->addLong(m_uProdInStock);
  pData->addLong(m_uUnitProdInStock);
  CoordsMap pos = getMapPos();
  pData->addLong(pos.x);
  pData->addLong(pos.y);

  // Buildings
  pData->addLong(m_pBuildings->size);
  Building * pBuild = (Building*) m_pBuildings->getFirst(0);
  while (pBuild != NULL)
  {
    pData->addLong(pBuild->getInstanceId());
    pData->addLong(pBuild->getXPos());
    pData->addLong(pBuild->getYPos());
    pData->addString(pBuild->getObjectName());
    pData->addLong(pBuild->isBuilt() ? 1 : 0);
    pBuild = (Building*) m_pBuildings->getNext(0);
  }
  pData->addString(m_sCurrentBuilding);

  //LuaTargetable::serializeEffects(pData, getAllEffects());
  //LuaTargetable::serializeEffects(pData, getDisabledEffects());

  // Unit being produced
  if (m_pCurrentBuildingUnit != NULL)
  {
    pData->addLong(1);
    pData->addString(m_pCurrentBuildingUnit->m_sId);
    pData->addLong(m_pCurrentBuildingUnit->m_uCost);
  }
  else
    pData->addLong(0);

  // Note: no need to serialize m_pHeroes because it's server-only list, no matters if the client doesn't see it
}

// -----------------------------------------------------------------
// Name : serializeForUpdate
// -----------------------------------------------------------------
void Town::serializeForUpdate(NetworkData * pData)
{
  pData->addString(m_sName);
  pData->addString(m_sEthnicityEdition);
  pData->addString(m_sEthnicityId);
  pData->addLong(m_uOwner);
  pData->addLong(m_uSize);
  LuaTargetable::serializeValues(pData);
  pData->addLong(m_iFoodPerTurn);
  pData->addLong(m_uFoodInStock);
  pData->addLong(m_uProdPerTurn);
  pData->addLong(m_uProdInStock);
  pData->addLong(m_uUnitProdInStock);
  CoordsMap pos = getMapPos();
  pData->addLong(pos.x);
  pData->addLong(pos.y);

  pData->addString(m_sCurrentBuilding);

  // Unit being produced
  if (m_pCurrentBuildingUnit != NULL)
  {
    pData->addLong(1);
    pData->addString(m_pCurrentBuildingUnit->m_sId);
    pData->addLong(m_pCurrentBuildingUnit->m_uCost);
  }
  else
    pData->addLong(0);

  // Note: no need to serialize m_pHeroes because it's server-only list, no matters if the client doesn't see it
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void Town::deserialize(NetworkData * pData, LocalClient * pLocalClient)
{
  m_pLocalClient = pLocalClient;
  m_uTownId = pData->readLong();
  pData->readString(m_sName);
  pData->readString(m_sEthnicityEdition);
  pData->readString(m_sEthnicityId);
  m_uOwner = pData->readLong();
  m_uSize = pData->readLong();
  LuaTargetable::deserializeValues(pData);
  m_iFoodPerTurn = pData->readLong();
  m_uFoodInStock = pData->readLong();
  m_uProdPerTurn = pData->readLong();
  m_uProdInStock = pData->readLong();
  m_uUnitProdInStock = pData->readLong();
  CoordsMap pos;
  pos.x = pData->readLong();
  pos.y = pData->readLong();
  setMapPos(pos);

  // Buildings
  m_pBuildings->deleteAll();
  long listsize = pData->readLong();
  while (listsize > 0)
  {
    u32 id = (u32) pData->readLong();
    int iX = (int) pData->readLong();
    int iY = (int) pData->readLong();
    wchar_t sName[NAME_MAX_CHARS];
    pData->readString(sName);
    Building * pBuilding = new Building(id, iX, iY, m_sEthnicityEdition, sName, m_pLocalClient->getDebug());
    bool bBuilt = (pData->readLong() == 1);
    pBuilding->setBuilt(bBuilt);
    m_pBuildings->addLast(pBuilding);
    listsize--;
  }
  pData->readString(m_sCurrentBuilding);

  // Unit being produced
  FREE(m_pCurrentBuildingUnit);
  listsize = pData->readLong();
  if (listsize != 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pData->readString(sName);
    u8 cost = (u8) pData->readLong();
    m_pCurrentBuildingUnit = new Ethnicity::TownUnit(sName, cost);
  }

  updateIdentifiers();
  // Set textures
  Edition * pEdition = pLocalClient->getDataFactory()->findEdition(m_sEthnicityEdition);
  assert(pEdition != NULL);
  Ethnicity * pEthn = pEdition->findEthnicity(m_sEthnicityId);
  assert(pEthn != NULL);
  for (int i = 0; i < 5; i++)
    wsafecpy(m_sTextures[i], MAX_PATH, pEthn->m_sTownTextures[i]);
  wsafecpy(m_sBigPicture, MAX_PATH, pEthn->m_sTownBigPict);
}

// -----------------------------------------------------------------
// Name : deserializeForUpdate
// -----------------------------------------------------------------
void Town::deserializeForUpdate(NetworkData * pData)
{
  pData->readString(m_sName);
  pData->readString(m_sEthnicityEdition);
  pData->readString(m_sEthnicityId);
  m_uOwner = pData->readLong();
  m_uSize = pData->readLong();
  LuaTargetable::deserializeValues(pData);
  m_iFoodPerTurn = pData->readLong();
  m_uFoodInStock = pData->readLong();
  m_uProdPerTurn = pData->readLong();
  m_uProdInStock = pData->readLong();
  m_uUnitProdInStock = pData->readLong();
  CoordsMap pos;
  pos.x = pData->readLong();
  pos.y = pData->readLong();
  setMapPos(pos);

  pData->readString(m_sCurrentBuilding);

  // Unit being produced
  FREE(m_pCurrentBuildingUnit);
  long listsize = pData->readLong();
  if (listsize != 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pData->readString(sName);
    u8 cost = (u8) pData->readLong();
    m_pCurrentBuildingUnit = new Ethnicity::TownUnit(sName, cost);
  }
}

// -----------------------------------------------------------------
// Name : getOrders
// -----------------------------------------------------------------
void Town::getOrders(NetworkData * pData)
{
  pData->addLong(m_uTownId);
  // Buildings
  pData->addString(m_sCurrentBuilding);

  // Unit being produced
  if (m_pCurrentBuildingUnit != NULL)
  {
    pData->addLong(1);
    pData->addString(m_pCurrentBuildingUnit->m_sId);
    pData->addLong(m_pCurrentBuildingUnit->m_uCost);
  }
  else
    pData->addLong(0);
}

// -----------------------------------------------------------------
// Name : updateOrders
// -----------------------------------------------------------------
void Town::updateOrders(NetworkData * pData)
{
  wchar_t sName[NAME_MAX_CHARS];
  pData->readString(sName);
  setCurrentBuilding(sName);
  
  // Unit being produced
  if (pData->readLong() == 1)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pData->readString(sName);
    u8 cost = (u8) pData->readLong();
    if (m_pCurrentBuildingUnit == NULL || wcscmp(m_pCurrentBuildingUnit->m_sId, sName) != 0)
    {
      FREE(m_pCurrentBuildingUnit);
      m_pCurrentBuildingUnit = new Ethnicity::TownUnit(sName, cost);
      m_uUnitProdInStock = 0;
    }
  }
  else
  {
    FREE(m_pCurrentBuildingUnit);
    m_uUnitProdInStock = 0;
  }
}

// -----------------------------------------------------------------
// Name : initGraphics
// -----------------------------------------------------------------
void Town::initGraphics(DisplayEngine * pDisplay)
{
  QuadData quad(0.0f, 0.8f, 0.0f, 0.8f, m_sTextures[m_uSize-1], pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
}

// -----------------------------------------------------------------
// Name : initServer
// -----------------------------------------------------------------
void Town::initServer()
{
  calculateUsedTiles();
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void Town::display()
{
  Coords3D pos3D = getDisplay()->get3DCoords(getMapPos(), BOARDPLANE);
//  Coords3D pos3D = getDisplay()->get3DCoords(getMapPos(), FARPLANE-0.005f);
  pos3D.x += 0.1f;
  pos3D.y += 0.1f;
  m_pGeometry->display(pos3D, F_RGBA_NULL);
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
int Town::getTexture()
{
  return ((GeometryQuads*)m_pGeometry)->getTexture();
}

// -----------------------------------------------------------------
// Name : getBigTexture
// -----------------------------------------------------------------
int Town::getBigTexture()
{
  return getDisplay()->getTextureEngine()->loadTexture(m_sBigPicture, false, 0, BIG_PICTURE_WIDTH, 0, BIG_PICTURE_HEIGHT);
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
wchar_t * Town::getInfo(wchar_t * sBuf, int iSize, InfoDest eDest)
{
  // Get ethnicity name
  wchar_t sEthn[NAME_MAX_CHARS];
  Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(m_sEthnicityEdition);
  assert(pEdition != NULL);
  Ethnicity * pEthn = pEdition->findEthnicity(m_sEthnicityId);
  assert(pEthn != NULL);
  pEthn->findLocalizedElement(sEthn, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");

  wchar_t sText[128];
  void * pPhraseArgs[2] = { getName(), sEthn };
  i18n->getText(L"TOWN_OF_%$1s_(%$2s)", sText, 128, pPhraseArgs);
  wsafecpy(sBuf, iSize, sText);
  return sBuf;
}

// -----------------------------------------------------------------
// Name : getUnitProdTime
// -----------------------------------------------------------------
u8 Town::getUnitProdTime()
{
  return (m_pCurrentBuildingUnit == NULL) ? 0 : (getUnitProdBonus() * m_pCurrentBuildingUnit->m_uCost) / 100;
}

// -----------------------------------------------------------------
// Name : getMaxFoodStorage
// -----------------------------------------------------------------
u16 Town::getMaxFoodStorage()
{
  return m_uSize * 30;
}

// -----------------------------------------------------------------
// Name : getUnitProdBonus
//  return percentage of basic cost
// -----------------------------------------------------------------
s16 Town::getUnitProdBonus()
{
  switch (m_uSize)
  {
  case 2:
    return (s16) ((getValue(STRING_UNITPROD) * 90) / 100);
  case 3:
    return (s16) ((getValue(STRING_UNITPROD) * 75) / 100);
  case 4:
    return (s16) ((getValue(STRING_UNITPROD) * 50) / 100);
  case 5:
    return (s16) ((getValue(STRING_UNITPROD) * 25) / 100);
  default:  // 1
    return (s16) getValue(STRING_UNITPROD);
  }
}

// -----------------------------------------------------------------
// Name : getFoodPerTurn
// -----------------------------------------------------------------
s16 Town::getFoodPerTurn()
{
  return (m_iFoodPerTurn * getValue(STRING_GROWTH)) / 100;
}

// -----------------------------------------------------------------
// Name : getProdPerTurn
// -----------------------------------------------------------------
u16 Town::getProdPerTurn()
{
  return (m_uProdPerTurn * getValue(STRING_PRODUCTIVITY)) / 100;
}

// -----------------------------------------------------------------
// Name : newTurn
//  Called by server only
// -----------------------------------------------------------------
void Town::newTurn(Player * pOwner, Server * pServer)
{
  // Update variable data
  // Owner
  if (pOwner->m_uPlayerId != m_uOwner)
  {
    setOwner(pOwner->m_uPlayerId);
    setBaseValue(STRING_HEROECHANCES, 0);
    if (pOwner->m_uPlayerId != 0)
      pServer->sendCustomLogToAll(L"(1s)_TOWN_CAPTURED_(2s)", 1, L"atp", (u8)LOG_ACTION_TOWNSCREEN, getId(), getId(), pOwner->m_uPlayerId);
    else
      pServer->sendCustomLogToAll(L"(s)_TOWN_LIBERATED", 1, L"at", (u8)LOG_ACTION_TOWNSCREEN, getId(), getId());
  }
  // Owner XP
  pOwner->m_uXPTownPoints += m_uSize;
  // Food & size
  m_uFoodInStock += getFoodPerTurn();
  if (m_uFoodInStock >= getMaxFoodStorage())
  {
    if (m_uSize < 4 || (m_uSize == 4 && m_uOwner != 0 && pOwner->m_pCapitalTown == NULL))
    {
      m_uFoodInStock = 0;
      m_uSize++;
      pServer->sendCustomLogToAll(L"TOWN_(s)_HAS_GROWN_TO_(d)", 0, L"ati", (u8)LOG_ACTION_TOWNSCREEN, getId(), getId(), (int)m_uSize);
      if (m_uSize == 5)
      {
        pOwner->m_pCapitalTown = this;
        pServer->sendCustomLogToAll(L"IT_IS_NOW_CAPITAL_OF_(s)", 0, L"p", pOwner->m_uPlayerId);
      }
    }
    else
      m_uFoodInStock = getMaxFoodStorage();
  }
  // Building production
  Building * pBuild = getCurrentBuilding();
  if (pBuild != NULL)
  {
    m_uProdInStock += getProdPerTurn();
    if (m_uProdInStock >= pBuild->getProductionCost())
    {
      pServer->sendCustomLogToAll(L"BUILDING_(1s)_WAS_BUILT_IN_(2s)", 0, L"abt", (u8)LOG_ACTION_TOWNSCREEN, getId(), getId(), m_sCurrentBuilding, getId());
      buildBuilding(m_sCurrentBuilding, pServer);
    }
  }
  // Unit formation
  if (m_pCurrentBuildingUnit != NULL)
  {
    m_uUnitProdInStock++;
    if (m_uUnitProdInStock >= getUnitProdTime())
    {
      Unit * pUnit = pServer->getSolver()->addUnitToPlayer(getEthnicityEdition(), m_pCurrentBuildingUnit->m_sId, m_uOwner, getMapPos());
      m_uUnitProdInStock = 0;
      FREE(m_pCurrentBuildingUnit);
      // Send data to clients
      NetworkData unitData(NETWORKMSG_CREATE_UNIT_DATA);
      pUnit->serialize(&unitData);
      pServer->sendMessageToAllClients(&unitData);
      pServer->sendCustomLogToAll(L"(1s)_PRODUCED_UNIT_(2s)", 0, L"atu", (u8)LOG_ACTION_TOWNSCREEN, getId(), getId(), pUnit->getOwner(), pUnit->getId());
      void * p = pUnit->getIdentifiers();
      callEffectHandler(L"onUnitProduced", L"s", &p);
    }
  }
  calculateUsedTiles();
  updateHeroes(pServer);
  // Re-calculate radius
  // Get ethnicity
  Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(m_sEthnicityEdition);
  assert(pEdition != NULL);
  Ethnicity * pEthn = pEdition->findEthnicity(m_sEthnicityId);
  assert(pEthn != NULL);
  setBaseValue(STRING_RADIUS, m_uSize + pEthn->m_iTownsRadiusBonus);
}

// -----------------------------------------------------------------
// Name : calculateUsedTiles
// -----------------------------------------------------------------
void Town::calculateUsedTiles()
{
  m_iFoodPerTurn = 0;
  m_uProdPerTurn = 0;
  if (m_uSize < 5)
  {
    bool bUsedTiles[9] = { false, false, false, false, false, false, false, false, false };
    for (int iSize = 0; iSize < 2 * m_uSize; iSize++)
    {
      int iBestTile = -1;
      int iBestFood = 0;
      int iBestProd = 0;
      MapTile * pBestTile = NULL;
      for (int iTile = 0; iTile < 9; iTile++)
      {
        if (!bUsedTiles[iTile])
        {
          MapTile * pTile = m_pMap->getTileAt(getMapPos() + CoordsMap(iTile / 3 - 1, iTile % 3 - 1));
          if (pTile != NULL)
          {
            int iFood = (int) pTile->getValue(STRING_FOOD);
            int iProd = (int) pTile->getValue(STRING_PROD);
            if (iBestTile == -1 ||
              iFood + iProd > iBestFood + iBestProd ||
              (iFood + iProd == iBestFood + iBestProd &&
              iFood > iBestFood))
            {
              iBestTile = iTile;
              pBestTile = pTile;
              iBestFood = iFood;
              iBestProd = iProd;
            }
          }
        }
      }
      if (iBestTile == -1)  // No more tile (can happen when town is on map border)
        break;
      bUsedTiles[iBestTile] = true;
      m_iFoodPerTurn += iBestFood;
      m_uProdPerTurn += iBestProd;
    }
  }
  else  // Capital => all tiles are used
  {
    for (int iTile = 0; iTile < 9; iTile++)
    {
      MapTile * pTile = m_pMap->getTileAt(getMapPos() + CoordsMap(iTile / 3 - 1, iTile % 3 - 1));
      m_iFoodPerTurn += pTile->getValue(STRING_FOOD);
      m_uProdPerTurn += pTile->getValue(STRING_PROD);
    }
  }
  m_iFoodPerTurn -= 2 * m_uSize;
  if (m_iFoodPerTurn < 0)
    m_iFoodPerTurn = 0;
}

// -----------------------------------------------------------------
// Name : updateHeroes
// -----------------------------------------------------------------
void Town::updateHeroes(Server * pServer)
{
  // Get ethnicity
  Edition * pEdition = m_pLocalClient->getDataFactory()->findEdition(m_sEthnicityEdition);
  assert(pEdition != NULL);
  Ethnicity * pEthn = pEdition->findEthnicity(m_sEthnicityId);
  assert(pEthn != NULL);

  // First update fear and happiness
  int baseHappy = pEthn->m_iTownsHappinessBonus - 2 * m_uSize;
  if (m_uOwner != 0)
  {
    UnitData * pAvatar = (UnitData*) pServer->getSolver()->findPlayer(m_uOwner)->m_pAvatarData;
    assert(pAvatar != NULL);
    if (wcscmp(pAvatar->m_sEdition, m_sEthnicityEdition) == 0 && wcscmp(pAvatar->m_sEthnicityId, m_sEthnicityId) == 0)
      baseHappy += m_uSize;
  }
  int baseFear = pEthn->m_iTownsFearBonus;
  MapTile * pTile = getMap()->getTileAt(getMapPos());
  assert(pTile != NULL);
  Unit * pUnit = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT);
  while (pUnit != NULL)
  {
    UnitData * pData = pUnit->getUnitData(m_pLocalClient);
    assert(pData != NULL);
    if (wcscmp(pData->m_sEdition, m_sEthnicityEdition) == 0 && wcscmp(pData->m_sEthnicityId, m_sEthnicityId) == 0)
      baseFear--;
    else
      baseFear++;
    pUnit = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT);
  }
  baseFear = max(baseFear, 0);

  setBaseValue(STRING_HAPPINESS, baseHappy);
  setBaseValue(STRING_FEAR, baseFear);
  int happ = getValue(STRING_HAPPINESS);
  int fear = max(0, getValue(STRING_FEAR));

  // Now parse town heroes; they can rompre leur allégeance
  pUnit = (Unit*) m_pHeroes->getFirst(0);
  while (pUnit != NULL)
  {
    if (pUnit->getOwner() == m_uOwner && happ < 0 && (int) getRandom(100) < -happ)
    {
      // The hero owner doesn't take care of the town correctly => -happ% chances that the hero goes to opponent!
      int nbPlayers = m_pLocalClient->getPlayerManager()->getPlayersCount();
      u8 owner = 0;
      if (nbPlayers < 2)
        break;
      do {
        owner = getRandom(nbPlayers) + 1;
      } while (owner == m_uOwner);
      pServer->sendCustomLogToAll(L"%$1s_HAS_TURNED_COAT_FOR_%$2s", 2, L"aup", (u8)LOG_ACTION_UNITSCREEN, pUnit->getOwner(), pUnit->getId(), pUnit->getOwner(), pUnit->getId(), owner);
      pServer->getSolver()->getSpellsSolver()->onChangeUnitOwner(m_uOwner, pUnit->getId(), owner);
    }
    pUnit = (Unit*) m_pHeroes->getNext(0);
  }

  if (m_uOwner == 0)
    return; // No heroe when no owner

  // Get unused heroe
  Ethnicity::TownHeroe * pHeroe = (Ethnicity::TownHeroe*) pEthn->m_pHeroes->getFirst(0);
  while (pHeroe != NULL)
  {
    if (!pHeroe->m_bUsed)
      break;
    pHeroe = (Ethnicity::TownHeroe*) pEthn->m_pHeroes->getNext(0);
  }

  if (pHeroe == NULL)
  {
    setBaseValue(STRING_HEROECHANCES, 0);
    return; // no more heroe available
  }

  int chances = max(0, getValue(STRING_HEROECHANCES, true) + (happ*happ - fear*fear) / 2);
  setBaseValue(STRING_HEROECHANCES, chances);
  chances = max(0, getValue(STRING_HEROECHANCES));

  if (getRandom(1000) < (unsigned) chances)
  {
    setBaseValue(STRING_HEROECHANCES, 0);
    u8 owner = 0;
    if (happ > 0)
      owner = m_uOwner;
    else
    {
      int nbPlayers = m_pLocalClient->getPlayerManager()->getPlayersCount();
      if (nbPlayers < 2)
        return;
      do {
        owner = getRandom(nbPlayers) + 1;
      } while (owner == m_uOwner);
    }

    pHeroe->m_bUsed = true;
    Unit * pUnit = pServer->getSolver()->addUnitToPlayer(getEthnicityEdition(), pHeroe->m_sId, owner, getMapPos());
    m_pHeroes->addLast(pUnit);
    // Send data to clients
    NetworkData unitData(NETWORKMSG_CREATE_UNIT_DATA);
    pUnit->serialize(&unitData);
    pServer->sendMessageToAllClients(&unitData);
    pServer->sendCustomLogToAll(L"A_HEROE_%$1s_IS_BORN_FOR_%$2s", 2, L"aup", (u8)LOG_ACTION_TOWNSCREEN, getId(), pUnit->getOwner(), pUnit->getId(), pUnit->getOwner());
  }
}

// -----------------------------------------------------------------
// Name : setCurrentBuilding
// -----------------------------------------------------------------
void Town::setCurrentBuilding(wchar_t * sName)
{
  if (wcscmp(m_sCurrentBuilding, sName) != 0)
  {
    wsafecpy(m_sCurrentBuilding, NAME_MAX_CHARS, sName);
    m_uProdInStock = 0;
  }
}

// -----------------------------------------------------------------
// Name : setCurrentUnit
//  Take a copy of pUnit, don't take pointer, to make it easier sending it through network
// -----------------------------------------------------------------
void Town::setCurrentUnit(Ethnicity::TownUnit * pUnit)
{
  if (pUnit == NULL)
  {
    FREE(m_pCurrentBuildingUnit);
    m_uUnitProdInStock = 0;
  }
  else if (m_pCurrentBuildingUnit == NULL)
  {
    m_pCurrentBuildingUnit = new Ethnicity::TownUnit(pUnit->m_sId, pUnit->m_uCost);
    m_uUnitProdInStock = 0;
  }
  else if (wcscmp(m_pCurrentBuildingUnit->m_sId, pUnit->m_sId) != 0)
  {
    wsafecpy(m_pCurrentBuildingUnit->m_sId, NAME_MAX_CHARS, pUnit->m_sId);
    m_pCurrentBuildingUnit->m_uCost = pUnit->m_uCost;
    m_uUnitProdInStock = 0;
  }
}

// -----------------------------------------------------------------
// Name : buildBuilding
// -----------------------------------------------------------------
void Town::buildBuilding(wchar_t * sName, Server * pServer)
{
  int _it = m_pBuildings->getIterator();
  Building * pBuild = getFirstBuilding(_it);
  while (pBuild != NULL)
  {
    if (wcscmp(pBuild->getObjectName(), sName) == 0)
    {
      if (!pBuild->isBuilt() && isBuildingAllowed(pBuild))
      {
        Building * pNew = pBuild;
        pNew->setBuilt(true);
        attachEffect(pNew);
        if (pServer != NULL)
        {
          NetworkData msg(NETWORKMSG_BUILDING_BUILT);
          msg.addLong(getId());
          msg.addString(sName);
          pServer->sendMessageToAllClients(&msg);
        }
      }
      if (wcscmp(sName, m_sCurrentBuilding) == 0)
        setCurrentBuilding(L"");
      m_pBuildings->releaseIterator(_it);
      return;
    }
    pBuild = getNextBuilding(_it);
  }
  m_pBuildings->releaseIterator(_it);
}

// -----------------------------------------------------------------
// Name : isBuildingAllowed
// -----------------------------------------------------------------
bool Town::isBuildingAllowed(Building * pBuild)
{
  if (pBuild->callLuaFunction(L"isAllowed", 1, L"l", (long)getId()))
  {
    double resp = pBuild->getLuaNumber();
    return (resp == 1);
  }
  return true;
}

// -----------------------------------------------------------------
// Name : getExtraBuildableUnit
// -----------------------------------------------------------------
ObjectList * Town::getExtraBuildableUnit()
{
  // Rebuild this list, using LUA m_pEffects
  m_pExtraBuildableUnits->deleteAll();

  LuaObject * pEffect = getFirstEffect(0);
  while (pEffect != NULL)
  {
    double d;
    if (pEffect->getLuaVarNumber(L"nbProducedUnits", &d))
    {
      int size = (int)d;
      if (size > 0)
      {
        pEffect->callLuaFunction(L"getProducedUnits", 2*size, L"");
        wchar_t sName[NAME_MAX_CHARS];
        for (int i = 0; i < size; i++)
        {
          u8 uCost = (u8) pEffect->getLuaNumber();
          pEffect->getLuaString(sName, NAME_MAX_CHARS);
          Ethnicity::TownUnit * pUnit = new Ethnicity::TownUnit(sName, uCost);
          m_pExtraBuildableUnits->addLast(pUnit);
        }
      }
    }
    pEffect = getNextEffect(0);
  }
  return m_pExtraBuildableUnits;
}

// -----------------------------------------------------------------
// Name : getCurrentBuilding
// -----------------------------------------------------------------
Building * Town::getCurrentBuilding()
{
  if (wcscmp(m_sCurrentBuilding, L"") == 0)
    return NULL;
  Building * pBuild = getFirstBuilding(0);
  while (pBuild != NULL)
  {
    if (wcscmp(pBuild->getObjectName(), m_sCurrentBuilding) == 0)
      return pBuild;
    pBuild = getNextBuilding(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getFirstBuilding
// -----------------------------------------------------------------
Building * Town::getFirstBuilding(int it)
{
  return (Building*) m_pBuildings->getFirst(it);
}

// -----------------------------------------------------------------
// Name : getNextBuilding
// -----------------------------------------------------------------
Building * Town::getNextBuilding(int it)
{
  return (Building*) m_pBuildings->getNext(it);
}

// -----------------------------------------------------------------
// Name : getInfluenceAt
// -----------------------------------------------------------------
long Town::getInfluenceAt(CoordsMap mp)
{
  CoordsMap pos = getMapPos();
  long dist = max(abs(mp.x - pos.x), abs(mp.y - pos.y));
  long radius = getValue(STRING_RADIUS);
  if (dist > radius)
    return 0;
  return (1 + radius - dist) * (1 + radius - dist);
}
