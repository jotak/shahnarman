#ifndef _PLAYER_H
#define _PLAYER_H

#include "Mana.h"
#include "../Data/LuaTargetable.h"

#define STRING_NBDRAWN      "spells_drawn"
#define STRING_MAXSPELLS    "max_spells"
#define STRING_SPELLSRANGE  "spells_range"
#define STRING_MANA_LIFE    "mana_life"
#define STRING_MANA_LAW     "mana_law"
#define STRING_MANA_DEATH   "mana_death"
#define STRING_MANA_CHAOS   "mana_chaos"

#define MAX_MAGIC_CIRCLES   100

class Unit;
class Spell;
class Town;
class DisplayEngine;
class GeometryQuads;
class NetworkData;
class LocalClient;
class Map;
class AvatarData;

enum PlayerState
{
    waiting,
    playing,
    finished,
    dead,
    connection_error
};

class Player : public BaseObject, public LuaTargetable
{
public:
    Player(u8 playerId, u8 clientId, ObjectList ** pGlobalEffects);
    virtual ~Player();

    virtual void serialize(NetworkData * pData, bool bShort);
    virtual void deserialize(NetworkData * pData, bool bShort, LocalClient * pLocalClient);
    virtual void serializeSpells(NetworkData * pData);
    virtual void deserializeSpells(NetworkData * pData, LocalClient * pLocalClient);
    virtual void serializeUnits(NetworkData * pData);
    virtual void deserializeUnits(NetworkData * pData, Map * pMap, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData);

    virtual void init(DisplayEngine * pDisplay);
    virtual void update(double delta);
    virtual char * getInfo(char * sBuf, int iSize);

    PlayerState getState()
    {
        return m_State;
    };
    void setState(PlayerState state)
    {
        m_State = state;
    };
    void resetState()
    {
        m_State = waiting;
    };

    void setAvatar(AvatarData * pAvatarData, Unit * pUnit);
    Unit * getAvatar();
    char * getAvatarName();
    int getAvatarTexture();

    // Spells
    void castSpell(Spell * pSpell);
    Spell * findSpell(int _it, u32 id, ObjectList * pList);

    // Units management
    Unit * getFirstUnplayedUnit();
    Unit * getNextUnplayedUnit();
    Unit * findUnit(u32 uUnit);

    // Other
    void shuffleDeck();
    Mana getMana();
    u8 getMana(u8 uColor);
    Mana getBaseMana();
    u8 getBaseMana(u8 uColor);
    void setBaseMana(Mana mana);
    void setBaseMana(u8 uColor, u8 value);

    u8 m_uPlayerId;
    u8 m_uClientId;
    char m_sProfileName[NAME_MAX_CHARS];
    GeometryQuads * m_pBannerGeometry;
    char m_sBanner[64];
    int m_iBannerTex;
    F_RGBA m_Color;
    bool m_bIsAI;
    ObjectList * m_pUnits;
    ObjectList * m_pDeadUnits;
    ObjectList * m_pDeck;
    ObjectList * m_pHand;
    ObjectList * m_pActiveSpells;
    ObjectList * m_pDiscard;
    Mana m_SpentMana;
    CoordsMap m_MagicCirclePos[MAX_MAGIC_CIRCLES];
    Town * m_pCapitalTown;
    AvatarData * m_pAvatarData;

    // Post-game data
    u32 m_uXPTownPoints;
    u8 m_uXPGross;
    u16 m_uXPNet;
    int m_iWonGold;
    ObjectList * m_pWonSpells;
    ObjectList * m_pWonArtifacts;
    ObjectList * m_pWonAvatars;

    // Used by server only during resolve phase
    ObjectList * m_pCastSpells;

protected:
    Unit * m_pAvatar;
    PlayerState m_State;
    double m_fTurnTimer;
};

#endif
