#include "Artifact.h"

// -----------------------------------------------------------------
// Name : Artifact
//  Constructor
// -----------------------------------------------------------------
Artifact::Artifact(const char * sEdition, const char * sId, const char * sTexture, u8 uPosition, bool bTwoHanded)
{
  wsafecpy(m_sEdition, NAME_MAX_CHARS, sEdition);
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, sId);
  wsafecpy(m_sTexture, MAX_PATH, sTexture);
  m_uPosition = uPosition;
  m_bTwoHanded = bTwoHanded;
  m_pEffects = new ObjectList(true);
  m_pOwner = NULL;
}

// -----------------------------------------------------------------
// Name : Artifact
//  Destructor
// -----------------------------------------------------------------
Artifact::~Artifact()
{
  delete m_pEffects;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
Artifact * Artifact::clone()
{
  Artifact * pClone = new Artifact(m_sEdition, m_sObjectId, m_sTexture, m_uPosition, m_bTwoHanded);
  LocalizedElement * pLoc = (LocalizedElement*) m_pLocalizedElements->getFirst(0);
  while (pLoc != NULL)
  {
    pClone->addLocalizedElement(pLoc->m_sKey, pLoc->m_sValue, pLoc->m_sLanguage);
    pLoc = (LocalizedElement*) m_pLocalizedElements->getNext(0);
  }
  ArtifactEffect * pEffect = (ArtifactEffect*) m_pEffects->getFirst(0);
  while (pEffect != NULL)
  {
    switch (pEffect->getType())
    {
    case ARTIFACT_EFFECT_CHARAC:
      {
        pClone->addArtifactEffect(new ArtifactEffect_Charac(((ArtifactEffect_Charac*)pEffect)->m_sKey, ((ArtifactEffect_Charac*)pEffect)->m_iModifier));
        break;
      }
    case ARTIFACT_EFFECT_SPELL:
      {
        pClone->addArtifactEffect(new ArtifactEffect_Spell(((ArtifactEffect_Spell*)pEffect)->m_sSpellEdition, ((ArtifactEffect_Spell*)pEffect)->m_sSpellName));
        break;
      }
    case ARTIFACT_EFFECT_SKILL:
      {
        pClone->addArtifactEffect(new ArtifactEffect_Skill(((ArtifactEffect_Skill*)pEffect)->m_sSkillEdition, ((ArtifactEffect_Skill*)pEffect)->m_sSkillName, ((ArtifactEffect_Skill*)pEffect)->m_sSkillParameters));
        break;
      }
    }
    pEffect = (ArtifactEffect*) m_pEffects->getNext(0);
  }
  return pClone;
}
