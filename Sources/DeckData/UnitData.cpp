// -----------------------------------------------------------------
// UNIT DATA
// -----------------------------------------------------------------
#include "UnitData.h"
#include "../Data/LocalisationTool.h"
#include "../Debug/DebugManager.h"
#include "../Data/Serializer.h"

// -----------------------------------------------------------------
// Name : UnitData
//  Constructor
// -----------------------------------------------------------------
UnitData::UnitData()
{
    m_pSkills = new ObjectList(true);
    wsafecpy(m_sTextureFilename, MAX_PATH, "");
    wsafecpy(m_sEdition, NAME_MAX_CHARS, "");
    wsafecpy(m_sEthnicityId, NAME_MAX_CHARS, "");
}

// -----------------------------------------------------------------
// Name : ~UnitData
//  Destructor
// -----------------------------------------------------------------
UnitData::~UnitData()
{
    delete m_pSkills;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void UnitData::serialize(Serializer * pSerializer)
{
    pSerializer->writeString(m_sEdition);
    pSerializer->writeString(m_sObjectId);
    pSerializer->writeString(m_sEthnicityId);
    pSerializer->writeString(m_sTextureFilename);
    // hashmap values
    pSerializer->writeLong((long) m_lValues.size());
    long_hash::iterator it;
    for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
    {
        pSerializer->writeString(it->first.c_str());
        pSerializer->writeLong((long) it->second);
    }
    // skills ref
    pSerializer->writeLong((long) m_pSkills->size);
    Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
    while (pSkill != NULL)
    {
        pSerializer->writeString(pSkill->getObjectEdition());
        pSerializer->writeString(pSkill->getObjectName());
        pSerializer->writeString(pSkill->getParameters());
        pSkill = (Skill*) m_pSkills->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void UnitData::deserialize(Serializer * pSerializer, DebugManager * pDebug)
{
    pSerializer->readString(m_sEdition);
    pSerializer->readString(m_sObjectId);
    pSerializer->readString(m_sEthnicityId);
    pSerializer->readString(m_sTextureFilename);
    // hashmap values
    int len = pSerializer->readLong();
    for (int i = 0; i < len; i++)
    {
        char str[1024];
        long val = 0;
        pSerializer->readString(str);
        val = pSerializer->readLong();
        m_lValues.insert(long_hash::value_type(str, val));
    }
    // skills ref
    len = pSerializer->readLong();
    for (int i = 0; i < len; i++)
    {
        char sEdition[NAME_MAX_CHARS];
        char sName[NAME_MAX_CHARS];
        char sParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
        pSerializer->readString(sEdition);
        pSerializer->readString(sName);
        pSerializer->readString(sParameters);
        Skill * pSkill = new Skill(sEdition, sName, sParameters, pDebug);
        m_pSkills->addLast(pSkill);
    }
}

// -----------------------------------------------------------------
// Name : getInfos
// -----------------------------------------------------------------
char * UnitData::getInfos(char * sBuf, int iSize, const char * sSeparator, bool bName, const char * sEthn, bool bAlign, bool bCharacs, bool bSkills, bool bDesc)
{
    // Item full description
    wsafecpy(sBuf, iSize, "");
    char sTemp[512];
    char s2P[8];
    char sSep[8] = "";
    i18n->getText("2P", s2P, 8);

    // Name
    if (bName)
    {
        findLocalizedElement(sTemp, 512, i18n->getCurrentLanguageName(), "name");
        wsafecat(sBuf, iSize, sTemp);
        wsafecpy(sSep, 8, sSeparator);
    }

    // Ethnicity
    if (sEthn != NULL)
    {
        wsafecat(sBuf, iSize, sSep);
        i18n->getText1stUp("ETHNICITY", sTemp, 512);
        wsafecat(sBuf, iSize, sTemp);
        wsafecat(sBuf, iSize, s2P);
        wsafecat(sBuf, iSize, sEthn);
        wsafecpy(sSep, 8, sSeparator);
    }

    // Alignment
    if (bAlign)
    {
        wsafecat(sBuf, iSize, sSep);
        i18n->getText1stUp(STRING_ALIGNMENT, sTemp, 512);
        wsafecat(sBuf, iSize, sTemp);
        wsafecat(sBuf, iSize, s2P);
        int iAlign = 0;
        long_hash::iterator it = m_lValues.find(STRING_ALIGNMENT);
        if (it != m_lValues.end())
            iAlign = it->second;
        getAlignmentInfos(iAlign, sTemp, 512);
        wsafecat(sBuf, iSize, sTemp);
        wsafecpy(sSep, 8, sSeparator);
    }

    // Characs
    if (bCharacs)
    {
        wsafecat(sBuf, iSize, sSep);
        i18n->long_hashToString(sTemp,
                                512, sSeparator,
                                &m_lValues,
                                5,
                                STRING_MELEE,
                                STRING_RANGE,
                                STRING_ARMOR,
                                STRING_ENDURANCE,
                                STRING_SPEED);
        wsafecat(sBuf, iSize, sTemp);
        wsafecpy(sSep, 8, sSeparator);
    }

    // Skills
    if (bSkills)
    {
        wsafecat(sBuf, iSize, sSep);
        i18n->getText1stUp("SKILLS", sTemp, 512);
        wsafecat(sBuf, iSize, sTemp);
        wsafecat(sBuf, iSize, s2P);
        char sSep2[4] = "";
        Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
        if (pSkill == NULL)
        {
            wsafecat(sBuf, iSize, i18n->getText("NONE", sTemp, 512));
        }
        while (pSkill != NULL)
        {
            wsafecat(sBuf, iSize, sSep2);
            wsafecpy(sSep2, 4, ", ");
            wsafecat(sBuf, iSize, pSkill->getLocalizedName());
            pSkill = (Skill*) m_pSkills->getNext(0);
        }
        wsafecpy(sSep, 8, sSeparator);
    }

    // Description
    if (bDesc)
    {
        wsafecat(sBuf, iSize, sSep);
        findLocalizedElement(sTemp, 512, i18n->getCurrentLanguageName(), "description");
        wsafecat(sBuf, iSize, sTemp);
    }
    return sBuf;
}

// -----------------------------------------------------------------
// Name : getAlignmentInfos
// -----------------------------------------------------------------
char * UnitData::getAlignmentInfos(int iValue, char * sBuf, int iSize)
{
    char sTemp[64];
    char sSeparator[8] = "";
    wsafecpy(sBuf, iSize, "");
    if (iValue == 0)
        wsafecat(sBuf, iSize, i18n->getText("NONE", sTemp, 64));
    else
    {
        if (iValue & ALIGNMENT_LIFE)
        {
            wsafecat(sBuf, iSize, i18n->getText("LIFE", sTemp, 64));
            wsafecpy(sSeparator, 8, ", ");
        }
        if (iValue & ALIGNMENT_LAW)
        {
            wsafecat(sBuf, iSize, sSeparator);
            wsafecat(sBuf, iSize, i18n->getText("LAW", sTemp, 64));
            wsafecpy(sSeparator, 8, ", ");
        }
        if (iValue & ALIGNMENT_DEATH)
        {
            wsafecat(sBuf, iSize, sSeparator);
            wsafecat(sBuf, iSize, i18n->getText("DEATH", sTemp, 64));
            wsafecpy(sSeparator, 8, ", ");
        }
        if (iValue & ALIGNMENT_CHAOS)
        {
            wsafecat(sBuf, iSize, sSeparator);
            wsafecat(sBuf, iSize, i18n->getText("CHAOS", sTemp, 64));
        }
    }
    return sBuf;
}
