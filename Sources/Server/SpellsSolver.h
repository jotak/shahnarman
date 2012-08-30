#ifndef _SPELLS_SOLVER_H
#define _SPELLS_SOLVER_H

#include "../Data/LuaContext.h"

enum ResolveSpellsState
{
  RSS_NotResolving = 0,
  RSS_ChildEffect,
  RSS_TurnSpells,
  RSS_BattleSpells,
  RSS_PostBattleSpells
};

class TurnSolver;
class Server;
class NetworkData;
class Player;
class Spell;
class ChildEffect;
class LuaObject;
class Unit;
class Town;
class MapObject;

class SpellsSolver
{
public:
  // Constructor / destructor
  SpellsSolver(Server * pServer, TurnSolver * pSolver);
  ~SpellsSolver();

  // Standard functions
  void Init();
  void Update(double delta);

  // Member access
  ResolveSpellsState getState() { return m_State; };

  // Other functions
  void startResolveSpells(ResolveSpellsState state);
  void waitForInstantSpells(ResolveSpellsState state);
  void resolveChildEffect(Player * pPlayer, Unit * pUnit, ChildEffect * pChild);
//  Spell * getSpellBeingResolved() { return m_pSpellBeingCast; };
//  ChildEffect * getEffectBeingResolved() { return m_pCurrentChildEffect; };
  LuaContext * retrieveLuaContext(u32 uExpectedType = 0, NetworkData * pSerialize = NULL, int iEffect = -1);

  // Messages from clients
  void receiveInstantSpells(int iClient, NetworkData * pData);
  void receiveSpells(NetworkData * pData);
  void receiveTargetOnResolve(NetworkData * pData);

  // Handlers called from LUA
  void onDamageUnit(u8 uPlayerId, u32 uUnitId, u8 uDamages);
  void onAddUnit(CoordsMap mapPos, const char * sName, u8 uOwner);
  void onAttachToUnit(u8 uPlayerId, u32 uUnitId);
  void onAttachToPlayer(u8 uPlayerId);
  void onAttachToTown(u32 uTownId);
  void onAttachToTemple(u32 uTempleId);
  void onAttachToTile(CoordsMap pos);
  void onAddChildEffectToUnit(int iEffectId, u8 uPlayerId, u32 uUnitId);
  void onRemoveChildEffectFromUnit(int iEffectId, u8 uPlayerId, u32 uUnitId);
  void onDiscardSpell(u8 uSrc, int iPlayerId, int iSpellId);
  void onDrawSpell(u8 uPlayerId, u32 uSpellId);
  void onRecallSpell(const wchar_t * sType, u8 uPlayerId, u32 uSpellId);
  void onAttachAsGlobal();
  void onDetachFromGlobal();
  void onSelectTargetThenResolve(u8 uType, u32 uConstraints, wchar_t * sCallback);
  void onDeactivateSkill(u8 uPlayerId, u32 uUnitId, u32 uSkillId);
  void onChangeSpellOwner(const wchar_t * sType, u8 uOldOwner, u32 uSpellId, u8 uNewOwner);
  void onChangeUnitOwner(u8 uOldOwner, u32 uUnitId, u8 uNewOwner);
  void onChangeTownOwner(u32 uTownId, u8 uNewOwner);
  void onBuildBuilding(u32 uTownId, wchar_t * sName);
  void onProduceMana(int playerId, CoordsMap srcPos, u8 * pMana);
  void onUpdateMaxMana(int playerId, CoordsMap srcPos, u8 * pMana);
  void onAddSkillToUnit(wchar_t * sSkillName, wchar_t * sSkillParams, u8 uPlayerId, u32 uUnitId);
  void onHideSpecialTile(u32 uTileId);
  void onTeleport(MapObject * pMapObj, CoordsMap pos);
  void onResurrect(u8 uPlayerId, u32 uUnitId);
  int onAddMagicCircle(Player * pPlayer, CoordsMap pos);
  void onRemoveMagicCircle(Player * pPlayer, int iCircle);
  void onAddGoldToPlayer(u8 uPlayerId, int iAmount);
  void onAddSpellToPlayer(u8 uPlayerId, wchar_t * sName);
  void onAddArtifactToPlayer(u8 uPlayerId, wchar_t * sName);
  void onAddAvatarToPlayer(u8 uPlayerId, wchar_t * sName);

private:
  // Turn management functions : resolve phase, spells
  bool startResolvePlayerSpell(Player * pPlayer);
  void endCastSpell(bool bCancel);
  void endActivateEffect(bool bCancel);

  // Other modules
  Server * m_pServer;
  TurnSolver * m_pSolver;

  // Resolve data
  ResolveSpellsState m_State;
  bool m_bSpellsCastThisTurn;
  bool m_bIdle;
  bool * m_bWaitingClient;
  int m_iPlayerIt;

  // Temporary spell data
//  Player * m_pCurrentCaster;
  LuaObject * m_pLuaBeingResolved;
//  NetworkData * m_pSpellNetworkData;
  bool m_bPauseResolving;
//  ChildEffect * m_pCurrentChildEffect;
  LuaContext m_LuaContext;
};

#endif
