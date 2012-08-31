#ifndef _AI_SPELL_H
#define _AI_SPELL_H

#include "Spell.h"

class AISpell : public Spell
{
public:
  AISpell(Spell * pToClone, DebugManager * pDebug);
  virtual ~AISpell();

  float getInterest() { return m_fInterest; };
  void setInterest(float fInterest) { m_fInterest = fInterest; };
  void setBestInterestParameters(wchar_t * sParams) { wsafecpy(m_sBestInterestParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, sParams); };
  wchar_t * getBestInterestParameters() { return m_sBestInterestParameters; };

protected:
  float m_fInterest;
  wchar_t m_sBestInterestParameters[LUA_FUNCTION_PARAMS_MAX_CHARS];
};

#endif
