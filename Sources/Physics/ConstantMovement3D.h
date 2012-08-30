#ifndef _CONSTANTMOVEMENT3D_H
#define _CONSTANTMOVEMENT3D_H

#include "Movement.h"

class ConstantMovement3D : public Movement
{
public:
  ConstantMovement3D(u16 uMoveId, double fDuration, Coords3D vect);
  ~ConstantMovement3D();

  virtual void applyMovement(double delta, Coords * pCoords);
  virtual Movement * clone(u16 uMoveId);

protected:
  Coords3D m_Vect;
  double m_fTimer;
  double m_fDuration;
};

#endif
