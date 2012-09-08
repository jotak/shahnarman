// -----------------------------------------------------------------
// SERVER
// Gère le réseau
// -----------------------------------------------------------------
#include "Server.h"
#include "MapReader.h"
#include "TurnSolver.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../LocalClient.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/Town.h"
#include "../Gameboard/Building.h"
#include "../Gameboard/Temple.h"
#include "../Gameboard/SpecialTile.h"
#include "../Debug/DebugManager.h"
#include "../Data/LocalisationTool.h"
#include "../DeckData/AvatarData.h"
#include "../Interface/LogDlg.h"

// -----------------------------------------------------------------
// Name : Server
// -----------------------------------------------------------------
Server::Server(LocalClient * pLocalClient)
{
    m_pLocalClient = pLocalClient;
    m_pSolver = new TurnSolver(this);
    m_pDebug = pLocalClient->getDebug();
    m_pAllClients = NULL;
    m_bGameOver = false;
    m_pGC = new ObjectList(true);
    m_iNbClients = 0;
}

// -----------------------------------------------------------------
// Name : ~Server
// -----------------------------------------------------------------
Server::~Server()
{
    delete m_pGC;
    FREE(m_pSolver);
    if (m_pAllClients != NULL)
        delete[] m_pAllClients;
    // delete remaining messages
    while (!m_Queue.empty())
    {
        delete m_Queue.front().pData;
        m_Queue.pop();
    }
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
bool Server::Init(const char * sGameName, int nbClients, ClientData * clients, MapReader * pMapReader, int iTurnTimer, int iDeckSize)
{
    wsafecpy(m_sGameName, 64, sGameName);
    m_bGameOver = false;

    // init ClientData
    m_iNbClients = nbClients;
    m_pAllClients = new ClientData[m_iNbClients];
    memcpy(m_pAllClients, clients, m_iNbClients * sizeof(ClientData));

    m_iMaxDeckSize = iDeckSize;
    for (int i = 0; i < m_iNbClients; i++)
    {
        NetworkData msg(NETWORKMSG_CLIENT_INFORMATION);
        msg.addLong(i);
        msg.addLong(iTurnTimer);
        sendMessageTo(i, &msg);
    }

#ifdef DEBUG
    bool bLog = (m_pLocalClient->getClientParameters()->iLogLevel >= 2);
#endif

#ifdef DEBUG
    if (bLog)
        m_pDebug->log("m_pSolver->Init");
#endif
    m_pSolver->Init();

#ifdef DEBUG
    if (bLog)
        m_pDebug->log("generateMap");
#endif
    return generateMap(pMapReader);
}

// -----------------------------------------------------------------
// Name : generateMap
// -----------------------------------------------------------------
bool Server::generateMap(MapReader * pMapReader)
{
    // Generate map
    if (!pMapReader->generate())
        return false;
    //std::vector<TownData> * towns = pMapReader->getTowns();
    //int nbTowns = towns->size();
    //TownData * pTowns = NULL;
    //if (nbTowns > 0)
    //{
    //  pTowns = new TownData[nbTowns];
    //  for (int i = 0; i < nbTowns; i++)
    //    memcpy(&(pTowns[i]), &((*towns)[i]), sizeof(TownData));
    //}
    m_Map.createFromServer(pMapReader, m_pLocalClient);
    return true;
}

// -----------------------------------------------------------------
// Name : onInitFinished
// -----------------------------------------------------------------
void Server::onInitFinished()
{
    // Draw initial spells
#ifdef DEBUG
    bool bLog = (m_pLocalClient->getClientParameters()->iLogLevel >= 2);
#endif

#ifdef DEBUG
    if (bLog)
        m_pDebug->log("m_pSolver->drawInitialSpells");
#endif
    m_pSolver->drawInitialSpells();
    NetworkData data(NETWORKMSG_CREATE_DATA);
#ifdef DEBUG
    if (bLog)
        m_pDebug->log("serializeMap");
#endif
    serializeMap(&data);
#ifdef DEBUG
    if (bLog)
        m_pDebug->log("serializePlayersData");
#endif
    serializePlayersData(&data);
#ifdef DEBUG
    if (bLog)
        m_pDebug->log("serializeLuaTargets");
#endif
    serializeLuaTargets(&data);
    sendMessageToAllClients(&data);
#ifdef DEBUG
    if (bLog)
        m_pDebug->log("m_pSolver->onInitFinished");
#endif
    m_pSolver->onInitFinished();
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void Server::Update(double delta)
{
#ifdef DEBUG
    bool bLog = (m_pLocalClient->getClientParameters()->iLogLevel >= 3 || (m_pLocalClient->getClientParameters()->iLogLevel >= 2 && m_pSolver->getState() != RS_NotResolving));
    if (bLog)
    {
        m_pDebug->log("Server::Update");
        m_pDebug->log("m_pGC->deleteAll");
    }
#endif
    m_pGC->deleteAll();
    if (!m_bGameOver)
    {
#ifdef DEBUG
        if (bLog)
            m_pDebug->log("processNextMessage");
#endif
        processNextMessage();
#ifdef DEBUG
        if (bLog)
            m_pDebug->log("m_pSolver->Update");
#endif
        m_pSolver->Update(delta);
    }
}

// -----------------------------------------------------------------
// Name : getFactory
// -----------------------------------------------------------------
DataFactory * Server::getFactory()
{
    return m_pLocalClient->getDataFactory();
}

// -----------------------------------------------------------------
// Name : serializeMap
// -----------------------------------------------------------------
void Server::serializeMap(NetworkData * pData)
{
    int nbTowns = 0;
    int nbTemples = 0;
    pData->addLong(m_Map.getWidth());
    pData->addLong(m_Map.getHeight());
    for (int x = 0; x < m_Map.getWidth(); x++)
    {
        for (int y = 0; y < m_Map.getHeight(); y++)
        {
            MapTile * pTile = m_Map.getTileAt(CoordsMap(x, y));
            pData->addLong(pTile->m_uTerrainType);
            if (pTile->getFirstMapObject(GOTYPE_TOWN) != NULL)
                nbTowns++;
            if (pTile->getFirstMapObject(GOTYPE_TEMPLE) != NULL)
                nbTemples++;
            if (pTile->m_pSpecialTile == NULL)
                pData->addLong(0);
            else
            {
                pData->addLong(1);
                pTile->m_pSpecialTile->serialize(pData);
            }
        }
    }
    pData->addLong(nbTowns);
    Town * pTown = m_Map.getFirstTown();
    while (pTown != NULL)
    {
        pTown->serialize(pData);
        pTown = m_Map.getNextTown();
    }
    pData->addLong(nbTemples);
    Temple * pTemple = m_Map.getFirstTemple();
    while (pTemple != NULL)
    {
        pTemple->serialize(pData);
        pTemple = m_Map.getNextTemple();
    }
}

// -----------------------------------------------------------------
// Name : deserializeMap
// -----------------------------------------------------------------
void Server::deserializeMap(NetworkData * pData)
{
    m_Map.createFromNetwork(pData, m_pLocalClient);
}

// -----------------------------------------------------------------
// Name : serializePlayersData
// -----------------------------------------------------------------
void Server::serializePlayersData(NetworkData * pData)
{
    m_pSolver->getNeutralPlayer()->serialize(pData, false);
    pData->addLong(m_pSolver->getPlayersList()->size);
    Player * pPlayer = (Player*) m_pSolver->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
        pPlayer->serialize(pData, false);
        pPlayer->serializeSpells(pData);
        pPlayer = (Player*) m_pSolver->getPlayersList()->getNext(0);
    }
    m_pSolver->getNeutralPlayer()->serializeUnits(pData);
    pPlayer = (Player*) m_pSolver->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
        pPlayer->serializeUnits(pData);
        pPlayer = (Player*) m_pSolver->getPlayersList()->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : serializeLuaTargets
// -----------------------------------------------------------------
void Server::serializeLuaTargets(NetworkData * pData)
{
    // Browse all possible LuaObjects, and serialize with their targets using LuaContext.
    LuaContext context;
    NetworkData msg((long)0); // temporary data
    int countObjects = 0;
    Player * pPlayer = (Player*) m_pSolver->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
        context.pPlayer = pPlayer;
        // Spells
        Spell * pSpell = (Spell*) pPlayer->m_pActiveSpells->getFirst(0);
        while (pSpell != NULL)
        {
            if (pSpell->getTargets()->size > 0)
            {
                context.pLua = pSpell;
                context.serializeTargets(&msg);
                countObjects++;
            }
            pSpell = (Spell*) pPlayer->m_pActiveSpells->getNext(0);
        }
        // Unit skills
        Unit * pUnit = (Unit*) pPlayer->m_pUnits->getFirst(0);
        while (pUnit != NULL)
        {
            context.pUnit = pUnit;
            Skill * pSkill = (Skill*) pUnit->getSkillsRef()->getFirst(0);
            while (pSkill != NULL)
            {
                if (pSkill->getTargets()->size > 0)
                {
                    context.pLua = pSkill;
                    context.serializeTargets(&msg);
                    countObjects++;
                }
                pSkill = (Skill*) pUnit->getSkillsRef()->getNext(0);
            }
            pUnit = (Unit*) pPlayer->m_pUnits->getNext(0);
        }
        pPlayer = (Player*) m_pSolver->getPlayersList()->getNext(0);
    }
    // Town buidlings
    Town * pTown = m_Map.getFirstTown();
    while (pTown != NULL)
    {
        context.pTown = pTown;
        Building * pBuild = pTown->getFirstBuilding(0);
        while (pBuild != NULL)
        {
            if (pBuild->getTargets()->size > 0)
            {
                context.pLua = pBuild;
                context.serializeTargets(&msg);
                countObjects++;
            }
            pBuild = pTown->getNextBuilding(0);
        }
        pTown = m_Map.getNextTown();
    }
    // Special tiles
    SpecialTile * pSpec = m_Map.getFirstSpecialTile();
    while (pSpec != NULL)
    {
        if (pSpec->getTargets()->size > 0)
        {
            context.pLua = pSpec;
            context.serializeTargets(&msg);
            countObjects++;
        }
        pSpec = m_Map.getNextSpecialTile();
    }
    pData->addLong(countObjects);
    pData->concat(&msg);
}

// -----------------------------------------------------------------
// Name : sendMessageToAllClients
// -----------------------------------------------------------------
void Server::sendMessageToAllClients(NetworkData * pData)
{
    for (int i = 0; i < m_iNbClients; i++)
        sendMessageTo(i, pData);
}

// -----------------------------------------------------------------
// Name : sendMessageToAllExcept
// -----------------------------------------------------------------
void Server::sendMessageToAllExcept(int iClient, NetworkData * pData)
{
    for (int i = 0; i < m_iNbClients; i++)
    {
        if (i != iClient)
            sendMessageTo(i, pData);
    }
}

// -----------------------------------------------------------------
// Name : sendMessageTo
// -----------------------------------------------------------------
void Server::sendMessageTo(int iClient, NetworkData * pData)
{
    NetworkData * pData2 = pData->clone();
    if (m_pAllClients[iClient].bLocal)
        m_pLocalClient->receiveMessage(pData2);
    else
    {
    }
}

// -----------------------------------------------------------------
// Name : receiveMessage
// -----------------------------------------------------------------
void Server::receiveMessage(int iClient, NetworkData * pData)
{
    ClientNetworkData p;
    p.iClient = iClient;
    p.pData = pData;
    m_Queue.push(p);
}

// -----------------------------------------------------------------
// Name : processNextMessage
// -----------------------------------------------------------------
void Server::processNextMessage()
{
    if (!m_Queue.empty())
    {
        ClientNetworkData p = m_Queue.front();
        m_Queue.pop();
        NetworkData * pData = p.pData;
        int iClient = p.iClient;

        long iMessage = pData->readLong();
        switch (iMessage)
        {
        case NETWORKMSG_PLAYER_STATE:
            updatePlayerStatus(iClient, pData);
            break;
        case NETWORKMSG_SEND_PLAYER_UNITS_ORDERS:
            updatePlayerUnitsOrder(pData);
            break;
        case NETWORKMSG_SEND_CAST_SPELLS_DATA:
            m_pSolver->getSpellsSolver()->receiveSpells(pData);
            break;
        case NETWORKMSG_SEND_UNITS_GROUPS_DATA:
            updatePlayerUnitsGroups(pData);
            break;
        case NETWORKMSG_SEND_TOWNS_ORDERS:
            updateTowns(pData);
            break;
        case NETWORKMSG_RESOLVE_DIALOG_NO_MORE_BATTLE:
            m_pSolver->playerFinishedBattles();
            break;
        case NETWORKMSG_RESOLVE_SELECT_BATTLE:
            m_pSolver->playerSelectedBattle(pData);
            break;
        case NETWORKMSG_RESOLVE_UNITS_CHOSEN:
            m_pSolver->attackerChoosedUnits(pData);
            break;
        case NETWORKMSG_CAST_BATTLE_SPELLS:
        case NETWORKMSG_CAST_POST_BATTLE_SPELLS:
            m_pSolver->getSpellsSolver()->receiveInstantSpells(iClient, pData);
            break;
        case NETWORKMSG_RESOLVE_TARGET_SELECTED:
            m_pSolver->getSpellsSolver()->receiveTargetOnResolve(pData);
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
// Name : updatePlayerStatus
// -----------------------------------------------------------------
void Server::updatePlayerStatus(int iFromClient, NetworkData * pData)
{
    Player * pPlayer = m_pSolver->findPlayer((u8) pData->readLong());
    if (pPlayer != NULL)
    {
        PlayerState state = (PlayerState) pData->readLong();
        pPlayer->setState(state);
        NetworkData msg(NETWORKMSG_PLAYER_STATE);
        msg.addLong((long)pPlayer->m_uPlayerId);
        msg.addLong((long)state);
        sendMessageToAllExcept(iFromClient, &msg);
    }
}

// -----------------------------------------------------------------
// Name : updatePlayerUnitsOrder
// -----------------------------------------------------------------
void Server::updatePlayerUnitsOrder(NetworkData * pData)
{
    Player * pPlayer = m_pSolver->findPlayer((u8) pData->readLong());
    if (pPlayer != NULL)
    {
        while (pData->dataYetToRead() > 0)
        {
            Unit * pUnit = pPlayer->findUnit((u32) pData->readLong());
            if (pUnit != NULL)
            {
                UnitOrder order = (UnitOrder) pData->readLong();
                switch (order)
                {
                case OrderNone:
                    pUnit->unsetOrder();
                    break;
                case OrderFortify:
                    pUnit->setFortifyOrder();
                    break;
                case OrderMove:
                {
                    int x = (int) pData->readLong();
                    int y = (int) pData->readLong();
                    pUnit->setMoveOrder(CoordsMap(x,y), NULL);
                    break;
                }
                case OrderAttack:
                {
                    Player * pTargetPlayer = m_pSolver->findPlayer((u8) pData->readLong());
                    if (pTargetPlayer != NULL)
                    {
                        Unit * pTargetUnit = pTargetPlayer->findUnit((u32) pData->readLong());
                        if (pTargetUnit != NULL)
                            pUnit->setAttackOrder(pTargetUnit, NULL);
                        else
                            pUnit->unsetOrder();
                    }
                    else
                        pUnit->unsetOrder();
                    break;
                }
                case OrderSkill:
                {
                    pUnit->unsetOrder();
                    // Find skill
                    u32 uSkillId = (u32) pData->readLong();
                    int iEffectId = (int) pData->readLong();
                    bool bIsActive;
                    Skill * pSkill = pUnit->findSkill(uSkillId, &bIsActive);
                    if (pSkill != NULL && bIsActive)
                    {
                        ChildEffect * pEffect = pSkill->getChildEffect(iEffectId);
                        if (pEffect != NULL)
                        {
                            char params[LUA_FUNCTION_PARAMS_MAX_CHARS];
                            pData->readString(params);
                            wsafecpy(pEffect->sResolveParams, LUA_FUNCTION_PARAMS_MAX_CHARS, params);
                            pUnit->setSkillOrder(uSkillId, iEffectId);
                        }
                    }
                    break;
                }
                }
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : updatePlayerUnitsGroups
// -----------------------------------------------------------------
void Server::updatePlayerUnitsGroups(NetworkData * pData)
{
    Player * pPlayer = m_pSolver->findPlayer((u8) pData->readLong());
    if (pPlayer != NULL)
    {
        m_Map.resetAllPlayerGroups(pPlayer->m_uPlayerId);
        while (pData->dataYetToRead() > 0)
        {
            int nbUnits = (int) pData->readLong();
            MetaObjectList * pGroup = m_Map.createNewGroup();
            for (int i = 0; i < nbUnits; i++)
            {
                Unit * pUnit = pPlayer->findUnit((u32) pData->readLong());
                if (pUnit != NULL)
                {
                    pGroup->addFirst(pUnit);
                    pUnit->setGroup(pGroup);
                }
            }
        }
    }
}

// -----------------------------------------------------------------
// Name : updateTowns
// -----------------------------------------------------------------
void Server::updateTowns(NetworkData * pData)
{
    while (pData->dataYetToRead() > 0)
    {
        Town * pTown = m_Map.findTown((u32) pData->readLong());
        if (pTown != NULL)
            pTown->updateOrders(pData);
        else
        {
            m_pDebug->notifyErrorMessage("Server message error: town not found.");
            return; // return here, to avoid inconsistant data messing all up
        }
    }
}

// -----------------------------------------------------------------
// Name : isResolving
// -----------------------------------------------------------------
bool Server::isResolving()
{
    return (m_pSolver->getState() != RS_NotResolving);
}

// -----------------------------------------------------------------
// Name : sendCustomLogToAll
//  This function send a message to all client that they will log in the log screen.
//  The message contains a key to a string to be localized, and any custom data such as unit name, town name, spell name etc.
//  Parameter "sData" is a string where each character indicates the type of a custom data to use.
//    For instance, 'i' for integer, 'u' for unit, 'p' for player etc.
//  After sData, a variable list of parameters is got. They are needed to identify the custom data.
// -----------------------------------------------------------------
void Server::sendCustomLogToAll(const char * sMsgKey, u8 uLevel, const char * sData, ...)
{
    NetworkData data(NETWORKMSG_CUSTOM_LOG_MESSAGE);
    data.addString(sMsgKey);
    data.addLong(uLevel);
    data.addLong(strlen(sData));

    // Initialize structure to read variable list of parameters
    va_list pArgs;
    va_start(pArgs, sData);
    int i = 0;
    while (sData[i] != '\0')
    {
        switch (sData[i])
        {
        case 'i':  // integer
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, int));
            break;
        case 'S':  // edition spell
            data.addLong(sData[i]);
            data.addString(va_arg(pArgs, char*));  // spell edition
            data.addString(va_arg(pArgs, char*));  // spell name
            break;
        case 'A':  // edition artifact
            data.addLong(sData[i]);
            data.addString(va_arg(pArgs, char*));  // art. edition
            data.addString(va_arg(pArgs, char*));  // art. name
            break;
        case 'U':  // edition unit
            data.addLong(sData[i]);
            data.addString(va_arg(pArgs, char*));  // unit edition
            data.addString(va_arg(pArgs, char*));  // unit name
            break;
        case 'p':  // player
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, u8)); // player id
            break;
        case 's':  // spell
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, u8));   // player id
            data.addString(va_arg(pArgs, char*));  // spell location
            data.addLong((long) va_arg(pArgs, u32));  // spell id
            break;
        case 'u':  // unit
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, u8));   // player id
            data.addLong((long) va_arg(pArgs, u32));  // unit id
            break;
        case 't':  // town
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, u32));  // town id
            break;
        case 'b':  // building
            data.addLong(sData[i]);
            data.addLong((long) va_arg(pArgs, u32));  // town id
            data.addString(va_arg(pArgs, char*));  // building name
            break;
        case 'a':  // log action onclick
            data.addLong(sData[i]);
            long action = (long) va_arg(pArgs, u8);
            data.addLong(action); // action id
            switch (action)
            {
            case LOG_ACTION_ZOOMTO:
                data.addLong((long) va_arg(pArgs, int));  // x
                data.addLong((long) va_arg(pArgs, int));  // y
                break;
            case LOG_ACTION_TOWNSCREEN:
                data.addLong((long) va_arg(pArgs, u32));  // town id
                break;
            case LOG_ACTION_UNITSCREEN:
                data.addLong((long) va_arg(pArgs, u8));   // player id
                data.addLong((long) va_arg(pArgs, u32));  // unit id
                break;
            }
            break;
        }
        i++;
    }

    sendMessageToAllClients(&data);
}

