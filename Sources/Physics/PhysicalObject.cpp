#include "PhysicalObject.h"

// -----------------------------------------------------------------
// Name : PhysicalObject
//  Constructor
// -----------------------------------------------------------------
PhysicalObject::PhysicalObject()
{
    m_pMovementsList = new ObjectList(true);
}

// -----------------------------------------------------------------
// Name : ~PhysicalObject
//  Destructor
// -----------------------------------------------------------------
PhysicalObject::~PhysicalObject()
{
    delete m_pMovementsList;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void PhysicalObject::update(double delta)
{
    Movement * pMvt = (Movement*) m_pMovementsList->getFirst(0);
    while (pMvt != NULL)
    {
        if (pMvt->isActive())
            pMvt->applyMovement(delta, &m_3DPosition);
        pMvt = (Movement*) m_pMovementsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : findMovement
// -----------------------------------------------------------------
Movement * PhysicalObject::findMovement(u16 uMoveId)
{
    Movement * pMvt = (Movement*) m_pMovementsList->getFirst(0);
    while (pMvt != NULL)
    {
        if (pMvt->getId() == uMoveId)
            return pMvt;
        pMvt = (Movement*) m_pMovementsList->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : bindMovement
// -----------------------------------------------------------------
void PhysicalObject::bindMovement(Movement * pMvt)
{
    m_pMovementsList->addFirst(pMvt);
}

// -----------------------------------------------------------------
// Name : unbindMovement
// -----------------------------------------------------------------
void PhysicalObject::unbindMovement(u16 uMoveId, bool bAll, bool bDelete)
{
    Movement * pMvt = (Movement*) m_pMovementsList->getFirst(0);
    while (pMvt != NULL)
    {
        if (pMvt->getId() == uMoveId)
        {
            m_pMovementsList->deleteCurrent(0, false, !bDelete);
            if (!bAll)
                return;
        }
        pMvt = (Movement*) m_pMovementsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : unbindInactiveMovements
// -----------------------------------------------------------------
void PhysicalObject::unbindInactiveMovements(bool bDelete)
{
    Movement * pMvt = (Movement*) m_pMovementsList->getFirst(0);
    while (pMvt != NULL)
    {
        if (!pMvt->isActive())
            m_pMovementsList->deleteCurrent(0, false, !bDelete);
        pMvt = (Movement*) m_pMovementsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : unbindInactiveMovements
// -----------------------------------------------------------------
void PhysicalObject::unbindAllMovements(bool bDelete)
{
    m_pMovementsList->deleteAll(!bDelete);
}

// -----------------------------------------------------------------
// Name : moveBy
// -----------------------------------------------------------------
void PhysicalObject::moveBy(Coords3D coords)
{
    m_3DPosition += coords;
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void PhysicalObject::moveTo(Coords3D coords)
{
    m_3DPosition = coords;
}
