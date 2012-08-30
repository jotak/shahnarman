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
ProgressionElement::ProgressionElement(wchar_t * sName, wchar_t * sTexture, u8 uLevel, ProgressionTree * pTree)
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
wchar_t * ProgressionElement::getDescription(wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient)
{
  return getDescription(this, m_pEffects, sBuf, iBufSize, pLocalClient);
}

// -----------------------------------------------------------------
// Name : getDescription
//  static
// -----------------------------------------------------------------
wchar_t * ProgressionElement::getDescription(XMLObject * pObj, ObjectList * pEffects, wchar_t * sBuf, int iBufSize, LocalClient * pLocalClient)
{
  pObj->findLocalizedElement(sBuf, iBufSize, i18n->getCurrentLanguageName(), L"name");
  wsafecat(sBuf, iBufSize, L"\n");
  ProgressionEffect * pEffect = (ProgressionEffect*) pEffects->getFirst(0);
  while (pEffect != NULL)
  {
    switch (pEffect->getType())
    {
    case PROGRESSION_EFFECT_CHARAC:
      {
        wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
        wchar_t sType[NAME_MAX_CHARS];
        wchar_t sDeuxPoints[8];
        i18n->getText1stUp(((ProgressionEffect_Charac*)pEffect)->m_sKey, sType, NAME_MAX_CHARS);
        i18n->getText(L"2P", sDeuxPoints, 8);
        if (((ProgressionEffect_Charac*)pEffect)->m_iModifier >= 0)
          swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s%s+%d\n", sType, sDeuxPoints, ((ProgressionEffect_Charac*)pEffect)->m_iModifier);
        else
          swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s%s%d\n", sType, sDeuxPoints, ((ProgressionEffect_Charac*)pEffect)->m_iModifier);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_SPELL:
      {
        wchar_t sType[NAME_MAX_CHARS];
        wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
        wchar_t sDeuxPoints[8];
        Spell * pSpell = pLocalClient->getDataFactory()->findSpell(((ProgressionEffect_Spell*)pEffect)->m_sSpellEdition, ((ProgressionEffect_Spell*)pEffect)->m_sSpellName);
        i18n->getText(L"SPELL", sType, NAME_MAX_CHARS);
        i18n->getText(L"2P", sDeuxPoints, 8);
        swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s %s%s%s\n", sType, pSpell->getLocalizedName(), sDeuxPoints, pSpell->getLocalizedDescription());
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_SKILL:
      {
        wchar_t sType[NAME_MAX_CHARS];
        wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
        wchar_t sDeuxPoints[8];
        Skill * pSkill = new Skill(((ProgressionEffect_Skill*)pEffect)->m_sSkillEdition, ((ProgressionEffect_Skill*)pEffect)->m_sSkillName, ((ProgressionEffect_Skill*)pEffect)->m_sSkillParameters, pLocalClient->getDebug());
        i18n->getText(L"SKILL", sType, NAME_MAX_CHARS);
        i18n->getText(L"2P", sDeuxPoints, 8);
        swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s %s%s%s\n", sType, pSkill->getLocalizedName(), sDeuxPoints, pSkill->getLocalizedDescription());
        wsafecat(sBuf, iBufSize, sBuf2);
        delete pSkill;
        break;
      }
    case PROGRESSION_EFFECT_ARTIFACT:
      {
        wchar_t sType[NAME_MAX_CHARS];
        wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
        wchar_t sName[NAME_MAX_CHARS];
        wchar_t sDesc[DESCRIPTION_MAX_CHARS];
        wchar_t sDeuxPoints[8];
        Edition * pEdition = pLocalClient->getDataFactory()->findEdition(((ProgressionEffect_Artifact*)pEffect)->m_sArtifactEdition);
        assert(pEdition != NULL);
        Artifact * pArtifact = pEdition->findArtifact(((ProgressionEffect_Artifact*)pEffect)->m_sArtifactName);
        i18n->getText(L"ARTIFACT", sType, NAME_MAX_CHARS);
        i18n->getText(L"2P", sDeuxPoints, 8);
        pArtifact->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
        pArtifact->findLocalizedElement(sDesc, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"description");
        swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s %s%s%s\n", sType, sName, sDeuxPoints, sDesc);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    case PROGRESSION_EFFECT_AVATAR:
      {
        wchar_t sType[NAME_MAX_CHARS];
        wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
        wchar_t sName[NAME_MAX_CHARS];
        wchar_t sDesc[DESCRIPTION_MAX_CHARS];
        wchar_t sDeuxPoints[8];
        UnitData * pAvatar = pLocalClient->getDataFactory()->getUnitData(((ProgressionEffect_Avatar*)pEffect)->m_sAvatarEdition, ((ProgressionEffect_Avatar*)pEffect)->m_sAvatarName);
        i18n->getText(L"AVATAR", sType, NAME_MAX_CHARS);
        i18n->getText(L"2P", sDeuxPoints, 8);
        pAvatar->findLocalizedElement(sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"name");
        pAvatar->findLocalizedElement(sDesc, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), L"description");
        swprintf_s(sBuf2, DESCRIPTION_MAX_CHARS, L"%s %s%s%s\n", sType, sName, sDeuxPoints, sDesc);
        wsafecat(sBuf, iBufSize, sBuf2);
        break;
      }
    }
    pEffect = (ProgressionEffect*) pEffects->getNext(0);
  }
  wchar_t sBuf2[DESCRIPTION_MAX_CHARS];
  pObj->findLocalizedElement(sBuf2, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), L"description");
  if (wcscmp(sBuf2, L"") == 0)
  {
    // chop ending "\n"
    int len = wcslen(sBuf);
    if (len > 0)
      sBuf[len-1] = L'\0';
  }
  else
    wsafecat(sBuf, iBufSize, sBuf2);
  return sBuf;
}
