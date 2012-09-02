#ifndef _ARTIFACT_H
#define _ARTIFACT_H

#include "../Data/XMLObject.h"
#include "../Data/LuaObject.h"

#define ARTIFACT_POSITION_HEAD    0
#define ARTIFACT_POSITION_BODY    1
#define ARTIFACT_POSITION_LHAND   2
#define ARTIFACT_POSITION_RHAND   3
#define ARTIFACT_POSITION_FOOT    4

#define ARTIFACT_EFFECT_CHARAC    1
#define ARTIFACT_EFFECT_SPELL     2
#define ARTIFACT_EFFECT_SKILL     3

class AvatarData;

class ArtifactEffect : public BaseObject
{
public:
  virtual u8 getType() = 0;
};
class ArtifactEffect_Charac : public ArtifactEffect
{
public:
  ArtifactEffect_Charac(const wchar_t * sKey, int iMod) { wsafecpy(m_sKey, NAME_MAX_CHARS, sKey); m_iModifier = iMod; };
  u8 getType() { return ARTIFACT_EFFECT_CHARAC; };
  wchar_t m_sKey[NAME_MAX_CHARS];
  int m_iModifier;
};
class ArtifactEffect_Spell : public ArtifactEffect
{
public:
  ArtifactEffect_Spell(const wchar_t * sEdition, const wchar_t * sName) { wsafecpy(m_sSpellEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sSpellName, NAME_MAX_CHARS, sName); };
  u8 getType() { return ARTIFACT_EFFECT_SPELL; };
  wchar_t m_sSpellEdition[NAME_MAX_CHARS];
  wchar_t m_sSpellName[NAME_MAX_CHARS];
};
class ArtifactEffect_Skill : public ArtifactEffect
{
public:
  ArtifactEffect_Skill(const wchar_t * sEdition, const wchar_t * sName, const wchar_t * sParams) { wsafecpy(m_sSkillEdition, NAME_MAX_CHARS, sEdition); wsafecpy(m_sSkillName, NAME_MAX_CHARS, sName); wsafecpy(m_sSkillParameters, NAME_MAX_CHARS, sParams); };
  u8 getType() { return ARTIFACT_EFFECT_SKILL; };
  wchar_t m_sSkillEdition[NAME_MAX_CHARS];
  wchar_t m_sSkillName[NAME_MAX_CHARS];
  wchar_t m_sSkillParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
};

class Artifact : public XMLObject
{
public:
  Artifact(const wchar_t * sEdition, const wchar_t * sId, const wchar_t * sTexture, u8 uPosition, bool bTwoHanded);
  ~Artifact();

  void addArtifactEffect(ArtifactEffect * pEffect) { m_pEffects->addLast(pEffect); };
  ObjectList * getArtifactEffects() { return m_pEffects; };
  wchar_t * getTexture() { return m_sTexture; };
  u8 getPosition() { return m_uPosition; };
  bool isTwoHanded() { return m_bTwoHanded; };
  wchar_t * getEdition() { return m_sEdition; };
  AvatarData * m_pOwner;
  Artifact * clone();

protected:
  wchar_t m_sEdition[MAX_PATH];
  wchar_t m_sTexture[MAX_PATH];
  u8 m_uPosition;
  bool m_bTwoHanded;
  ObjectList * m_pEffects;
};

#endif
