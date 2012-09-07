// -----------------------------------------------------------------
// SERVER UNIT
// -----------------------------------------------------------------
#include "../Gameboard/Unit.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../Debug/DebugManager.h"
#include "../Players/Spell.h"
#include "../Server/Server.h"
#include "../Physics/ConstantMovement3D.h"

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void Unit::init(u32 uUnitId, u8 uPlayerId, UnitData * pData, DebugManager * pDebug)
{
  m_uUnitId = uUnitId;
  m_uOwner = uPlayerId;
  updateIdentifiers();

  // Set data
  wsafecpy(m_sUnitId, NAME_MAX_CHARS, pData->m_sObjectId);
  wsafecpy(m_sEdition, NAME_MAX_CHARS, pData->m_sEdition);
  long_hash::iterator it;
  for (it = pData->m_lValues.begin(); it != pData->m_lValues.end(); ++it)
    setBaseValue(it->first.c_str(), it->second);
  Skill * pSkill = (Skill*) pData->m_pSkills->getFirst(0);
  while (pSkill != NULL)
  {
    Skill * pClone = pSkill->clone(false, pDebug);
    addSkill(pClone);
    pSkill = (Skill*) pData->m_pSkills->getNext(0);
  }

  m_Order = m_PreviousOrder = OrderNone;
  m_pAttackTarget = m_pPreviousAttackTarget = NULL;
  m_Status = US_Normal;
  m_bModified = false;
  setBaseValue(STRING_LIFE, getValue(STRING_ENDURANCE, true));
}

// -----------------------------------------------------------------
// Name : setMapPos
// -----------------------------------------------------------------
void Unit::setMapPos(CoordsMap coords)
{
  CoordsMap oldPos = getMapPos();
  MapObject::setMapPos(coords);
  m_bModified = true;
  if (getDisplay() != NULL) {
    // Translate move
    ConstantMovement3D * pMove = new ConstantMovement3D(0, 1.0f, getDisplay()->get3DCoords(getMapPos()) - getDisplay()->get3DCoords(oldPos));
    bindMovement(pMove);
  }
}

// -----------------------------------------------------------------
// Name : onNewTurn
// -----------------------------------------------------------------
void Unit::onNewTurn()
{
  // Heal
  int life = getValue(STRING_LIFE);
  int endurance = getValue(STRING_ENDURANCE);
  // Note: les unités ne régénèrent plus automatiquement
//  if (life > 0 && life < endurance)
//    setBaseValue(STRING_LIFE, life + 1);
/*  else*/ if (life > endurance)
    setBaseValue(STRING_LIFE, endurance);
  // If target is dead, remove attack order
  if (m_Order == OrderAttack)
  {
    if (m_pAttackTarget->getStatus() != US_Normal)
      unsetOrder();
  }
  m_bNewTurnDone = true;
}

// -----------------------------------------------------------------
// Name : setBaseValue
// -----------------------------------------------------------------
bool Unit::setBaseValue(const char * sName, long val)
{
  m_bModified = true;
  return LuaTargetable::setBaseValue(sName, val);
}

// -----------------------------------------------------------------
// Name : getUnitData
// -----------------------------------------------------------------
UnitData * Unit::getUnitData(Server * pServer)
{
  return pServer->getFactory()->getUnitData(m_sEdition, m_sUnitId);
}
