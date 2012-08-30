#ifndef _MANA_H
#define _MANA_H

#include "../Common/BaseObject.h"

#define MANA_LIFE   0
#define MANA_LAW    1
#define MANA_DEATH  2
#define MANA_CHAOS  3

#define MANA_SIGNS  { L'¤', L'|', L'µ', L'§' }
#define MANA_TEXTS  { L"LIFE_MANA", L"LAW_MANA", L"DEATH_MANA", L"CHAOS_MANA" }

class Mana : public BaseObject
{
public:
  Mana(u8 life = 0, u8 law = 0, u8 death = 0, u8 chaos = 0);

  void reset();
  u8 operator[](int idx) { return mana[idx]; };
  Mana operator+(Mana addMana);
  Mana operator+(Mana* addMana);
  void operator+=(Mana addMana);
  void operator+=(Mana* addMana);
  void operator-=(Mana remMana);
  bool operator<=(Mana cmpMana);
  bool operator<(Mana cmpMana);
  int amount() { return mana[0] + mana[1] + mana[2] + mana[3]; };

  u8 mana[4];

  Mana * clone() { Mana * pClone = new Mana(mana[0], mana[1], mana[2], mana[3]); return pClone; }
};

#endif