// -----------------------------------------------------------------
// Name : saveGame
// -----------------------------------------------------------------
void Server::saveGame()
{
    char sFilePath[MAX_PATH];
    snprintf(sFilePath, MAX_PATH, "%s%s.sav", SAVES_PATH, m_sGameName);

    FILE * f = NULL;
    if (0 != fopen_s(&f, sFilePath, "wb"))
    {
        char sError[512] = "";
        snprintf(sError, 512, "Error: cannot open file %s for writing. Operation cancelled.", sFilePath);
        m_pLocalClient->getDebug()->notifyErrorMessage(sError);
        return;
    }

    NetworkData data(NETWORKMSG_SAVE_DATA);
    data.addString(m_sGameName);
    data.addLong(m_iNbClients);
    data.addCustom(m_pAllClients, m_iNbClients * sizeof(ClientData));
    serializeMap(&data);
    serializePlayersData(&data);
    serializeLuaTargets(&data);
    m_pSolver->serialize(&data);

    data.saveToFile(f);
    fclose(f);

    sendCustomLogToAll("GAME_SAVED_SUCCESS");
}

// -----------------------------------------------------------------
// Name : loadGame
// -----------------------------------------------------------------
bool Server::loadGame(const char * sGameName)
{
    char sFilePath[MAX_PATH];
    wsafecpy(m_sGameName, 64, sGameName);
    snprintf(sFilePath, MAX_PATH, "%s%s.sav", SAVES_PATH, m_sGameName);

    FILE * f = NULL;
    if (0 != fopen_s(&f, sFilePath, "rb"))
    {
        char sError[512] = "";
        snprintf(sError, 512, "Error: cannot open file %s for reading. Operation cancelled.", sFilePath);
        m_pLocalClient->getDebug()->notifyErrorMessage(sError);
        return false;
    }

    NetworkData data(f);
    fclose(f);
    data.readLong();  // Read but ignore message type
    data.readString(m_sGameName);
    m_iNbClients = data.readLong();
    m_pAllClients = new ClientData[m_iNbClients];
    data.readCustom(m_pAllClients);

    deserializeMap(&data);
    m_pSolver->deserializePlayersData(&data, m_pLocalClient, getMap());
    m_pSolver->deserializeLuaTargets(&data, m_pLocalClient, getMap());
    m_pSolver->Init();
    m_pSolver->deserialize(&data);

    // Send data
    NetworkData data2(NETWORKMSG_CREATE_DATA);
    serializeMap(&data2);
    serializePlayersData(&data2);
    serializeLuaTargets(&data2);
    sendMessageToAllClients(&data2);

    return true;
}

