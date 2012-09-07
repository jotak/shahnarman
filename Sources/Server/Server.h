#ifndef _SERVER_H
#define _SERVER_H

#include "../Gameboard/Map.h"

// Network messages
//  Game general messages (1-50)
#define NETWORKMSG_CREATE_DATA                              1
#define NETWORKMSG_STOP_SERVER                              2
#define NETWORKMSG_CLIENT_QUITS                             3
#define NETWORKMSG_CUSTOM_LOG_MESSAGE                       4
#define NETWORKMSG_SAVE_DATA                                5
#define NETWORKMSG_GAME_OVER                                6
#define NETWORKMSG_CLIENT_INFORMATION                       7

//  Game objects update (51-150)
#define NETWORKMSG_PLAYER_STATE                             51
#define NETWORKMSG_SEND_PLAYER_MANA                         52
#define NETWORKMSG_SEND_PLAYER_UNITS_ORDERS                 53
#define NETWORKMSG_SEND_UNIT_DATA                           54
#define NETWORKMSG_DEAD_UNITS                               55
#define NETWORKMSG_UPDATE_PLAYER                            56
#define NETWORKMSG_SEND_CAST_SPELLS_DATA                    57
#define NETWORKMSG_BUILDING_BUILT                           58
#define NETWORKMSG_SEND_UNITS_GROUPS_DATA                   59
#define NETWORKMSG_SEND_TOWNS_DATA                          60
#define NETWORKMSG_SEND_TOWNS_ORDERS                        61
#define NETWORKMSG_SEND_INFLUENCE_DATA                      62
//#define NETWORKMSG_REMOVE_ACTIVE_EFFECTS                    63

//  Turn solver messages (151-250)
#define NETWORKMSG_RESOLVE_PHASE_BEGINS                     151
#define NETWORKMSG_RESOLVE_PHASE_ENDS                       152
#define NETWORKMSG_RESOLVING_SPELL_ORDERS                   153
#define NETWORKMSG_RESOLVING_MOVE_ORDERS                    154
#define NETWORKMSG_RESOLVE_SELECT_BATTLE                    156
#define NETWORKMSG_RESOLVE_OTHER_PLAYER_SELECTS_BATTLE      157
#define NETWORKMSG_RESOLVE_DIALOG_NO_MORE_BATTLE            158
#define NETWORKMSG_SET_RESOLVE_DIALOG_UNITS                 159
#define NETWORKMSG_RESOLVE_UNITS_CHOSEN                     160
#define NETWORKMSG_RESOLVE_NEUTRAL_AI                       161
#define NETWORKMSG_PROCESS_AI                               162
//#define NETWORKMSG_xxxxxxxxxxxxFREE_SLOTxxx               163
#define NETWORKMSG_RESOLVE_START_CAST_BATTLE_SPELL          164
#define NETWORKMSG_CAST_BATTLE_SPELLS                       165
#define NETWORKMSG_RESOLVE_START_CAST_POST_BATTLE_SPELL     166
#define NETWORKMSG_CAST_POST_BATTLE_SPELLS                  167
#define NETWORKMSG_RESOLVE_DIALOG_UPDATE_TOWNS              168
#define NETWORKMSG_RESOLVE_NEED_SELECT_TARGET               169
#define NETWORKMSG_RESOLVE_TARGET_SELECTED                  170

// LUA messages (251-500)
#define NETWORKMSG_CREATE_UNIT_DATA                         251
#define NETWORKMSG_CHILD_EFFECT_ATTACHED                    252
#define NETWORKMSG_CHILD_EFFECT_DETACHED                    253
#define NETWORKMSG_CUSTOM_LUA_UPDATE                        254
#define NETWORKMSG_DISCARD_SPELLS                           255
#define NETWORKMSG_DRAW_SPELLS                              256
#define NETWORKMSG_DEACTIVATE_SKILLS                        257
#define NETWORKMSG_ENABLE_ALL_EFFECTS                       258
#define NETWORKMSG_CHANGE_SPELL_OWNER                       259
#define NETWORKMSG_CHANGE_UNIT_OWNER                        260
#define NETWORKMSG_LUA_ATTACHED                             261
#define NETWORKMSG_LUA_DETACHED                             262
#define NETWORKMSG_ADD_SKILL                                263
#define NETWORKMSG_HIDE_SPECTILE                            264
#define NETWORKMSG_CHANGE_TERRAIN                           265
#define NETWORKMSG_MOVE_MAPOBJ                              266
#define NETWORKMSG_RESURRECT                                267
#define NETWORKMSG_CHANGE_TOWN_OWNER                        268
#define NETWORKMSG_ADD_MAGIC_CIRCLE                         269
#define NETWORKMSG_REMOVE_MAGIC_CIRCLE                      270
#define NETWORKMSG_RECALL_SPELLS                            271
#define NETWORKMSG_REMOVE_UNIT                              272

class DataFactory;
class TurnSolver;
class NetworkData;
class DebugManager;
class LocalClient;
class MapReader;

class ClientData
{
public:
  bool bLocal;
};

struct ClientNetworkData
{
  NetworkData * pData;
  int iClient;
};

class Server
{
public:
  // Constructor / destructor
  Server(LocalClient * pLocalClient);
  ~Server();

  bool Init(const wchar_t * sGameName, int nbClients, ClientData * clients, MapReader * pMapReader, int iTurnTimer, int iDeckSize);
  void Update(double delta);

  DataFactory * getFactory();
  TurnSolver * getSolver() { return m_pSolver; };
  void sendMessageToAllClients(NetworkData * pData);
  void sendMessageToAllExcept(int iClient, NetworkData * pData);
  void sendMessageTo(int iClient, NetworkData * pData);
  void receiveMessage(int iClient, NetworkData * pData);
  Map * getMap() { return &m_Map; };
  int getNbClients() { return m_iNbClients; };
  DebugManager * getDebug() { return m_pDebug; };
  bool isResolving();
  void onInitFinished();
  void saveGame();
  bool loadGame(const wchar_t * sGameName);
  void sendCustomLogToAll(const wchar_t * sMsgKey, u8 uLevel = 0, const wchar_t * sData = L"", ...);
  void gameOver(ObjectList * pTieList);
  bool isGameOver() { return m_bGameOver; };
  void addGarbage(BaseObject * pObj) { m_pGC->addLast(pObj); };
  int getMaxDeckSize() { return m_iMaxDeckSize; };

protected:
  void serializeMap(NetworkData * pData);
  void deserializeMap(NetworkData * pData);
  void serializePlayersData(NetworkData * pData);
  void serializeLuaTargets(NetworkData * pData);
  bool generateMap(MapReader * pMapReader);

  // Message processing
  void processNextMessage();
  void updatePlayerStatus(int iFromClient, NetworkData * pData);
  void updatePlayerUnitsOrder(NetworkData * pData);
  void updatePlayerUnitsGroups(NetworkData * pData);
  void updateTowns(NetworkData * pData);

  std::queue<ClientNetworkData> m_Queue;
  TurnSolver * m_pSolver;
  ClientData * m_pAllClients;
  LocalClient * m_pLocalClient;
  int m_iNbClients;
  DebugManager * m_pDebug;
  bool m_bGameOver;

  // Server parameters
  wchar_t m_sGameName[64];
  int m_iMaxDeckSize;

  // Permanent map data
  Map m_Map;
  ObjectList * m_pGC;
};

#endif
