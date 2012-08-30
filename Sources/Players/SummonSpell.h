#ifndef _SUMMON_SPELL_H
#define _SUMMON_SPELL_H

#include "Spell.h"

class SummonSpell : public Spell
{
public:
  SummonSpell();
  ~SummonSpell();

  wchar_t m_sCreatureName[NAME_MAX_CHARS];
};

#endif
