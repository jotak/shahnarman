// -----------------------------------------------------------------
// DEBUG MANAGER
// Manage and display debug information
// -----------------------------------------------------------------

#include "DebugManager.h"
#include "../LocalClient.h"
#include "../errorcodes.h"
#include "../Data/IniFile.h"
#include "../Geometries/GeometryText.h"
#include "../Input/InputEngine.h"
#include "../Data/XMLLiteReader.h"

// -----------------------------------------------------------------
// Name : DebugManager
// -----------------------------------------------------------------
DebugManager::DebugManager(LocalClient * pLocalClient)
{
    m_pLocalClient = pLocalClient;
    for (int i = 0; i < DBG_MAX_LINES; i++)
        m_pGeometries[i] = NULL;
    m_pFPSGeometry = NULL;
    m_iFontId = INVALID_FONTID;
    m_iCustomInfoNbLines = 0;
    m_bShowFPS = true;
    FILE * f = NULL;
    fopen_s(&f, "logs/out.log", "w");
    if (f)
        fclose(f);
}

// -----------------------------------------------------------------
// Name : ~DebugManager
// -----------------------------------------------------------------
DebugManager::~DebugManager()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy DebugManager\n");
#endif
    for (int i = 0; i < DBG_MAX_LINES; i++)
    {
        if (m_pGeometries[i] != NULL)
            delete m_pGeometries[i];
    }
    if (m_pFPSGeometry != NULL)
        delete m_pFPSGeometry;
#ifdef DBG_VERBOSE1
    printf("End destroy DebugManager\n");
#endif
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void DebugManager::Init()
{
    m_refreshTC.start(0.2f);
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void DebugManager::Update(double delta)
{
    if (m_bShowFPS)
    {
        m_refreshTC.update(delta);
        if (m_refreshTC.getState() == TC_DelayReached)
        {
            float fFps = 999;
            if (delta != 0)
                fFps = 1 / delta;
            CoordsScreen cs = m_pLocalClient->getInput()->getCurrentCursorPosition();
            cs.z = BOARDPLANE;
            Coords3D c3 = m_pLocalClient->getDisplay()->get3DCoords(cs, DMS_3D);
            CoordsMap cm = m_pLocalClient->getDisplay()->getMapCoords(cs);
            Coords3D cam = m_pLocalClient->getDisplay()->getCamera();
            char sInfo[512];
            snprintf(sInfo, 512, "ScreenX=%d ; ScreenY=%d\n3dX=%.1f ; 3dY=%.1f\nMapX=%d ; MapY=%d\nCamX=%.1f ; CamY=%.1f ; CamZ=%.1f\nFPS : %.0f", cs.x, cs.y, c3.x, c3.y, cm.x, cm.y, cam.x, cam.y, cam.z, fFps);
            if (m_pFPSGeometry == NULL)
                m_pFPSGeometry = new GeometryText(sInfo, m_iFontId, VB_Static, m_pLocalClient->getDisplay());
            else
                m_pFPSGeometry->setText(sInfo, m_iFontId);
        }
    }
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void DebugManager::Display()
{
    int yPxl = 0;
    CoordsScreen coords;
    for (int i = 0; i < m_iCustomInfoNbLines; i++)
    {
        yPxl += 15;
        coords = CoordsScreen(5, yPxl, GUIPLANE);
        m_pGeometries[i]->display(coords, F_RGBA_NULL);
    }
    if (m_bShowFPS && m_pFPSGeometry != NULL)
    {
        coords = CoordsScreen(5, m_pLocalClient->getClientParameters()->screenYSize - 80, GUIPLANE);
        m_pFPSGeometry->display(coords, F_RGBA_NULL);
    }
}

// -----------------------------------------------------------------
// Name : addCustomeLine
// -----------------------------------------------------------------
void DebugManager::addCustomeLine(const char * sLine)
{
    if (m_pLocalClient->getDisplay()->isReady() && IS_VALID_FONTID(m_iFontId))
    {
        if (m_iCustomInfoNbLines < DBG_MAX_LINES - 1)
            m_pGeometries[m_iCustomInfoNbLines++] = new GeometryText(sLine, m_iFontId, VB_Static, m_pLocalClient->getDisplay());
        else if (m_iCustomInfoNbLines == DBG_MAX_LINES - 1)
            m_pGeometries[m_iCustomInfoNbLines++] = new GeometryText("Max number of lines reached!", m_iFontId, VB_Static, m_pLocalClient->getDisplay());
    }
    else
    {
        printf("%s", sLine);
        printf("\n");
    }
    FILE * f = NULL;
    fopen_s(&f, "logs/out.log", "a");
    if (f)
    {
        fputs(sLine, f);
        fputs("\n", f);
        fclose(f);
    }
    else
        printf("Can't open log file!\n");
}

// -----------------------------------------------------------------
// Name : log
// -----------------------------------------------------------------
void DebugManager::log(const char * sMsg)
{
    if (m_pLocalClient->getClientParameters()->iLogLevel > 0)
    {
        FILE * f = NULL;
        fopen_s(&f, "logs/out.log", "a");
        if (f)
        {
            fputs(sMsg, f);
            fputs("\n", f);
            fclose(f);
        }
        else
            printf("Can't open log file!\n");
    }
}

// -----------------------------------------------------------------
// Name : clear
// -----------------------------------------------------------------
void DebugManager::clear()
{
    m_iCustomInfoNbLines = 0;
    for (int i = 0; i < DBG_MAX_LINES; i++)
    {
        if (m_pGeometries[i] != NULL)
        {
            delete m_pGeometries[i];
            m_pGeometries[i] = NULL;
        }
    }
}

// -----------------------------------------------------------------
// Name : getErrorMessage
// -----------------------------------------------------------------
char * DebugManager::getErrorMessage(char * errorMsg, s16 errorCode)
{
    switch (errorCode)
    {
    case 0:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "No error");
        break;
    case TEX_FILENOTFOUND:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "TEX_FILENOTFOUND");
        break;
    case TEX_ERRORONREADING:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "TEX_ERRORONREADING");
        break;
    case TEX_INVALIDFORMAT:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "TEX_INVALIDFORMAT");
        break;
    case TEX_NOTLOADED:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "TEX_NOTLOADED");
        break;
    case TEX_PNGERROR:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "TEX_PNGERROR");
        break;
    case FNT_FILENOTFOUND:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "FNT_FILENOTFOUND");
        break;
    case FNT_ERRORONREADING:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "FNT_ERRORONREADING");
        break;
    case FNT_INVALIDFORMAT:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "FNT_INVALIDFORMAT");
        break;
    case FNT_NOTLOADED:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "FNT_NOTLOADED");
        break;
    default:
        wsafecpy(errorMsg, ERROR_MESSAGE_SIZE, "Unregistered error");
        break;
    }
    return errorMsg;
}

