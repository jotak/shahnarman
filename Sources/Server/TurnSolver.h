#ifndef _TURN_SOLVER_H
#define _TURN_SOLVER_H

#include "SpellsSolver.h"
#include "../Players/PlayerManagerAbstract.h"

enum ResolveState
{
  RS_NotResolving = 0,
  RS_ResolveAI,
  RS_ResolveNeutral,
  RS_ResolveEOT,
  RS_ResolveSpells,
  RS_ResolveMoves,
  RS_FindBattles,
  RS_ResolveBattle,
  RS_UpdateTowns,
  RS_EndResolve
};

class Server;
class UnitData;
class Unit;
class MapTile;
class AISolver;

class TurnSolver : public PlayerManagerAbstract
{
public:
  // Constructor / destructor
  TurnSolver(Server * pServer);
  ~TurnSolver();

  // Standard functions
  void Init();
  void onInitFinished();
  void Update(double delta);

  // Other functions
  Unit * addUnitToPlayer(wchar_t * sEdition, wchar_t * sUnitId, u8 uPlayerId, CoordsMap mapPos);
  Player * getCurrentPlayer() { return m_pCurrentPlayer; };
  ResolveState getState() { return m_ResolveState; };
  SpellsSolver * getSpellsSolver() { return m_pSpellsSolver; };
  void checkAllUnitUpdates(bool bUnsetModified);
  void setInitialAvatar(UnitData * pData, Player * pPlayer, CoordsMap pos);
  void addInitialPlayerSpell(Player * pPlayer, wchar_t * sEdition, wchar_t * sName);
  void drawInitialSpells();
  void allowCastSpells(bool bPostBattle);
  Unit * getCurrentUnit() { return m_pCurrentUnit; };
  Unit * getAttacker() { return m_pCurrentUnit; };  // Actually the same as getCurrentUnit, but for conveniance we keep that
  Unit * getDefender() { return m_pDefendingUnit; };

  // Messages from clients
  void playerSelectedBattle(NetworkData * pData);
  void attackerChoosedUnits(NetworkData * pData);
  void playerChoosedDefender(NetworkData * pData);
  void playerFinishedBattles() { nextPhase(); };

  // Load / save
  void serialize(NetworkData * pData);
  void deserialize(NetworkData * pData);

private:
  // Turn management functions : resolve phase, other
  bool resolvePlayerMoves();
  bool findPlayerBattles();
  void resolveUnitMove();
  void resolveBattle();
  bool findDefenders(MapTile * pTile, Unit * pAttacker, bool bRange, ObjectList * pDefenders);

  void drawPlayerSpells();
  void updateTowns();
  void updateTilesInfluence();
  void nextPhase(ResolveSpellsState oldstate = RSS_NotResolving);
  void resetDataForNextTurn();
  void removePlayer(Player * pDead);
  void callNewTurnHandlers(u8 uStep);

  // Permanent players data
  Server * m_pServer;
  SpellsSolver * m_pSpellsSolver;
  AISolver * m_pAISolver;
  u8 m_uNbHumanPlayers;

  // Resolve data
  ResolveState m_ResolveState;
  Unit * m_pCurrentUnit;
  Unit * m_pDefendingUnit;
  bool m_bIsRangeAttack;
  int m_iPlayerIt;
  Player * m_pCurrentPlayer;

  // Units and spells count
  unsigned long m_uUnitsCount;    // up to more than 4 000 000 000 units => should be ok not to check this number

public:
  // Resolve data that can be retrieved / modified by LUA effects
  bool m_bUnitHasAttacked;
  int m_iAttackerDamages;
  int m_iDefenderDamages;
  int m_iAttackerArmor;
  int m_iDefenderArmor;
  int m_iAttackerLife;
  int m_iDefenderLife;
};

#endif
