// -----------------------------------------------------------------
// LOCAL CLIENT
// Gère les managers clients
// -----------------------------------------------------------------
#include "LocalClient.h"
#include "Data/LocalisationTool.h"

// Low level (engines)
#include "Display/DisplayEngine.h"
#include "Input/PCInputEngine.h"

// High level (managers)
#include "Gameboard/GameboardManager.h"
#include "Interface/InterfaceManager.h"
#include "Fx/FxManager.h"
#include "Audio/AudioManager.h"
#include "Debug/DebugManager.h"
#include "Players/PlayerManager.h"

// Other modules
#include "Data/Parameters.h"
#include "Data/DataFactory.h"

// Server
#include "Server/Server.h"
#include "Server/MapReader.h"
#include "Server/TurnSolver.h"

// Other
#include "Players/Player.h"
#include "Players/Spell.h"
#include "Players/Artifact.h"
#include "Gameboard/Unit.h"
#include "Gameboard/Town.h"
#include "Gameboard/Building.h"
#include "DeckData/AvatarData.h"
#include "DeckData/Profile.h"
#include "DeckData/Edition.h"
#include "Interface/ResolveDlg.h"
#include "Interface/LogDlg.h"
#include "Interface/InfoboxDlg.h"

//#define DEBUG_AUTOSTART_LOOP    50
#ifdef DEBUG_AUTOSTART_LOOP
  int iDbgLoop = DEBUG_AUTOSTART_LOOP;
  bool bWaitEnd = false;
#endif

// -----------------------------------------------------------------
// Name : LocalClient
// -----------------------------------------------------------------
LocalClient::LocalClient()
{
  // Engines
  m_pDisplayEngine = new DisplayEngine();
  m_pInputEngine = new PCInputEngine();

  // Managers
  m_pGameboardManager = new GameboardManager(this);
  m_pInterfaceManager = new InterfaceManager(this);
  m_pFxManager = new FxManager(this);
  m_pAudioManager = AudioManager::getInstance();
  m_pDebugManager = new DebugManager(this);
  m_pPlayerManager = new PlayerManager(this);

  // Other modules
  m_pClientParameters = new Parameters();
  m_pDataFactory = new DataFactory();
  i18n;

  m_pServer = NULL;
  m_bIsHostingServer = false;
  m_bGameInitialized = false;
  m_GameStep = GS_InitialisingGame;
  m_iMessage = MSG_NONE;
  m_uClientId = 0;
}

