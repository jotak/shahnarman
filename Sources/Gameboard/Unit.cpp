#include "Unit.h"
#include "GameboardManager.h"
#include "../Data/LocalisationTool.h"
#include "../Data/DataFactory.h"
#include "../LocalClient.h"
#include "Skill.h"
#include "../DeckData/UnitData.h"
#include "../Players/PlayerManager.h"
#include "../Players/Player.h"
#include "../Players/Spell.h"
#include "../Interface/UnitOptionsDlg.h"
#include "../Geometries/GeometryCylinder.h"

// -----------------------------------------------------------------
// Name : Unit
//  Constructor
// -----------------------------------------------------------------
Unit::Unit(CoordsMap mapPos, Map * pMap, ObjectList ** pGlobalEffects) : MapObject(mapPos, pMap, pGlobalEffects, "")
{
    m_pAStarPath = new ObjectList(true);
    m_iPathTurnIterator = m_pAStarPath->getIterator();
    registerValue(STRING_MELEE, 0);
    registerValue(STRING_RANGE, 0);
    registerValue(STRING_ARMOR, 0);
    registerValue(STRING_ENDURANCE, 0);
    registerValue(STRING_SPEED, 0);
    registerValue(STRING_LIFE, 0);
    registerValue(STRING_ALIGNMENT, 0);
    registerValue(STRING_MCPLAIN, 1);
    registerValue(STRING_MCFOREST, 2);
    registerValue(STRING_MCMOUNTAIN, 3);
    registerValue(STRING_MCTOUNDRA, 2);
    registerValue(STRING_MCDESERT, 2);
    registerValue(STRING_MCSEA, -1);
    m_PreviousOrder = OrderNone;
    m_pGroup = NULL;
    m_pSkillsRef = new ObjectList(true);
    m_uActivatedSkillId = 0;
    m_uActivatedSkillEffectId = 0;
}

// -----------------------------------------------------------------
// Name : ~Unit
//  Destructor
// -----------------------------------------------------------------
Unit::~Unit()
{
    delete m_pAStarPath;
    delete m_pSkillsRef;
}

// -----------------------------------------------------------------
// Name : serialize
// -----------------------------------------------------------------
void Unit::serialize(NetworkData * pData)
{
    pData->addLong(m_uUnitId);
    pData->addLong(m_uOwner);
    CoordsMap pos = getMapPos();
    pData->addLong(pos.x);
    pData->addLong(pos.y);
    pData->addString(m_sUnitId);
    pData->addString(m_sEdition);
    LuaTargetable::serializeValues(pData);
    pData->addLong((long)m_Order);
    CoordsMap goal = getDestination();
    pData->addLong(goal.x);
    pData->addLong(goal.y);
    pData->addLong(m_pAttackTarget != NULL ? 1 : 0);
    if (m_pAttackTarget != NULL)
    {
        pData->addLong((long)m_pAttackTarget->getOwner());
        pData->addLong((long)m_pAttackTarget->getId());
    }
    pData->addLong((long)m_uActivatedSkillId);
    pData->addLong((long)m_uActivatedSkillEffectId);
    pData->addLong((long)m_Status);

    // Serialize skills ref
    pData->addLong(m_pSkillsRef->size);
    Skill * pSkill = (Skill*) m_pSkillsRef->getFirst(0);
    while (pSkill != NULL)
    {
        pData->addLong(pSkill->getInstanceId());
        pData->addString(pSkill->getObjectEdition());
        pData->addString(pSkill->getObjectName());
        pData->addString(pSkill->getParameters());
        pSkill = (Skill*) m_pSkillsRef->getNext(0);
    }
    //LuaTargetable::serializeEffects(pData, getAllEffects());
    //LuaTargetable::serializeEffects(pData, getDisabledEffects());
}