// -----------------------------------------------------------------
// Name : notifyErrorMessage
// -----------------------------------------------------------------
void DebugManager::notifyErrorMessage(s16 errorCode, const char * additionalInfo)
{
    char msg[ERROR_MESSAGE_SIZE + 128] = "";
    getErrorMessage(msg, errorCode);
    if (additionalInfo != NULL)
    {
        wsafecat(msg, ERROR_MESSAGE_SIZE + 128, " - ");
        wsafecat(msg, ERROR_MESSAGE_SIZE + 128, additionalInfo);
    }
    addCustomeLine(msg);
}

// -----------------------------------------------------------------
// Name : notifyErrorMessage
// -----------------------------------------------------------------
void DebugManager::notifyErrorMessage(const char * errorMsg)
{
    addCustomeLine(errorMsg);
}

// -----------------------------------------------------------------
// Name : notifyLoadingMessage
// -----------------------------------------------------------------
void DebugManager::notifyLoadingMessage(const char * msg)
{
    printf("\t");
    printf("%s", msg);
    printf("\n");
}

// -----------------------------------------------------------------
// Name : notifyINIErrorMessage
// -----------------------------------------------------------------
void DebugManager::notifyINIErrorMessage(const char * sFile, int errorCode)
{
    char sError[1024] = "";
    switch (errorCode)
    {
    case INIREADER_ERROR_CANT_OPEN_FILE:
        snprintf(sError, 1024, "%s: INIREADER_ERROR_CANT_OPEN_FILE.", sFile);
        break;
    default:
        snprintf(sError, 1024, "%s: Unknown error.", sFile);
        break;
    }
    addCustomeLine(sError);
}

