#include "Edition.h"
#include "../Debug/DebugManager.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "ShopItem.h"
#include "../Data/XMLLiteReader.h"
#include "../GUIClasses/guiSmartSlider.h"
#include "AvatarData.h"
#include "Ethnicity.h"
#include "ProgressionTree.h"
#include "ProgressionElement.h"
#include "Profile.h"
#include "ShahmahCreation.h"
#include "AIData.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../Gameboard/SpecialTile.h"
#include "../Common/ObjectList.h"
#include "../LocalClient.h"
#include <locale.h>

// -----------------------------------------------------------------
// Name : Edition
//  Constructor
// -----------------------------------------------------------------
Edition::Edition(wchar_t * sName, LocalClient * pLocalClient)
{
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, sName);
  wsafecpy(m_sLocale, 32, L"en_EN");
  wsafecpy(m_sVersion, 16, L"n/a");
  m_bActive = false;
  m_pAIs = new ObjectList(true);
  m_pUnits = new ObjectList(true);
  m_pSpells = new ObjectList(true);
  m_pSpecTiles = new ObjectList(true);
  m_pArtifacts = new ObjectList(true);
  m_pSkillNames = new ObjectList(true);
  m_pEthnicities = new ObjectList(true);
  m_pDependencies = new ObjectList(false);
  m_pProgressionTrees = new ObjectList(true);
  m_iTotalFreq = 0;
  m_pShahmahCreation = NULL;

  wchar_t loadingMsg[128] = L"";
  swprintf_s(loadingMsg, 128, L"Edition: %s", sName);
  pLocalClient->getDebug()->notifyLoadingMessage(loadingMsg);

  // Read edition.xml
  XMLLiteReader reader;
  wchar_t sFileName[MAX_PATH];
  swprintf_s(sFileName, MAX_PATH, L"%s%s/edition.xml", EDITIONS_PATH, m_sObjectId);
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pLocalClient->getDebug());
  if (pRootNode != NULL)
  {
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    if (pNode != NULL && 0 == _wcsicmp(pNode->getName(), L"edition"))
    {
      XMLLiteAttribute * pLocaleAttr = pNode->getAttributeByName(L"locale");
      if (pLocaleAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"locale\" attribute in node edition. Check out file %s.", pRootNode->getName());
        pLocalClient->getDebug()->notifyErrorMessage(sError);
      }
      else
      {
        wsafecpy(m_sLocale, 32, pLocaleAttr->getCharValue());
        readLocalizedElementsFromXml(pNode);
        XMLLiteElement * pDataElt = pNode->getChildByName(L"version");
        if (pDataElt != NULL)
          wsafecpy(m_sVersion, 16, pDataElt->getCharValue());
        pDataElt = pNode->getFirstChild();
        while (pDataElt != NULL)
        {
          if (0 == _wcsicmp(pDataElt->getName(), L"dependency"))
          {
            Edition * pEdition = pLocalClient->getDataFactory()->findEdition(pDataElt->getCharValue());
            if (pEdition == NULL)
            {
              swprintf_s(sError, 1024, L"XML formation error: dependency \"%s\" not found in node edition. Check out file %s.", pDataElt->getCharValue(), pRootNode->getName());
              pLocalClient->getDebug()->notifyErrorMessage(sError);
            }
            else
              m_pDependencies->addLast(pEdition);
          }
          pDataElt = pNode->getNextChild();
        }
      }
    }
    else
    {
      swprintf_s(sError, 1024, L"XML formation error: node \"edition\" was expected. Check out file %s.", pRootNode->getName());
      pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
  }
  swprintf_s(loadingMsg, 128, L"%s: checksum", sName);
  pLocalClient->getDebug()->notifyLoadingMessage(loadingMsg);
  computeChecksum(pLocalClient->getDebug());
}

// -----------------------------------------------------------------
// Name : ~Edition
//  Destructor
// -----------------------------------------------------------------
Edition::~Edition()
{
  delete m_pEthnicities;
  delete m_pProgressionTrees;
  delete m_pUnits;
  delete m_pSpells;
  delete m_pSpecTiles;
  delete m_pArtifacts;
  delete m_pDependencies;
  delete m_pSkillNames;
  delete m_pAIs;
  FREE(m_pShahmahCreation);
}

