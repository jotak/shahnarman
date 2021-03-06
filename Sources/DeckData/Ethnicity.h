#ifndef _ETHNICITY_H
#define _ETHNICITY_H

#include "../Data/XMLObject.h"

class XMLLiteElement;
class DebugManager;

class Ethnicity : public XMLObject
{
public:
    Ethnicity(const char * sEditionName, XMLLiteElement * pNode, DebugManager * pDebug);
    ~Ethnicity();

    void resetUsedTownNames();
    void resetUsedHeroes();
    char * getRandomTownName();

    char m_sEdition[NAME_MAX_CHARS];
    u8 m_uTownsFreqOnPlain;
    u8 m_uTownsFreqOnForest;
    u8 m_uTownsFreqOnMountain;
    u8 m_uTownsFreqOnToundra;
    u8 m_uTownsFreqOnDesert;
    u16 m_uTownsGrowth;
    u16 m_uTownsProductivity;
    u16 m_uTownsUnitProd;
    s8 m_iTownsHappinessBonus;
    s8 m_iTownsFearBonus;
    s8 m_iTownsRadiusBonus;
    ObjectList * m_pTownNames;
    ObjectList * m_pBuildingFiles;
    ObjectList * m_pBaseUnits;
    ObjectList * m_pHeroes;
    char m_sTownTextures[5][MAX_PATH];
    char m_sTownBigPict[MAX_PATH];

    class TownName : public BaseObject
    {
    public:
        TownName(const char * pName)
        {
            wsafecpy(m_sName, NAME_MAX_CHARS, pName);
            m_bUsed = false;
        };
        char m_sName[NAME_MAX_CHARS];
        bool m_bUsed;
    };
    class BuildingFile : public BaseObject
    {
    public:
        BuildingFile(const char * sId, int x, int y)
        {
            wsafecpy(m_sId, NAME_MAX_CHARS, sId);
            m_iX = x;
            m_iY = y;
        };
        char m_sId[NAME_MAX_CHARS];
        int m_iX, m_iY;
        bool m_bBase;
    };
    class TownUnit : public BaseObject
    {
    public:
        TownUnit(const char * sId, u8 uCost)
        {
            wsafecpy(m_sId, NAME_MAX_CHARS, sId);
            m_uCost = uCost;
        };
        char m_sId[NAME_MAX_CHARS];
        u8 m_uCost;
    };
    class TownHeroe : public BaseObject
    {
    public:
        TownHeroe(const char * sId)
        {
            wsafecpy(m_sId, NAME_MAX_CHARS, sId);
            m_bUsed = false;
        };
        char m_sId[NAME_MAX_CHARS];
        bool m_bUsed;
    };

private:
    void readFromNode(XMLLiteElement * pNode, DebugManager * pDebug);
};

#endif
