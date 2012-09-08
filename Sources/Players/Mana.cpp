#include "Mana.h"

// -----------------------------------------------------------------
// Name : Mana
//  Constructor
// -----------------------------------------------------------------
Mana::Mana(u8 life, u8 law, u8 death, u8 chaos)
{
    mana[0] = life;
    mana[1] = law;
    mana[2] = death;
    mana[3] = chaos;
}

// -----------------------------------------------------------------
// Name : reset
// -----------------------------------------------------------------
void Mana::reset()
{
    mana[0] = mana[1] = mana[2] = mana[3] = 0;
}

// -----------------------------------------------------------------
// Name : operator+
// -----------------------------------------------------------------
Mana Mana::operator+(Mana addMana)
{
    Mana result;
    for (int i = 0; i < 4; i++)
        result.mana[i] = mana[i] + addMana.mana[i];
    return result;
}

// -----------------------------------------------------------------
// Name : operator+
// -----------------------------------------------------------------
Mana Mana::operator+(Mana * addMana)
{
    Mana result;
    for (int i = 0; i < 4; i++)
        result.mana[i] = mana[i] + addMana->mana[i];
    return result;
}

// -----------------------------------------------------------------
// Name : operator-
// -----------------------------------------------------------------
Mana Mana::operator-(Mana remMana)
{
    Mana result;
    for (int i = 0; i < 4; i++)
        result.mana[i] = mana[i] - min(remMana.mana[i], mana[i]);
    return result;
}

// -----------------------------------------------------------------
// Name : operator-
// -----------------------------------------------------------------
Mana Mana::operator-(Mana * remMana)
{
    Mana result;
    for (int i = 0; i < 4; i++)
        result.mana[i] = mana[i] - min(remMana->mana[i], mana[i]);
    return result;
}

// -----------------------------------------------------------------
// Name : operator+=
// -----------------------------------------------------------------
void Mana::operator+=(Mana addMana)
{
    for (int i = 0; i < 4; i++)
        mana[i] += addMana.mana[i];
}

// -----------------------------------------------------------------
// Name : operator+=
// -----------------------------------------------------------------
void Mana::operator+=(Mana * addMana)
{
    for (int i = 0; i < 4; i++)
        mana[i] += addMana->mana[i];
}

// -----------------------------------------------------------------
// Name : operator-=
// -----------------------------------------------------------------
void Mana::operator-=(Mana remMana)
{
    for (int i = 0; i < 4; i++)
        mana[i] -= min(remMana.mana[i], mana[i]);
}

// -----------------------------------------------------------------
// Name : operator<=
// -----------------------------------------------------------------
bool Mana::operator<=(Mana cmpMana)
{
    for (int i = 0; i < 4; i++)
    {
        if (mana[i] > cmpMana.mana[i])
            return false;
    }
    return true;
}

// -----------------------------------------------------------------
// Name : operator<
// -----------------------------------------------------------------
bool Mana::operator<(Mana cmpMana)
{
    for (int i = 0; i < 4; i++)
    {
        if (mana[i] >= cmpMana.mana[i])
            return false;
    }
    return true;
}
