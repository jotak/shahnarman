#ifndef _LUA_CONTEXT_H
#define _LUA_CONTEXT_H

#include "../utils.h"

class PlayerManagerAbstract;
class NetworkData;
class Player;
class LuaObject;
class Unit;
class Town;
class Map;

class LuaContext
{
public:
  LuaContext() { pLua = NULL; pPlayer = NULL; pUnit = NULL; pTown = NULL; };
  bool retrieve(PlayerManagerAbstract * pMngr);
  void serialize(NetworkData * pData);
  bool deserialize(NetworkData * pData, PlayerManagerAbstract * pMngr, Map * pMap);
  void serializeTargets(NetworkData * pData);
  bool deserializeTargets(NetworkData * pData, PlayerManagerAbstract * pMngr, Map * pMap);

  LuaObject * pLua;
  Player * pPlayer;
  Unit * pUnit;
  Town * pTown;
  char sError[256];
};

#endif
