// -----------------------------------------------------------------
// AVATAR DATA
// -----------------------------------------------------------------
#include "AvatarData.h"
#include "Edition.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
#include "../Display/TextureEngine.h"
#include "../Data/DataFactory.h"
#include "../Data/LocalisationTool.h"
#include "../Data/NetworkSerializer.h"

#define NB_LEVELS   15
//      LEVELS        : 1    2    3,   4,    5,    6,    7,    8,    9,   10,    11,    12,    13,    14,    15
#define XP_LEVELS     { 0, 100, 300, 600, 1000, 1600, 2500, 3900, 6000, 9100, 14000, 22000, 34000, 52000, 80000 }
#define BANNERS       { "blason1", "blason2", "blason3", "blason4", "blason5", "blason6", "blason7", "blason8", "blason9", "blason10", "blason11", "blason12" }

// -----------------------------------------------------------------
// Name : AvatarData
//  Constructor
// -----------------------------------------------------------------
AvatarData::AvatarData()
{
    for (int i = 0; i < NB_PROGRESSION_TREES; i++)
    {
        wsafecpy(m_pProgression[i].sTreeName, NAME_MAX_CHARS, "");
        for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
            wsafecpy(m_pProgression[i].sElements[j], NAME_MAX_CHARS, "");
    }
    for (int i = 0; i < 5; i++)
        m_pEquippedArtifacts[i] = NULL;
    m_uXP = 0;
    m_pAllFames = new ObjectList(true);
    wsafecpy(m_sCustomDescription, CUSTOMDESC_MAX_CHARS, "");
    wsafecpy(m_sCustomName, NAME_MAX_CHARS, "");
    m_bLoaded = false;
    m_pOwner = NULL;
    m_uBanner = 0;
}