// -----------------------------------------------------------------
// Name : ~LocalClient
// -----------------------------------------------------------------
LocalClient::~LocalClient()
{
  FREE(m_pGameboardManager);
  FREE(m_pInterfaceManager);
  FREE(m_pFxManager);
  FREE(m_pAudioManager);
  FREE(m_pDebugManager);
  FREE(m_pPlayerManager);
  FREE(m_pClientParameters);
  FREE(m_pInputEngine);
  FREE(m_pDataFactory);
  FREE(m_pServer);
  FREE(m_pDisplayEngine);
  delete i18n;
  // delete remaining messages
  while (!m_Queue.empty())
  {
    delete m_Queue.front();
    m_Queue.pop();
  }
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void LocalClient::Init()
{
  m_GameStep = GS_InMenus;
  m_iMessage = MSG_NONE;

  // Init modules / managers / engines
  m_pDebugManager->Init();
  m_pClientParameters->Init(m_pDebugManager);

#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 2);
#endif

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->Init");
#endif
  m_pDisplayEngine->Init(m_pClientParameters, m_pDebugManager);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->getFontEngine()->resetAllFonts");
#endif
  m_pDisplayEngine->getFontEngine()->resetAllFonts();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("i18n->Init");
#endif
  i18n->Init(m_pClientParameters, m_pDebugManager);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pAudioManager->Init");
#endif
  m_pAudioManager->Init(this);

  // Register textures
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("guiObject::registerTextures");
#endif
  guiObject::registerTextures(m_pDisplayEngine->getTextureEngine(), m_pDisplayEngine->getFontEngine());
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDebugManager->registerTextures");
#endif
  m_pDebugManager->registerTextures(m_pDisplayEngine->getTextureEngine(), m_pDisplayEngine->getFontEngine());

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDataFactory->Init");
#endif
  m_pDataFactory->Init(this);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInterfaceManager->Init");
#endif
  m_pInterfaceManager->Init();

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->setReady");
#endif
  m_pDisplayEngine->setReady();
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void LocalClient::Update(double delta)
{
#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 3 || (m_pClientParameters->iLogLevel >= 2 && m_pServer != NULL && m_pServer->getSolver() != NULL && m_pServer->getSolver()->getState() != RS_NotResolving));
  if (bLog)
    m_pDebugManager->log("LocalClient::Update");
#endif

  if (m_GameStep == GS_GameIntro)
  {
    m_pDebugManager->Update(delta);
    m_pInterfaceManager->Update(delta);
    m_pAudioManager->Update(delta);
    m_pFxManager->Update(delta);
    if (m_pFxManager->isGameIntroFinished())
      m_GameStep = GS_InGame;
    return;
  }

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("processNextMessage");
#endif
  processNextMessage();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInputEngine->update");
#endif
  m_pInputEngine->update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDebugManager->Update");
#endif
  m_pDebugManager->Update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInterfaceManager->Update");
#endif
  m_pInterfaceManager->Update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pAudioManager->Update");
#endif
  m_pAudioManager->Update(delta);

  if (m_GameStep == GS_InGame)
  {
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pPlayerManager->Update");
#endif
    m_pPlayerManager->Update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->Update");
#endif
    m_pGameboardManager->Update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pFxManager->Update");
#endif
    m_pFxManager->Update(delta);

#ifdef DEBUG_AUTOSTART_LOOP
    if (iDbgLoop > 0)
    {
      endGame();
      bWaitEnd = false;
  FILE * f = NULL;
  fopen_s(&f, "logs/out.log", "w");
  if (f)
    fclose(f);
    }
#endif
  }
  else if (m_GameStep == GS_GameOver)
  {
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->Update");
#endif
    m_pGameboardManager->Update(delta);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pFxManager->Update");
#endif
    m_pFxManager->Update(delta);
  }
#ifdef DEBUG_AUTOSTART_LOOP
  else {
    if (!bWaitEnd && iDbgLoop > 0)
    {
      getDebug()->autoStartGame();
      iDbgLoop--;
      bWaitEnd = true;
    }
  }
#endif
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void LocalClient::Display()
{
#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 3 || (m_pClientParameters->iLogLevel >= 2 && m_pServer != NULL && m_pServer->getSolver() != NULL && m_pServer->getSolver()->getState() != RS_NotResolving));
  if (bLog)
    m_pDebugManager->log("LocalClient::Display");
#endif
  if (!m_pDisplayEngine->isReady())
    return;

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->beginDisplay");
#endif
  m_pDisplayEngine->beginDisplay();
  switch (m_GameStep)
  {
    case GS_InitialisingGame:
      break;
    case GS_InMenus:
    {
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->begin2D");
#endif
      m_pDisplayEngine->begin2D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInterfaceManager->Display");
#endif
      m_pInterfaceManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDebugManager->Display");
#endif
      m_pDebugManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->end2D");
#endif
      m_pDisplayEngine->end2D();
      break;
    }
    case GS_GameIntro:
    case GS_InGame:
    case GS_GameOver:
    {
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->begin2D");
#endif
      m_pDisplayEngine->begin2D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->displayBackground");
#endif
      m_pGameboardManager->displayBackground();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->end2D");
#endif
      m_pDisplayEngine->end2D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->begin3D");
#endif
      m_pDisplayEngine->begin3D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->Display");
#endif
      m_pGameboardManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pFxManager->Display");
#endif
      m_pFxManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->end3D");
#endif
      m_pDisplayEngine->end3D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->begin2D");
#endif
      m_pDisplayEngine->begin2D();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInterfaceManager->Display");
#endif
      m_pInterfaceManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pFxManager->displayHUD");
#endif
      m_pFxManager->displayHUD();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDebugManager->Display");
#endif
      m_pDebugManager->Display();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->end2D");
#endif
      m_pDisplayEngine->end2D();
      break;
    }
  }
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->endDisplay");
#endif
  m_pDisplayEngine->endDisplay();
}

// -----------------------------------------------------------------
// Name : initServer
// -----------------------------------------------------------------
Server * LocalClient::initServer(const char * sGameName, int nbClients, ClientData * clients, MapReader * pMapReader, int iTurnTimer, int iDeckSize)
{
#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 2);
#endif

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("new Server");
#endif
  FREE(m_pServer);
  m_pServer = new Server(this);
  m_bIsHostingServer = true;
  m_uClientId = 0;
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pServer->Init");
#endif
  bool bOk = m_pServer->Init(sGameName, nbClients, clients, pMapReader, iTurnTimer, iDeckSize);
  if (!bOk)
  {
    FREE(m_pServer);
    m_bIsHostingServer = false;
    return NULL;
  }
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->setMapData");
#endif
  m_pDisplayEngine->setMapData(pMapReader->getMapWidth(), pMapReader->getMapHeight());
  // other initialisations will be done after receiving CREATE_DATA message
  return m_pServer;
}

// -----------------------------------------------------------------
// Name : loadServer
// -----------------------------------------------------------------
Server * LocalClient::loadServer(const char * sGameName)
{
#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 2);
#endif

#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("new Server");
#endif
  FREE(m_pServer);
  m_pServer = new Server(this);
  m_bIsHostingServer = true;
  m_uClientId = 0;
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pServer->loadGame");
#endif
  m_pServer->loadGame(sGameName);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pDisplayEngine->setMapData");
#endif
  m_pDisplayEngine->setMapData(m_pServer->getMap()->getWidth(), m_pServer->getMap()->getHeight());
  // other initialisations will be done after receiving CREATE_DATA message
  return m_pServer;
}

// -----------------------------------------------------------------
// Name : receiveMessage
// -----------------------------------------------------------------
void LocalClient::receiveMessage(NetworkData * pData)
{
  m_Queue.push(pData);
}

// -----------------------------------------------------------------
// Name : processNextMessage
// -----------------------------------------------------------------
void LocalClient::processNextMessage()
{
  if (!m_Queue.empty())
  {
    NetworkData * pData = m_Queue.front();
    m_Queue.pop();
    long iMessage = pData->readLong();
    switch (iMessage)
    {
      case NETWORKMSG_CLIENT_INFORMATION:
        m_uClientId = (u8) pData->readLong();
        m_iTurnTimer = (int) pData->readLong();
        break;
      case NETWORKMSG_CREATE_DATA:
        createGameData(pData);
        break;
      case NETWORKMSG_CREATE_UNIT_DATA:
        m_pPlayerManager->createUnitData(pData);
        break;
      case NETWORKMSG_PLAYER_STATE:
        m_pPlayerManager->setPlayerState(pData);
        break;
      case NETWORKMSG_UPDATE_PLAYER:
        {
          u8 uPlayer = (u8) pData->readLong();
          Player * pPlayer = (Player*) m_pPlayerManager->findPlayer(uPlayer);
          assert(pPlayer != NULL);
          pPlayer->deserialize(pData, true, this);
          m_pInterfaceManager->getInfoDialog()->updatePlayersState();
          break;
        }
      case NETWORKMSG_SEND_PLAYER_MANA:
        m_pPlayerManager->setPlayerMana(pData);
        break;
      case NETWORKMSG_DRAW_SPELLS:
        m_pPlayerManager->drawSpells(pData);
        break;
      case NETWORKMSG_RECALL_SPELLS:
        m_pPlayerManager->recallSpells(pData);
        break;
      case NETWORKMSG_DISCARD_SPELLS:
        m_pPlayerManager->discardSpells(pData);
        break;
//      case NETWORKMSG_REMOVE_ACTIVE_EFFECTS:
//        m_pPlayerManager->removeActiveEffects(pData);
//        break;
      case NETWORKMSG_SEND_CAST_SPELLS_DATA:
        m_pPlayerManager->updateCastSpellData(pData);
        break;
      case NETWORKMSG_LUA_ATTACHED:
        m_pPlayerManager->onLuaAttached(pData);
        break;
      case NETWORKMSG_LUA_DETACHED:
        m_pPlayerManager->onLuaDetached(pData);
        break;
      case NETWORKMSG_CHILD_EFFECT_ATTACHED:
        m_pPlayerManager->onChildEffectAttached(pData);
        break;
      case NETWORKMSG_CHILD_EFFECT_DETACHED:
        m_pPlayerManager->onChildEffectDetached(pData);
        break;
      case NETWORKMSG_CUSTOM_LUA_UPDATE:
        m_pPlayerManager->onCustomLuaUpdate(pData);
        break;
      case NETWORKMSG_RESOLVE_PHASE_BEGINS:
        getGameboard()->setBattleMode();
        m_pInterfaceManager->showResolveDialog();
        break;
      case NETWORKMSG_RESOLVE_PHASE_ENDS:
        m_iTurn++;
        m_pPlayerManager->setResolutionIdx(pData);
        m_pPlayerManager->updateMagicCirclePositions();
        getGameboard()->unsetBattleMode();
        m_pInterfaceManager->hideResolveDialog();
        m_pInterfaceManager->getLogDialog()->logNewTurn();
        waitLocalPlayer();
        break;
      case NETWORKMSG_PROCESS_AI:
      case NETWORKMSG_RESOLVE_NEUTRAL_AI:
      case NETWORKMSG_RESOLVING_SPELL_ORDERS:
      case NETWORKMSG_RESOLVING_MOVE_ORDERS:
      case NETWORKMSG_RESOLVE_SELECT_BATTLE:
      case NETWORKMSG_RESOLVE_OTHER_PLAYER_SELECTS_BATTLE:
      case NETWORKMSG_SET_RESOLVE_DIALOG_UNITS:
      case NETWORKMSG_RESOLVE_START_CAST_BATTLE_SPELL:
      case NETWORKMSG_RESOLVE_START_CAST_POST_BATTLE_SPELL:
      case NETWORKMSG_RESOLVE_DIALOG_UPDATE_TOWNS:
      case NETWORKMSG_RESOLVE_NEED_SELECT_TARGET:
        m_pInterfaceManager->getResolveDialog()->onMessage(iMessage, pData);
        break;
      case NETWORKMSG_SEND_UNIT_DATA:
        m_pPlayerManager->updateUnitsData(pData);
        break;
      case NETWORKMSG_DEAD_UNITS:
        m_pPlayerManager->updateDeadUnits(pData);
        break;
      case NETWORKMSG_SEND_TOWNS_DATA:
        getGameboard()->updateTownsData(pData);
        break;
      case NETWORKMSG_SEND_INFLUENCE_DATA:
        getGameboard()->updateTilesInfluence(pData);
        break;
      case NETWORKMSG_DEACTIVATE_SKILLS:
        m_pPlayerManager->deactivateSkills(pData);
        break;
      case NETWORKMSG_ENABLE_ALL_EFFECTS:
        m_pPlayerManager->enableAllEffects(pData);
        break;
      case NETWORKMSG_CHANGE_SPELL_OWNER:
        m_pPlayerManager->changeSpellOwner(pData);
        break;
      case NETWORKMSG_CHANGE_UNIT_OWNER:
        m_pPlayerManager->changeUnitOwner(pData);
        break;
      case NETWORKMSG_CHANGE_TOWN_OWNER:
        m_pPlayerManager->changeTownOwner(pData);
        break;
      case NETWORKMSG_ADD_SKILL:
        m_pPlayerManager->addSkillToUnit(pData);
        break;
      case NETWORKMSG_HIDE_SPECTILE:
        m_pPlayerManager->hideSpecialTile(pData);
        break;
      case NETWORKMSG_BUILDING_BUILT:
        m_pPlayerManager->buildBuilding(pData);
        break;
      case NETWORKMSG_CHANGE_TERRAIN:
        {
          int x = (int) pData->readLong();
          int y = (int) pData->readLong();
          u8 uType = (u8) pData->readLong();
          m_pGameboardManager->getMap()->changeTerrainType(CoordsMap(x, y), uType, NULL);
          break;
        }
      case NETWORKMSG_MOVE_MAPOBJ:
        m_pPlayerManager->teleport(pData);
        break;
      case NETWORKMSG_RESURRECT:
        m_pPlayerManager->resurrectUnit(pData);
        break;
      case NETWORKMSG_REMOVE_UNIT:
        m_pPlayerManager->removeUnit(pData);
        break;
      case NETWORKMSG_ADD_MAGIC_CIRCLE:
        m_pPlayerManager->addMagicCircle(pData);
        break;
      case NETWORKMSG_REMOVE_MAGIC_CIRCLE:
        m_pPlayerManager->removeMagicCircle(pData);
        break;
      case NETWORKMSG_CUSTOM_LOG_MESSAGE:
        logCustomMessage(pData);
        break;
      case NETWORKMSG_GAME_OVER:
        gameOver(pData);
        break;
      default:
        {
          char sError[512];
          snprintf(sError, 512, "Unknown message sent to server: code %d", (int)iMessage);
          getDebug()->notifyErrorMessage(sError);
          break;
        }
    }
    delete pData;
  }
}

// -----------------------------------------------------------------
// Name : sendMessage
// -----------------------------------------------------------------
void LocalClient::sendMessage(NetworkData * pData)
{
  NetworkData * pData2 = pData->clone();
  if (m_bIsHostingServer)
    m_pServer->receiveMessage(m_uClientId, pData2);
  else
  {
  }
}

// -----------------------------------------------------------------
// Name : createGameData
// -----------------------------------------------------------------
void LocalClient::createGameData(NetworkData * pData)
{
#ifdef DEBUG
  bool bLog = (m_pClientParameters->iLogLevel >= 2);
#endif

  m_pAudioManager->stopMusic();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->getMap()->createFromNetwork");
#endif
  m_pGameboardManager->getMap()->createFromNetwork(pData, this);
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pPlayerManager->deserializePlayersData");
#endif
  m_pPlayerManager->deserializePlayersData(pData, this, m_pGameboardManager->getMap());
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pPlayerManager->deserializeLuaTargets");
#endif
  m_pPlayerManager->deserializeLuaTargets(pData, this, m_pGameboardManager->getMap());
  // Init other managers
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pFxManager->Init");
#endif
  m_pFxManager->Init();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pGameboardManager->Init");
#endif
  m_pGameboardManager->Init();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pPlayerManager->Init");
#endif
  m_pPlayerManager->Init();
#ifdef DEBUG
  if (bLog)
    m_pDebugManager->log("m_pInterfaceManager->InitGame");
#endif
  m_pInterfaceManager->InitGame();
  m_iTurn = 0;
  m_iMessage = MSG_SERVER_WAS_CREATED;
  m_GameStep = GS_GameIntro;
  m_pAudioManager->playMusic(getRandom(2) == 1 ? MUSIC_INGAME1 : MUSIC_INGAME2);
}

// -----------------------------------------------------------------
// Name : waitLocalPlayer
// -----------------------------------------------------------------
void LocalClient::waitLocalPlayer()
{
  m_pFxManager->unzoom();
  m_pGameboardManager->disableNoPlayer();
  m_pInterfaceManager->disableNoPlayer();
  m_pInterfaceManager->waitLocalPlayer();
}

// -----------------------------------------------------------------
// Name : startPlayerTurn
// -----------------------------------------------------------------
void LocalClient::startPlayerTurn(Player * nextPlayer)
{
  m_pGameboardManager->enableNextPlayer(nextPlayer);
  m_pInterfaceManager->enableNextPlayer(nextPlayer);
  m_pPlayerManager->setNextPlayerReady(nextPlayer);
}

// -----------------------------------------------------------------
// Name : endGame
// -----------------------------------------------------------------
void LocalClient::endGame()
{
  // delete any messages
  while (!m_Queue.empty())
  {
    delete m_Queue.front();
    m_Queue.pop();
  }
  if (m_bIsHostingServer)
  {
    NetworkData data(NETWORKMSG_STOP_SERVER);
    m_pServer->sendMessageToAllExcept(m_uClientId, &data);
    FREE(m_pServer);
    m_bIsHostingServer = false;
  }
  else
  {
    NetworkData data(NETWORKMSG_CLIENT_QUITS);
    sendMessage(&data);
  }
  FREE(m_pGameboardManager);
  m_pGameboardManager = new GameboardManager(this);
  FREE(m_pPlayerManager);
  m_pPlayerManager = new PlayerManager(this);
  FREE(m_pFxManager);
  m_pFxManager = new FxManager(this);
  m_pInterfaceManager->InitMenu();
  m_iMessage = MSG_SERVER_WAS_STOPPED;
  m_GameStep = GS_InMenus;
}

// -----------------------------------------------------------------
// Name : gameOver
// -----------------------------------------------------------------
void LocalClient::gameOver(NetworkData * pData)
{
  m_GameStep = GS_GameOver;
  int nbWinners = pData->readLong();
  ObjectList * pWinners = new ObjectList(false);
  for (int i = 0; i < nbWinners; i++)
  {
    int iPlayer = pData->readLong();
    Player * pPlayer = getPlayerManager()->findPlayer(iPlayer);
    assert(pPlayer != NULL);
    pWinners->addLast(pPlayer);
  }
  while (pData->dataYetToRead() > 0)
  {
    int iPlayer = pData->readLong();
    Player * pPlayer = getPlayerManager()->findPlayer(iPlayer);
    assert(pPlayer != NULL);
    pPlayer->m_uXPGross = pData->readLong();
    pPlayer->m_uXPNet = pData->readLong();
    pPlayer->m_pAvatarData->m_uXP = pData->readLong();
    pPlayer->m_iWonGold = pData->readLong();
    int size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      char sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      char sName[NAME_MAX_CHARS];
      pData->readString(sName);
      Spell * pSpell = m_pDataFactory->findSpell(sEdition, sName);
      assert(pSpell != NULL);
      pPlayer->m_pWonSpells->addLast(pSpell);
    }
    size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      char sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      char sName[NAME_MAX_CHARS];
      pData->readString(sName);
      Edition * pEdition = m_pDataFactory->findEdition(sEdition);
      assert(pEdition != NULL);
      Artifact * pArtifact = pEdition->findArtifact(sName);
      assert(pArtifact != NULL);
      pPlayer->m_pWonArtifacts->addLast(pArtifact);
    }
    size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
      char sEdition[NAME_MAX_CHARS];
      pData->readString(sEdition);
      char sName[NAME_MAX_CHARS];
      pData->readString(sName);
      UnitData * pData = m_pDataFactory->getUnitData(sEdition, sName);
      assert(pData != NULL);
      pPlayer->m_pWonAvatars->addLast(pData);
    }
  }
  // Save avatars
  Player * pPlayer = (Player*) getPlayerManager()->getPlayersList()->getFirst(0);
  while (pPlayer != NULL)
  {
    if (pPlayer->m_uClientId == getClientId() && !pPlayer->m_bIsAI)
    {
      // Problem here: the "AvatarData" we have is a cloned version of the one contained in Profile.
      //  So we have to retrieve the corresponding profile, and replace the old AvatarData by the new one.
      // Retrieve owner
      Profile * pProfile = getDataFactory()->findProfile(pPlayer->m_sProfileName);
      pProfile->replaceAvatar(pPlayer->m_pAvatarData->clone(this)); // clone because it's going to be deleted by Player's destructor
      pProfile->addCash(pPlayer->m_iWonGold);
      Spell * pSpell = (Spell*) pPlayer->m_pWonSpells->getFirst(0);
      while (pSpell != NULL)
      {
        pProfile->addSpell(pSpell->getObjectEdition(), pSpell->getObjectName());
        pSpell = (Spell*) pPlayer->m_pWonSpells->getNext(0);
      }
      Artifact * pArtifact = (Artifact*) pPlayer->m_pWonArtifacts->getFirst(0);
      while (pArtifact != NULL)
      {
        pProfile->addArtifact(pArtifact->getEdition(), pArtifact->m_sObjectId);
        pArtifact = (Artifact*) pPlayer->m_pWonArtifacts->getNext(0);
      }
      AvatarData * pAvatar = (AvatarData*) pPlayer->m_pWonAvatars->getFirst(0);
      while (pAvatar != NULL)
      {
        pProfile->addAvatar(pAvatar->m_sEdition, pAvatar->m_sObjectId);
        pAvatar = (AvatarData*) pPlayer->m_pWonAvatars->getNext(0);
      }
      pProfile->save();
    }
    pPlayer = (Player*) getPlayerManager()->getPlayersList()->getNext(0);
  }
  getInterface()->showGameOverDialog(pWinners);
}

