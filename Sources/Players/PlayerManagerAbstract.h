#ifndef _PLAYERMANAGER_ABSTRACT_H
#define _PLAYERMANAGER_ABSTRACT_H

#include "../utils.h"

class Player;
class ObjectList;
class NetworkData;
class LocalClient;
class Map;
class LuaTargetable;

#define RELINK_TYPE_ATTACKING_UNIT        1
//#define RELINK_TYPE_EFFECT_TARGET_UNIT    2
//#define RELINK_TYPE_EFFECT_TARGET_PLAYER  3

class PlayerManagerAbstract
{
public:
  // Constructor / destructor
  PlayerManagerAbstract();
  ~PlayerManagerAbstract();

  // Manager functions
  void Update(double delta);

  // Retrieve data from server
  void deserializePlayersData(NetworkData * pData, LocalClient * pLocalClient, Map * pMap);
  void deserializeLuaTargets(NetworkData * pData, LocalClient * pLocalClient, Map * pMap);

  // Player management functions
  u8 getPlayersCount();
  ObjectList * getPlayersList() { return m_pPlayersList; };
  ObjectList * getDeadPlayers() { return m_pDeadPlayers; };
  Player * findPlayer(u8 uPlayerId);
  u8 getFirstResolutionIdx() { return m_uFirstResolutionListIdx; };
  ObjectList * getGlobalSpells() { return m_pGlobalSpells; };
  ObjectList ** getGlobalSpellsPtr() { return &m_pGlobalSpells; };
  void setNeutralPlayer(Player * pPlayer) { m_pNeutralPlayer = pPlayer; };
  Player * getNeutralPlayer() { return m_pNeutralPlayer; };
  Player * getFirstPlayerAndNeutral(int _it);
  Player * getNextPlayerAndNeutral(int _it);

  // Other
  LuaTargetable * findTargetFromIdentifiers(long iType, wchar_t * sIds, Map * pMap);
  wchar_t * retrieveTargetsNames(wchar_t * sBuf, int iSize, wchar_t * sIds, Map * pMap);

protected:
  u8 m_uFirstResolutionListIdx;
  ObjectList * m_pPlayersList;
  ObjectList * m_pDeadPlayers;
  Player * m_pNeutralPlayer;
  ObjectList * m_pGlobalSpells;
};

#endif
