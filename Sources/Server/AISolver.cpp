#include "AISolver.h"
#include "Server.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/MapTile.h"
#include "../Gameboard/SpecialTile.h"

// -----------------------------------------------------------------
// Name : AISolver
//  Constructor
// -----------------------------------------------------------------
AISolver::AISolver(Server * pServer)
{
  m_pServer = pServer;
}

// -----------------------------------------------------------------
// Name : ~AISolver
//  Destructor
// -----------------------------------------------------------------
AISolver::~AISolver()
{
}

// -----------------------------------------------------------------
// Name : resolveNeutralAI
// Quite basic AI:
//  - if the unit is already located on an "interesting" tile, it fortifies (or attack if there's an opponent)
//  - else, check all tiles on the board: it will go to the nearest "interesting" tile.
// -----------------------------------------------------------------
void AISolver::resolveNeutralAI(Unit * pUnit)
{
  Unit * pOpponent;
  Map * pMap = pUnit->getMap();
  assert(pMap != NULL);
  MapTile * pTile = pMap->getTileAt(pUnit->getMapPos());
  if (isInterestedByTile(pUnit, pTile, &pOpponent))
  {
    if (pOpponent != NULL)
      pUnit->setAttackOrder(pOpponent, NULL);
    else
      pUnit->setFortifyOrder();
    return;
  }
  s16 iInteresting = -1;
  Unit * pInterestingOpponent = NULL;
  CoordsMap interestingPos;
  for (int x = 0; x < pMap->getWidth(); x++)
  {
    for (int y = 0; y < pMap->getHeight(); y++)
    {
      pTile = pMap->getTileAt(CoordsMap(x, y));
      if (isInterestedByTile(pUnit, pTile, &pOpponent))
      {
        s16 dist = pMap->findPath(pUnit, CoordsMap(x, y), NULL);
        if (dist >= 0 && (iInteresting < 0 || dist < iInteresting))
        {
          iInteresting = dist;
          interestingPos = CoordsMap(x, y);
          pInterestingOpponent = pOpponent;
        }
      }
    }
  }
  if (iInteresting >= 0)
  {
    if (pInterestingOpponent != NULL)
      pUnit->setAttackOrder(pInterestingOpponent, NULL);
    else
      pUnit->setMoveOrder(interestingPos, NULL);
    return;
  }
}

// -----------------------------------------------------------------
// Name : isInterestedByTile
//  Look what's on pTile, and see if there's an interesting item
//  First see if there's an opponent; then, any other item (town, spec tile, ...)
// -----------------------------------------------------------------
bool AISolver::isInterestedByTile(Unit * pUnit, MapTile * pTile, Unit ** pOpponent)
{
  // First check any opponent on tile
  int iMostInterestingOpponent = -1;
  Unit * pMostInterestingOpponent = NULL;
  int it = pTile->m_pMapObjects->getIterator();
  Unit * pOther = (Unit*) pTile->getFirstMapObject(GOTYPE_UNIT, it);
  while (pOther != NULL)
  {
    if (pOther->getOwner() > 0) // Not neutral = opponent
    {
      int iInterest = getOpponentInterest(pUnit, pOther);
      if (pMostInterestingOpponent == NULL || iMostInterestingOpponent < iInterest)
      {
        iMostInterestingOpponent = iInterest;
        pMostInterestingOpponent = pOther;
      }
    }
    pOther = (Unit*) pTile->getNextMapObject(GOTYPE_UNIT, it);
  }
  if (iMostInterestingOpponent >= 0)
  {
    *pOpponent = pMostInterestingOpponent;
    return true;
  }
  else
    *pOpponent = NULL;

  // Then any city or temple
  MapObject * pObj = pTile->getFirstMapObject(GOTYPE_MAPOBJECT, it);
  while (pObj != NULL)
  {
    if (pObj->getType() & GOTYPE_TEMPLE)
      return true;
    if (pObj->getType() & GOTYPE_TOWN)
    {
      // Go to town only if same ethnicity
      if (wcscmp(((Town*)pObj)->getEthnicityId(), pUnit->getUnitData(m_pServer)->m_sEthnicityId) == 0)
        return true;
    }
    pObj = pTile->getNextMapObject(GOTYPE_MAPOBJECT, it);
  }
  pTile->m_pMapObjects->releaseIterator(it);

  // Then attracting special tile
  if (pTile->m_pSpecialTile != NULL && pTile->m_pSpecialTile->isAIAttracting())
    return true;
  return false;
}

// -----------------------------------------------------------------
// Name : getOpponentInterest
//  The more the opponent is strong / unbeatable, the less the score is
//  TODO: at this time, LUA handlers like onRangeAttack or onMeleeAttack (etc.) are not called => function is not much acurate
// -----------------------------------------------------------------
int AISolver::getOpponentInterest(Unit * pUnit, Unit * pOpponent, bool * bRange)
{
  int iRangeInterest = -1;
  if (pUnit->getValue(STRING_RANGE) > 0)
  {
    int attDamages = max(0, pUnit->getValue(STRING_RANGE) - pOpponent->getValue(STRING_ARMOR));
    int life = pOpponent->getValue(STRING_LIFE);
    iRangeInterest = min(attDamages, life);
    if (life <= attDamages)
      iRangeInterest *= 3;
  }
  int attDamages = max(0, pUnit->getValue(STRING_MELEE) - pOpponent->getValue(STRING_ARMOR));
  int defDamages = max(0, pOpponent->getValue(STRING_MELEE) - pUnit->getValue(STRING_ARMOR));
  int attLife = pUnit->getValue(STRING_LIFE);
  int defLife = pOpponent->getValue(STRING_LIFE);
  int iMeleeInterest = min(attDamages, defLife) - min(defDamages, attLife);
  if (defLife <= attDamages)
    iMeleeInterest *= 3;
  if (attLife <= defDamages)
    iMeleeInterest /= 3;
  if (bRange != NULL)
    *bRange = (iRangeInterest > iMeleeInterest);
  return max(iRangeInterest, iMeleeInterest);
}