// -----------------------------------------------------------------
// Name : logCustomMessage
// -----------------------------------------------------------------
void LocalClient::logCustomMessage(NetworkData * pData)
{
  char sMsgKey[256];
  char sMsg[512];
  char sNotFound[64] = "[not found]";
  u8 uAction = LOG_ACTION_NONE;
  void * pAction = NULL;
  CoordsMap cm;
  pData->readString(sMsgKey);
  u8 uLevel = pData->readLong();
  int nbData = (int) pData->readLong();
  if (nbData > 0)
  {
    void ** pPhraseArgs = new void*[nbData];
    int * integers = new int[nbData];
    for (int i = 0; i < nbData; i++)
    {
      switch (pData->readLong())
      {
      case 'i': // integer
        {
          integers[i] = (int)pData->readLong();
          pPhraseArgs[i] = &(integers[i]);
          break;
        }
      case 'S': // edition spell
        {
          char sEdition[NAME_MAX_CHARS];
          char sName[NAME_MAX_CHARS];
          pData->readString(sEdition);
          pData->readString(sName);
          Spell * pSpell = m_pDataFactory->findSpell(sEdition, sName);
          assert(pSpell != NULL);
          pPhraseArgs[i] = pSpell->getLocalizedName();
          break;
        }
      case 'A': // edition artifact
        {
          char sEdition[NAME_MAX_CHARS];
          char sName[NAME_MAX_CHARS];
          pData->readString(sEdition);
          pData->readString(sName);
          Edition * pEdition = m_pDataFactory->findEdition(sEdition);
          assert(pEdition != NULL);
          Artifact * pArtifact = pEdition->findArtifact(sName);
          assert(pArtifact != NULL);
          pArtifact->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
          pPhraseArgs[i] = sName;
          break;
        }
      case 'U': // edition unit
        {
          char sEdition[NAME_MAX_CHARS];
          char sName[NAME_MAX_CHARS];
          pData->readString(sEdition);
          pData->readString(sName);
          UnitData * pData = m_pDataFactory->getUnitData(sEdition, sName);
          assert(pData != NULL);
          pData->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
          pPhraseArgs[i] = sName;
          break;
        }
      case 'p': // player
        {
          u8 uPlayerId = (u8) pData->readLong();
          Player * pPlayer = m_pPlayerManager->findPlayer(uPlayerId);
          assert(pPlayer != NULL);
          pPhraseArgs[i] = pPlayer->getAvatarName();
          break;
        }
      case 's': // spell
        {
          u8 uPlayerId = (u8) pData->readLong();
          Player * pPlayer = m_pPlayerManager->findPlayer(uPlayerId);
          assert(pPlayer != NULL);
          char sSrc[16];
          pData->readString(sSrc);
          ObjectList * pList = NULL;
          if (strcmp(sSrc, "hand") == 0)
            pList = pPlayer->m_pHand;
          else if (strcmp(sSrc, "deck") == 0)
            pList = pPlayer->m_pDeck;
          else if (strcmp(sSrc, "active") == 0)
            pList = pPlayer->m_pActiveSpells;
          else if (strcmp(sSrc, "discard") == 0)
            pList = pPlayer->m_pDiscard;
          assert(pList != NULL);
          Spell * pSpell = pPlayer->findSpell(0, (u32) pData->readLong(), pList);
          assert(pSpell != NULL);
          pPhraseArgs[i] = pSpell->getLocalizedName();
          break;
        }
      case 'u': // unit
        {
          u8 uPlayerId = (u8) pData->readLong();
          Player * pPlayer = m_pPlayerManager->findPlayer(uPlayerId);
          assert(pPlayer != NULL);
          Unit * pUnit = pPlayer->findUnit((u32) pData->readLong());
          assert(pUnit != NULL);
          pPhraseArgs[i] = pUnit->getName();
          break;
        }
      case 't': // town
        {
          u32 uTownId = (u32) pData->readLong();
          Town * pTown = m_pGameboardManager->getMap()->findTown(uTownId);
          assert(pTown != NULL);
          pPhraseArgs[i] = pTown->getName();
          break;
        }
      case 'b': // building
        {
          u32 uTownId = (u32) pData->readLong();
          Town * pTown = m_pGameboardManager->getMap()->findTown(uTownId);
          assert(pTown != NULL);
          char sName[NAME_MAX_CHARS];
          pData->readString(sName);
          pPhraseArgs[i] = sNotFound;
          Building * pBuild = pTown->getFirstBuilding(0);
          while (pBuild != NULL)
          {
            if (strcmp(pBuild->getObjectName(), sName) == 0)
            {
              pPhraseArgs[i] = pBuild->getLocalizedName();
              break;
            }
            pBuild = pTown->getNextBuilding(0);
          }
          break;
        }
      case 'a': // define click action on log
        {
          i--;  // because pPhraseArgs is not used here
          uAction = (u8) pData->readLong();
          switch (uAction)
          {
          case LOG_ACTION_ZOOMTO:
            {
              cm.x = pData->readLong();
              cm.y = pData->readLong();
              pAction = &cm;
              break;
            }
          case LOG_ACTION_TOWNSCREEN:
            {
              u32 uTownId = (u32) pData->readLong();
              pAction = m_pGameboardManager->getMap()->findTown(uTownId);
              assert(pAction != NULL);
              break;
            }
          case LOG_ACTION_UNITSCREEN:
            {
              u8 uPlayerId = (u8) pData->readLong();
              Player * pPlayer = m_pPlayerManager->findPlayer(uPlayerId);
              assert(pPlayer != NULL);
              pAction = pPlayer->findUnit((u32) pData->readLong());
              assert(pAction != NULL);
              break;
            }
          }
          break;
        }
      }
    }
    i18n->getText(sMsgKey, sMsg, 512, pPhraseArgs);
    delete[] pPhraseArgs;
    delete[] integers;
  }
  else
    i18n->getText(sMsgKey, sMsg, 512);
  getInterface()->getLogDialog()->log(sMsg, uLevel, uAction, pAction);
}
