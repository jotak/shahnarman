#ifndef _MANA_H
#define _MANA_H

#include "../Common/BaseObject.h"

#define MANA_LIFE   0
#define MANA_LAW    1
#define MANA_DEATH  2
#define MANA_CHAOS  3

#define MANA_LIFE_CHAR  L'\u00A4'
#define MANA_LAW_CHAR   L'\u007C'
#define MANA_DEATH_CHAR L'\u00B5'
#define MANA_CHAOS_CHAR L'\u00A7'

//#define MANA_SIGNS  { '¤', '|', 'µ', '§' }
#define MANA_SIGNS  { L'\u00A4', L'\u007C', L'\u00B5', L'\u00A7' }
#define MANA_TEXTS  { "LIFE_MANA", "LAW_MANA", "DEATH_MANA", "CHAOS_MANA" }

class Mana : public BaseObject
{
public:
    Mana(u8 life = 0, u8 law = 0, u8 death = 0, u8 chaos = 0);

    void reset();
    u8 operator[](int idx)
    {
        return mana[idx];
    };
    Mana operator+(Mana addMana);
    Mana operator+(Mana* addMana);
    Mana operator-(Mana addMana);
    Mana operator-(Mana* addMana);
    void operator+=(Mana addMana);
    void operator+=(Mana* addMana);
    void operator-=(Mana remMana);
    bool operator<=(Mana cmpMana);
    bool operator<(Mana cmpMana);
    int amount()
    {
        return mana[0] + mana[1] + mana[2] + mana[3];
    };

    u8 mana[4];

    Mana * clone()
    {
        Mana * pClone = new Mana(mana[0], mana[1], mana[2], mana[3]);
        return pClone;
    }
};

#endif