// -----------------------------------------------------------------
// Name : notifyXMLErrorMessage
// -----------------------------------------------------------------
void DebugManager::notifyXMLErrorMessage(const char * sFile, int errorCode, int line, int col)
{
    char sError[1024] = "";
    switch (errorCode)
    {
    case XMLLITE_ERROR_ELEMENT_EXPECTED:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_ELEMENT_EXPECTED: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_EOF_NOT_EXPECTED:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_EOF_NOT_EXPECTED: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_LINEBREAK_IN_ELEMENT:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_LINEBREAK_IN_ELEMENT: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_ELEMENT_END_EXPECTED:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_ELEMENT_END_EXPECTED: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_LINEBREAK_IN_ATTRIBUTE: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_EQUAL_EXPECTED_IN_ATTRIBUTE: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_CLOSING_TAG_DOESNT_MATCH: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_CLOSING_TAG_EXPECTED:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_CLOSING_TAG_EXPECTED: line %d, col %d.", sFile, line, col);
        break;
    case XMLLITE_ERROR_CANT_OPEN_FILE:
        snprintf(sError, 1024, "%s: XMLLITE_ERROR_CANT_OPEN_FILE: line %d, col %d.", sFile, line, col);
        break;
    default:
        snprintf(sError, 1024, "%s: Unknown error: line %d, col %d.", sFile, line, col);
        break;
    }
    addCustomeLine(sError);
}

// -----------------------------------------------------------------
// Name : registerTextures
//  Static function
// -----------------------------------------------------------------
void DebugManager::registerTextures(TextureEngine * pTexEngine, FontEngine * pFontEngine)
{
    m_iFontId = pFontEngine->registerFont("BookAntiqua_16", pTexEngine);
}

