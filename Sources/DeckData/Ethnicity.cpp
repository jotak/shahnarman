// -----------------------------------------------------------------
// ETHNICITY
// -----------------------------------------------------------------
#include "Ethnicity.h"
#include "../Debug/DebugManager.h"
#include "../Data/XMLLiteReader.h"
#include <stdio.h>

// -----------------------------------------------------------------
// Name : Ethnicity
//  Constructor
// -----------------------------------------------------------------
Ethnicity::Ethnicity(const char * sEditionName, XMLLiteElement * pNode, DebugManager * pDebug)
{
    wsafecpy(m_sEdition, NAME_MAX_CHARS, sEditionName);
    m_uTownsFreqOnPlain = 0;
    m_uTownsFreqOnForest = 0;
    m_uTownsFreqOnMountain = 0;
    m_uTownsFreqOnToundra = 0;
    m_uTownsFreqOnDesert = 0;
    m_uTownsGrowth = 100;
    m_uTownsProductivity = 100;
    m_uTownsUnitProd = 100;
    m_iTownsHappinessBonus = 0;
    m_iTownsFearBonus = 0;
    m_iTownsRadiusBonus = 0;
    m_pTownNames = new ObjectList(true);
    m_pBuildingFiles = new ObjectList(true);
    m_pBaseUnits = new ObjectList(true);
    m_pHeroes = new ObjectList(true);
    for (int i = 0; i < 5; i++)
        wsafecpy(m_sTownTextures[i], MAX_PATH, "");
    wsafecpy(m_sTownBigPict, MAX_PATH, "");

    readFromNode(pNode, pDebug);
}

// -----------------------------------------------------------------
// Name : ~Ethnicity
//  Destructor
// -----------------------------------------------------------------
Ethnicity::~Ethnicity()
{
    delete m_pTownNames;
    delete m_pBuildingFiles;
    delete m_pBaseUnits;
    delete m_pHeroes;
}

