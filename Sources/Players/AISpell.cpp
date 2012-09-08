#include "AISpell.h"

// -----------------------------------------------------------------
// Name : AISpell
//  Constructor
// -----------------------------------------------------------------
AISpell::AISpell(Spell * pToClone, DebugManager * pDebug) : Spell(pToClone->getInstanceId(), pToClone->getPlayerId(), pToClone->getObjectEdition(), pToClone->getFrequency(), pToClone->getObjectName(), pDebug)
{
    m_fInterest = 0.0f;
    wsafecpy(m_sBestInterestParameters, LUA_FUNCTION_PARAMS_MAX_CHARS, "");
}

// -----------------------------------------------------------------
// Name : ~AISpell
//  Destructor
// -----------------------------------------------------------------
AISpell::~AISpell()
{
}
