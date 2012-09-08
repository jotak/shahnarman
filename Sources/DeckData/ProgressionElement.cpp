// -----------------------------------------------------------------
// PROGRESSION ELEMENT
// -----------------------------------------------------------------
#include "ProgressionElement.h"
#include "../Gameboard/Skill.h"
#include "../Players/Spell.h"
#include "../Players/Artifact.h"
#include "../DeckData/Edition.h"
#include "../DeckData/UnitData.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../LocalClient.h"

// -----------------------------------------------------------------
// Name : ProgressionElement
//  Constructor
// -----------------------------------------------------------------
ProgressionElement::ProgressionElement(char * sName, char * sTexture, u8 uLevel, ProgressionTree * pTree)
{
  m_uLevel = uLevel;
  m_pTree = pTree;
  m_iNbParents = 0;
  wsafecpy(m_sObjectId, NAME_MAX_CHARS, sName);
  wsafecpy(m_sTexture, MAX_PATH, sTexture);
  m_pEffects = new ObjectList(true);
  m_pChildren = new ObjectList(false);
}

// -----------------------------------------------------------------
// Name : ~ProgressionElement
//  Destructor
// -----------------------------------------------------------------
ProgressionElement::~ProgressionElement()
{
  delete m_pEffects;
  delete m_pChildren;
}

// -----------------------------------------------------------------
// Name : getDescription
// -----------------------------------------------------------------
char * ProgressionElement::getDescription(char * sBuf, int iBufSize, LocalClient * pLocalClient)
{
  return getDescription(this, m_pEffects, sBuf, iBufSize, pLocalClient);
}

// -----------------------------------------------------------------
// Name : getDescription
//  static
// -----------------------------------------------------------------
char * ProgressionElement::getDescription(XMLObject * pObj, ObjectList * pEffects, char * sBuf, int iBufSize, LocalClient * pLocalClient)
{
  pObj->findLocalizedElement(sBuf, iBufSize, i18n->getCurrentLanguageName(), "name");
  wsafecat(sBuf, iBufSize, "\n");
  ProgressionEffect * pEffect = (ProgressionEffect*) pEffects->getFirst(0);
  while (pEffect != NULL)
  {
    switch (pEffect->getType())
    {
    case PROGRESSION_EFFECT_CHARAC:
      {
        char sBuf2[DESCRIPTION_MAX_CHARS];
        char sType[NAME_MAX_CHARS];
        char sDeuxPoints[8];
        i18n->getText1stUp(((ProgressionEffect_Charac*)pEffect)->m_sKey, sType, NAME_MAX_CHARS);
        i18n->getText("2P", sDeuxPoints, 8);
        if (((ProgressionEffect_Charac*)pEffect)->m_iModifier >= 0)
          snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s%s+%d\n", sType, sDeuxPoints, ((ProgressionEffect_Charac*)pEffect)->m_iModifier);
        else
          snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s%s%d\n", sType, sDeuxPoints, ((ProgressionEffect_Charac*)pEffect)->m_iModifier);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_SPELL:
      {
        char sType[NAME_MAX_CHARS];
        char sBuf2[DESCRIPTION_MAX_CHARS];
        char sDeuxPoints[8];
        Spell * pSpell = pLocalClient->getDataFactory()->findSpell(((ProgressionEffect_Spell*)pEffect)->m_sSpellEdition, ((ProgressionEffect_Spell*)pEffect)->m_sSpellName);
        i18n->getText("SPELL", sType, NAME_MAX_CHARS);
        i18n->getText("2P", sDeuxPoints, 8);
        snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s %s%s%s\n", sType, pSpell->getLocalizedName(), sDeuxPoints, pSpell->getLocalizedDescription());
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_SKILL:
      {
        char sType[NAME_MAX_CHARS];
        char sBuf2[DESCRIPTION_MAX_CHARS];
        char sDeuxPoints[8];
        Skill * pSkill = new Skill(((ProgressionEffect_Skill*)pEffect)->m_sSkillEdition, ((ProgressionEffect_Skill*)pEffect)->m_sSkillName, ((ProgressionEffect_Skill*)pEffect)->m_sSkillParameters, pLocalClient->getDebug());
        i18n->getText("SKILL", sType, NAME_MAX_CHARS);
        i18n->getText("2P", sDeuxPoints, 8);
        snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s %s%s%s\n", sType, pSkill->getLocalizedName(), sDeuxPoints, pSkill->getLocalizedDescription());
        wsafecat(sBuf, iBufSize, sBuf2);
        delete pSkill;
        break;
      }
    case PROGRESSION_EFFECT_ARTIFACT:
      {
        char sType[NAME_MAX_CHARS];
        char sBuf2[DESCRIPTION_MAX_CHARS];
        char sName[NAME_MAX_CHARS];
        char sDesc[DESCRIPTION_MAX_CHARS];
        char sDeuxPoints[8];
        Edition * pEdition = pLocalClient->getDataFactory()->findEdition(((ProgressionEffect_Artifact*)pEffect)->m_sArtifactEdition);
        assert(pEdition != NULL);
        Artifact * pArtifact = pEdition->findArtifact(((ProgressionEffect_Artifact*)pEffect)->m_sArtifactName);
        i18n->getText("ARTIFACT", sType, NAME_MAX_CHARS);
        i18n->getText("2P", sDeuxPoints, 8);
        pArtifact->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
        pArtifact->findLocalizedElement(sDesc, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
        snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s %s%s%s\n", sType, sName, sDeuxPoints, sDesc);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_AVATAR:
      {
        char sType[NAME_MAX_CHARS];
        char sBuf2[DESCRIPTION_MAX_CHARS];
        char sName[NAME_MAX_CHARS];
        char sDesc[DESCRIPTION_MAX_CHARS];
        char sDeuxPoints[8];
        UnitData * pAvatar = pLocalClient->getDataFactory()->getUnitData(((ProgressionEffect_Avatar*)pEffect)->m_sAvatarEdition, ((ProgressionEffect_Avatar*)pEffect)->m_sAvatarName);
        i18n->getText("AVATAR", sType, NAME_MAX_CHARS);
        i18n->getText("2P", sDeuxPoints, 8);
        pAvatar->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
        pAvatar->findLocalizedElement(sDesc, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
        snprintf(sBuf2, DESCRIPTION_MAX_CHARS, "%s %s%s%s\n", sType, sName, sDeuxPoints, sDesc);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    }
    pEffect = (ProgressionEffect*) pEffects->getNext(0);
  }
  char sBuf2[DESCRIPTION_MAX_CHARS];
  pObj->findLocalizedElement(sBuf2, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
  if (strcmp(sBuf2, "") == 0)
  {
    // chop ending "\n"
    int len = strlen(sBuf);
    if (len > 0)
      sBuf[len-1] = '\0';
  }
  else
    wsafecat(sBuf, iBufSize, sBuf2);
  return sBuf;
}