// -----------------------------------------------------------------
// Name : deserialize
// -----------------------------------------------------------------
void Unit::deserialize(NetworkData * pData, LocalClient * pLocalClient, std::queue<RELINK_PTR_DATA> * relinkPtrData)
{
    m_uUnitId = (u32) pData->readLong();
    m_uOwner = (u8) pData->readLong();
    CoordsMap pos;
    pos.x = (int) pData->readLong();
    pos.y = (int) pData->readLong();
    setMapPos(pos);
    moveTo(pLocalClient->getDisplay()->get3DCoords(pos)); // 3D position
    pData->readString(m_sUnitId);
    pData->readString(m_sEdition);
    LuaTargetable::deserializeValues(pData);
    m_Order = (UnitOrder) pData->readLong();

    CoordsMap goal;
    goal.x = pData->readLong();
    goal.y = pData->readLong();
    if (m_Order == OrderMove)
        computePath(goal);

    m_pAttackTarget = NULL;
    long isAttacking = pData->readLong();
    if (isAttacking)
    {
        int owner = (int) pData->readLong();
        u32 id = (u32) pData->readLong();
        if (relinkPtrData != NULL)
        {
            // Here we won't directly update m_pAttackTarget, because the unit described by "owner" and "id" may be obsolete or not yet existing.
            // We must use the "RELINK_PTR_DATA" structure instead.
            RELINK_PTR_DATA data;
            data.type = RELINK_TYPE_ATTACKING_UNIT;
            data.data1 = (u32) this;
            data.data2 = owner;
            data.data3 = id;
            relinkPtrData->push(data);
        }
        else
        {
            // else if relinkPtrData is null, it means that the unit was created individually, so we should find the attacked unit
            Player * pPlayer = pLocalClient->getPlayerManager()->findPlayer(owner);
            if (pPlayer != NULL)
                m_pAttackTarget = pPlayer->findUnit(id);
            if (m_Order == OrderAttack)
                recomputePath();
        }
    }
    m_uActivatedSkillId = (u32) pData->readLong();
    m_uActivatedSkillEffectId = (u16) pData->readLong();
    m_Status = (UnitStatus) pData->readLong();

    // Skills references
    m_pSkillsRef->deleteAll();
    long size = pData->readLong();
    for (int i = 0; i < size; i++)
    {
        char sEdition[NAME_MAX_CHARS];
        char sName[NAME_MAX_CHARS];
        char sParams[LUA_FUNCTION_PARAMS_MAX_CHARS];
        u32 id = (u32) pData->readLong();
        pData->readString(sEdition);
        pData->readString(sName);
        pData->readString(sParams);
        Skill * pSkill = new Skill(id, sEdition, sName, sParams, pLocalClient->getDebug());
        m_pSkillsRef->addLast(pSkill);
    }

    // Lua effects
    //removeAllEffects();
    //LuaTargetable::deserializeEffects(pData, getAllEffects(), m_pSkillsRef, pLocalClient, relinkPtrData, RELINK_TYPE_SPELL_TARGET_UNIT);
    //LuaTargetable::deserializeEffects(pData, getDisabledEffects(), m_pSkillsRef, pLocalClient, relinkPtrData, RELINK_TYPE_SPELL_TARGET_UNIT);

    // Find associated unit data
    UnitData * pUnitData = pLocalClient->getDataFactory()->getUnitData(m_sEdition, m_sUnitId);
    // Note that pUnitData is NULL when loading a created avatar (which doesn't have localized elements)
    // In this case name & description are retrieved directly from AvatarData
    if (pUnitData != NULL)
    {
        // L12N elements + texture
        pUnitData->findLocalizedElement(m_sName, NAME_MAX_CHARS, i18n->getCurrentLanguageName(), "name");
        pUnitData->findLocalizedElement(m_sDescription, DESCRIPTION_MAX_CHARS, i18n->getCurrentLanguageName(), "description");
        wsafecpy(m_sTexture, MAX_PATH, pUnitData->m_sTextureFilename);
    }

    updateIdentifiers();
}

