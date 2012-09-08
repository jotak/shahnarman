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
ProgressionTree::ProgressionTree(char * sEdition, char * sName, u8 uType, DebugManager * pDebug)
{
    wsafecpy(m_sEdition, NAME_MAX_CHARS, sEdition);
    wsafecpy(m_sObjectId, NAME_MAX_CHARS, sName);
    m_uType = uType;
    for (int i = 0; i < NB_PROGRESSION_LEVELS; i++)
        m_pElements[i] = new ObjectList(true);
    m_pBaseEffects = new ObjectList(true);

    // Parse file
    char sFileName[MAX_PATH];
    XMLLiteReader reader;
    snprintf(sFileName, MAX_PATH, "%s%s/progressions/%s.xml", EDITIONS_PATH, m_sEdition, m_sObjectId);
    char sError[1024] = "";
    XMLLiteElement * pRootNode = NULL;
    int error;
    pRootNode = reader.parseFile(sFileName, &error);
    if (error != 0)
    {
        pDebug->notifyXMLErrorMessage(sFileName, error, reader.getCurrentLine(), reader.getCurrentCol());
        return;
    }
    if (pRootNode != NULL)
    {
        XMLLiteElement * pNode = pRootNode->getFirstChild();
        while (pNode != NULL)
        {
            if (strcasecmp(pNode->getName(), "leve") != 0)
            {
                pNode = pRootNode->getNextChild();
                continue;
            }
            XMLLiteAttribute * pAttr = pNode->getAttributeByName("id");
            if (pAttr == NULL)
            {
                snprintf(sError, 1024, "XML formation error in progression tree %s: missing level id", pRootNode->getName());
                pDebug->notifyErrorMessage(sError);
                pNode = pRootNode->getNextChild();
                continue;
            }
            u8 uLevel = (u8) pAttr->getIntValue();
            if (uLevel >= NB_PROGRESSION_LEVELS)
            {
                snprintf(sError, 1024, "XML formation error in progression tree %s: invalid level id (%d)", pRootNode->getName(), (int)uLevel);
                pDebug->notifyErrorMessage(sError);
                pNode = pRootNode->getNextChild();
                continue;
            }
            XMLLiteElement * pOption = pNode->getFirstChild();
            while (pOption != NULL)
            {
                if (strcasecmp(pOption->getName(), "option") != 0)
                {
                    pOption = pNode->getNextChild();
                    continue;
                }
                char sTexture[MAX_PATH];
                char sId[NAME_MAX_CHARS];
                pAttr = pOption->getAttributeByName("id");
                if (pAttr == NULL)
                {
                    snprintf(sError, 1024, "XML formation error in progression tree %s: missing option id", pRootNode->getName());
                    pDebug->notifyErrorMessage(sError);
                    pOption = pNode->getNextChild();
                    continue;
                }
                wsafecpy(sId, NAME_MAX_CHARS, pAttr->getCharValue());
                pAttr = pOption->getAttributeByName("texture");
                if (pAttr == NULL)
                {
                    snprintf(sError, 1024, "XML formation error in progression tree %s: missing option texture", pRootNode->getName());
                    pDebug->notifyErrorMessage(sError);
                    pOption = pNode->getNextChild();
                    continue;
                }
                snprintf(sTexture, MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                ProgressionElement * pElement = new ProgressionElement(sId, sTexture, uLevel, this);
                m_pElements[uLevel]->addLast(pElement);
                pElement->readLocalizedElementsFromXml(pOption);
                XMLLiteElement * pOptData = pOption->getFirstChild();
                while (pOptData != NULL)
                {
                    if (strcasecmp(pOptData->getName(), "skill") == 0
                            || strcasecmp(pOptData->getName(), "spell") == 0
                            || strcasecmp(pOptData->getName(), "modify") == 0
                            || strcasecmp(pOptData->getName(), "artifact") == 0
                            || strcasecmp(pOptData->getName(), STRING_AVATAR_XML) == 0)
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
            if (strcasecmp(pNode->getName(), "tree") != 0)
            {
                pNode = pRootNode->getNextChild();
                continue;
            }
            XMLLiteElement * pOption = pNode->getFirstChild();
            while (pOption != NULL)
            {
                if (strcasecmp(pOption->getName(), "option") != 0)
                {
                    pOption = pNode->getNextChild();
                    continue;
                }
                char sId[NAME_MAX_CHARS];
                XMLLiteAttribute * pAttr = pOption->getAttributeByName("id");
                if (pAttr == NULL)
                {
                    snprintf(sError, 1024, "XML formation error in progression tree %s: missing option id", pRootNode->getName());
                    pDebug->notifyErrorMessage(sError);
                    pOption = pNode->getNextChild();
                    continue;
                }
                wsafecpy(sId, NAME_MAX_CHARS, pAttr->getCharValue());
                ProgressionElement * pElement = findElement(sId);
                if (pElement == NULL)
                {
                    snprintf(sError, 1024, "XML formation error in progression tree %s: undefined option id (%s)", pRootNode->getName(), sId);
                    pDebug->notifyErrorMessage(sError);
                    pOption = pNode->getNextChild();
                    continue;
                }
                XMLLiteElement * pOptData = pOption->getFirstChild();
                while (pOptData != NULL)
                {
                    if (strcasecmp(pOptData->getName(), "child") == 0)
                    {
                        char * sChildName = pOptData->getCharValue();
                        ProgressionElement * pChild = findElement(sChildName);
                        if (pChild == NULL)
                        {
                            snprintf(sError, 1024, "XML formation error in progression tree %s: unknown child for option %s", pRootNode->getName(), sId);
                            pDebug->notifyErrorMessage(sError);
                            pOptData = pOption->getNextChild();
                            continue;
                        }
                        if (pChild->m_uLevel != pElement->m_uLevel + 1)
                        {
                            snprintf(sError, 1024, "XML formation error in progression tree %s: child %s should be level 'n+1' for option %s", pRootNode->getName(), sChildName, sId);
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
                char sError[1024];
                snprintf(sError, 1024, "Warning in progression tree formation: element %s has no parent in tree %s", pElt->m_sObjectId, m_sObjectId);
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
ProgressionElement * ProgressionTree::findElement(char * sId)
{
    for (int i = 0; i < NB_PROGRESSION_LEVELS; i++)
    {
        ProgressionElement * pElt = (ProgressionElement*) m_pElements[i]->getFirst(0);
        while (pElt != NULL)
        {
            if (strcmp(pElt->m_sObjectId, sId) == 0)
                return pElt;
            pElt = (ProgressionElement*) m_pElements[i]->getNext(0);
        }
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : readXMLEffect
// -----------------------------------------------------------------
ProgressionEffect * ProgressionTree::readXMLEffect(XMLLiteElement * pNode, XMLLiteElement * pRootNode, char * sId, DebugManager * pDebug)
{
    char sError[1024];
    XMLLiteAttribute * pAttr = NULL;
    if (strcasecmp(pNode->getName(), "skill") == 0)
    {
        char sName[NAME_MAX_CHARS];
        char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS] = "";
        pAttr = pNode->getAttributeByName("name");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"skill\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            return NULL;
        }
        wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
        pAttr = pNode->getAttributeByName("parameters");
        if (pAttr != NULL)
            wsafecpy(sParams, LUA_FUNCTION_PARAMS_MAX_CHARS, pAttr->getCharValue());
        return new ProgressionEffect_Skill(m_sEdition, sName, sParams);
    }
    else if (strcasecmp(pNode->getName(), "spell") == 0)
    {
        char sName[NAME_MAX_CHARS];
        pAttr = pNode->getAttributeByName("name");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"spell\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            return NULL;
        }
        wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
        return new ProgressionEffect_Spell(m_sEdition, sName);
    }
    else if (strcasecmp(pNode->getName(), "modify") == 0)
    {
        char sKey[NAME_MAX_CHARS];
        int iValue;
        pAttr = pNode->getAttributeByName("key");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"key\" missing in element \"modify\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            return NULL;
        }
        wsafecpy(sKey, NAME_MAX_CHARS, pAttr->getCharValue());
        pAttr = pNode->getAttributeByName("value");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"value\" missing in element \"modify\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            return NULL;
        }
        iValue = pAttr->getIntValue();
        return new ProgressionEffect_Charac(sKey, iValue);
    }
    else if (strcasecmp(pNode->getName(), "artifact") == 0)
    {
        char sName[NAME_MAX_CHARS];
        pAttr = pNode->getAttributeByName("name");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"artefact\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
            pDebug->notifyErrorMessage(sError);
            return NULL;
        }
        wsafecpy(sName, NAME_MAX_CHARS, pAttr->getCharValue());
        return new ProgressionEffect_Artifact(m_sEdition, sName);
    }
    else if (strcasecmp(pNode->getName(), STRING_AVATAR_XML) == 0)
    {
        char sName[NAME_MAX_CHARS];
        pAttr = pNode->getAttributeByName("name");
        if (pAttr == NULL)
        {
            snprintf(sError, 1024, "XML formation error: attribute \"name\" missing in element \"Shahmah\", in tree %s definition. Check out file %s.", sId, pRootNode->getName());
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
char * ProgressionTree::getDescription(char * sBuf, int iBufSize, LocalClient * pLocalClient)
{
    return ProgressionElement::getDescription(this, m_pBaseEffects, sBuf, iBufSize, pLocalClient);
}
