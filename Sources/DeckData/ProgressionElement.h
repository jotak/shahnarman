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
  ProgressionEffect_Charac(wchar_t * sKey, int iMod) { wsafecpy(m_sKey, NAME_MAX_CHARS, sKey); m_iModifier = iMod; };
  u8 getType() { return PROGRESSION_EFFECT_CHARAC; };
  wchar_t m_sKey[NAME_MAX_CHARS];
  int m_iModifier;
};
class ProgressionEffect_Spell : public ProgressionEffect
{
public:
  ProgressionEffect_Spell(wchar_t * sEdition, wchar_t * sName) { wsafecpy(m_sSpellEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sSpellName, NAME_MAX_CHARS, sName); };
  u8 getType() { return PROGRESSION_EFFECT_SPELL; };
  wchar_t m_sSpellEdition[NAME_MAX_CHARS];
  wchar_t m_sSpellName[NAME_MAX_CHARS];
};
class ProgressionEffect_Skill : public ProgressionEffect
{
public:
  ProgressionEffect_Skill(wchar_t * sEdition, wchar_t * sName, wchar_t * sParams) { wsafecpy(m_sSkillEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sSkillName, NAME_MAX_CHARS, sName); wsafecpy(m_sSkillParameters, NAME_MAX_CHARS, sParams); };
  u8 getType() { return PROGRESSION_EFFECT_SKILL; };
  wchar_t m_sSkillEdition[NAME_MAX_CHARS];
  wchar_t m_sSkillName[NAME_MAX_CHARS];
  wchar_t m_sSkillParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
};
class ProgressionEffect_Artifact : public ProgressionEffect
{
public:
  ProgressionEffect_Artifact(wchar_t * sEdition, wchar_t * sName) { wsafecpy(m_sArtifactEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sArtifactName, NAME_MAX_CHARS, sName); };
  u8 getType() { return PROGRESSION_EFFECT_ARTIFACT; };
  wchar_t m_sArtifactEdition[NAME_MAX_CHARS];
  wchar_t m_sArtifactName[NAME_MAX_CHARS];
};
class ProgressionEffect_Avatar : public ProgressionEffect
{
public:
  ProgressionEffect_Avatar(wchar_t * sEdition, wchar_t * sName) { wsafecpy(m_sAvatarEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sAvatarName, NAME_MAX_CHARS, sName); };
  u8 getType() { return PROGRESSION_EFFECT_AVATAR; };
  wchar_t m_sAvatarEdition[NAME_MAX_CHARS];
  wchar_t m_sAvatarName[NAME_MAX_CHARS];
};

class ProgressionElement : public XMLObject
{
public:
  ProgressionElement(wchar_t * sName, wchar_t * sTexture, u8 uLevel, ProgressionTree * pTree);
  ~ProgressionElement();

  static wchar_t * getDescription(XMLObject * pObj, ObjectList * pEffects, wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient);
  wchar_t * getDescription(wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient);

  wchar_t m_sTexture[MAX_PATH];
  ProgressionTree * m_pTree;
  ObjectList * m_pEffects;
  ObjectList * m_pChildren;
  u8 m_uLevel;
  int m_iNbParents;
};

#endif
