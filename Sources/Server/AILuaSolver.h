#ifndef _AI_LUA_SOLVER_H
#define _AI_LUA_SOLVER_H

#include "../Common/ObjectList.h"
#include "../lua_callbacks.h"

class Player;
class Server;
class LuaObject;
class AISpell;
class Spell;
class Unit;

class AILuaSolver
{
friend class AISolver;

public:
  // Class EvaluationTargetInfo
  class EvaluationTargetInfo : public BaseObject
  {
  public:
    EvaluationTargetInfo(u8 uTargetType, u32 uConstraints) { m_uTargetType = uTargetType; m_uConstraints = uConstraints; it1 = it2 = 0; };
    u8 m_uTargetType;
    u32 m_uConstraints;
    int it1;
    int it2;
    bool bCurrentIsNeutral;
  };
  class TargetData : public BaseObject
  {
  public:
    TargetData() { m_pTarget = NULL; wsafecpy(params, 64, L""); };
    BaseObject * m_pTarget;
    wchar_t params[64];
  };

  // Constructor / destructor
  AILuaSolver(Server * pServer);
  ~AILuaSolver();

  void resolveAISpells(Player * pPlayer);

protected:
  void addEvaluationTarget(EvaluationTargetInfo * pEval) { m_pEvaluationTargets->addLast(pEval); };
  void addInterestForCurrentSpell(float fInterest) { m_fCurrentInterest += fInterest; };
  AISpell * evaluateSpell(Player * pCaster, Spell * pRealSpell);
  TargetData * findTargetForLua(Player * pCaster, EvaluationTargetInfo ** pInfoPtr);
  void getTargetsForSpell(Player * pCaster, Spell * pSpell, EvaluationTargetInfo * pInfo, MetaObjectList * pList);
  void addCurrentSpellInterestForDamages(Player * pPlayer, Unit * pUnit, u8 damages);

  Server * m_pServer;
  ObjectList * m_pEvaluationTargets;
  ObjectList * m_pTryTargets;
  float m_fCurrentInterest;
  Player * m_pCurrentPlayer;
  LuaObject * m_pCurrentLua;
};

#endif