#define VICTORY_XP  40
#define TIE_XP      20
#define TOWNS_XP    20.0f
// -----------------------------------------------------------------
// Name : gameOver
// -----------------------------------------------------------------
void Server::gameOver(ObjectList * pTieList)
{
    m_bGameOver = true;
    NetworkData msg(NETWORKMSG_GAME_OVER);
    // Find winner(s)
    // Tie?
    if (pTieList != NULL)
    {
        int nbTies = pTieList->size;
        msg.addLong(nbTies);
        int iXP = TIE_XP / nbTies;
        Player * pPlayer = (Player*) pTieList->getFirst(0);
        while (pPlayer != NULL)
        {
            msg.addLong(pPlayer->m_uPlayerId);
            pPlayer->m_uXPGross = iXP;
            pPlayer = (Player*) pTieList->getNext(0);
        }
    }
    else
    {
        // There's a winner
        assert(m_pSolver->getPlayersList()->size == 1);
        Player * pPlayer = (Player*) m_pSolver->getPlayersList()->getFirst(0);
        assert(pPlayer != NULL);
        msg.addLong(1);
        msg.addLong(pPlayer->m_uPlayerId);
        pPlayer->m_uXPGross = VICTORY_XP;
    }
    // For next tasks, we'll build a temp list of players (both dead and alive)
    ObjectList * pPlayers = new ObjectList(false);
    Player * pPlayer = (Player*) m_pSolver->getPlayersList()->getFirst(0);
    while (pPlayer != NULL)
    {
        pPlayers->addLast(pPlayer);
        pPlayer = (Player*) m_pSolver->getPlayersList()->getNext(0);
    }
    pPlayer = (Player*) m_pSolver->getDeadPlayers()->getFirst(0);
    while (pPlayer != NULL)
    {
        pPlayers->addLast(pPlayer);
        pPlayer = (Player*) m_pSolver->getDeadPlayers()->getNext(0);
    }
    // Compute Towns XP
    u32 totalPoints = 0;
    pPlayer = (Player*) pPlayers->getFirst(0);
    while (pPlayer != NULL)
    {
        totalPoints += pPlayer->m_uXPTownPoints;
        pPlayer = (Player*) pPlayers->getNext(0);
    }
    if (totalPoints == 0)
        totalPoints = 1; // avoid division / 0 without changing score
    // Total towns points ok ; now calculate XP for each player
    pPlayer = (Player*) pPlayers->getFirst(0);
    while (pPlayer != NULL)
    {
        pPlayer->m_uXPGross += (int)(TOWNS_XP * (double)pPlayer->m_uXPTownPoints / (double)totalPoints);
        pPlayer = (Player*) pPlayers->getNext(0);
    }
    // For each player calculate net XP (depends on opponent's level)
    // Calculate sum of all levels
    u32 allLevels = 0;
    pPlayer = (Player*) pPlayers->getFirst(0);
    while (pPlayer != NULL)
    {
        allLevels += pPlayer->m_pAvatarData->getRealLevel();
        pPlayer = (Player*) pPlayers->getNext(0);
    }
    double averageLevel = (double)allLevels / (double)(pPlayers->size);
    // Update net XP
    pPlayer = (Player*) pPlayers->getFirst(0);
    while (pPlayer != NULL)
    {
        double coef = averageLevel / (double) (pPlayer->m_pAvatarData->getRealLevel());
        pPlayer->m_uXPNet = (u16) ((double)(pPlayer->m_uXPGross) * coef);
        pPlayer->m_pAvatarData->m_uXP += pPlayer->m_uXPNet;
        pPlayer = (Player*) pPlayers->getNext(0);
    }
    // Notify clients
    pPlayer = (Player*) pPlayers->getFirst(0);
    while (pPlayer != NULL)
    {
        msg.addLong(pPlayer->m_uPlayerId);
        msg.addLong(pPlayer->m_uXPGross);
        msg.addLong(pPlayer->m_uXPNet);
        msg.addLong(pPlayer->m_pAvatarData->m_uXP);
        msg.addLong(pPlayer->m_iWonGold);
        msg.addLong(pPlayer->m_pWonSpells->size);
        Spell * pSpell = (Spell*) pPlayer->m_pWonSpells->getFirst(0);
        while (pSpell != NULL)
        {
            msg.addString(pSpell->getObjectEdition());
            msg.addString(pSpell->getObjectName());
            pSpell = (Spell*) pPlayer->m_pWonSpells->getNext(0);
        }
        msg.addLong(pPlayer->m_pWonArtifacts->size);
        Artifact * pArtifact = (Artifact*) pPlayer->m_pWonArtifacts->getFirst(0);
        while (pArtifact != NULL)
        {
            msg.addString(pArtifact->getEdition());
            msg.addString(pArtifact->m_sObjectId);
            pArtifact = (Artifact*) pPlayer->m_pWonArtifacts->getNext(0);
        }
        msg.addLong(pPlayer->m_pWonAvatars->size);
        AvatarData * pAvatar = (AvatarData*) pPlayer->m_pWonAvatars->getFirst(0);
        while (pAvatar != NULL)
        {
            msg.addString(pAvatar->m_sEdition);
            msg.addString(pAvatar->m_sObjectId);
            pAvatar = (AvatarData*) pPlayer->m_pWonAvatars->getNext(0);
        }
        pPlayer = (Player*) pPlayers->getNext(0);
    }
    sendMessageToAllClients(&msg);
    // Free mem
    delete pPlayers;
}