// -----------------------------------------------------------------
// Name : activate
// -----------------------------------------------------------------
bool Edition::activate(DebugManager * pDebug)
{
  wchar_t sFileName[MAX_PATH];

  // Set locale
  char sLocale[32];
  wtostr(sLocale, 32, m_sLocale);
  if (setlocale(LC_ALL, sLocale) == NULL)
    pDebug->notifyErrorMessage(L"Error: locale not recognized.");

  // Load ethnicities
  XMLLiteReader reader;
  swprintf_s(sFileName, MAX_PATH, L"%s%s/ethnicities.xml", EDITIONS_PATH, m_sObjectId);
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load progression trees
  swprintf_s(sFileName, MAX_PATH, L"%s%s/progressions.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != _wcsicmp(pNode->getName(), L"tree"))
        continue;
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName(L"id");
      if (pIdAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"id\" attribute in node tree. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        continue;
      }
      XMLLiteAttribute * pTypeAttr = pNode->getAttributeByName(L"type");
      if (pTypeAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"type\" attribute in node tree. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      ProgressionTree * pTree = NULL;
      wchar_t * sType = pTypeAttr->getCharValue();
      if (_wcsicmp(sType, L"ethnicity") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_ETHNICITY, pDebug);
      else if (_wcsicmp(sType, L"magic") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_MAGIC, pDebug);
      else if (_wcsicmp(sType, L"trait") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_TRAIT, pDebug);
      else
      {
        swprintf_s(sError, 1024, L"XML formation error: unknown attribute \"type\" for tree id %s. Check out file %s.", pIdAttr->getCharValue(), pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      if (pTree != NULL)
      {
        pTree->readLocalizedElementsFromXml(pNode);
        XMLLiteElement * pDataElt = pNode->getFirstChild();
        while (pDataElt != NULL)
        {
          if (_wcsicmp(pDataElt->getName(), L"skill") == 0
                || _wcsicmp(pDataElt->getName(), L"spell") == 0
                || _wcsicmp(pDataElt->getName(), L"modify") == 0
                || _wcsicmp(pDataElt->getName(), L"artifact") == 0
                || _wcsicmp(pDataElt->getName(), STRING_AVATAR_XML) == 0)
          {
            ProgressionEffect * pEffect = pTree->readXMLEffect(pDataElt, pRootNode, pTree->m_sObjectId, pDebug);
            if (pEffect != NULL)
              pTree->m_pBaseEffects->addLast(pEffect);
          }
          pDataElt = pNode->getNextChild();
        }
        m_pProgressionTrees->addLast(pTree);
      }
      pNode = pRootNode->getNextChild();
    }
  }

  // Look for skills
  // Init temporary list
  pDebug->notifyLoadingMessage(L"Skills");
  wchar_t ** sList;
  sList = new wchar_t*[MAX_SKILLS];
  for (int i = 0; i < MAX_SKILLS; i++)
    sList[i] = new wchar_t[NAME_MAX_CHARS];

  // Start process
  int nbSkills = getSkills(sList, MAX_SKILLS, NAME_MAX_CHARS, m_sObjectId);
  for (int i = 0; i < nbSkills; i++)
    m_pSkillNames->addLast(new StringObject(sList[i]));

  for (int i = 0; i < MAX_SKILLS; i++)
    delete[] sList[i];
  delete[] sList;

  // Load avatars
  swprintf_s(sFileName, MAX_PATH, L"%s%s/shahmahs.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load units
  swprintf_s(sFileName, MAX_PATH, L"%s%s/units.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load spells
  m_iTotalFreq = 0;
  swprintf_s(sFileName, MAX_PATH, L"%s%s/spells.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != _wcsicmp(pNode->getName(), L"spell"))
        continue;
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName(L"id");
      if (pIdAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"id\" attribute in node spell. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        continue;
      }
      XMLLiteAttribute * pFreqAttr = pNode->getAttributeByName(L"freq");
      if (pFreqAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"freq\" attribute (spell frequency) in node spell. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        continue;
      }
      int freq = (int) pFreqAttr->getIntValue();
      m_iTotalFreq += freq;
      Spell * pSpell = new Spell(0, m_sObjectId, (short) freq, pIdAttr->getCharValue(), pDebug);
      m_pSpells->addFirst(pSpell);
      pNode = pRootNode->getNextChild();
    }
  }

  // Load special tiles
  swprintf_s(sFileName, MAX_PATH, L"%s%s/spectiles.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != _wcsicmp(pNode->getName(), L"spectile"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName(L"id");
      if (pIdAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"id\" attribute in node spectile. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pFreqAttr = pNode->getAttributeByName(L"freq");
      if (pFreqAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"freq\" attribute (special tile frequency) in node spectile. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      int freq = (int) pFreqAttr->getIntValue();
      SpecialTile * pSpec = new SpecialTile(0, freq, CoordsMap(), m_sObjectId, pIdAttr->getCharValue(), pDebug);
      m_pSpecTiles->addFirst(pSpec);
      pNode = pRootNode->getNextChild();
    }
  }

  // Load artifacts
  swprintf_s(sFileName, MAX_PATH, L"%s%s/artifacts.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load ShahmahCreation
  swprintf_s(sFileName, MAX_PATH, L"%s%s/creation.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    m_pShahmahCreation = new ShahmahCreation();
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != _wcsicmp(pNode->getName(), L"shahmah_creation"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteElement * pElt = pNode->getFirstChild();
      while (pElt != NULL)
      {
        if (0 == _wcsicmp(pElt->getName(), L"skill"))
        {
          XMLLiteAttribute * pFileAttr = pElt->getAttributeByName(L"luafile");
          XMLLiteAttribute * pParamsAttr = pElt->getAttributeByName(L"parameters");
          XMLLiteAttribute * pCostAttr = pElt->getAttributeByName(L"cost");
          if (pFileAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: missing \"luafile\" attribute in node shahmah_creation / skill. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          if (pCostAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: missing \"cost\" attribute in node shahmah_creation / skill. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          Edition * pEdition = findSkillEdition(pFileAttr->getCharValue());
          if (pEdition != NULL)
          {
            wchar_t * pParams = (pParamsAttr == NULL) ? NULL : pParamsAttr->getCharValue();
            Skill * pSkill = new Skill(pEdition->m_sObjectId, pFileAttr->getCharValue(), pParams, pDebug);
            m_pShahmahCreation->m_pSkills->addLast(pSkill, pCostAttr->getIntValue());
          }
          else
          {
            swprintf_s(sError, 1024, L"XML error: skill \"%s\" not found in edition and dependencies. Check out file %s.", pFileAttr->getCharValue(), pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
        }
        else if (0 == _wcsicmp(pElt->getName(), L"ethnicity"))
        {
          XMLLiteAttribute * pNameAttr = pElt->getAttributeByName(L"name");
          XMLLiteAttribute * pCostAttr = pElt->getAttributeByName(L"cost");
          if (pNameAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: missing \"name\" attribute in node shahmah_creation / ethnicity. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          if (pCostAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: missing \"cost\" attribute in node shahmah_creation / ethnicity. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          Ethnicity * pEthn = findEthnicity(pNameAttr->getCharValue());
          if (pEthn != NULL)
          {
            m_pShahmahCreation->m_pPeoples->addLast(pEthn, pCostAttr->getIntValue());
            XMLLiteElement * pSubElt = pElt->getFirstChild();
            while (pSubElt != NULL)
            {
              if (0 == _wcsicmp(pSubElt->getName(), L"image"))
              {
                XMLLiteAttribute * pNameAttr = pSubElt->getAttributeByName(L"name");
                if (pNameAttr == NULL)
                {
                  swprintf_s(sError, 1024, L"XML formation error: missing \"name\" attribute in node shahmah_creation / image. Check out file %s.", pRootNode->getName());
                  pDebug->notifyErrorMessage(sError);
                  pSubElt = pNode->getNextChild();
                  continue;
                }
                StringObject * pStr = new StringObject(pNameAttr->getCharValue());
                pStr->setAttachment(pEthn);
                m_pShahmahCreation->m_pImages->addLast(pStr);
              }
              pSubElt = pElt->getNextChild();
            }
          }
          else
          {
            swprintf_s(sError, 1024, L"XML error: ethnicity \"%s\" not found in edition and dependencies. Check out file %s.", pNameAttr->getCharValue(), pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
        }
        pElt = pNode->getNextChild();
      }
      pNode = pRootNode->getNextChild();
    }
    // Check that we have at least 1 image and 1 people
    // Else, it's not valid; send warning message and delete object
    if (m_pShahmahCreation->m_pImages->size < 1 || m_pShahmahCreation->m_pPeoples->size < 1)
    {
      swprintf_s(sError, 1024, L"Warning: Shahmah creation data is not valid (missing images or ethnicities). Check out file %s.", pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      delete m_pShahmahCreation;
      m_pShahmahCreation = NULL;
    }
  }

  // Load AIs
  swprintf_s(sFileName, MAX_PATH, L"%s%s/ais.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    wchar_t sError[1024] = L"";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL) {
      if (0 != _wcsicmp(pNode->getName(), L"ai"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      // id and shahmah
      XMLLiteAttribute * pId = pNode->getAttributeByName(L"id");
      XMLLiteAttribute * pShahmah = pNode->getAttributeByName(L"shahmah");
      if (pId == NULL || pShahmah == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: missing \"id\" and/or \"shahmah\" attributes in node item. Check out file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      AIData * pAI = new AIData();
      m_pAIs->addLast(pAI);
      wsafecpy(pAI->m_sObjectId, NAME_MAX_CHARS, pId->getCharValue());
      wsafecpy(pAI->m_sEdition, NAME_MAX_CHARS, m_sObjectId);
      wsafecpy(pAI->m_sAvatarId, NAME_MAX_CHARS, pShahmah->getCharValue());
      // L12N elements
      pAI->readLocalizedElementsFromXml(pNode);
      // spells packs
      XMLLiteElement * pSpellsNode = pNode->getFirstChild();
      while (pSpellsNode != NULL) {
        // For each spells node, add a "SpellsPackContent" object in the pack
        if (_wcsicmp(pSpellsNode->getName(), L"spells") == 0)
        {
          SpellsPackContent * pPack = readSpellsPackContent(pSpellsNode, pDebug, sFileName);
          if (pPack != NULL) {
            pAI->m_pSpellsPacks->addFirst(pPack);
          }
        }
        pSpellsNode = pNode->getNextChild();
      }
      pNode = pRootNode->getNextChild();
    }
  }

  m_bActive = true;
  return true;
}

// -----------------------------------------------------------------
// Name : deactivate
// -----------------------------------------------------------------
void Edition::deactivate()
{
  m_bActive = false;
  // free memory (remove unit data from data factory)
  m_pEthnicities->deleteAll();
  m_pProgressionTrees->deleteAll();
  m_pUnits->deleteAll();
  m_pSpells->deleteAll();
  m_pSpecTiles->deleteAll();
  m_pArtifacts->deleteAll();
}

// -----------------------------------------------------------------
// Name : loadXMLFile
// -----------------------------------------------------------------
XMLLiteElement * Edition::loadXMLFile(XMLLiteReader * pReader, wchar_t * fileName, DebugManager * pDebug)
{
  wchar_t sError[1024] = L"";
  XMLLiteElement * pRootNode = NULL;
  try {
    pRootNode = pReader->parseFile(fileName);
  }
  catch (int errorCode)
  {
    if (errorCode != XMLLITE_ERROR_CANT_OPEN_FILE)  // If file not found, don't show error (because it's not)
      pDebug->notifyXMLErrorMessage(fileName, errorCode, pReader->getCurrentLine(), pReader->getCurrentCol());
    return NULL;
  }
  return pRootNode;
}

// -----------------------------------------------------------------
// Name : parseXMLObjectData
// -----------------------------------------------------------------
void Edition::parseXMLObjectData(XMLLiteElement * pRootNode, DebugManager * pDebug)
{
  wchar_t sError[1024] = L"";
  XMLLiteElement * pNode = pRootNode->getFirstChild();
  while (pNode != NULL)
  {
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // First check if ethnicity
    if (0 == _wcsicmp(pNode->getName(), L"ethnicity"))
    {
      Ethnicity * pData = new Ethnicity(m_sObjectId, pNode, pDebug);
      m_pEthnicities->addLast(pData);
      pNode = pRootNode->getNextChild();
      continue;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Then, check if artifact
    else if (0 == _wcsicmp(pNode->getName(), L"artifact"))
    {
      wchar_t sId[NAME_MAX_CHARS] = L"";
      wchar_t sTexture[MAX_PATH] = L"";
      int iPosition = -1;
      bool bTwoHanded = false;
      // Get id
      XMLLiteElement * pElt = pNode->getChildByName(L"id");
      if (pElt == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: element \"id\" missing in artifact definition. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      wsafecpy(sId, NAME_MAX_CHARS, pElt->getCharValue());
      // Get texture
      pElt = pNode->getChildByName(L"texture");
      if (pElt == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: element \"texture\" missing in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      swprintf_s(sTexture, MAX_PATH, L"%s/%s", m_sObjectId, pElt->getCharValue());
      // Get position
      pElt = pNode->getChildByName(L"position");
      if (pElt == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error: element \"position\" missing in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      if (_wcsicmp(pElt->getCharValue(), L"head") == 0)
        iPosition = ARTIFACT_POSITION_HEAD;
      else if (_wcsicmp(pElt->getCharValue(), L"body") == 0)
        iPosition = ARTIFACT_POSITION_BODY;
      else if (_wcsicmp(pElt->getCharValue(), L"left_hand") == 0)
        iPosition = ARTIFACT_POSITION_LHAND;
      else if (_wcsicmp(pElt->getCharValue(), L"right_hand") == 0)
        iPosition = ARTIFACT_POSITION_RHAND;
      else if (_wcsicmp(pElt->getCharValue(), L"foot") == 0)
        iPosition = ARTIFACT_POSITION_FOOT;
      else
      {
        swprintf_s(sError, 1024, L"XML formation error: invalid value for element \"position\" in artifact %s. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      // Is two-handed?
      pElt = pNode->getChildByName(L"two_handed");
      if (pElt != NULL)
        bTwoHanded = (pElt->getIntValue() != 0);
      // Create artifact
      Artifact * pArtifact = new Artifact(m_sObjectId, sId, sTexture, (u8)iPosition, bTwoHanded);
      // Read artifact data
      pArtifact->readLocalizedElementsFromXml(pNode);
      XMLLiteElement * pDataElt = pNode->getFirstChild();
      while (pDataElt != NULL)
      {
        const wchar_t* pName = pDataElt->getName();
        if (0 == _wcsicmp(pName, L"modify"))
        {
          wchar_t sKey[NAME_MAX_CHARS];
          int iValue;
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName(L"key");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"key\" missing in element \"modify\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sKey, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName(L"value");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"value\" missing in element \"modify\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          iValue = pAttr->getIntValue();
          pArtifact->addArtifactEffect(new ArtifactEffect_Charac(sKey, iValue));
        }
        else if (0 == _wcsicmp(pName, L"spell"))
        {
          wchar_t sEdition[NAME_MAX_CHARS];
          wchar_t sName[NAME_MAX_CHARS];
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName(L"edition");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"edition\" missing in element \"spell\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sEdition, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName(L"name");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"spell\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
          pArtifact->addArtifactEffect(new ArtifactEffect_Spell(sEdition, sName));
        }
        else if (0 == _wcsicmp(pName, L"skill"))
        {
          wchar_t sEdition[NAME_MAX_CHARS];
          wchar_t sName[NAME_MAX_CHARS];
          wchar_t sParams[LUA_FUNCTION_PARAMS_MAX_CHARS] = L"";
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName(L"edition");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"edition\" missing in element \"skill\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sEdition, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName(L"name");
          if (pAttr == NULL)
          {
            swprintf_s(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"skill\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName(L"parameters");
          if (pAttr != NULL)
            wsafecpy(sParams, LUA_FUNCTION_PARAMS_MAX_CHARS, pAttr->getCharValue());
          pArtifact->addArtifactEffect(new ArtifactEffect_Skill(sEdition, sName, sParams));
        }
        pDataElt = pNode->getNextChild();
      }
      m_pArtifacts->addLast(pArtifact);
      pNode = pRootNode->getNextChild();
      continue;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Then, unit/avatar
    UnitData * pData = NULL;
    if (0 == _wcsicmp(pNode->getName(), STRING_AVATAR_XML))
      pData = new AvatarData();
    else if (0 == _wcsicmp(pNode->getName(), L"unit"))
      pData = new UnitData();
    else
    {
      swprintf_s(sError, 1024, L"XML formation error: unexpected node \"%s\". Check out file %s.", pNode->getName(), pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      pNode = pRootNode->getNextChild();
      continue;
    }
    assert(pData != NULL);

    wsafecpy(pData->m_sEdition, NAME_MAX_CHARS, m_sObjectId);
    pData->readLocalizedElementsFromXml(pNode);
    XMLLiteElement * pDataElt = pNode->getFirstChild();
    while (pDataElt != NULL)
    {
      const wchar_t* pName = pDataElt->getName();
      if (0 == _wcsicmp(pName, L"id"))
        wsafecpy(pData->m_sObjectId, NAME_MAX_CHARS, pDataElt->getCharValue());
      else if (0 == _wcsicmp(pName, L"ethnicity"))
        wsafecpy(pData->m_sEthnicityId, NAME_MAX_CHARS, pDataElt->getCharValue());
      else if (0 == _wcsicmp(pName, STRING_MELEE) || 0 == _wcsicmp(pName, STRING_RANGE) || 0 == _wcsicmp(pName, STRING_ARMOR) ||
               0 == _wcsicmp(pName, STRING_ENDURANCE) || 0 == _wcsicmp(pName, STRING_SPEED) || 0 == _wcsicmp(pName, STRING_ALIGNMENT))
        pData->m_lValues.insert(long_hash_pair(pName, pDataElt->getIntValue()));
      else if (0 == _wcsicmp(pName, L"texture"))
        swprintf_s(pData->m_sTextureFilename, MAX_PATH, L"%s/%s", m_sObjectId, pDataElt->getCharValue());
      else if (0 == _wcsicmp(pName, L"skill"))
      {
        XMLLiteAttribute * pSkillFile = pDataElt->getAttributeByName(L"luafile");
        XMLLiteAttribute * pSkillParams = pDataElt->getAttributeByName(L"parameters");
        if (pSkillFile != NULL)
        {
          Edition * pEdition = findSkillEdition(pSkillFile->getCharValue());
          if (pEdition != NULL)
          {
            wchar_t * pParams = (pSkillParams == NULL) ? NULL : pSkillParams->getCharValue();
            Skill * pSkill = new Skill(pEdition->m_sObjectId, pSkillFile->getCharValue(), pParams, pDebug);
            pData->m_pSkills->addFirst(pSkill);
          }
          else
          {
            swprintf_s(sError, 1024, L"XML error: skill \"%s\" not found in edition and dependencies. Check out file %s.", pSkillFile->getCharValue(), pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
          }
        }
      }
      pDataElt = pNode->getNextChild();
    }
    m_pUnits->addLast(pData);
    pNode = pRootNode->getNextChild();
  }
}

// -----------------------------------------------------------------
// Name : addShopItems
// -----------------------------------------------------------------
void Edition::addShopItems(Profile * pPlayer, guiSmartSlider * pShopSlider, DebugManager * pDebug)
{
  // Read and parse shop.xml file in edition's folder
  wchar_t sFileName[MAX_PATH];
  swprintf_s(sFileName, MAX_PATH, L"%s%s/shop.xml", EDITIONS_PATH, m_sObjectId);

  XMLLiteReader reader;
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode == NULL)
    return;

  wchar_t sError[1024] = L"";
  XMLLiteElement * pNode = pRootNode->getFirstChild();
  while (pNode != NULL)
  {
    // For each item, add a ShopItem in slider
    if (0 != _wcsicmp(pNode->getName(), L"item"))
    {
      pNode = pRootNode->getNextChild();
      continue;
    }

    XMLLiteAttribute * pAttr = pNode->getAttributeByName(L"type");
    if (pAttr == NULL)
    {
      swprintf_s(sError, 1024, L"XML formation error : missing \"type\" attribute in node item. Check out file %s.", sFileName);
      pDebug->notifyErrorMessage(sError);
      pNode = pRootNode->getNextChild();
      continue;
    }
    wchar_t * sType = pAttr->getCharValue();
    if (_wcsicmp(sType, L"pack") == 0)
    {
      PackShopItem * pItem = new PackShopItem();
      pItem->m_bEnabled = true;
      wsafecpy(pItem->m_sEdition, NAME_MAX_CHARS, m_sObjectId);
      pItem->m_pXml->readLocalizedElementsFromXml(pNode);
      pItem->m_pXml->findLocalizedElement(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName(L"cost");
      if (pChild != NULL)
        pItem->m_iCost = (int) pChild->getIntValue();
      else
        pItem->m_iCost = -1;

      // Item texture
      pChild = pNode->getChildByName(L"texture");
      if (pChild != NULL)
      {
        wchar_t sStr[MAX_PATH];
        swprintf_s(sStr, MAX_PATH, L"%s/%s", m_sObjectId, pChild->getCharValue());
        pItem->m_iTexId = pShopSlider->getDisplay()->getTextureEngine()->loadTexture(sStr);
      }
      else
      {
        swprintf_s(sError, 1024, L"Texture tag not found in file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
      }

      // Content data
      pChild = pNode->getChildByName(L"content");
      if (pChild != NULL)
      {
        XMLLiteElement * pSpellsNode = pChild->getFirstChild();
        while (pSpellsNode != NULL)
        {
          // For each spells node, add a "PackShopItem_content" object in the pack
          if (_wcsicmp(pSpellsNode->getName(), L"spells") == 0)
          {
            SpellsPackContent * pPack = readSpellsPackContent(pSpellsNode, pDebug, sFileName);
            if (pPack != NULL) {
              pItem->m_pContent->addFirst(pPack);
            }
          }
          pSpellsNode = pChild->getNextChild();
        }
      }
      else
      {
        swprintf_s(sError, 1024, L"Content tag not found in file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
      }

      // Finally add item to slider
      checkShopItemValidity(pPlayer, pItem);
      pShopSlider->addItem(pItem);
    }
    else if (_wcsicmp(sType, STRING_AVATAR_XML) == 0)
    {
      // Get avatar id
      pAttr = pNode->getAttributeByName(L"id");
      if (pAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error : missing \"id\" attribute in node item for type avatar. Check out file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      wchar_t * sId = pAttr->getCharValue();
      AvatarData * pAvatar = (AvatarData*) findUnitData(sId);
      if (pAvatar == NULL)
      {
        swprintf_s(sError, 1024, L"Unknown avatar %s, defined in file %s.", sId, sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }

      AvatarShopItem * pItem = new AvatarShopItem();
      pItem->m_bEnabled = true;
      wsafecpy(pItem->m_sEdition, NAME_MAX_CHARS, m_sObjectId);

      // Avatar id
      wsafecpy(pItem->m_sAvatarId, NAME_MAX_CHARS, sId);

      // Item name
      wchar_t sName[SLIDER_ITEM_MAX_CHARS];
      wchar_t str[64];
      pAvatar->findLocalizedElement(sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
      i18n->getText(L"AVATAR", str, 64);  // "Avatar"
      swprintf_s(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, L"%s (%s)", sName, str);

      // Item full description
      wchar_t sEthnicityName[NAME_MAX_CHARS];
      Ethnicity * pEthn = findEthnicity(pAvatar->m_sEthnicityId);
      if (pEthn == NULL)
      {
        swprintf_s(sError, 1024, L"Error: Avatar %s should have an ethnicity.", sName);
        pDebug->notifyErrorMessage(sError);
      }
      else
        pEthn->findLocalizedElement(sEthnicityName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");

      wchar_t sDesc[DESCRIPTION_MAX_CHARS] = L"";
      pAvatar->getInfos(sDesc, DESCRIPTION_MAX_CHARS, L"\n", false, sEthnicityName);
      pItem->m_pXml->addLocalizedElement(L"description", sDesc, i18n->getCurrentLanguageName());

      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName(L"cost");
      if (pChild != NULL)
        pItem->m_iCost = (int) pChild->getIntValue();
      else
        pItem->m_iCost = -1;

      // Item texture
      pItem->m_iTexId = pShopSlider->getDisplay()->getTextureEngine()->loadTexture(pAvatar->m_sTextureFilename);

      // Finally add item to slider
      checkShopItemValidity(pPlayer, pItem);
      pShopSlider->addItem(pItem);
    }
    else if (_wcsicmp(sType, L"artifact") == 0)
    {
      // Get artifact id
      pAttr = pNode->getAttributeByName(L"id");
      if (pAttr == NULL)
      {
        swprintf_s(sError, 1024, L"XML formation error : missing \"id\" attribute in node item for type artifact. Check out file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      wchar_t * sId = pAttr->getCharValue();
      Artifact * pArtifact = (Artifact*) findArtifact(sId);
      if (pArtifact == NULL)
      {
        swprintf_s(sError, 1024, L"Unknown artifact %s, defined in file %s.", sId, sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }

      ArtifactShopItem * pItem = new ArtifactShopItem();
      pItem->m_bEnabled = true;
      wsafecpy(pItem->m_sEdition, NAME_MAX_CHARS, m_sObjectId);

      // Artifact id
      wsafecpy(pItem->m_sArtifactId, NAME_MAX_CHARS, sId);

      // Item name
      wchar_t sName[SLIDER_ITEM_MAX_CHARS];
      wchar_t str[64];
      pArtifact->findLocalizedElement(sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
      i18n->getText(L"ARTIFACT", str, 64);  // "Avatar"
      swprintf_s(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, L"%s (%s)", sName, str);

      // Item full description
      wchar_t sDesc[DESCRIPTION_MAX_CHARS] = L"";
      pArtifact->findLocalizedElement(sDesc, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), L"description");
      pItem->m_pXml->addLocalizedElement(L"description", sDesc, i18n->getCurrentLanguageName());

      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName(L"cost");
      if (pChild != NULL)
        pItem->m_iCost = (int) pChild->getIntValue();
      else
        pItem->m_iCost = -1;

      // Item texture
      pItem->m_iTexId = pShopSlider->getDisplay()->getTextureEngine()->loadTexture(pArtifact->getTexture());

      // Finally add item to slider
      checkShopItemValidity(pPlayer, pItem);
      pShopSlider->addItem(pItem);
    }
    else
    {
      swprintf_s(sError, 1024, L"XML formation error : \"type\" attribute has an invalid value : %s. Check out file %s.", sType, sFileName);
      pDebug->notifyErrorMessage(sError);
      pNode = pRootNode->getNextChild();
      continue;
    }
    pNode = pRootNode->getNextChild();
  }
}

// -----------------------------------------------------------------
// Name : readSpellsPackContent
// -----------------------------------------------------------------
SpellsPackContent * Edition::readSpellsPackContent(XMLLiteElement * pSpellsNode, DebugManager * pDebug, wchar_t * sFileName)
{
  wchar_t sError[1024];
  XMLLiteAttribute * pQuantity = pSpellsNode->getAttributeByName(L"quantity");
  XMLLiteAttribute * pMode = pSpellsNode->getAttributeByName(L"mode");
  XMLLiteAttribute * pSpellId = pSpellsNode->getAttributeByName(L"id");
  if (pQuantity == NULL || pMode == NULL)
  {
    swprintf_s(sError, 1024, L"XML formation error: missing attributes in node spells. Check out file %s.", sFileName);
    pDebug->notifyErrorMessage(sError);
    return NULL;
  }
  SpellsPackContent * pPack = NULL;
  wchar_t * sMode = pMode->getCharValue();
  if (_wcsicmp(sMode, L"fixed") == 0) {
    if (pSpellId != NULL) {
      pPack = new SpellsPackContent();
      pPack->m_iMode = PACK_MODE_FIXED;
      wsafecpy(pPack->m_sSpellId, NAME_MAX_CHARS, pSpellId->getCharValue());
    }
    else {
      swprintf_s(sError, 1024, L"XML formation error: missing attribute 'id' in node spells. Check out file %s.", sFileName);
      pDebug->notifyErrorMessage(sError);
    }
  }
  else {
    int mode = -1;
    if (_wcsicmp(sMode, L"random") == 0)
      mode = PACK_MODE_RANDOM;
    else if (_wcsicmp(sMode, L"random-rare") == 0)
      mode = PACK_MODE_RANDOM_RARE;
    else if (_wcsicmp(sMode, L"random-life") == 0)
      mode = PACK_MODE_RANDOM_LIFE;
    else if (_wcsicmp(sMode, L"random-law") == 0)
      mode = PACK_MODE_RANDOM_LAW;
    else if (_wcsicmp(sMode, L"random-death") == 0)
      mode = PACK_MODE_RANDOM_DEATH;
    else if (_wcsicmp(sMode, L"random-chaos") == 0)
      mode = PACK_MODE_RANDOM_CHAOS;
    if (mode != -1) {
      pPack = new SpellsPackContent();
      pPack->m_iMode = mode;
    }
    else {
      swprintf_s(sError, 1024, L"XML formation error: invalid attribute 'mode' in node spells. Check out file %s.", sFileName);
      pDebug->notifyErrorMessage(sError);
    }
  }
  if (pPack != NULL) {
    pPack->m_iNbSpells = pQuantity->getIntValue();
  }
  return pPack;
}

// -----------------------------------------------------------------
// Name : checkShopItemValidity
// -----------------------------------------------------------------
void Edition::checkShopItemValidity(Profile * pPlayer, ShopItem * pItem)
{
  ObjectList * pAvatarsList = pPlayer->getAvatarsList();
  if (pItem->m_iType == 1) // avatar?
  {
    AvatarData * p = (AvatarData*) pAvatarsList->getFirst(0);
    while (p != NULL)
    {
      if (wcscmp(((AvatarShopItem*)pItem)->m_sAvatarId, p->m_sObjectId) == 0 &&
        wcscmp(pItem->m_sEdition, p->m_sEdition) == 0)  // already got this one?
      {
        pItem->m_bEnabled = false;
        i18n->getText(L"ALREADY_OWN", pItem->m_sDisabledReason, SLIDER_ITEM_MAX_CHARS);
        return;
      }
      p = (AvatarData*) pAvatarsList->getNext(0);
    }
  }
  if (pPlayer->getCash() < pItem->m_iCost) // not rich enough, my dear
  {
    pItem->m_bEnabled = false;
    i18n->getText(L"NEED_MORE_CASH", pItem->m_sDisabledReason, SLIDER_ITEM_MAX_CHARS);
    return;
  }
}

// -----------------------------------------------------------------
// Name : findEthnicity
// -----------------------------------------------------------------
Ethnicity * Edition::findEthnicity(wchar_t * strId, bool bLookDependencies)
{
  Ethnicity * pData = (Ethnicity*) m_pEthnicities->getFirst(0);
  while (pData != NULL)
  {
    if (0 == _wcsicmp(strId, pData->m_sObjectId))
      return pData;
    pData = (Ethnicity*) m_pEthnicities->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pData = pEdition->findEthnicity(strId, true);
      if (pData != NULL)
        return pData;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findUnitData
// -----------------------------------------------------------------
UnitData * Edition::findUnitData(wchar_t * strId, bool bLookDependencies)
{
  UnitData * pData = (UnitData*) m_pUnits->getFirst(0);
  while (pData != NULL)
  {
    if (0 == _wcsicmp(strId, pData->m_sObjectId))
      return pData;
    pData = (UnitData*) m_pUnits->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pData = pEdition->findUnitData(strId, true);
      if (pData != NULL)
        return pData;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findSpell
// -----------------------------------------------------------------
Spell * Edition::findSpell(wchar_t * sName, bool bLookDependencies)
{
  Spell * pSpell = (Spell*) m_pSpells->getFirst(0);
  while (pSpell != NULL)
  {
    if (pSpell->isUniqueId(m_sObjectId, SPELL_OBJECT_NAME, sName))
      return pSpell;
    pSpell = (Spell*) m_pSpells->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pSpell = pEdition->findSpell(sName, true);
      if (pSpell != NULL)
        return pSpell;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findSpecialTile
// -----------------------------------------------------------------
SpecialTile * Edition::findSpecialTile(wchar_t * sName, bool bLookDependencies)
{
  SpecialTile * pSpec = (SpecialTile*) m_pSpecTiles->getFirst(0);
  while (pSpec != NULL)
  {
    if (wcscmp(sName, pSpec->getObjectName()) == 0)
      return pSpec;
    pSpec = (SpecialTile*) m_pSpecTiles->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pSpec = pEdition->findSpecialTile(sName, true);
      if (pSpec != NULL)
        return pSpec;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findProgressionTree
// -----------------------------------------------------------------
ProgressionTree * Edition::findProgressionTree(wchar_t * strId, bool bLookDependencies)
{
  ProgressionTree * pTree = (ProgressionTree*) m_pProgressionTrees->getFirst(0);
  while (pTree != NULL)
  {
    if (wcscmp(pTree->m_sObjectId, strId) == 0)
      return pTree;
    pTree = (ProgressionTree*) m_pProgressionTrees->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pTree = pEdition->findProgressionTree(strId, true);
      if (pTree != NULL)
        return pTree;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : findArtifact
// -----------------------------------------------------------------
Artifact * Edition::findArtifact(wchar_t * strId, bool bLookDependencies)
{
  Artifact * pArtifact = (Artifact*) m_pArtifacts->getFirst(0);
  while (pArtifact != NULL)
  {
    if (wcscmp(pArtifact->m_sObjectId, strId) == 0)
      return pArtifact;
    pArtifact = (Artifact*) m_pArtifacts->getNext(0);
  }
  if (bLookDependencies)
  {
    Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
    while (pEdition != NULL)
    {
      pArtifact = pEdition->findArtifact(strId, true);
      if (pArtifact != NULL)
        return pArtifact;
      pEdition = (Edition*) m_pDependencies->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : selectRandomSpell
// -----------------------------------------------------------------
Spell * Edition::selectRandomSpell(int iSelectMode)
{
  int iMana = -1;
  switch (iSelectMode)
  {
  case PACK_MODE_RANDOM:
    {
      int iRand = (int) getRandom((u32)m_iTotalFreq);
      int i = 0;
      Spell * pSpell = (Spell*) m_pSpells->getFirst(0);
      while (pSpell != NULL)
      {
        i += pSpell->getFrequency();
        if (iRand < i)
          return pSpell;
        pSpell = (Spell*) m_pSpells->getNext(0);
      }
      return NULL;
    }
    break;
  case PACK_MODE_RANDOM_RARE:
    {
      // Compute total frequency
      u32 totalFreq = 0;
      Spell * pSpell = (Spell*) m_pSpells->getFirst(0);
      while (pSpell != NULL)
      {
        // Only look at rare spells (freq <= 3)
        if (pSpell->getFrequency() <= 3)
          totalFreq += (unsigned) pSpell->getFrequency();
        pSpell = (Spell*) m_pSpells->getNext(0);
      }
      int iRand = (int) getRandom(totalFreq);
      int i = 0;
      pSpell = (Spell*) m_pSpells->getFirst(0);
      while (pSpell != NULL)
      {
        // Only look at rare spells (freq <= 3)
        if (pSpell->getFrequency() <= 3) {
          i += pSpell->getFrequency();
          if (iRand < i)
            return pSpell;
        }
        pSpell = (Spell*) m_pSpells->getNext(0);
      }
      // No rare spell found => use normal random mode
      return selectRandomSpell(PACK_MODE_RANDOM);
    }
    break;
  case PACK_MODE_RANDOM_LIFE:
    iMana = MANA_LIFE;
    break;
  case PACK_MODE_RANDOM_LAW:
    iMana = MANA_LAW;
    break;
  case PACK_MODE_RANDOM_DEATH:
    iMana = MANA_DEATH;
    break;
  case PACK_MODE_RANDOM_CHAOS:
    iMana = MANA_CHAOS;
    break;
  }
  if (iMana >= 0) {
    // Compute total frequency
    u32 totalFreq = 0;
    Spell * pSpell = (Spell*) m_pSpells->getFirst(0);
    while (pSpell != NULL)
    {
      // Only look at spells according to wanted mana
      if (pSpell->getCost().mana[iMana] > 0)
        totalFreq += (unsigned) pSpell->getFrequency();
      pSpell = (Spell*) m_pSpells->getNext(0);
    }
    int iRand = (int) getRandom(totalFreq);
    int i = 0;
    pSpell = (Spell*) m_pSpells->getFirst(0);
    while (pSpell != NULL)
    {
      // Only look at spells according to wanted mana
      if (pSpell->getCost().mana[iMana] > 0) {
        i += pSpell->getFrequency();
        if (iRand < i)
          return pSpell;
      }
      pSpell = (Spell*) m_pSpells->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getAllTreesByType
// -----------------------------------------------------------------
void Edition::getAllTreesByType(ObjectList * pList, u8 uType)
{
  ProgressionTree * pTree = (ProgressionTree*) m_pProgressionTrees->getFirst(0);
  while (pTree != NULL)
  {
    if (uType == pTree->m_uType)
      pList->addLast(pTree);
    pTree = (ProgressionTree*) m_pProgressionTrees->getNext(0);
  }
}

// -----------------------------------------------------------------
// Name : findSkillEdition
// -----------------------------------------------------------------
Edition * Edition::findSkillEdition(wchar_t * sName)
{
  StringObject * pString = (StringObject*) m_pSkillNames->getFirst(0);
  while (pString != NULL)
  {
    if (wcscmp(pString->m_sString, sName) == 0)
      return this;
    pString = (StringObject*) m_pSkillNames->getNext(0);
  }
  Edition * pEdition = (Edition*) m_pDependencies->getFirst(0);
  while (pEdition != NULL)
  {
    Edition * pEd2 = pEdition->findSkillEdition(sName);
    if (pEd2 != NULL)
      return pEd2;
    pEdition = (Edition*) m_pDependencies->getNext(0);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : computeChecksum
// -----------------------------------------------------------------
void Edition::computeChecksum(DebugManager * pDebug)
{
  //FILE * pFile = NULL;
  //u32 filessize = 0;
  wchar_t sPath[MAX_PATH];
  swprintf_s(sPath, MAX_PATH, L"%s%s", EDITIONS_PATH, m_sObjectId);
  if (!md5folder(sPath, m_sChecksum))
  {
    wchar_t sError[256];
    swprintf_s(sError, 256, L"Error: can't do checksum on edition %s. The edition will be deactivated. Please check that you haven't opened any file from this extension.", m_sObjectId);
    pDebug->notifyErrorMessage(sError);
    deactivate();
  }
  // NOTE : checksum is not used in local. It will be sent to network peers when starting a LAN or net game, and checked against other peers checksums.
  //if (wcscmp(m_sChecksum, m_sObjectId) != 0)  // checksum changed
  //{
  //  wchar_t sWarning[256];
  //  swprintf_s(sWarning, 256, L"Warning: checksum changed on edition %s. Folder will be renamed.", m_sObjectId);
  //  pDebug->notifyErrorMessage(sWarning);
  //  wchar_t sNewPath[MAX_PATH];
  //  swprintf_s(sNewPath, MAX_PATH, L"%s%s", EDITIONS_PATH, sChecksum);
  //  int err = _wrename(sPath, sNewPath);
  //  if (err != 0)
  //  {
  //    wchar_t sKeyword[16] = L"Unknown";
  //    switch (err)
  //    {
  //    case EACCES:
  //      wsafecpy(sKeyword, 16, L"EACCES");
  //      break;
  //    case ENOENT:
  //      wsafecpy(sKeyword, 16, L"ENOENT");
  //      break;
  //    case EINVAL:
  //      wsafecpy(sKeyword, 16, L"EINVAL");
  //      break;
  //    }
  //    wchar_t sError[256];
  //    swprintf_s(sError, 256, L"Error: can't rename folder (code: %s). The edition will be deactivated. Please check that you haven't opened any file from this extension.", sKeyword);
  //    pDebug->notifyErrorMessage(sError);
  //    desactivate();
  //  }
  //  else
  //    wsafecpy(m_sObjectId, NAME_MAX_CHARS, sChecksum);
  //}
}
