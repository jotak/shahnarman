// -----------------------------------------------------------------
// PROGRESSION TREE
// -----------------------------------------------------------------
#include "ProgressionTree.h"
#include "ProgressionElement.h"
#include "../Debug/DebugManager.h"
#include "../Data/XMLLiteReader.h"
#include "Edition.h"

// -----------------------------------------------------------------
// Name : ProgressionTree
//  Constructor
// -----------------------------------------------------------------
ProgressionTree::ProgressionTree(wchar_t * sEdition, wchar_t * sName, u8 uType, DebugManager * pDebug)
{
  wsafecpy(m_sEdition, NAME_MAX_CHARS, sEdition);
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, sName);
  m_uType = uType;
  for (int i = 0; i < NB_PROGRESSION_LEVELS; i++)
    m_pElements[i] = new ObjectList(true);
  m_pBaseEffects = new ObjectList(true);

  // Parse file
  wchar_t sFileName[MAX_PATH];
  XMLLiteReader reader;
  swprintf(sFileName, MAX_PATH, L"%s%s/progressions/%s.xml", EDITIONS_PATH, m_sEdition, m_sObjectId);
  wchar_t sError[1024] = L"";
  XMLLiteElement * pRootNode = NULL;
  try {
    pRootNode = reader.parseFile(sFileName);
  }
  catch (int errorCode)
  {
    pDebug->notifyXMLErrorMessage(sFileName, errorCode, reader.getCurrentLine(), reader.getCurrentCol());
    return;
  }
  if (pRootNode != NULL)
  {
    XMLLiteElement * pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (_wcsicmp(pNode->getName(), L"level") != 0)
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pAttr = pNode->getAttributeByName(L"id");
      if (pAttr == NULL)
      {
        swprintf(sError, 1024, L"XML formation error in progression tree %s: missing level id", pRootNode->getName());
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      u8 uLevel = (u8) pAttr->getIntValue();
      if (uLevel >= NB_PROGRESSION_LEVELS)
      {
        swprintf(sError, 1024, L"XML formation error in progression tree %s: invalid level id (%d)", pRootNode->getName(), (int)uLevel);
        pDebug->notifyErrorMessage(sError);
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteElement * pOption = pNode->getFirstChild();
      while (pOption != NULL)
      {
        if (_wcsicmp(pOption->getName(), L"option") != 0)
        {
          pOption = pNode->getNextChild();
          continue;
        }
        wchar_t sTexture[MAX_PATH];
        wchar_t sId[NAME_MAX_CHARS];
        pAttr = pOption->getAttributeByName(L"id");
        if (pAttr == NULL)
        {
          swprintf(sError, 1024, L"XML formation error in progression tree %s: missing option id", pRootNode->getName());
          pDebug->notifyErrorMessage(sError);
          pOption = pNode->getNextChild();
          continue;
        }
        wsafecpy(sId, NAME_MAX_CHARS, pAttr->getCharValue());
        pAttr = pOption->getAttributeByName(L"texture");
        if (pAttr == NULL)
        {
          swprintf(sError, 1024, L"XML formation error in progression tree %s: missing option texture", pRootNode->getName());
          pDebug->notifyErrorMessage(sError);
          pOption = pNode->getNextChild();
          continue;
        }
        swprintf(sTexture, MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        ProgressionElement * pElement = new ProgressionElement(sId, sTexture, uLevel, this);
        m_pElements[uLevel]->addLast(pElement);
        pElement->readLocalizedElementsFromXml(pOption);
        XMLLiteElement * pOptData = pOption->getFirstChild();
        while (pOptData != NULL)
        {
          if (_wcsicmp(pOptData->getName(), L"skill") == 0
                || _wcsicmp(pOptData->getName(), L"spell") == 0
                || _wcsicmp(pOptData->getName(), L"modify") == 0
                || _wcsicmp(pOptData->getName(), L"artifact") == 0
                || _wcsicmp(pOptData->getName(), STRING_AVATAR_XML) == 0)
          {
            ProgressionEffect * pEffect = readXMLEffect(pOptData, pRootNode, sId, pDebug);
            if (pEffect != NULL)
              pElement->m_pEffects->addLast(pEffect);
          }
          pOptData = pOption->getNextChild();
        }
        pOption = pNode->getNextChild();
      }
      pNode = pRootNode->getNextChild();
    }

    // Now parse "tree" (in same file) to build the tree
    pNode = pRootNode->getFirstChild();
    while (pNode != NULL)
    {
      if (_wcsicmp(pNode->getName(), L"tree") != 0)
      {
        pNode = pRootNode->getNextChild();
        continue;
      }
      XMLLiteElement * pOption = pNode->getFirstChild();
      while (pOption != NULL)
      {
        if (_wcsicmp(pOption->getName(), L"option") != 0)
        {
          pOption = pNode->getNextChild();
          continue;
        }
        wchar_t sId[NAME_MAX_CHARS];
        XMLLiteAttribute * pAttr = pOption->getAttributeByName(L"id");
        if (pAttr == NULL)
        {
          swprintf(sError, 1024, L"XML formation error in progression tree %s: missing option id", pRootNode->getName());
          pDebug->notifyErrorMessage(sError);
          pOption = pNode->getNextChild();
          continue;
        }
        wsafecpy(sId, NAME_MAX_CHARS, pAttr->getCharValue());
        ProgressionElement * pElement = findElement(sId);
        if (pElement == NULL)
        {
          swprintf(sError, 1024, L"XML formation error in progression tree %s: undefined option id (%s)", pRootNode->getName(), sId);
          pDebug->notifyErrorMessage(sError);
          pOption = pNode->getNextChild();
          continue;
        }
        XMLLiteElement * pOptData = pOption->getFirstChild();
        while (pOptData != NULL)
        {
          if (_wcsicmp(pOptData->getName(), L"child") == 0)
          {
            wchar_t * sChildName = pOptData->getCharValue();
            ProgressionElement * pChild = findElement(sChildName);
            if (pChild == NULL)
            {
              swprintf(sError, 1024, L"XML formation error in progression tree %s: unknown child for option %s", pRootNode->getName(), sId);
              pDebug->notifyErrorMessage(sError);
              pOptData = pOption->getNextChild();
              continue;
            }
            if (pChild->m_uLevel != pElement->m_uLevel + 1)
            {
              swprintf(sError, 1024, L"XML formation error in progression tree %s: child %s should be level 'n+1' for option %s", pRootNode->getName(), sChildName, sId);
              pDebug->notifyErrorMessage(sError);
              pOptData = pOption->getNextChild();
              continue;
            }
            pElement->m_pChildren->addLast(pChild);
            pChild->m_iNbParents++;
          }
          pOptData = pOption->getNextChild();
        }
        pOption = pNode->getNextChild();
      }
      pNode = pRootNode->getNextChild();
    }
  }

  // Check mal formation
  for (int i = 1; i < NB_PROGRESSION_LEVELS; i++)
  {
    ProgressionElement * pElt = (ProgressionElement*)m_pElements[i]->getFirst(0);
    while (pElt != NULL)
    {
      if (pElt->m_iNbParents == 0)
      {
        wchar_t sError[1024];
        swprintf(sError, 1024, L"Warning in progression tree formation: element %s has no parent in tree %s", pElt->m_sObjectId, m_sObjectId);
        pDebug->notifyErrorMessage(sError);
      }
      pElt = (ProgressionElement*)m_pElements[i]->getNext(0);
    }
  }
}

// -----------------------------------------------------------------
// Name : ~ProgressionTree
//  Destructor
// -----------------------------------------------------------------
ProgressionTree::~ProgressionTree()
{
  for (int i = 0; i < NB_PROGRESSION_LEVELS; i++)
    delete m_pElements[i];
  delete m_pBaseEffects;
}

// -----------------------------------------------------------------
// Name : findElement
// -----------------------------------------------------------------
ProgressionElement * ProgressionTree::findElement(wchar_t * sId)
{
  for (int i = 0; i < NB_PROGRESSION_LEVELS; i++)
  {
    ProgressionElement * pElt = (ProgressionElement*) m_pElements[i]->getFirst(0);
    while (pElt != NULL)
    {
      if (wcscmp(pElt->m_sObjectId, sId) == 0)
        return pElt;
      pElt = (ProgressionElement*) m_pElements[i]->getNext(0);
    }
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : readXMLEffect
// -----------------------------------------------------------------
ProgressionEffect * ProgressionTree::readXMLEffect(XMLLiteElement * pNode, XMLLiteElement * pRootNode, wchar_t * sId, DebugManager * pDebug)
{
  wchar_t sError[1024];
  XMLLiteAttribute * pAttr = NULL;
  if (_wcsicmp(pNode->getName(), L"skill") == 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    wchar_t sParams[LUA_FUNCTION_PARAMS_MAX_CHARS] = L"";
    pAttr = pNode->getAttributeByName(L"name");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"skill\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
    pAttr = pNode->getAttributeByName(L"parameters");
    if (pAttr != NULL)
      wsafecpy(sParams, LUA_FUNCTION_PARAMS_MAX_CHARS, pAttr->getCharValue());
    return new ProgressionEffect_Skill(m_sEdition, sName, sParams);
  }
  else if (_wcsicmp(pNode->getName(), L"spell") == 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pAttr = pNode->getAttributeByName(L"name");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"spell\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
    return new ProgressionEffect_Spell(m_sEdition, sName);
  }
  else if (_wcsicmp(pNode->getName(), L"modify") == 0)
  {
    wchar_t sKey[NAME_MAX_CHARS];
    int iValue;
    pAttr = pNode->getAttributeByName(L"key");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"key\" missing in element \"modify\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    wsafecpy(sKey, NAME_MAX_CHARS, pAttr->getCharValue());
    pAttr = pNode->getAttributeByName(L"value");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"value\" missing in element \"modify\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    iValue = pAttr->getIntValue();
    return new ProgressionEffect_Charac(sKey, iValue);
  }
  else if (_wcsicmp(pNode->getName(), L"artifact") == 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pAttr = pNode->getAttributeByName(L"name");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"artefact\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
    return new ProgressionEffect_Artifact(m_sEdition, sName);
  }
  else if (_wcsicmp(pNode->getName(), STRING_AVATAR_XML) == 0)
  {
    wchar_t sName[NAME_MAX_CHARS];
    pAttr = pNode->getAttributeByName(L"name");
    if (pAttr == NULL)
    {
      swprintf(sError, 1024, L"XML formation error: attribute \"name\" missing in element \"Shahmah\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
      pDebug->notifyErrorMessage(sError);
      return NULL;
    }
    wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
    return new ProgressionEffect_Avatar(m_sEdition, sName);
  }
  return NULL;
}

// -----------------------------------------------------------------
// Name : getDescription
// -----------------------------------------------------------------
wchar_t * ProgressionTree::getDescription(wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient)
{
  return ProgressionElement::getDescription(this, m_pBaseEffects, sBuf, iBufSize, pLocalClient);
}
