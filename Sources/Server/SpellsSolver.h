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
    ResolveSpellsState getState()
    {
        return m_State;
    };

    // Other functions
    void startResolveSpells(ResolveSpellsState state);
    void waitForInstantSpells(ResolveSpellsState state);
    void resolveChildEffect(Player * pPlayer, Unit * pUnit, ChildEffect * pChild);
    LuaContext * retrieveLuaContext(u32 uExpectedType = 0, NetworkData * pSerialize = NULL, int iEffect = -1);

    // Messages from clients
    void receiveInstantSpells(int iClient, NetworkData * pData);
    void receiveSpells(NetworkData * pData);
    void receiveTargetOnResolve(NetworkData * pData);

    // Handlers called from LUA
    void onDamageUnit(u8 uPlayerId, u32 uUnitId, u8 uDamages);
    Unit * onAddUnit(CoordsMap mapPos, const char * sName, u8 uOwner, bool bSimulate = false);
    void onAttachToUnit(u8 uPlayerId, u32 uUnitId);
    void onAttachToPlayer(u8 uPlayerId);
    void onAttachToTown(u32 uTownId);
    void onAttachToTemple(u32 uTempleId);
    void onAttachToTile(CoordsMap pos);
    void onAddChildEffectToUnit(int iEffectId, u8 uPlayerId, u32 uUnitId);
    void onRemoveChildEffectFromUnit(int iEffectId, u8 uPlayerId, u32 uUnitId);
    void onAddChildEffectToTown(int iEffectId, u32 uTownId);
    void onRemoveChildEffectFromTown(int iEffectId, u32 uTownId);
    void onDiscardSpell(u8 uSrc, int iPlayerId, int iSpellId);
    void onDrawSpell(u8 uPlayerId, u32 uSpellId);
    void onRecallSpell(const char * sType, u8 uPlayerId, u32 uSpellId);
    void onAttachAsGlobal();
    void onDetachFromGlobal();
    void onSelectTargetThenResolve(u8 uType, u32 uConstraints, const char * sCallback);
    void onDeactivateSkill(long iPlayerId, long iUnitId, long iSkillId);
    void onChangeSpellOwner(const char * sType, u8 uOldOwner, u32 uSpellId, u8 uNewOwner);
    void onChangeUnitOwner(u8 uOldOwner, u32 uUnitId, u8 uNewOwner);
    void onChangeTownOwner(u32 uTownId, u8 uNewOwner);
    void onBuildBuilding(u32 uTownId, const char * sName);
    void onProduceMana(int playerId, CoordsMap srcPos, u8 * pMana);
    //void onUpdateMaxMana(int playerId, CoordsMap srcPos, u8 * pMana);
    void onAddSkillToUnit(const char * sSkillName, const char * sSkillParams, u8 uPlayerId, u32 uUnitId);
    void onHideSpecialTile(u32 uTileId);
    void onTeleport(MapObject * pMapObj, CoordsMap pos);
    void onResurrect(u8 uPlayerId, u32 uUnitId);
    void onRemoveUnit(u8 uPlayerId, u32 uUnitId);
    int onAddMagicCircle(Player * pPlayer, CoordsMap pos);
    void onRemoveMagicCircle(Player * pPlayer, int iCircle);
    void onAddGoldToPlayer(u8 uPlayerId, int iAmount);
    void onAddSpellToPlayer(u8 uPlayerId, const char * sName);
    void onAddArtifactToPlayer(u8 uPlayerId, const char * sName);
    void onAddAvatarToPlayer(u8 uPlayerId, const char * sName);

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
    LuaObject * m_pLuaBeingResolved;
    bool m_bPauseResolving;
    LuaContext m_LuaContext;
};

#endif
