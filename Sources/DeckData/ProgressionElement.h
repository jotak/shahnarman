#ifndef _PROGRESSION_ELEMENT_H
#define _PROGRESSION_ELEMENT_H

#include "../Data/XMLObject.h"
#include "../Data/LuaObject.h"

#define PROGRESSION_EFFECT_CHARAC    1
#define PROGRESSION_EFFECT_SPELL     2
#define PROGRESSION_EFFECT_SKILL     3
#define PROGRESSION_EFFECT_ARTIFACT  4
#define PROGRESSION_EFFECT_AVATAR    5

class LocalClient;
class ProgressionTree;

class ProgressionEffect : public BaseObject
{
public:
    virtual u8 getType() = 0;
};
class ProgressionEffect_Charac : public ProgressionEffect
{
public:
    ProgressionEffect_Charac(char * sKey, int iMod)
    {
        wsafecpy(m_sKey, NAME_MAX_CHARS, sKey);
        m_iModifier = iMod;
    };
    u8 getType()
    {
        return PROGRESSION_EFFECT_CHARAC;
    };
    char m_sKey[NAME_MAX_CHARS];
    int m_iModifier;
};
class ProgressionEffect_Spell : public ProgressionEffect
{
public:
    ProgressionEffect_Spell(char * sEdition, char * sName)
    {
        wsafecpy(m_sSpellEdition, NAME_MAX_CHARS, sEdition);
        wsafecpy(m_sSpellName, NAME_MAX_CHARS, sName);
    };
    u8 getType()
    {
        return PROGRESSION_EFFECT_SPELL;
    };
    char m_sSpellEdition[NAME_MAX_CHARS];
    char m_sSpellName[NAME_MAX_CHARS];
};
class ProgressionEffect_Skill : public ProgressionEffect
{
public:
    ProgressionEffect_Skill(char * sEdition, char * sName, char * sParams)
    {
        wsafecpy(m_sSkillEdition, NAME_MAX_CHARS, sEdition);
        wsafecpy(m_sSkillName, NAME_MAX_CHARS, sName);
        wsafecpy(m_sSkillParameters, NAME_MAX_CHARS, sParams);
    };
    u8 getType()
    {
        return PROGRESSION_EFFECT_SKILL;
    };
    char m_sSkillEdition[NAME_MAX_CHARS];
    char m_sSkillName[NAME_MAX_CHARS];
    char m_sSkillParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
};
class ProgressionEffect_Artifact : public ProgressionEffect
{
public:
    ProgressionEffect_Artifact(char * sEdition, char * sName)
    {
        wsafecpy(m_sArtifactEdition, NAME_MAX_CHARS, sEdition);
        wsafecpy(m_sArtifactName, NAME_MAX_CHARS, sName);
    };
    u8 getType()
    {
        return PROGRESSION_EFFECT_ARTIFACT;
    };
    char m_sArtifactEdition[NAME_MAX_CHARS];
    char m_sArtifactName[NAME_MAX_CHARS];
};
class ProgressionEffect_Avatar : public ProgressionEffect
{
public:
    ProgressionEffect_Avatar(char * sEdition, char * sName)
    {
        wsafecpy(m_sAvatarEdition, NAME_MAX_CHARS, sEdition);
        wsafecpy(m_sAvatarName, NAME_MAX_CHARS, sName);
    };
    u8 getType()
    {
        return PROGRESSION_EFFECT_AVATAR;
    };
    char m_sAvatarEdition[NAME_MAX_CHARS];
    char m_sAvatarName[NAME_MAX_CHARS];
};

class ProgressionElement : public XMLObject
{
public:
    ProgressionElement(char * sName, char * sTexture, u8 uLevel, ProgressionTree * pTree);
    ~ProgressionElement();

    static char * getDescription(XMLObject * pObj, ObjectList * pEffects, char * sBuf, int iBufSize, LocalClient * pLocalClient);
    char * getDescription(char * sBuf, int iBufSize, LocalClient * pLocalClient);

    char m_sTexture[MAX_PATH];
    ProgressionTree * m_pTree;
    ObjectList * m_pEffects;
    ObjectList * m_pChildren;
    u8 m_uLevel;
    int m_iNbParents;
};

#endif
