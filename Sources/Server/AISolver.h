#ifndef _AI_SOLVER_H
#define _AI_SOLVER_H

#include "AILuaSolver.h"

// Interest coefficients
#define AI_INTEREST_MELEE       3.0f
#define AI_INTEREST_RANGE       4.0f
#define AI_INTEREST_ARMOR       4.0f
#define AI_INTEREST_ENDURANCE   2.0f
#define AI_INTEREST_SPEED       2.0f
#define AI_INTEREST_LIFE        5.0f

class MapTile;


class AISolver
{
public:
  // Constructor / destructor
  AISolver(Server * pServer);
  ~AISolver();

  void resolveNeutralAI(Unit * pUnit);
  void resolveAI(Player * pPlayer);

  // Simulation functions
  int getOpponentInterest(Unit * pUnit, Unit * pOpponent, bool * bRange = NULL);
  void addEvaluationTarget(AILuaSolver::EvaluationTargetInfo * pEval) { m_pLuaSolver->m_pEvaluationTargets->addLast(pEval); };
  void addInterestForCurrentSpell(float fInterest) { m_pLuaSolver->m_fCurrentInterest += fInterest; };
  void addCurrentSpellInterestForDamages(Player * pPlayer, Unit * pUnit, u8 damages) { m_pLuaSolver->addCurrentSpellInterestForDamages(pPlayer, pUnit, damages); };
  Player * getCurrentPlayer() { return m_pLuaSolver->m_pCurrentPlayer; };
  float evaluateBattleModifications(int * pNewAttackLife, int * pNewDefendLife, int * pNewAttackArmor, int * pNewDefendArmor, int * pNewAttackDamages, int * pNewDefendDamages);
  float evaluateUnit(Unit * pUnit);
  float evaluateUnit(CoordsMap mapPos, const char * sName, u8 uPlayer);

protected:
  bool isInterestedByTile(Unit * pUnit, MapTile * pTile, Unit ** pOpponent);

  Server * m_pServer;
  AILuaSolver * m_pLuaSolver;
};

#endif
