// -----------------------------------------------------------------
// SERVER DATA FACTORY
// -----------------------------------------------------------------
#include "DataFactory.h"
#include "Parameters.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../DeckData/Edition.h"
#include "../DeckData/Profile.h"
#include "../DeckData/AvatarData.h"

#define MAX_EDITIONS    100

// -----------------------------------------------------------------
// Name : DataFactory
// -----------------------------------------------------------------
DataFactory::DataFactory()
{
  m_pEditions = new ObjectList(true);
  m_pAllProfiles = new ObjectList(true);
  m_pGameAvatarsData = new ObjectList(false);
}

// -----------------------------------------------------------------
// Name : ~DataFactory
// -----------------------------------------------------------------
DataFactory::~DataFactory()
{
  delete m_pEditions;
  delete m_pAllProfiles;
  delete m_pGameAvatarsData;
}

// -----------------------------------------------------------------
// Name : Init
// -----------------------------------------------------------------
void DataFactory::Init(LocalClient * pLocalClient)
{
  // Find editions
  // Open active editions file
  pLocalClient->getClientParameters()->resetLocale(pLocalClient->getDebug());
  wchar_t sFile[MAX_PATH];
  FILE * pFile = NULL;
  swprintf(sFile, MAX_PATH, L"%sactive.txt", EDITIONS_PATH);
  if (0 != wfopen(&pFile, sFile, L"r"))
  {
    pLocalClient->getDebug()->notifyErrorMessage(L"Error: active.txt file not found.");
    return;
  }

  // First read editions from active.txt
  while (!feof(pFile))
  {
    wchar_t sName[NAME_MAX_CHARS];
    if (NULL == fgetws(sName, NAME_MAX_CHARS, pFile))
      break;
    wchop(sName);
    // Test if edition exists
    wchar_t sFilePath[MAX_PATH];
    swprintf(sFilePath, MAX_PATH, L"%s%s/edition.xml", EDITIONS_PATH, sName);
    FILE * pTmp = NULL;
    if (0 == wfopen(&pTmp, sFilePath, L"r"))
    {
      // Ok => assume edition is valid
      fclose(pTmp);
      Edition * pEdition = new Edition(sName, pLocalClient);
      m_pEditions->addLast(pEdition);
      pEdition->activate(pLocalClient->getDebug());
    }
  }
  fclose(pFile);

  // Then read other (deactivated) editions
  // Init temporary list
  wchar_t ** sEditionsList;
  sEditionsList = new wchar_t*[MAX_EDITIONS];
  for (int i = 0; i < MAX_EDITIONS; i++)
    sEditionsList[i] = new wchar_t[NAME_MAX_CHARS];

  // Start process
  int nbEditions = getEditions(sEditionsList, MAX_EDITIONS, NAME_MAX_CHARS);
  for (int i = 0; i < nbEditions; i++)
  {
    if (findEdition(sEditionsList[i]) == NULL)  // add this edition only if it wasn't added previously
    {
      Edition * pEdition = new Edition(sEditionsList[i], pLocalClient);
      m_pEditions->addLast(pEdition);
    }
  }

  for (int i = 0; i < MAX_EDITIONS; i++)
    delete[] sEditionsList[i];
  delete[] sEditionsList;

  // Load profiles list
  wchar_t ** sProfilesList;
  sProfilesList = new wchar_t*[100];
  for (int i = 0; i < 100; i++)
    sProfilesList[i] = new wchar_t[MAX_PATH];

  int nbProfiles = getProfiles(sProfilesList, 100, MAX_PATH);
  for (int i = 0; i < nbProfiles; i++)
  {
    Profile * profile = new Profile(pLocalClient);
    profile->load(sProfilesList[i]);
    m_pAllProfiles->addLast(profile);
  }

  for (int i = 0; i < 100; i++)
    delete[] sProfilesList[i];
  delete[] sProfilesList;
}

// -----------------------------------------------------------------
// Name : getUnitData
// -----------------------------------------------------------------
UnitData * DataFactory::getUnitData(wchar_t * sEdition, wchar_t * strId)
{
  // Game avatar?
  AvatarData * pAvatar = (AvatarData*) m_pGameAvatarsData->getFirst(0);
  while (pAvatar != NULL)
  {
    if (wcscmp(sEdition, pAvatar->m_sEdition) == 0 && wcscmp(strId, pAvatar->m_sObjectId) == 0)
      return pAvatar;
    pAvatar = (AvatarData*) m_pGameAvatarsData->getNext(0);
  }

  Edition * pEdition = findEdition(sEdition);
  if (pEdition != NULL)
    return pEdition->findUnitData(strId);
  return NULL;
}

// -----------------------------------------------------------------
// Name : findSpell
// -----------------------------------------------------------------
Spell * DataFactory::findSpell(wchar_t * sEdition, wchar_t * sName)
{
  Edition * pEdition = findEdition(sEdition);
  if (pEdition != NULL)
    return pEdition->findSpell(sName);
  return NULL;
}

// -----------------------------------------------------------------
// Name : findEdition
// -----------------------------------------------------------------
Edition * DataFactory::findEdition(wchar_t * sId)
{
  // Look in loaded editions
  Edition * pEdition = (Edition*) m_pEditions->getFirst(0);
  while (pEdition != NULL)
  {
    if (wcscmp(sId, pEdition->m_sObjectId) == 0)
      return pEdition;
    pEdition = (Edition*) m_pEditions->getNext(0);
  }

  // Not found
  return NULL;
}

// -----------------------------------------------------------------
// Name : findProfile
// -----------------------------------------------------------------
Profile * DataFactory::findProfile(wchar_t * sName)
{
  Profile * pProfile = (Profile*) m_pAllProfiles->getFirst(0);
  while (pProfile != NULL)
  {
    if (wcscmp(sName, pProfile->getName()) == 0)
      return pProfile;
    pProfile = (Profile*) m_pAllProfiles->getNext(0);
  }
  // Not found
  return NULL;
}

// -----------------------------------------------------------------
// Name : onProfileAdded
// -----------------------------------------------------------------
void DataFactory::onProfileAdded(Profile * pProfile)
{
  m_pAllProfiles->addLast(pProfile);
}

// -----------------------------------------------------------------
// Name : onProfileDeleted
// -----------------------------------------------------------------
void DataFactory::onProfileDeleted(Profile * pProfile)
{
  m_pAllProfiles->deleteObject(pProfile, true);
}

// -----------------------------------------------------------------
// Name : resetGameAvatarData
// -----------------------------------------------------------------
void DataFactory::resetGameAvatarData()
{
  m_pGameAvatarsData->deleteAll();
}

// -----------------------------------------------------------------
// Name : addGameAvatarData
// -----------------------------------------------------------------
void DataFactory::addGameAvatarData(AvatarData * pAvatar)
{
  m_pGameAvatarsData->addLast(pAvatar);
}