// -----------------------------------------------------------------
// Name : serializeForUpdate
// -----------------------------------------------------------------
void Unit::serializeForUpdate(NetworkData * pData)
{
    CoordsMap pos = getMapPos();
    pData->addLong(pos.x);
    pData->addLong(pos.y);
    pData->addLong(m_lValues.size());
    long_hash::iterator it;
    for (it = m_lValues.begin(); it != m_lValues.end(); ++it)
    {
        pData->addString(it->first.c_str());
        pData->addLong(it->second);
    }
    pData->addLong((long)m_Order);
    if (m_Order != OrderAttack)
        m_pAttackTarget = NULL;
    CoordsMap goal = getDestination();
    pData->addLong(goal.x);
    pData->addLong(goal.y);
    pData->addLong(m_pAttackTarget != NULL ? 1 : 0);
    if (m_pAttackTarget != NULL)
    {
        pData->addLong((long)m_pAttackTarget->getOwner());
        pData->addLong((long)m_pAttackTarget->getId());
    }
    pData->addLong((long)m_uActivatedSkillId);
    pData->addLong((long)m_uActivatedSkillEffectId);
    pData->addLong((long)m_Status);
}

// -----------------------------------------------------------------
// Name : deserializeForUpdate
// -----------------------------------------------------------------
void Unit::deserializeForUpdate(NetworkData * pData, LocalClient * pLocalClient)
{
    CoordsMap pos;
    pos.x = (int) pData->readLong();
    pos.y = (int) pData->readLong();
    setMapPos(pos);
    int nbValues = (int) pData->readLong();
    for (int i = 0; i < nbValues; i++)
    {
        char sKey[64];
        pData->readString(sKey);
        setBaseValue(sKey, (double) pData->readLong());
    }
    m_Order = (UnitOrder) pData->readLong();

    CoordsMap goal;
    goal.x = pData->readLong();
    goal.y = pData->readLong();
    if (m_Order == OrderMove)
        computePath(goal);

    m_pAttackTarget = NULL;
    long isAttacking = pData->readLong();
    if (isAttacking)
    {
        int owner = (int) pData->readLong();
        u32 id = (u32) pData->readLong();
        Player * pPlayer = pLocalClient->getPlayerManager()->findPlayer(owner);
        if (pPlayer != NULL)
            m_pAttackTarget = pPlayer->findUnit(id);
    }
    m_uActivatedSkillId = (u32) pData->readLong();
    m_uActivatedSkillEffectId = (u16) pData->readLong();
    m_Status = (UnitStatus) pData->readLong();
}

