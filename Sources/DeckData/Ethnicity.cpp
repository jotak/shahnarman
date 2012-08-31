// -----------------------------------------------------------------
// ETHNICITY
// -----------------------------------------------------------------
#include "Ethnicity.h"
#include "../Debug/DebugManager.h"
#include "../Data/XMLLiteReader.h"

// -----------------------------------------------------------------
// Name : Ethnicity
//  Constructor
// -----------------------------------------------------------------
Ethnicity::Ethnicity(wchar_t * sEditionName, XMLLiteElement * pNode, DebugManager * pDebug)
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
    wsafecpy(m_sTownTextures[i], MAX_PATH, L"");
  wsafecpy(m_sTownBigPict, MAX_PATH, L"");

  readFromNode(pNode, pDebug);
}

// -----------------------------------------------------------------
// Name : ~Ethnicity
//  Destructor
// -----------------------------------------------------------------
Ethnicity::~Ethnicity()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Ethnicity\n");
#endif
  delete m_pTownNames;
  delete m_pBuildingFiles;
  delete m_pBaseUnits;
  delete m_pHeroes;
#ifdef DBG_VERBOSE1
  printf("End destroy Ethnicity\n");
#endif
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
    if (0 == _wcsicmp(pDataElt->getName(), L"id"))
      wsafecpy(m_sObjectId, NAME_MAX_CHARS, pDataElt->getCharValue());
    else if (0 == _wcsicmp(pDataElt->getName(), L"towns"))
    {
      XMLLiteAttribute * pTownAttr = pDataElt->getFirstAttribute();
      while (pTownAttr != NULL)
      {
        if (wcscmp(pTownAttr->getName(), L"freq_on_plain") == 0)
          m_uTownsFreqOnPlain = (u8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"freq_on_forest") == 0)
          m_uTownsFreqOnForest = (u8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"freq_on_mountain") == 0)
          m_uTownsFreqOnMountain = (u8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"freq_on_toundra") == 0)
          m_uTownsFreqOnToundra = (u8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"freq_on_desert") == 0)
          m_uTownsFreqOnDesert = (u8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"growth") == 0)
          m_uTownsGrowth = (u16) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"productivity") == 0)
          m_uTownsProductivity = (u16) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"unitprod") == 0)
          m_uTownsUnitProd = (u16) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"happiness_bonus") == 0)
          m_iTownsHappinessBonus = (s8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"fear_bonus") == 0)
          m_iTownsFearBonus = (s8) pTownAttr->getIntValue();
        else if (wcscmp(pTownAttr->getName(), L"radius_bonus") == 0)
          m_iTownsRadiusBonus = (s8) pTownAttr->getIntValue();
        pTownAttr = pDataElt->getNextAttribute();
      }
      XMLLiteElement * pTownElt = pDataElt->getFirstChild();
      while (pTownElt != NULL)
      {
        if (wcscmp(pTownElt->getName(), L"name") == 0)
          m_pTownNames->addLast(new TownName(pTownElt->getCharValue()));
        pTownElt = pDataElt->getNextChild();
      }
    }
    else if (0 == _wcsicmp(pDataElt->getName(), L"textures"))
    {
      XMLLiteAttribute * pAttr = pDataElt->getFirstAttribute();
      while (pAttr != NULL)
      {
        if (wcscmp(pAttr->getName(), L"town1") == 0)
          swprintf_s(m_sTownTextures[0], MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        else if (wcscmp(pAttr->getName(), L"town2") == 0)
          swprintf_s(m_sTownTextures[1], MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        else if (wcscmp(pAttr->getName(), L"town3") == 0)
          swprintf_s(m_sTownTextures[2], MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        else if (wcscmp(pAttr->getName(), L"town4") == 0)
          swprintf_s(m_sTownTextures[3], MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        else if (wcscmp(pAttr->getName(), L"town5") == 0)
          swprintf_s(m_sTownTextures[4], MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        else if (wcscmp(pAttr->getName(), L"townbigpic") == 0)
          swprintf_s(m_sTownBigPict, MAX_PATH, L"%s/%s", m_sEdition, pAttr->getCharValue());
        pAttr = pDataElt->getNextAttribute();
      }
    }
    else if (0 == _wcsicmp(pDataElt->getName(), L"building"))
    {
      XMLLiteAttribute * pBldId = pDataElt->getAttributeByName(L"id");
      if (pBldId == NULL)
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"XML formation error: attribute \"id\" missing in node \"building\". Check out ethnicity %s.", m_sObjectId);
        pDebug->notifyErrorMessage(sError);
        pDataElt = pNode->getNextChild();
        continue;
      }
      XMLLiteAttribute * pBldX = pDataElt->getAttributeByName(L"x");
      XMLLiteAttribute * pBldY = pDataElt->getAttributeByName(L"y");
      if (pBldX == NULL || pBldY == NULL)
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"XML formation error: missing attributes X, Y in node \"building\". Check out ethnicity %s.", m_sObjectId);
        pDebug->notifyErrorMessage(sError);
        pDataElt = pNode->getNextChild();
        continue;
      }
	    m_pBuildingFiles->addLast(new BuildingFile(pBldId->getCharValue(), pBldX->getIntValue(), pBldY->getIntValue()));
    }
    else if (0 == _wcsicmp(pDataElt->getName(), L"base_unit"))
    {
      XMLLiteAttribute * pIdAttr = pDataElt->getAttributeByName(L"id");
      XMLLiteAttribute * pCostAttr = pDataElt->getAttributeByName(L"cost");
      if (pIdAttr == NULL)
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"XML formation error: attribute \"id\" missing in node \"base_unit\". Check out ethnicity %s.", m_sObjectId);
        pDebug->notifyErrorMessage(sError);
        pDataElt = pNode->getNextChild();
        continue;
      }
      else if (pCostAttr == NULL)
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"XML formation error: attribute \"cost\" missing in node \"base_unit\". Check out ethnicity %s.", m_sObjectId);
        pDebug->notifyErrorMessage(sError);
        pDataElt = pNode->getNextChild();
        continue;
      }
      m_pBaseUnits->addLast(new TownUnit(pIdAttr->getCharValue(), (u8) pCostAttr->getIntValue()));
    }
    else if (0 == _wcsicmp(pDataElt->getName(), L"heroe"))
    {
      XMLLiteAttribute * pAttr = pDataElt->getAttributeByName(L"id");
      if (pAttr == NULL)
      {
        wchar_t sError[1024];
        swprintf_s(sError, 1024, L"XML formation error: attribute \"id\" missing in node \"heroe\". Check out ethnicity %s.", m_sObjectId);
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
wchar_t * Ethnicity::getRandomTownName()
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
