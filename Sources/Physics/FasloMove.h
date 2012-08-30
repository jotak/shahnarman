#ifndef _FASLOMOVE_H
#define _FASLOMOVE_H

#include "Movement.h"

class FasloMove : public Movement
{
public:
  FasloMove(u16 uMoveId, Coords3D vector, float speedMod = 1.0f);
  ~FasloMove();

  virtual void applyMovement(double delta, Coords * pCoords);
  virtual Movement * clone(u16 uMoveId);

protected:
  Coords3D m_Vector;
  float m_fSpeedMod;
  float m_fPositionOnVector;
};

#endif
