#ifndef _PHYSICALOBJECT_H
#define _PHYSICALOBJECT_H

#include "../Common/ObjectList.h"
#include "Movement.h"

// This class has no inheritance because intended to be used in polymorphism
class PhysicalObject
{
public:
  // Constructor / destructor
  PhysicalObject();
  ~PhysicalObject();

  // update
  virtual void update(double delta);

  // Auto-movements functions
  void bindMovement(Movement * pMvt);
  Movement * findMovement(u16 uMoveId);
  void unbindMovement(u16 uMoveId, bool bAll, bool bDelete);
  void unbindInactiveMovements(bool bDelete);
  void unbindAllMovements(bool bDelete);

  // Explicit size / position
  void moveBy(Coords3D coords);
  void moveTo(Coords3D coords);
  Coords3D get3DPos() { return m_3DPosition; };

protected:
  ObjectList * m_pMovementsList;
  Coords3D m_3DPosition;
};

#endif