// -----------------------------------------------------------------
// Name : initGraphics
// -----------------------------------------------------------------
void Unit::initGraphics(DisplayEngine * pDisplay)
{
//  QuadData quad(0.0f, 0.8f, 0.0f, 0.8f, m_sTexture, pDisplay);
    int iTex = pDisplay->getTextureEngine()->loadTexture(m_sTexture, true);
    m_pGeometry = new GeometryCylinder(0.8f, 0.008f, 32, iTex, VB_Static, pDisplay);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void Unit::display()
{
    Coords3D pos3D = getDisplay()->get3DCoords(getMapPos(), BOARDPLANE);
    pos3D = get3DPos();
    pos3D.x += 0.1f;
    pos3D.y += 0.1f;
//  pos3D.z = BOARDPLANE;
    ((GeometryCylinder*)m_pGeometry)->display(pos3D, F_RGBA_NULL, F_RGBA_MULTIPLY(m_PlayerColor, rgba(0.5f, 0.5f, 0.5f, 0.9f)));
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void Unit::update(double delta)
{
    PhysicalObject::update(delta);
    unbindInactiveMovements(true);
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
int Unit::getTexture()
{
    return ((GeometryCylinder*)m_pGeometry)->getTexture();
}

#include "../GameRoot.h"
#include "../LocalClient.h"
#include "../Debug/DebugManager.h"
// -----------------------------------------------------------------
// Name : computePath
// -----------------------------------------------------------------
bool Unit::computePath(CoordsMap mapPos)
{
    m_pAStarPath->deleteAll();
    if (!m_pMap->isInBounds(mapPos))
        return false;

    int iSpeed = getValue(STRING_SPEED);
    if (iSpeed <= 0)
        return false;

    // Call pathfinder
    CoordsMap * path = new CoordsMap[m_pMap->getNumberOfTiles()];
    s16 idx = m_pMap->findPath(this, mapPos, &path);
    if (idx < 0)
    {
        // Check infinite loop spy
        if (idx == -99)
        {
            extern GameRoot * g_pMainGameRoot;
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("SPY infinite loop in pathfinder!!!");
        }
        if (idx == -98)
        {
            extern GameRoot * g_pMainGameRoot;
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("SPY infinite loop in pathfinder: open list overflow");
        }
        if (idx == -97)
        {
            extern GameRoot * g_pMainGameRoot;
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("SPY infinite loop in pathfinder: closed list overflow");
        }
        if (idx == -96)
        {
            extern GameRoot * g_pMainGameRoot;
            g_pMainGameRoot->m_pLocalClient->getDebug()->notifyErrorMessage("SPY infinite loop in pathfinder: extract, parent not found in closed list");
        }
        return false;
    }

    // Store results in m_pAStarPath list
    for (int i = 0; i <= idx; i++)
    {
        AStarStep * pStep = new AStarStep(path[i]);
        m_pAStarPath->addLast(pStep);
    }

    // Find next turn position
    int nextTurn = findNextTurnPosition(path, idx + 1);
    m_pAStarPath->goTo(m_iPathTurnIterator, nextTurn);

    delete[] path;
    return true;
}

// -----------------------------------------------------------------
// Name : computeGroupPath
// -----------------------------------------------------------------
bool Unit::computeGroupPath(CoordsMap mapPos)
{
    m_pAStarPath->deleteAll();

    // Reset group goal data
    Unit * pOther = (Unit*) m_pGroup->getFirst(0);
    while (pOther != NULL)
    {
        if (pOther != this)
            pOther->m_pAStarPath->deleteAll();
        pOther = (Unit*) m_pGroup->getNext(0);
    }

    if (!m_pMap->isInBounds(mapPos))
        return false;

    // Call pathfinder for group
    CoordsMap * path = new CoordsMap[m_pMap->getNumberOfTiles()];
    s16 idx = m_pMap->findPath(m_pGroup, mapPos, &path);
    if (idx < 0)
        return false;

    // Store results in m_pAStarPath list for all units in group
    for (int i = 0; i <= idx; i++)
    {
        Unit * pOther = (Unit*) m_pGroup->getFirst(0);
        while (pOther != NULL)
        {
            AStarStep * pStep = new AStarStep(path[i]);
            pOther->m_pAStarPath->addLast(pStep);
            pOther = (Unit*) m_pGroup->getNext(0);
        }
    }

    // Find smallest next turn position
    int smallest = -1;
    pOther = (Unit*) m_pGroup->getFirst(0);
    while (pOther != NULL)
    {
        int nextTurn = pOther->findNextTurnPosition(path, idx + 1);
        if (smallest < 0 || nextTurn < smallest)
            smallest = nextTurn;
        pOther = (Unit*) m_pGroup->getNext(0);
    }

    // Set smallest turn idx to all group members
    pOther = (Unit*) m_pGroup->getFirst(0);
    while (pOther != NULL)
    {
        pOther->m_pAStarPath->goTo(pOther->m_iPathTurnIterator, smallest);
        pOther = (Unit*) m_pGroup->getNext(0);
    }

    return true;
}

// -----------------------------------------------------------------
// Name : findNextTurnPosition
// -----------------------------------------------------------------
int Unit::findNextTurnPosition(CoordsMap * path, int pathSize)
{
    int iMovesLeft = getValue(STRING_SPEED);
    for (int i = 1; i < pathSize; i++)
    {
        // Find next turn position
        iMovesLeft -= getMoveCost(getMap()->getTileAt(path[i])->m_uTerrainType);
        if (iMovesLeft < 0)
            return max(1, i - 1); // allow at least movement on the adjacent tile
    }
    return pathSize - 1;
}

// -----------------------------------------------------------------
// Name : resetMoveData
// -----------------------------------------------------------------
void Unit::resetMoveData()
{
    m_pAStarPath->deleteAll();
    m_Order = OrderNone;
    m_bModified = true;
}

// -----------------------------------------------------------------
// Name : getMoveCost
// -----------------------------------------------------------------
short Unit::getMoveCost(unsigned char terrainType)
{
    switch (terrainType)
    {
    case TERRAIN_PLAIN:
        return getValue(STRING_MCPLAIN);
    case TERRAIN_FOREST:
        return getValue(STRING_MCFOREST);
    case TERRAIN_MOUNTAIN:
        return getValue(STRING_MCMOUNTAIN);
    case TERRAIN_TOUNDRA:
        return getValue(STRING_MCTOUNDRA);
    case TERRAIN_DESERT:
        return getValue(STRING_MCDESERT);
    case TERRAIN_SEA:
        return getValue(STRING_MCSEA);
    }
    return -1;
}

// -----------------------------------------------------------------
// Name : saveCurrentOrder
// -----------------------------------------------------------------
void Unit::saveCurrentOrder()
{
    m_PreviousOrder = m_Order;
    if (m_PreviousOrder == OrderMove)
        m_PreviousGoal = getDestination();
    else if (m_PreviousOrder == OrderAttack)
        m_pPreviousAttackTarget = m_pAttackTarget;
}

// -----------------------------------------------------------------
// Name : retrievePreviousOrder
// -----------------------------------------------------------------
void Unit::retrievePreviousOrder(UnitOptionsDlg * pDlg)
{
    if (pDlg != NULL && m_PreviousOrder == OrderSkill)
        pDlg->redoSkillAction(this);
    m_Order = m_PreviousOrder;
    m_PreviousOrder = OrderNone;
    switch (m_Order)
    {
    case OrderMove:
    {
        if (!computePath(m_PreviousGoal))
            m_Order = OrderNone;
        break;
    }
    case OrderAttack:
    {
        m_pAttackTarget = m_pPreviousAttackTarget;
        if (m_pAttackTarget != NULL)
            computePath(m_pAttackTarget->getMapPos());
        break;
    }
    default:
        break;
    }
}

// -----------------------------------------------------------------
// Name : unsetOrder
// -----------------------------------------------------------------
void Unit::unsetOrder()
{
    m_Order = m_PreviousOrder = OrderNone;
    m_bModified = true;
}

// -----------------------------------------------------------------
// Name : setFortifyOrder
// -----------------------------------------------------------------
void Unit::setFortifyOrder()
{
    m_Order = OrderFortify;
    m_bModified = true;
}

// -----------------------------------------------------------------
// Name : setMoveOrder
//  returns true if the move is valid
// -----------------------------------------------------------------
bool Unit::setMoveOrder(CoordsMap mapPos, UnitOptionsDlg * pDlg)
{
    if (pDlg != NULL && m_Order == OrderSkill)
        pDlg->cancelSkillAction(this);
    m_Order = OrderMove;
    m_bModified = true;
    return computePath(mapPos);
}

// -----------------------------------------------------------------
// Name : setGroupMoveOrder
//  returns true if the move is valid
// -----------------------------------------------------------------
bool Unit::setGroupMoveOrder(CoordsMap mapPos, UnitOptionsDlg * pDlg)
{
    Unit * pOther = (Unit*) m_pGroup->getFirst(0);
    while (pOther != NULL)
    {
        if (pDlg != NULL && pOther->getOrder() == OrderSkill)
            pDlg->cancelSkillAction(pOther);
        pOther->m_Order = OrderMove;
        pOther->m_bModified = true;
        pOther = (Unit*) m_pGroup->getNext(0);
    }
    return computeGroupPath(mapPos);
}

// -----------------------------------------------------------------
// Name : setAttackOrder
// -----------------------------------------------------------------
bool Unit::setAttackOrder(Unit * pAttackTarget, UnitOptionsDlg * pDlg)
{
    if (pDlg != NULL && m_Order == OrderSkill)
        pDlg->cancelSkillAction(this);
    m_Order = OrderAttack;
    m_pAttackTarget = pAttackTarget;
    m_bModified = true;
    if (pAttackTarget != NULL)
        return computePath(pAttackTarget->getMapPos());
    return false;
}

// -----------------------------------------------------------------
// Name : setGroupAttackOrder
// -----------------------------------------------------------------
bool Unit::setGroupAttackOrder(Unit * pAttackTarget, UnitOptionsDlg * pDlg)
{
    Unit * pOther = (Unit*) m_pGroup->getFirst(0);
    while (pOther != NULL)
    {
        if (pDlg != NULL && pOther->getOrder() == OrderSkill)
            pDlg->cancelSkillAction(pOther);
        pOther->m_Order = OrderAttack;
        pOther->m_pAttackTarget = pAttackTarget;
        pOther->m_bModified = true;
        pOther = (Unit*) m_pGroup->getNext(0);
    }
    if (pAttackTarget != NULL)
        return computeGroupPath(pAttackTarget->getMapPos());
    return false;
}

// -----------------------------------------------------------------
// Name : recomputePath
//  Return true if there is a path
// -----------------------------------------------------------------
bool Unit::recomputePath()
{
    if (m_Order == OrderMove)
    {
        CoordsMap goal = getDestination();
        if (m_pGroup != NULL && m_pGroup->size > 1)
            return computeGroupPath(goal);
        else
            return computePath(goal);
    }
    else if (m_Order == OrderAttack)
    {
        if (m_pAttackTarget == NULL)
            return false;
        if (m_pGroup != NULL && m_pGroup->size > 1)
            return computeGroupPath(m_pAttackTarget->getMapPos());
        else
            return computePath(m_pAttackTarget->getMapPos());
    }
    return false;
}

// -----------------------------------------------------------------
// Name : getDestination
// -----------------------------------------------------------------
CoordsMap Unit::getDestination()
{
    if (m_pAStarPath->last != NULL)
        return ((AStarStep*) m_pAStarPath->last->obj)->pos;
    else
        return CoordsMap(-1, -1);
}

// -----------------------------------------------------------------
// Name : setSkillOrder
// -----------------------------------------------------------------
void Unit::setSkillOrder(u32 uSkillId, u16 uEffectId)
{
    m_Order = OrderSkill;
    m_uActivatedSkillEffectId = uEffectId;
    m_uActivatedSkillId = uSkillId;
    m_bModified = true;
}

// -----------------------------------------------------------------
// Name : getSkillOrder
// -----------------------------------------------------------------
ChildEffect * Unit::getSkillOrder()
{
    LuaObject * pLua = (LuaObject*) getFirstEffect(0);
    while (pLua != NULL)
    {
        if (pLua->getType() == LUAOBJECT_SKILL && pLua->getInstanceId() == m_uActivatedSkillId)
            return pLua->getChildEffect(m_uActivatedSkillEffectId);
        pLua = (LuaObject*) getNextEffect(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : findSkill
// -----------------------------------------------------------------
Skill * Unit::findSkill(u32 uSkillId, bool * isActive)
{
    LuaObject * pLua = (LuaObject*) m_pSkillsRef->getFirst(0);
    while (pLua != NULL)
    {
        if (pLua->getInstanceId() == uSkillId)
        {
            if (isActive != NULL)
                *isActive = !getDisabledEffects()->goTo(0, pLua);
            return (Skill*) pLua;
        }
        pLua = (LuaObject*) m_pSkillsRef->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : getUnitData
// -----------------------------------------------------------------
UnitData * Unit::getUnitData(LocalClient * pLocalClient)
{
    return pLocalClient->getDataFactory()->getUnitData(m_sEdition, m_sUnitId);
}

// -----------------------------------------------------------------
// Name : addSkill
// -----------------------------------------------------------------
void Unit::addSkill(Skill * pSkill)
{
    // First check if we already have thise skill and if it should be "merged" with the existing one (like mana producer)
    if (pSkill->isMergeable() || !pSkill->isCumulative())
    {
        Skill * pOther = (Skill*) m_pSkillsRef->getFirst(0);
        while (pOther != NULL)
        {
            if (strcmp(pOther->getObjectEdition(), pSkill->getObjectEdition()) == 0 && strcmp(pOther->getObjectName(), pSkill->getObjectName()) == 0)
            {
                if (pSkill->isMergeable())
                    pOther->merge(pSkill);
                delete pSkill;
                return;
            }
            pOther = (Skill*) m_pSkillsRef->getNext(0);
        }
    }
    m_pSkillsRef->addLast(pSkill);
    attachEffect(pSkill);
    pSkill->addTarget(this, SELECT_TYPE_UNIT);
    pSkill->setCaster(this);
    pSkill->callLuaFunction("setAttachedUnit", 0, "s", m_sIdentifiers);
}

// -----------------------------------------------------------------
// Name : setNameDescriptionTexture
// -----------------------------------------------------------------
void Unit::setNameDescriptionTexture(char * sName, char * sDesc, char * sTex)
{
    wsafecpy(m_sName, NAME_MAX_CHARS, sName);
    wsafecpy(m_sDescription, DESCRIPTION_MAX_CHARS, sDesc);
    wsafecpy(m_sTexture, MAX_PATH, sTex);
}

// -----------------------------------------------------------------
// Name : getValue
// -----------------------------------------------------------------
long Unit::getValue(const char * sName, bool bBase, bool * bFound)
{
    long val = LuaTargetable::getValue(sName, bBase, bFound);
    if (strcmp(sName, STRING_MELEE) == 0 && m_Order == OrderFortify && !bBase)
        val = max((val * 6) / 5, val+1);
    return val;
}

// -----------------------------------------------------------------
// Name : getInfo
// -----------------------------------------------------------------
char * Unit::getInfo(char * sBuf, int iSize, InfoDest eDest)
{
    char sName[NAME_MAX_CHARS + 16];
    if (m_Status == US_Dead)
    {
        char sDead[16];
        i18n->getText("DEAD", sDead, 16);
        snprintf(sName, NAME_MAX_CHARS + 16, "%s (%s)", m_sName, sDead);
    }
    else
        wsafecpy(sName, NAME_MAX_CHARS, m_sName);

    if (eDest == Dest_ShortInfoDialog)
    {
        wsafecpy(sBuf, iSize, sName);
        return sBuf;
    }

    char s2P[8];
    i18n->getText("2P", s2P, 8);

    // Alignment
    char sAlignment[128];
    char sTemp1[64];
    char sTemp2[64];
    int alignment = getValue(STRING_ALIGNMENT);
    snprintf(sAlignment, 128, "%s%s%s", i18n->getText1stUp(STRING_ALIGNMENT, sTemp1, 64), s2P, UnitData::getAlignmentInfos(alignment, sTemp2, 64));

    // Skills
    char sSkills[512] = "";
    i18n->getText1stUp("SKILLS", sSkills, 512);
    wsafecat(sSkills, 512, s2P);
    char sSep2[4] = "";
    Skill * pSkill = (Skill*) m_pSkillsRef->getFirst(0);
    while (pSkill != NULL)
    {
        wsafecat(sSkills, 512, sSep2);
        wsafecpy(sSep2, 4, ", ");
        wsafecat(sSkills, 512, pSkill->getLocalizedName());
        pSkill = (Skill*) m_pSkillsRef->getNext(0);
    }

    if (eDest == Dest_InfoDialog)
    {
        snprintf(sBuf, iSize, "%s\n", sName);
        getInfo_AddValue(sBuf, iSize, STRING_MELEE, "\n");
        getInfo_AddValue(sBuf, iSize, STRING_RANGE, "\n");
        getInfo_AddValue(sBuf, iSize, STRING_ARMOR, "\n");
        getInfo_AddValue(sBuf, iSize, STRING_ENDURANCE, "\n");
        getInfo_AddValue(sBuf, iSize, STRING_SPEED, "\n");
        getInfo_AddValue(sBuf, iSize, STRING_LIFE, "\n");
        wsafecat(sBuf, iSize, sAlignment);
        wsafecat(sBuf, iSize, "\n");
        wsafecat(sBuf, iSize, sSkills);
    }
    else
    {
        wsafecpy(sBuf, iSize, "");
        getInfo_AddValue(sBuf, iSize, STRING_MELEE, "    ");
        getInfo_AddValue(sBuf, iSize, STRING_RANGE, "    ");
        getInfo_AddValue(sBuf, iSize, STRING_ARMOR, "    ");
        getInfo_AddValue(sBuf, iSize, STRING_ENDURANCE, "    ");
        getInfo_AddValue(sBuf, iSize, STRING_SPEED, "    ");
        getInfo_AddValue(sBuf, iSize, STRING_LIFE, "    ");
        wsafecat(sBuf, iSize, sAlignment);
        wsafecat(sBuf, iSize, "\n");
        wsafecat(sBuf, iSize, sSkills);
        wsafecat(sBuf, iSize, "\n");
        wsafecat(sBuf, iSize, m_sDescription);
    }
    return sBuf;
}