// -----------------------------------------------------------------
// Name : ~AvatarData
//  Destructor
// -----------------------------------------------------------------
AvatarData::~AvatarData()
{
    delete m_pAllFames;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void AvatarData::serialize(Serializer * pSerializer)
{
    UnitData::serialize(pSerializer);
    pSerializer->writeChar((char) m_uBanner);
    pSerializer->writeShort((short) m_uXP);
    for (int j = 0; j < NB_PROGRESSION_TREES; j++)
    {
        pSerializer->writeString(m_pProgression[j].sTreeName);
        for (int k = 0; k < NB_PROGRESSION_LEVELS; k++)
        {
            pSerializer->writeString(m_pProgression[j].sElements[k]);
        }
    }
    pSerializer->writeLong((long) m_pAllFames->size);
    AvatarData::EthnicityAndFame * pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getFirst(0);
    while (pFameObj != NULL)
    {
        pSerializer->writeString(pFameObj->sEthnicity);
        pSerializer->writeLong((long) pFameObj->iFame);
        pFameObj = (AvatarData::EthnicityAndFame*) m_pAllFames->getNext(0);
    }
    pSerializer->writeString(m_sCustomName);
    pSerializer->writeString(m_sCustomDescription);
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void AvatarData::deserialize(Serializer * pSerializer, DebugManager * pDebug)
{
    UnitData::deserialize(pSerializer, pDebug);
    m_uBanner = pSerializer->readChar();
    m_uXP = pSerializer->readShort();
    for (int j = 0; j < NB_PROGRESSION_TREES; j++)
    {
        pSerializer->readString(m_pProgression[j].sTreeName);
        for (int k = 0; k < NB_PROGRESSION_LEVELS; k++)
        {
            pSerializer->readString(m_pProgression[j].sElements[k]);
        }
    }
    int len = pSerializer->readLong();
    for (int i = 0; i < len; i++)
    {
        AvatarData::EthnicityAndFame * pFameObj = new AvatarData::EthnicityAndFame();
        pSerializer->readString(pFameObj->sEthnicity);
        pFameObj->iFame = pSerializer->readLong();
        m_pAllFames->addLast(pFameObj);
    }
    pSerializer->readString(m_sCustomName);
    pSerializer->readString(m_sCustomDescription);
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
AvatarData * AvatarData::clone(LocalClient * pLocalClient)
{
    NetworkData dummy(1);
    serialize(NetworkSerializer::getInstance(&dummy));
    dummy.readLong();
    AvatarData * pClone = new AvatarData();
    pClone->deserialize(NetworkSerializer::getInstance(&dummy), pLocalClient->getDebug());
    return pClone;
}

// -----------------------------------------------------------------
// Name : cloneStaticData
//  This function is used to copy data from XML model to avatar instance.
// -----------------------------------------------------------------
AvatarData * AvatarData::cloneStaticData(Profile * pOwner, DebugManager * pDebug)
{
    AvatarData * pClone = new AvatarData();
    // Copy XMLObject data
    wsafecpy(pClone->m_sObjectId, NAME_MAX_CHARS, m_sObjectId);
    LocalizedElement * pLoc = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
    while (pLoc != NULL)
    {
        pClone->addLocalizedElement(pLoc->m_sKey, pLoc->m_sValue, pLoc->m_sLanguage);
        pLoc = (LocalizedElement*) m_pLocalizedElements->getNext(0);
    }
    // Copy UnitData data
    wsafecpy(pClone->m_sEdition, NAME_MAX_CHARS, m_sEdition);
    wsafecpy(pClone->m_sTextureFilename, MAX_PATH, m_sTextureFilename);

    long_hash::iterator it;
    for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
        pClone->m_lValues.insert(long_hash::value_type(it->first, it->second));
    Skill * pSkill = (Skill*) m_pSkills->getFirst(0);
    while (pSkill != NULL)
    {
        Skill * pClonedSkill = pSkill->clone(true, pDebug);
        pClone->m_pSkills->addFirst(pClonedSkill);
        pSkill = (Skill*) m_pSkills->getNext(0);
    }
    // Copy AvatarData data
    pClone->m_pOwner = pOwner;
    wsafecpy(pClone->m_sEthnicityId, NAME_MAX_CHARS, m_sEthnicityId);
    wsafecpy(pClone->m_pProgression[0].sTreeName, NAME_MAX_CHARS, m_sEthnicityId);
    pClone->m_bLoaded = true;
    // Set custom name & description
    pClone->findLocalizedElement(pClone->m_sCustomName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
    pClone->findLocalizedElement(pClone->m_sCustomDescription, CUSTOMDESC_MAX_CHARS, i18n->getCurrentLanguageName(), "description");

    return pClone;
}

// -----------------------------------------------------------------
// Name : isLevelUp
// -----------------------------------------------------------------
bool AvatarData::isLevelUp()
{
    u16 uLevel = getRealLevel();
    return (getPotentialLevel() > uLevel && uLevel < MAX_LEVELS);
}

// -----------------------------------------------------------------
// Name : getRealLevel
// -----------------------------------------------------------------
u16 AvatarData::getRealLevel()
{
    u16 currentlevel = 0;
    // Find current level
    for (int i = 0; i < NB_PROGRESSION_TREES; i++)
    {
        if (strcmp(m_pProgression[i].sTreeName, "") != 0)
        {
            currentlevel++;
            for (int j = 0; j < NB_PROGRESSION_LEVELS; j++)
            {
                if (strcmp(m_pProgression[i].sElements[j], "") != 0)
                    currentlevel++;
            }
        }
    }
    return currentlevel;
}

// -----------------------------------------------------------------
// Name : getPotentialLevel
// -----------------------------------------------------------------
u16 AvatarData::getPotentialLevel()
{
    int iLevelXP[] = XP_LEVELS;
    for (int i = NB_LEVELS-1; i >= 0; i--)
    {
        if (m_uXP >= iLevelXP[i])
            return (u16)(i + 1);
    }
    assert(false);  // should not be there
    return 0;
}

// -----------------------------------------------------------------
// Name : getNextLevelXP
// -----------------------------------------------------------------
u16 AvatarData::getNextLevelXP()
{
    int iLevelXP[] = XP_LEVELS;
    for (int i = NB_LEVELS-1; i >= 0; i--)
    {
        if (m_uXP >= iLevelXP[i])
            return (u16) iLevelXP[i+1];
    }
    assert(false);  // should not be there
    return 0;
}

// -----------------------------------------------------------------
// Name : getBanner
//  Static
// -----------------------------------------------------------------
void AvatarData::getBanner(u8 uBanner, char * sBuf, int iBufSize)
{
    const char * sBanners[] = BANNERS;
    wsafecpy(sBuf, iBufSize, sBanners[uBanner]);
}

// -----------------------------------------------------------------
// Name : getBanner
// -----------------------------------------------------------------
void AvatarData::getBanner(char * sBuf, int iBufSize)
{
    const char * sBanners[] = BANNERS;
    wsafecpy(sBuf, iBufSize, sBanners[m_uBanner]);
}