// -----------------------------------------------------------------
// Name : readFromNode
// -----------------------------------------------------------------
void Ethnicity::readFromNode(XMLLiteElement * pNode, DebugManager * pDebug)
{
    readLocalizedElementsFromXml(pNode);
    XMLLiteElement * pDataElt = pNode->getFirstChild();
    while (pDataElt != NULL)
    {
        if (0 == strcasecmp(pDataElt->getName(), "id"))
            wsafecpy(m_sObjectId, NAME_MAX_CHARS, pDataElt->getCharValue());
        else if (0 == strcasecmp(pDataElt->getName(), "towns"))
        {
            XMLLiteAttribute * pTownAttr = pDataElt->getFirstAttribute();
            while (pTownAttr != NULL)
            {
                if (strcmp(pTownAttr->getName(), "freq_on_plain") == 0)
                    m_uTownsFreqOnPlain = (u8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "freq_on_forest") == 0)
                    m_uTownsFreqOnForest = (u8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "freq_on_mountain") == 0)
                    m_uTownsFreqOnMountain = (u8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "freq_on_toundra") == 0)
                    m_uTownsFreqOnToundra = (u8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "freq_on_desert") == 0)
                    m_uTownsFreqOnDesert = (u8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "growth") == 0)
                    m_uTownsGrowth = (u16) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "productivity") == 0)
                    m_uTownsProductivity = (u16) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "unitprod") == 0)
                    m_uTownsUnitProd = (u16) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "happiness_bonus") == 0)
                    m_iTownsHappinessBonus = (s8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "fear_bonus") == 0)
                    m_iTownsFearBonus = (s8) pTownAttr->getIntValue();
                else if (strcmp(pTownAttr->getName(), "radius_bonus") == 0)
                    m_iTownsRadiusBonus = (s8) pTownAttr->getIntValue();
                pTownAttr = pDataElt->getNextAttribute();
            }
            XMLLiteElement * pTownElt = pDataElt->getFirstChild();
            while (pTownElt != NULL)
            {
                if (strcmp(pTownElt->getName(), "name") == 0)
                    m_pTownNames->addLast(new TownName(pTownElt->getCharValue()));
                pTownElt = pDataElt->getNextChild();
            }
        }
        else if (0 == strcasecmp(pDataElt->getName(), "textures"))
        {
            XMLLiteAttribute * pAttr = pDataElt->getFirstAttribute();
            while (pAttr != NULL)
            {
                if (strcmp(pAttr->getName(), "town1") == 0)
                    snprintf(m_sTownTextures[0], MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                else if (strcmp(pAttr->getName(), "town2") == 0)
                    snprintf(m_sTownTextures[1], MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                else if (strcmp(pAttr->getName(), "town3") == 0)
                    snprintf(m_sTownTextures[2], MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                else if (strcmp(pAttr->getName(), "town4") == 0)
                    snprintf(m_sTownTextures[3], MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                else if (strcmp(pAttr->getName(), "town5") == 0)
                    snprintf(m_sTownTextures[4], MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                else if (strcmp(pAttr->getName(), "townbigpic") == 0)
                    snprintf(m_sTownBigPict, MAX_PATH, "%s/%s", m_sEdition, pAttr->getCharValue());
                pAttr = pDataElt->getNextAttribute();
            }
        }
        else if (0 == strcasecmp(pDataElt->getName(), "building"))
        {
            XMLLiteAttribute * pBldId = pDataElt->getAttributeByName("id");
            if (pBldId == NULL)
            {
                char sError[1024];
                snprintf(sError, 1024, "XML formation error: attribute \"id\" missing in node \"building\". Check out ethnicity %s.", m_sObjectId);
                pDebug->notifyErrorMessage(sError);
                pDataElt = pNode->getNextChild();
                continue;
            }
            XMLLiteAttribute * pBldX = pDataElt->getAttributeByName("x");
            XMLLiteAttribute * pBldY = pDataElt->getAttributeByName("y");
            if (pBldX == NULL || pBldY == NULL)
            {
                char sError[1024];
                snprintf(sError, 1024, "XML formation error: missing attributes X, Y in node \"building\". Check out ethnicity %s.", m_sObjectId);
                pDebug->notifyErrorMessage(sError);
                pDataElt = pNode->getNextChild();
                continue;
            }
            m_pBuildingFiles->addLast(new BuildingFile(pBldId->getCharValue(), pBldX->getIntValue(), pBldY->getIntValue()));
        }
        else if (0 == strcasecmp(pDataElt->getName(), "base_unit"))
        {
            XMLLiteAttribute * pIdAttr = pDataElt->getAttributeByName("id");
            XMLLiteAttribute * pCostAttr = pDataElt->getAttributeByName("cost");
            if (pIdAttr == NULL)
            {
                char sError[1024];
                snprintf(sError, 1024, "XML formation error: attribute \"id\" missing in node \"base_unit\". Check out ethnicity %s.", m_sObjectId);
                pDebug->notifyErrorMessage(sError);
                pDataElt = pNode->getNextChild();
                continue;
            }
            else if (pCostAttr == NULL)
            {
                char sError[1024];
                snprintf(sError, 1024, "XML formation error: attribute \"cost\" missing in node \"base_unit\". Check out ethnicity %s.", m_sObjectId);
                pDebug->notifyErrorMessage(sError);
                pDataElt = pNode->getNextChild();
                continue;
            }
            m_pBaseUnits->addLast(new TownUnit(pIdAttr->getCharValue(), (u8) pCostAttr->getIntValue()));
        }
        else if (0 == strcasecmp(pDataElt->getName(), "heroe"))
        {
            XMLLiteAttribute * pAttr = pDataElt->getAttributeByName("id");
            if (pAttr == NULL)
            {
                char sError[1024];
                snprintf(sError, 1024, "XML formation error: attribute \"id\" missing in node \"heroe\". Check out ethnicity %s.", m_sObjectId);
                pDebug->notifyErrorMessage(sError);
                pDataElt = pNode->getNextChild();
                continue;
            }
            m_pHeroes->addLast(new TownHeroe(pAttr->getCharValue()));
        }
        pDataElt = pNode->getNextChild();
    }
}

// -----------------------------------------------------------------
// Name : resetUsedHeroes
// -----------------------------------------------------------------
void Ethnicity::resetUsedHeroes()
{
    TownHeroe * pName = (TownHeroe*) m_pHeroes->getFirst(0);
    while (pName != NULL)
    {
        pName->m_bUsed = false;
        pName = (TownHeroe*) m_pHeroes->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : resetUsedTownNames
// -----------------------------------------------------------------
void Ethnicity::resetUsedTownNames()
{
    TownName * pName = (TownName*) m_pTownNames->getFirst(0);
    while (pName != NULL)
    {
        pName->m_bUsed = false;
        pName = (TownName*) m_pTownNames->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : getRandomTownName
// -----------------------------------------------------------------
char * Ethnicity::getRandomTownName()
{
    int nbUnused = 0;
    TownName * pName = (TownName*) m_pTownNames->getFirst(0);
    while (pName != NULL)
    {
        if (!pName->m_bUsed)
            nbUnused++;
        pName = (TownName*) m_pTownNames->getNext(0);
    }
    if (nbUnused == 0)
        return NULL;
    int iRnd = getRandom(nbUnused);
    pName = (TownName*) m_pTownNames->getFirst(0);
    while (pName != NULL)
    {
        if (!pName->m_bUsed)
        {
            iRnd--;
            if (iRnd < 0)
            {
                pName->m_bUsed = true;
                return pName->m_sName;
            }
        }
        pName = (TownName*) m_pTownNames->getNext(0);
    }
    return NULL;
}
