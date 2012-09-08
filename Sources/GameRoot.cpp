// -----------------------------------------------------------------
// GAME ROOT
// Main game loop ; just dispatch to LocalClient and, if not NULL, to Server
// -----------------------------------------------------------------
#include "GameRoot.h"
#include "Server/Server.h"
#include "LocalClient.h"

// -----------------------------------------------------------------
// Name : GameRoot
// Constructor
// -----------------------------------------------------------------
GameRoot::GameRoot()
{
//  m_pServer = NULL;
    m_pLocalClient = new LocalClient();
}

// -----------------------------------------------------------------
// Name : ~GameRoot
// -----------------------------------------------------------------
GameRoot::~GameRoot()
{
    FREE(m_pLocalClient);
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void GameRoot::Init()
{
    m_pLocalClient->Init();
}

// -----------------------------------------------------------------
// Name : Update
// -----------------------------------------------------------------
void GameRoot::Update(double delta)
{
#ifdef DEBUG
    if (delta > 1.0f) // probably being debugged step by step
        delta = 0.05f;  // put some kind of consistant value
#endif
    Server * pServer = m_pLocalClient->getServer();
    if (pServer != NULL)
        pServer->Update(delta);
    m_pLocalClient->Update(delta);

    switch (m_pLocalClient->getMessage())
    {
    case MSG_REQUEST_EXIT:
        exit(0);
        break;
        //case MSG_SERVER_WAS_CREATED:
        //  m_pServer = m_pLocalClient->getServer();
        //  break;
        //case MSG_SERVER_WAS_STOPPED:
        //  m_pServer = NULL;
        //  break;
    default:
        break;
    }
}

// -----------------------------------------------------------------
// Name : Display
// -----------------------------------------------------------------
void GameRoot::Display()
{
    m_pLocalClient->Display();
}
