#ifndef _PLAYERMANAGER_H
#define _PLAYERMANAGER_H

#include "PlayerManagerAbstract.h"

class Player;
class LocalClient;
class ObjectList;
class NetworkData;
class Map;
class Unit;
class Spell;
class LuaObject;
class ChildEffect;
class LuaTargetable;
class Mana;

class PlayerManager : public PlayerManagerAbstract
{
public:
    // Constructor / destructor
    PlayerManager(LocalClient * pLocalClient);
    ~PlayerManager();

    // Manager functions
    void Init();
    void Update(double delta);

    // Retrieve data from server
    void createUnitData(NetworkData * pData);
    void setPlayerState(NetworkData * pData);
    void setPlayerMana(NetworkData * pData);
    void setResolutionIdx(NetworkData * pData);
    void updateUnitsData(NetworkData * pData);
    void updateDeadUnits(NetworkData * pData);
    void updateCastSpellData(NetworkData * pData);
    void onLuaAttached(NetworkData * pData);
    void onLuaDetached(NetworkData * pData);
    void onChildEffectAttached(NetworkData * pData);
    void onChildEffectDetached(NetworkData * pData);
    void onCustomLuaUpdate(NetworkData * pData);
    void drawSpells(NetworkData * pData);
    void recallSpells(NetworkData * pData);
    void discardSpells(NetworkData * pData);
    //void removeActiveEffects(NetworkData * pData);
    void deactivateSkills(NetworkData * pData);
    void enableAllEffects(NetworkData * pData);
    void changeSpellOwner(NetworkData * pData);
    void changeUnitOwner(NetworkData * pData);
    void changeTownOwner(NetworkData * pData);
    void addSkillToUnit(NetworkData * pData);
    void hideSpecialTile(NetworkData * pData);
    LuaTargetable * findLuaTarget(NetworkData * pData, u32 * pType);
    void buildBuilding(NetworkData * pData);
    void resurrectUnit(NetworkData * pData);
    void removeUnit(NetworkData * pData);
    void teleport(NetworkData * pData);
    void addMagicCircle(NetworkData * pData);
    void removeMagicCircle(NetworkData * pData);

    // Turn management functions : orders phase
    void enableEOT(bool bEnabled);
    bool requestEndPlayerOrders();
    void setNextPlayerReady(Player * pPlayer);
    void updateMagicCirclePositions();

    // Player management functions
    Player * getActiveLocalPlayer()
    {
        return m_pActiveLocalPlayer;
    };
    bool isPlayerReady(u8 uPlayerId);
    u8 getLocalPlayersCount(bool bCountAI);
    double getTurnTimer()
    {
        return m_fTurnTimer;
    };

    // Unit & skill management functions
    void setFocusUnit(Unit * unit);
    void doSkillEffect(Unit * pUnit, ChildEffect * pSkillEffect);
    void skillWasActivated(bool bOk, bool bOnResolve);
    void setSkillBeingActivatedOnResolve(Unit * pUnit, ChildEffect * pSkillEffect);
    ChildEffect * getSkillBeingActivated()
    {
        return m_pSkillBeingActivated;
    };
    Unit * getUnitActivatingSkill()
    {
        return m_pUnitActivatingSkill;
    };

    // Spell management functions
    void castSpell(Player * pPlayer, Spell * pSpell);
    void castSpellFinished(bool bCastOk, bool bOnResolve);
    void setSpellBeingCastOnResolve(Spell * pSpell);
    Spell * getSpellBeingCast()
    {
        return m_pSpellBeingCast;
    };
    void addExtraMana(u32 uType, bool bOk, const char * sCallback, Mana amount);

//  static LuaObject * deepFindLuaObject(u32 uId, ObjectList * pPlayers, Map * pMap); // This function searches in all game data, so don't use it when not necessary

private:
    LocalClient * m_pLocalClient;
    Player * m_pActiveLocalPlayer;
    Spell * m_pSpellBeingCast;
    Unit * m_pUnitActivatingSkill;
    ChildEffect * m_pSkillBeingActivated;
    bool m_bCanEOT;
    double m_fTurnTimer;
};

#endif
