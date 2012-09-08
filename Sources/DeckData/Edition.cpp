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
Edition::Edition(const char * sName, LocalClient * pLocalClient)
{
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, sName);
  wsafecpy(m_sLocale, 32, "en_EN");
  wsafecpy(m_sVersion, 16, "n/a");
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

  char loadingMsg[128] = "";
  snprintf(loadingMsg, 128, "Edition: %s", sName);
  pLocalClient->getDebug()->notifyLoadingMessage(loadingMsg);

  // Read edition.xml
  XMLLiteReader reader;
  char sFileName[MAX_PATH];
  snprintf(sFileName, MAX_PATH, "%s%s/edition.xml", EDITIONS_PATH, m_sObjectId);
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pLocalClient->getDebug());
  if (pRootNode != NULL)
  {
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    if (pNode != NULL && 0 == strcasecmp(pNode->getName(), "edition"))
    {
      XMLLiteAttribute * pLocaleAttr = pNode->getAttributeByName("locale");
      if (pLocaleAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"locale\" attribute in node edition. Check out file %s.", pRootNode->getName());
        pLocalClient->getDebug()->notifyErrorMessage(sError);
      }
      else
      {
        wsafecpy(m_sLocale, 32, pLocaleAttr->getCharValue());
        readLocalizedElementsFromXml(pNode);
        XMLLiteElement * pDataElt = pNode->getChildByName("version");
        if (pDataElt != NULL)
          wsafecpy(m_sVersion, 16, pDataElt->getCharValue());
        pDataElt = pNode->getFirstChild();
        while (pDataElt != NULL)
        {
          if (0 == strcasecmp(pDataElt->getName(), "dependency"))
          {
            Edition * pEdition = pLocalClient->getDataFactory()->findEdition(pDataElt->getCharValue());
            if (pEdition == NULL)
            {
              snprintf(sError, 1024, "XML formation error: dependency \"%s\" not found in node edition. Check out file %s.", pDataElt->getCharValue(), pRootNode->getName());
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
      snprintf(sError, 1024, "XML formation error: node \"edition\" was expected. Check out file %s.", pRootNode->getName());
      pLocalClient->getDebug()->notifyErrorMessage(sError);
    }
  }
  snprintf(loadingMsg, 128, "%s: checksum", sName);
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
  char sFileName[MAX_PATH];

//  // Set locale
//  char sLocale[32];
//  wtostr(sLocale, 32, m_sLocale);
//  if (setlocale(LC_ALL, sLocale) == NULL)
//    pDebug->notifyErrorMessage("Error: locale not recognized.");

  // Load ethnicities
  XMLLiteReader reader;
  snprintf(sFileName, MAX_PATH, "%s%s/ethnicities.xml", EDITIONS_PATH, m_sObjectId);
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load progression trees
  snprintf(sFileName, MAX_PATH, "%s%s/progressions.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != strcasecmp(pNode->getName(), "tree"))
        continue;
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName("id");
      if (pIdAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"id\" attribute in node tree. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        continue;
      }
      XMLLiteAttribute * pTypeAttr = pNode->getAttributeByName("type");
      if (pTypeAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"type\" attribute in node tree. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      ProgressionTree * pTree = NULL;
      char * sType = pTypeAttr->getCharValue();
      if (strcasecmp(sType, "ethnicity") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_ETHNICITY, pDebug);
      else if (strcasecmp(sType, "magic") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_MAGIC, pDebug);
      else if (strcasecmp(sType, "trait") == 0)
        pTree = new ProgressionTree(m_sObjectId, pIdAttr->getCharValue(), PROGRESSION_TRAIT, pDebug);
      else
      {
        snprintf(sError, 1024, "XML formation error: unknown attribute \"type\" for tree id %s. Check out file %s.", pIdAttr->getCharValue(), pRootNode->getName());
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
          if (strcasecmp(pDataElt->getName(), "skill") == 0
                || strcasecmp(pDataElt->getName(), "spell") == 0
                || strcasecmp(pDataElt->getName(), "modify") == 0
                || strcasecmp(pDataElt->getName(), "artifact") == 0
                || strcasecmp(pDataElt->getName(), STRING_AVATAR_XML) == 0)
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
  pDebug->notifyLoadingMessage("Skills");
  char ** sList;
  sList = new char*[MAX_SKILLS];
  for (int i = 0; i < MAX_SKILLS; i++)
    sList[i] = new char[NAME_MAX_CHARS];

  // Start process
  int nbSkills = getSkills(sList, MAX_SKILLS, NAME_MAX_CHARS, m_sObjectId);
  for (int i = 0; i < nbSkills; i++)
    m_pSkillNames->addLast(new StringObject(sList[i]));

  for (int i = 0; i < MAX_SKILLS; i++)
    delete[] sList[i];
  delete[] sList;

  // Load avatars
  snprintf(sFileName, MAX_PATH, "%s%s/shahmahs.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load units
  snprintf(sFileName, MAX_PATH, "%s%s/units.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load spells
  m_iTotalFreq = 0;
  snprintf(sFileName, MAX_PATH, "%s%s/spells.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != strcasecmp(pNode->getName(), "spell"))
        continue;
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName("id");
      if (pIdAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"id\" attribute in node spell. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        continue;
      }
      XMLLiteAttribute * pFreqAttr = pNode->getAttributeByName("freq");
      if (pFreqAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"freq\" attribute (spell frequency) in node spell. Check out file %s.", pRootNode->getName());
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
  snprintf(sFileName, MAX_PATH, "%s%s/spectiles.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != strcasecmp(pNode->getName(), "spectile"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pIdAttr = pNode->getAttributeByName("id");
      if (pIdAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"id\" attribute in node spectile. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pFreqAttr = pNode->getAttributeByName("freq");
      if (pFreqAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"freq\" attribute (special tile frequency) in node spectile. Check out file %s.", pRootNode->getName());
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
  snprintf(sFileName, MAX_PATH, "%s%s/artifacts.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    parseXMLObjectData(pRootNode, pDebug);
  }

  // Load ShahmahCreation
  snprintf(sFileName, MAX_PATH, "%s%s/creation.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL)
  {
    pDebug->notifyLoadingMessage(sFileName);
    m_pShahmahCreation = new ShahmahCreation();
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (0 != strcasecmp(pNode->getName(), "shahmah_creation"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteElement * pElt = pNode->getFirstChild();
      while (pElt != NULL)
      {
        if (0 == strcasecmp(pElt->getName(), "skill"))
        {
          XMLLiteAttribute * pFileAttr = pElt->getAttributeByName("luafile");
          XMLLiteAttribute * pParamsAttr = pElt->getAttributeByName("parameters");
          XMLLiteAttribute * pCostAttr = pElt->getAttributeByName("cost");
          if (pFileAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: missing \"luafile\" attribute in node shahmah_creation / skill. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          if (pCostAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: missing \"cost\" attribute in node shahmah_creation / skill. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          Edition * pEdition = findSkillEdition(pFileAttr->getCharValue());
          if (pEdition != NULL)
          {
            char * pParams = (pParamsAttr == NULL) ? NULL : pParamsAttr->getCharValue();
            Skill * pSkill = new Skill(pEdition->m_sObjectId, pFileAttr->getCharValue(), pParams, pDebug);
            m_pShahmahCreation->m_pSkills->addLast(pSkill, pCostAttr->getIntValue());
          }
          else
          {
            snprintf(sError, 1024, "XML error: skill \"%s\" not found in edition and dependencies. Check out file %s.", pFileAttr->getCharValue(), pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
        }
        else if (0 == strcasecmp(pElt->getName(), "ethnicity"))
        {
          XMLLiteAttribute * pNameAttr = pElt->getAttributeByName("name");
          XMLLiteAttribute * pCostAttr = pElt->getAttributeByName("cost");
          if (pNameAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: missing \"name\" attribute in node shahmah_creation / ethnicity. Check out file %s.", pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pElt = pNode->getNextChild();
            continue;
          }
          if (pCostAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: missing \"cost\" attribute in node shahmah_creation / ethnicity. Check out file %s.", pRootNode->getName());
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
              if (0 == strcasecmp(pSubElt->getName(), "image"))
              {
                XMLLiteAttribute * pNameAttr = pSubElt->getAttributeByName("name");
                if (pNameAttr == NULL)
                {
                  snprintf(sError, 1024, "XML formation error: missing \"name\" attribute in node shahmah_creation / image. Check out file %s.", pRootNode->getName());
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
            snprintf(sError, 1024, "XML error: ethnicity \"%s\" not found in edition and dependencies. Check out file %s.", pNameAttr->getCharValue(), pRootNode->getName());
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
      snprintf(sError, 1024, "Warning: Shahmah creation data is not valid (missing images or ethnicities). Check out file %s.", pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      delete m_pShahmahCreation;
      m_pShahmahCreation = NULL;
    }
  }

  // Load AIs
  snprintf(sFileName, MAX_PATH, "%s%s/ais.xml", EDITIONS_PATH, m_sObjectId);
  pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode != NULL) {
    pDebug->notifyLoadingMessage(sFileName);
    char sError[1024] = "";
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL) {
      if (0 != strcasecmp(pNode->getName(), "ai"))
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      // id and shahmah
      XMLLiteAttribute * pId = pNode->getAttributeByName("id");
      XMLLiteAttribute * pShahmah = pNode->getAttributeByName("shahmah");
      if (pId == NULL || pShahmah == NULL)
      {
        snprintf(sError, 1024, "XML formation error: missing \"id\" and/or \"shahmah\" attributes in node item. Check out file %s.", sFileName);
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
        if (strcasecmp(pSpellsNode->getName(), "spells") == 0)
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
XMLLiteElement * Edition::loadXMLFile(XMLLiteReader * pReader, const char * fileName, DebugManager * pDebug)
{
  char sError[1024] = "";
  XMLLiteElement * pRootNode = NULL;
  int error;
    pRootNode = pReader->parseFile(fileName, &error);
    if (error != 0)
  {
    if (error != XMLLITE_ERROR_CANT_OPEN_FILE)  // If file not found, don't show error (because it's not)
      pDebug->notifyXMLErrorMessage(fileName, error, pReader->getCurrentLine(), pReader->getCurrentCol());
    return NULL;
  }
  return pRootNode;
}

// -----------------------------------------------------------------
// Name : parseXMLObjectData
// -----------------------------------------------------------------
void Edition::parseXMLObjectData(XMLLiteElement * pRootNode, DebugManager * pDebug)
{
  char sError[1024] = "";
  XMLLiteElement * pNode = pRootNode->getFirstChild();
  while (pNode != NULL)
  {
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // First check if ethnicity
    if (0 == strcasecmp(pNode->getName(), "ethnicity"))
    {
      Ethnicity * pData = new Ethnicity(m_sObjectId, pNode, pDebug);
      m_pEthnicities->addLast(pData);
      pNode = pRootNode->getNextChild();
      continue;
    }
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Then, check if artifact
    else if (0 == strcasecmp(pNode->getName(), "artifact"))
    {
      char sId[NAME_MAX_CHARS] = "";
      char sTexture[MAX_PATH] = "";
      int iPosition = -1;
      bool bTwoHanded = false;
      // Get id
      XMLLiteElement * pElt = pNode->getChildByName("id");
      if (pElt == NULL)
      {
        snprintf(sError, 1024, "XML formation error: element \"id\" missing in artifact definition. Check out file %s.", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      wsafecpy(sId, NAME_MAX_CHARS, pElt->getCharValue());
      // Get texture
      pElt = pNode->getChildByName("texture");
      if (pElt == NULL)
      {
        snprintf(sError, 1024, "XML formation error: element \"texture\" missing in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      snprintf(sTexture, MAX_PATH, "%s/%s", m_sObjectId, pElt->getCharValue());
      // Get position
      pElt = pNode->getChildByName("position");
      if (pElt == NULL)
      {
        snprintf(sError, 1024, "XML formation error: element \"position\" missing in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      if (strcasecmp(pElt->getCharValue(), "head") == 0)
        iPosition = ARTIFACT_POSITION_HEAD;
      else if (strcasecmp(pElt->getCharValue(), "body") == 0)
        iPosition = ARTIFACT_POSITION_BODY;
      else if (strcasecmp(pElt->getCharValue(), "left_hand") == 0)
        iPosition = ARTIFACT_POSITION_LHAND;
      else if (strcasecmp(pElt->getCharValue(), "right_hand") == 0)
        iPosition = ARTIFACT_POSITION_RHAND;
      else if (strcasecmp(pElt->getCharValue(), "foot") == 0)
        iPosition = ARTIFACT_POSITION_FOOT;
      else
      {
        snprintf(sError, 1024, "XML formation error: invalid value for element \"position\" in artifact %s. Check out file %s.", sId, pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      // Is two-handed?
      pElt = pNode->getChildByName("two_handed");
      if (pElt != NULL)
        bTwoHanded = (pElt->getIntValue() != 0);
      // Create artifact
      Artifact * pArtifact = new Artifact(m_sObjectId, sId, sTexture, (u8)iPosition, bTwoHanded);
      // Read artifact data
      pArtifact->readLocalizedElementsFromXml(pNode);
      XMLLiteElement * pDataElt = pNode->getFirstChild();
      while (pDataElt != NULL)
      {
        const char* pName = pDataElt->getName();
        if (0 == strcasecmp(pName, "modify"))
        {
          char sKey[NAME_MAX_CHARS];
          int iValue;
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName("key");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"key\" missing in element \"modify\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sKey, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName("value");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"value\" missing in element \"modify\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          iValue = pAttr->getIntValue();
          pArtifact->addArtifactEffect(new ArtifactEffect_Charac(sKey, iValue));
        }
        else if (0 == strcasecmp(pName, "spell"))
        {
          char sEdition[NAME_MAX_CHARS];
          char sName[NAME_MAX_CHARS];
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName("edition");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"edition\" missing in element \"spell\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sEdition, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName("name");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"spell\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
          pArtifact->addArtifactEffect(new ArtifactEffect_Spell(sEdition, sName));
        }
        else if (0 == strcasecmp(pName, "skill"))
        {
          char sEdition[NAME_MAX_CHARS];
          char sName[NAME_MAX_CHARS];
          char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS] = "";
          XMLLiteAttribute * pAttr = pDataElt->getAttributeByName("edition");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"edition\" missing in element \"skill\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sEdition, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName("name");
          if (pAttr == NULL)
          {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"skill\", in artifact %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            pDataElt = pNode->getNextChild();
            continue;
          }
          wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
          pAttr = pDataElt->getAttributeByName("parameters");
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
    if (0 == strcasecmp(pNode->getName(), STRING_AVATAR_XML))
      pData = new AvatarData();
    else if (0 == strcasecmp(pNode->getName(), "unit"))
      pData = new UnitData();
    else
    {
      snprintf(sError, 1024, "XML formation error: unexpected node \"%s\". Check out file %s.", pNode->getName(), pRootNode->getName());
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
      const char* pName = pDataElt->getName();
      if (0 == strcasecmp(pName, "id"))
        wsafecpy(pData->m_sObjectId, NAME_MAX_CHARS, pDataElt->getCharValue());
      else if (0 == strcasecmp(pName, "ethnicity"))
        wsafecpy(pData->m_sEthnicityId, NAME_MAX_CHARS, pDataElt->getCharValue());
      else if (0 == strcasecmp(pName, STRING_MELEE) || 0 == strcasecmp(pName, STRING_RANGE) || 0 == strcasecmp(pName, STRING_ARMOR) ||
               0 == strcasecmp(pName, STRING_ENDURANCE) || 0 == strcasecmp(pName, STRING_SPEED) || 0 == strcasecmp(pName, STRING_ALIGNMENT))
        pData->m_lValues.insert(long_hash::value_type(pName, pDataElt->getIntValue()));
      else if (0 == strcasecmp(pName, "texture"))
        snprintf(pData->m_sTextureFilename, MAX_PATH, "%s/%s", m_sObjectId, pDataElt->getCharValue());
      else if (0 == strcasecmp(pName, "skill"))
      {
        XMLLiteAttribute * pSkillFile = pDataElt->getAttributeByName("luafile");
        XMLLiteAttribute * pSkillParams = pDataElt->getAttributeByName("parameters");
        if (pSkillFile != NULL)
        {
          Edition * pEdition = findSkillEdition(pSkillFile->getCharValue());
          if (pEdition != NULL)
          {
            char * pParams = (pSkillParams == NULL) ? NULL : pSkillParams->getCharValue();
            Skill * pSkill = new Skill(pEdition->m_sObjectId, pSkillFile->getCharValue(), pParams, pDebug);
            pData->m_pSkills->addFirst(pSkill);
          }
          else
          {
            snprintf(sError, 1024, "XML error: skill \"%s\" not found in edition and dependencies. Check out file %s.", pSkillFile->getCharValue(), pRootNode->getName());
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
  char sFileName[MAX_PATH];
  snprintf(sFileName, MAX_PATH, "%s%s/shop.xml", EDITIONS_PATH, m_sObjectId);

  XMLLiteReader reader;
  XMLLiteElement * pRootNode = loadXMLFile(&reader, sFileName, pDebug);
  if (pRootNode == NULL)
    return;

  char sError[1024] = "";
  XMLLiteElement * pNode = pRootNode->getFirstChild();
  while (pNode != NULL)
  {
    // For each item, add a ShopItem in slider
    if (0 != strcasecmp(pNode->getName(), "item"))
    {
      pNode = pRootNode->getNextChild();
      continue;
    }

    XMLLiteAttribute * pAttr = pNode->getAttributeByName("type");
    if (pAttr == NULL)
    {
      snprintf(sError, 1024, "XML formation error : missing \"type\" attribute in node item. Check out file %s.", sFileName);
      pDebug->notifyErrorMessage(sError);
      pNode = pRootNode->getNextChild();
      continue;
    }
    char * sType = pAttr->getCharValue();
    if (strcasecmp(sType, "pack") == 0)
    {
      PackShopItem * pItem = new PackShopItem();
      pItem->m_bEnabled = true;
      wsafecpy(pItem->m_sEdition, NAME_MAX_CHARS, m_sObjectId);
      pItem->m_pXml->readLocalizedElementsFromXml(pNode);
      pItem->m_pXml->findLocalizedElement(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName("cost");
      if (pChild != NULL)
        pItem->m_iCost = (int) pChild->getIntValue();
      else
        pItem->m_iCost = -1;

      // Item texture
      pChild = pNode->getChildByName("texture");
      if (pChild != NULL)
      {
        char sStr[MAX_PATH];
        snprintf(sStr, MAX_PATH, "%s/%s", m_sObjectId, pChild->getCharValue());
        pItem->m_iTexId = pShopSlider->getDisplay()->getTextureEngine()->loadTexture(sStr);
      }
      else
      {
        snprintf(sError, 1024, "Texture tag not found in file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
      }

      // Content data
      pChild = pNode->getChildByName("content");
      if (pChild != NULL)
      {
        XMLLiteElement * pSpellsNode = pChild->getFirstChild();
        while (pSpellsNode != NULL)
        {
          // For each spells node, add a "PackShopItem_content" object in the pack
          if (strcasecmp(pSpellsNode->getName(), "spells") == 0)
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
        snprintf(sError, 1024, "Content tag not found in file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
      }

      // Finally add item to slider
      checkShopItemValidity(pPlayer, pItem);
      pShopSlider->addItem(pItem);
    }
    else if (strcasecmp(sType, STRING_AVATAR_XML) == 0)
    {
      // Get avatar id
      pAttr = pNode->getAttributeByName("id");
      if (pAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error : missing \"id\" attribute in node item for type avatar. Check out file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      char * sId = pAttr->getCharValue();
      AvatarData * pAvatar = (AvatarData*) findUnitData(sId);
      if (pAvatar == NULL)
      {
        snprintf(sError, 1024, "Unknown avatar %s, defined in file %s.", sId, sFileName);
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
      char sName[SLIDER_ITEM_MAX_CHARS];
      char str[64];
      pAvatar->findLocalizedElement(sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
      i18n->getText("AVATAR", str, 64);  // "Avatar"
      snprintf(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, "%s (%s)", sName, str);

      // Item full description
      char sEthnicityName[NAME_MAX_CHARS];
      Ethnicity * pEthn = findEthnicity(pAvatar->m_sEthnicityId);
      if (pEthn == NULL)
      {
        snprintf(sError, 1024, "Error: Avatar %s should have an ethnicity.", sName);
        pDebug->notifyErrorMessage(sError);
      }
      else
        pEthn->findLocalizedElement(sEthnicityName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");

      char sDesc[DESCRIPTION_MAX_CHARS] = "";
      pAvatar->getInfos(sDesc, DESCRIPTION_MAX_CHARS, "\n", false, sEthnicityName);
      pItem->m_pXml->addLocalizedElement("description", sDesc, i18n->getCurrentLanguageName());

      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName("cost");
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
    else if (strcasecmp(sType, "artifact") == 0)
    {
      // Get artifact id
      pAttr = pNode->getAttributeByName("id");
      if (pAttr == NULL)
      {
        snprintf(sError, 1024, "XML formation error : missing \"id\" attribute in node item for type artifact. Check out file %s.", sFileName);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      char * sId = pAttr->getCharValue();
      Artifact * pArtifact = (Artifact*) findArtifact(sId);
      if (pArtifact == NULL)
      {
        snprintf(sError, 1024, "Unknown artifact %s, defined in file %s.", sId, sFileName);
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
      char sName[SLIDER_ITEM_MAX_CHARS];
      char str[64];
      pArtifact->findLocalizedElement(sName, SLIDER_ITEM_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
      i18n->getText("ARTIFACT", str, 64);  // "Avatar"
      snprintf(pItem->m_sName, SLIDER_ITEM_MAX_CHARS, "%s (%s)", sName, str);

      // Item full description
      char sDesc[DESCRIPTION_MAX_CHARS] = "";
      pArtifact->findLocalizedElement(sDesc, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
      pItem->m_pXml->addLocalizedElement("description", sDesc, i18n->getCurrentLanguageName());

      // Item cost
      XMLLiteElement * pChild = pNode->getChildByName("cost");
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
      snprintf(sError, 1024, "XML formation error : \"type\" attribute has an invalid value : %s. Check out file %s.", sType, sFileName);
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
SpellsPackContent * Edition::readSpellsPackContent(XMLLiteElement * pSpellsNode, DebugManager * pDebug, const char * sFileName)
{
  char sError[1024];
  XMLLiteAttribute * pQuantity = pSpellsNode->getAttributeByName("quantity");
  XMLLiteAttribute * pMode = pSpellsNode->getAttributeByName("mode");
  XMLLiteAttribute * pSpellId = pSpellsNode->getAttributeByName("id");
  if (pQuantity == NULL || pMode == NULL)
  {
    snprintf(sError, 1024, "XML formation error: missing attributes in node spells. Check out file %s.", sFileName);
    pDebug->notifyErrorMessage(sError);
    return NULL;
  }
  SpellsPackContent * pPack = NULL;
  char * sMode = pMode->getCharValue();
  if (strcasecmp(sMode, "fixed") == 0) {
    if (pSpellId != NULL) {
      pPack = new SpellsPackContent();
      pPack->m_iMode = PACK_MODE_FIXED;
      wsafecpy(pPack->m_sSpellId, NAME_MAX_CHARS, pSpellId->getCharValue());
    }
    else {
      snprintf(sError, 1024, "XML formation error: missing attribute 'id' in node spells. Check out file %s.", sFileName);
      pDebug->notifyErrorMessage(sError);
    }
  }
  else {
    int mode = -1;
    if (strcasecmp(sMode, "random") == 0)
      mode = PACK_MODE_RANDOM;
    else if (strcasecmp(sMode, "random-rare") == 0)
      mode = PACK_MODE_RANDOM_RARE;
    else if (strcasecmp(sMode, "random-life") == 0)
      mode = PACK_MODE_RANDOM_LIFE;
    else if (strcasecmp(sMode, "random-law") == 0)
      mode = PACK_MODE_RANDOM_LAW;
    else if (strcasecmp(sMode, "random-death") == 0)
      mode = PACK_MODE_RANDOM_DEATH;
    else if (strcasecmp(sMode, "random-chaos") == 0)
      mode = PACK_MODE_RANDOM_CHAOS;
    if (mode != -1) {
      pPack = new SpellsPackContent();
      pPack->m_iMode = mode;
    }
    else {
      snprintf(sError, 1024, "XML formation error: invalid attribute 'mode' in node spells. Check out file %s.", sFileName);
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
      if (strcmp(((AvatarShopItem*)pItem)->m_sAvatarId, p->m_sObjectId) == 0 &&
        strcmp(pItem->m_sEdition, p->m_sEdition) == 0)  // already got this one?
      {
        pItem->m_bEnabled = false;
        i18n->getText("ALREADY_OWN", pItem->m_sDisabledReason, SLIDER_ITEM_MAX_CHARS);
        return;
      }
      p = (AvatarData*) pAvatarsList->getNext(0);
    }
  }
  if (pPlayer->getCash() < pItem->m_iCost) // not rich enough, my dear
  {
    pItem->m_bEnabled = false;
    i18n->getText("NEED_MORE_CASH", pItem->m_sDisabledReason, SLIDER_ITEM_MAX_CHARS);
    return;
  }
}

// -----------------------------------------------------------------
// Name : findEthnicity
// -----------------------------------------------------------------
Ethnicity * Edition::findEthnicity(const char * strId, bool bLookDependencies)
{
  Ethnicity * pData = (Ethnicity*) m_pEthnicities->getFirst(0);
  while (pData != NULL)
  {
    if (0 == strcasecmp(strId, pData->m_sObjectId))
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
UnitData * Edition::findUnitData(const char * strId, bool bLookDependencies)
{
  UnitData * pData = (UnitData*) m_pUnits->getFirst(0);
  while (pData != NULL)
  {
    if (0 == strcasecmp(strId, pData->m_sObjectId))
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
Spell * Edition::findSpell(const char * sName, bool bLookDependencies)
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
SpecialTile * Edition::findSpecialTile(const char * sName, bool bLookDependencies)
{
  SpecialTile * pSpec = (SpecialTile*) m_pSpecTiles->getFirst(0);
  while (pSpec != NULL)
  {
    if (strcmp(sName, pSpec->getObjectName()) == 0)
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
ProgressionTree * Edition::findProgressionTree(const char * strId, bool bLookDependencies)
{
  ProgressionTree * pTree = (ProgressionTree*) m_pProgressionTrees->getFirst(0);
  while (pTree != NULL)
  {
    if (strcmp(pTree->m_sObjectId, strId) == 0)
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
Artifact * Edition::findArtifact(const char * strId, bool bLookDependencies)
{
  Artifact * pArtifact = (Artifact*) m_pArtifacts->getFirst(0);
  while (pArtifact != NULL)
  {
    if (strcmp(pArtifact->m_sObjectId, strId) == 0)
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
Edition * Edition::findSkillEdition(const char * sName)
{
  StringObject * pString = (StringObject*) m_pSkillNames->getFirst(0);
  while (pString != NULL)
  {
    if (strcmp(pString->m_sString, sName) == 0)
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
  char sPath[MAX_PATH];
  snprintf(sPath, MAX_PATH, "%s%s/", EDITIONS_PATH, m_sObjectId);
  if (!md5folder(sPath, m_sChecksum))
  {
    char sError[256];
    snprintf(sError, 256, "Error: can't do checksum on edition %s. The edition will be deactivated. Please check that you haven't opened any file from this extension.", m_sObjectId);
    pDebug->notifyErrorMessage(sError);
    deactivate();
  }
  // NOTE : checksum is not used in local. It will be sent to network peers when starting a LAN or net game, and checked against other peers checksums.
  //if (strcmp(m_sChecksum, m_sObjectId) != 0)  // checksum changed
  //{
  //  char sWarning[256];
  //  snprintf(sWarning, 256, "Warning: checksum changed on edition %s. Folder will be renamed.", m_sObjectId);
  //  pDebug->notifyErrorMessage(sWarning);
  //  char sNewPath[MAX_PATH];
  //  snprintf(sNewPath, MAX_PATH, "%s%s", EDITIONS_PATH, sChecksum);
  //  int err = _wrename(sPath, sNewPath);
  //  if (err != 0)
  //  {
  //    char sKeyword[16] = "Unknown";
  //    switch (err)
  //    {
  //    case EACCES:
  //      wsafecpy(sKeyword, 16, "EACCES");
  //      break;
  //    case ENOENT:
  //      wsafecpy(sKeyword, 16, "ENOENT");
  //      break;
  //    case EINVAL:
  //      wsafecpy(sKeyword, 16, "EINVA");
  //      break;
  //    }
  //    char sError[256];
  //    snprintf(sError, 256, "Error: can't rename folder (code: %s). The edition will be deactivated. Please check that you haven't opened any file from this extension.", sKeyword);
  //    pDebug->notifyErrorMessage(sError);
  //    desactivate();
  //  }
  //  else
  //    wsafecpy(m_sObjectId, NAME_MAX_CHARS, sChecksum);
  //}
}