#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../DeckData/Profile.h"
#include "../DeckData/AvatarData.h"
#include "../Server/Server.h"
#include "../Server/MapReader.h"
#include "../Server/TurnSolver.h"
#include "../GUIClasses/guiLabel.h"
#include "../Players/Player.h"
#include "../Players/Artifact.h"
#include "../Gameboard/Unit.h"
// -----------------------------------------------------------------
// Name : autoStartGame
// -----------------------------------------------------------------
void DebugManager::autoStartGame()
{
    // Build client data
    int nbClients = 1;
    ClientData * clients = new ClientData[nbClients];
    int iClient = 0;
    clients[iClient].bLocal = true;

    // Re-init map data
    MapReader * pMapReader = new MapReader(m_pLocalClient);
    pMapReader->init("standard.lua");
    ObjectList * pMapParameters = new ObjectList(true);
    pMapReader->getMapParameters(pMapParameters, LABEL_MAX_CHARS);

    int * pCustomParams = NULL;
    if (pMapParameters->size > 0)
        pCustomParams = new int[pMapParameters->size];

    // Map custom parameters
    int i = 0;
    MapReader::MapParameters * pParam = (MapReader::MapParameters*) pMapParameters->getFirst(0);
    while (pParam != NULL)
    {
        pCustomParams[i++] = pParam->defaultValueIndex;
        pParam = (MapReader::MapParameters*) pMapParameters->getNext(0);
    }

    // Init map generator (we will not delete it here, as the pointer now belong to Server object)
    pMapReader->setMapParameters(pCustomParams, pMapParameters->size, 2);
    delete[] pCustomParams;
    MapReader::deleteMapParameters(pMapParameters);
    delete pMapParameters;

    // Init server
    Server * pServer = m_pLocalClient->initServer("", 1, clients, pMapReader, -1, -1);
    delete[] clients;
    if (pServer == NULL)
    {
        notifyErrorMessage("Error: server could not be initialized.");
        return;
    }

    // Build players data
    ObjectList * pServerPlayers = pServer->getSolver()->getPlayersList();
    // Create neutral player
    char sName[NAME_MAX_CHARS];
    i18n->getText("NEUTRA", sName, NAME_MAX_CHARS);
    Player * pPlayer = new Player(0, 0, pServer->getSolver()->getGlobalSpellsPtr());
    wsafecpy(pPlayer->m_sProfileName, NAME_MAX_CHARS, sName);
    pPlayer->m_Color = rgb(0.5, 0.5, 0.5);
    wsafecpy(pPlayer->m_sBanner, 64, "blason1");
    pServer->getSolver()->setNeutralPlayer(pPlayer);
    // Human players
    int playerId = 1;
    for (int fdfdf = 0; fdfdf < 2; fdfdf++)
    {
        // Create player object
        pPlayer = new Player(playerId, 0, pServer->getSolver()->getGlobalSpellsPtr());
        snprintf(pPlayer->m_sProfileName, NAME_MAX_CHARS, "test%d", playerId);
        Profile * pProfile = m_pLocalClient->getDataFactory()->findProfile(pPlayer->m_sProfileName);
        AvatarData * pAvatar = (AvatarData*) pProfile->getAvatarsList()->getFirst(0);
        pPlayer->m_Color = rgb(1, 1, 1);
        pAvatar->getBanner(pPlayer->m_sBanner, 64);
        pServerPlayers->addLast(pPlayer);
        // Set Avatar
        CoordsMap pos = pMapReader->getPlayerPosition(playerId-1);
        pServer->getSolver()->setInitialAvatar(pAvatar->clone(m_pLocalClient), pPlayer, pos);
        // Add spells that are equipped
        Profile::SpellData * pSpellDesc = (Profile::SpellData*) pProfile->getSpellsList()->getFirst(0);
        while (pSpellDesc != NULL)
        {
            AvatarData * pOwner = pSpellDesc->m_pOwner;
            if (pOwner != NULL && strcmp(pAvatar->m_sEdition, pOwner->m_sEdition) == 0
                    && strcmp(pAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
                pServer->getSolver()->addInitialPlayerSpell(pPlayer, pSpellDesc->m_sEdition, pSpellDesc->m_sName);
            pSpellDesc = (Profile::SpellData*) pProfile->getSpellsList()->getNext(0);
        }
        // Add equipped artifacts
        Artifact * pArtifact = (Artifact*) pProfile->getArtifactsList()->getFirst(0);
        while (pArtifact != NULL)
        {
            AvatarData * pOwner = pArtifact->m_pOwner;
            if (pOwner != NULL && strcmp(pAvatar->m_sEdition, pOwner->m_sEdition) == 0
                    && strcmp(pAvatar->m_sObjectId, pOwner->m_sObjectId) == 0)
            {
                Unit * pAvatarInGame = pPlayer->getAvatar();
                assert(pAvatarInGame != NULL);
                ArtifactEffect * pEffect = (ArtifactEffect*) pArtifact->getArtifactEffects()->getFirst(0);
                while (pEffect != NULL)
                {
                    switch (pEffect->getType())
                    {
                    case ARTIFACT_EFFECT_CHARAC:
                    {
                        bool bFound = true;
                        long val = pAvatarInGame->getValue(((ArtifactEffect_Charac*)pEffect)->m_sKey, false, &bFound);
                        if (bFound)
                            pAvatarInGame->setBaseValue(((ArtifactEffect_Charac*)pEffect)->m_sKey, max(0, val + ((ArtifactEffect_Charac*)pEffect)->m_iModifier));
                        else
                        {
                            char sError[1024];
                            snprintf(sError, 1024, "Warning: artifact %s tries to modify characteristic that doesn't exist (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Charac*)pEffect)->m_sKey);
                            m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                        }
                        break;
                    }
                    case ARTIFACT_EFFECT_SPELL:
                    {
                        Spell * pSpell = m_pLocalClient->getDataFactory()->findSpell(((ArtifactEffect_Spell*)pEffect)->m_sSpellEdition, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                        if (pSpell != NULL)
                            pServer->getSolver()->addInitialPlayerSpell(pPlayer, ((ArtifactEffect_Spell*)pEffect)->m_sSpellEdition, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                        else
                        {
                            char sError[1024];
                            snprintf(sError, 1024, "Warning: artifact %s tries to add spell that doesn't exist (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName);
                            m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                        }
                        break;
                    }
                    case ARTIFACT_EFFECT_SKILL:
                    {
                        Skill * pSkill = new Skill(((ArtifactEffect_Skill*)pEffect)->m_sSkillEdition, ((ArtifactEffect_Skill*)pEffect)->m_sSkillName, ((ArtifactEffect_Skill*)pEffect)->m_sSkillParameters, pServer->getDebug());
                        if (pSkill != NULL && pSkill->isLoaded())
                            pAvatarInGame->addSkill(pSkill);
                        else
                        {
                            char sError[1024];
                            snprintf(sError, 1024, "Warning: artifact %s tries to add skill that doesn't exist or that can't be loaded (%s)", pArtifact->m_sObjectId, ((ArtifactEffect_Skill*)pEffect)->m_sSkillName);
                            m_pLocalClient->getDebug()->notifyErrorMessage(sError);
                        }
                        break;
                    }
                    }
                    pEffect = (ArtifactEffect*) pArtifact->getArtifactEffects()->getNext(0);
                }
            }
            pArtifact = (Artifact*) pProfile->getArtifactsList()->getNext(0);
        }
        playerId++;
    }
    delete pMapReader;
    pServer->onInitFinished();
}
