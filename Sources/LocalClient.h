#ifndef _LOCALCLIENT_H
#define _LOCALCLIENT_H

#include "utils.h"

class DisplayEngine;
class InputEngine;
class GameboardManager;
class InterfaceManager;
class FxManager;
class AudioManager;
class DebugManager;
class PlayerManager;
class Parameters;
class DataFactory;
class Server;
class Player;
class NetworkData;
class ClientData;
class MapReader;

// Messages
#define MSG_NONE                  0
#define MSG_REQUEST_EXIT          1
#define MSG_SERVER_WAS_CREATED    2
#define MSG_SERVER_WAS_STOPPED    3

enum GameStep
{
    GS_InMenus = 0,
    GS_InitialisingGame,
    GS_GameIntro,
    GS_InGame,
    GS_GameOver
};

class LocalClient
{
public:
    // Constructor / destructor
    LocalClient();
    ~LocalClient();

    // Init / Update / Display
    void Init();
    void Update(double delta);
    void Display();

    // Engines access
    DisplayEngine * getDisplay()
    {
        return m_pDisplayEngine;
    };
    InputEngine * getInput()
    {
        return m_pInputEngine;
    };

    // Managers access
    GameboardManager * getGameboard()
    {
        return m_pGameboardManager;
    };
    InterfaceManager * getInterface()
    {
        return m_pInterfaceManager;
    };
    FxManager * getFx()
    {
        return m_pFxManager;
    };
    AudioManager * getAudio()
    {
        return m_pAudioManager;
    };
    DebugManager * getDebug()
    {
        return m_pDebugManager;
    };
    PlayerManager * getPlayerManager()
    {
        return m_pPlayerManager;
    };
    u8 getClientId()
    {
        return m_uClientId;
    };

    // Other modules access
    Parameters * getClientParameters()
    {
        return m_pClientParameters;
    };
    DataFactory * getDataFactory()
    {
        return m_pDataFactory;
    };

    // Server related functions
    Server * initServer(const char * sGameName, int nbClients, ClientData * clients, MapReader * pMapReader, int iTurnTimer, int iDeckSize);
    Server * loadServer(const char * sGameName);
    Server * getServer()
    {
        return m_pServer;
    };
    void receiveMessage(NetworkData * pData);
    void sendMessage(NetworkData * pData);

    // Messages
    short getMessage()
    {
        short msg = m_iMessage;
        m_iMessage = MSG_NONE;
        return msg;
    };
    void requestExit()
    {
        m_iMessage = MSG_REQUEST_EXIT;
    };

    // Other
    void waitLocalPlayer();
    void startPlayerTurn(Player * nextPlayer);
    GameStep getGameStep()
    {
        return m_GameStep;
    };
    void endGame();
    int getTurnTimer()
    {
        return m_iTurnTimer;
    };
    int getTurn()
    {
        return m_iTurn;
    };

protected:
    void processNextMessage();
    void createGameData(NetworkData * pData);
    void gameOver(NetworkData * pData);
    void logCustomMessage(NetworkData * pData);

    std::queue<NetworkData*> m_Queue;
    GameStep m_GameStep;
    short m_iMessage;

    // Engines
    DisplayEngine * m_pDisplayEngine;
    InputEngine * m_pInputEngine;

    // Managers
    GameboardManager * m_pGameboardManager;
    InterfaceManager * m_pInterfaceManager;
    FxManager * m_pFxManager;
    AudioManager * m_pAudioManager;
    DebugManager * m_pDebugManager;
    PlayerManager * m_pPlayerManager;

    // Other modules
    Parameters * m_pClientParameters;
    DataFactory * m_pDataFactory;

    // Server
    Server * m_pServer;
    bool m_bIsHostingServer;
    u8 m_uClientId;

    // Other
    bool m_bGameInitialized;
    int m_iTurnTimer;
    int m_iTurn;
};

#endif
