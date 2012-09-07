#include "AISolver.h"
#include "Server.h"
#include "TurnSolver.h"
#include "SpellsSolver.h"
#include "../Data/DataFactory.h"
#include "../Players/Player.h"
#include "../Players/AISpell.h"
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
  m_pLuaSolver = new AILuaSolver(pServer);
}

// -----------------------------------------------------------------
// Name : ~AISolver
//  Destructor
// -----------------------------------------------------------------
AISolver::~AISolver()
{
  delete m_pLuaSolver;
}

// -----------------------------------------------------------------
// Name : resolveAI
// -----------------------------------------------------------------
void AISolver::resolveAI(Player * pPlayer)
{
  // Do spells
  m_pLuaSolver->resolveAISpells(pPlayer);
  // Do units orders
  // Finished
  pPlayer->setState(finished);
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
      if (strcmp(((Town*)pObj)->getEthnicityId(), pUnit->getUnitData(m_pServer)->m_sEthnicityId) == 0)
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

// -----------------------------------------------------------------
// Name : evaluateUnit
// -----------------------------------------------------------------
float AISolver::evaluateUnit(CoordsMap mapPos, const char * sName, u8 uPlayer)
{
  Unit * pUnit = m_pServer->getSolver()->getSpellsSolver()->onAddUnit(mapPos, sName, uPlayer, true);
  if (pUnit == NULL)
    return 0;
  float fInterest = evaluateUnit(pUnit);
  delete pUnit;
  return fInterest;
}

// -----------------------------------------------------------------
// Name : evaluateUnit
// -----------------------------------------------------------------
float AISolver::evaluateUnit(Unit * pUnit)
{
  float fInterest = 0.0f;
  fInterest += pUnit->getValue(STRING_MELEE) * AI_INTEREST_MELEE;
  fInterest += pUnit->getValue(STRING_RANGE) * AI_INTEREST_RANGE;
  fInterest += pUnit->getValue(STRING_ARMOR) * AI_INTEREST_ARMOR;
  fInterest += pUnit->getValue(STRING_ENDURANCE) * AI_INTEREST_ENDURANCE;
  fInterest += pUnit->getValue(STRING_SPEED) * AI_INTEREST_SPEED;
  fInterest += pUnit->getValue(STRING_LIFE) * AI_INTEREST_LIFE;
  fInterest += pUnit->callEffectHandler("getAIInterest", "", NULL, HANDLER_RESULT_TYPE_ADD);
  return fInterest;
}

// -----------------------------------------------------------------
// Name : evaluateBattleModifications
//  Appelée par les fonctions d'interface LUA telles que "setAttackerLife" en mode simulation
// -----------------------------------------------------------------
float AISolver::evaluateBattleModifications(int * pNewAttackLife, int * pNewDefendLife, int * pNewAttackArmor, int * pNewDefendArmor, int * pNewAttackDamages, int * pNewDefendDamages)
{
  Unit * pAttacker = m_pServer->getSolver()->getAttacker();
  Unit * pDefender = m_pServer->getSolver()->getDefender();

  if (pAttacker == NULL || pDefender == NULL)
    return 0;

  int newAttackLife = (pNewAttackLife == NULL) ? m_pServer->getSolver()->m_iAttackerLife : *pNewAttackLife;
  int newDefendLife = (pNewDefendLife == NULL) ? m_pServer->getSolver()->m_iDefenderLife : *pNewDefendLife;
  int newAttackArmor = (pNewAttackArmor == NULL) ? m_pServer->getSolver()->m_iAttackerArmor : *pNewAttackArmor;
  int newDefendArmor = (pNewDefendArmor == NULL) ? m_pServer->getSolver()->m_iDefenderArmor : *pNewDefendArmor;
  int newAttackDamages = (pNewAttackDamages == NULL) ? m_pServer->getSolver()->m_iAttackerDamages : *pNewAttackDamages;
  int newDefendDamages = (pNewDefendDamages == NULL) ? m_pServer->getSolver()->m_iDefenderDamages : *pNewDefendDamages;

  int iOldAttackerLife = m_pServer->getSolver()->m_iAttackerLife;
  int iOldDefenderLife = m_pServer->getSolver()->m_iDefenderLife;
  int iFinalAttackerLife = newAttackLife - max(0, newDefendDamages - newAttackArmor);
  int iFinalDefenderLife = newDefendLife - max(0, newAttackDamages - newDefendArmor);

  float fInterest = 0.0f;
  if (iFinalAttackerLife != iOldAttackerLife) {
    // iSign positive for ennemies
    int iSign = (pAttacker->getOwner() == getCurrentPlayer()->m_uPlayerId) ? -1 : 1;
    // except if life is increased
    iSign = (iFinalAttackerLife > iOldAttackerLife) ? -iSign : iSign;
    float fUnitInterest = evaluateUnit(pAttacker);
    fInterest += (iSign * fUnitInterest * (iFinalAttackerLife <= 0 ? 1.0f : 0.33f));
    fInterest += (iSign * (iOldAttackerLife - iFinalAttackerLife) * AI_INTEREST_LIFE);
  }
  if (iFinalDefenderLife != iOldDefenderLife) {
    // iSign positive for ennemies
    int iSign = (pDefender->getOwner() == getCurrentPlayer()->m_uPlayerId) ? -1 : 1;
    // except if life is increased
    iSign = (iFinalDefenderLife > iOldDefenderLife) ? -iSign : iSign;
    float fUnitInterest = evaluateUnit(pDefender);
    fInterest += (iSign * fUnitInterest * (iFinalDefenderLife <= 0 ? 1.0f : 0.33f));
    fInterest += (iSign * (iOldDefenderLife - iFinalDefenderLife) * AI_INTEREST_LIFE);
  }

  return fInterest;
}
