#include "ConstantMovement3D.h"

// -----------------------------------------------------------------
// Name : ConstantMovement3D
//  Constructor
// -----------------------------------------------------------------
ConstantMovement3D::ConstantMovement3D(u16 uMoveId, double fDuration, Coords3D vect)
      : Movement(uMoveId)
{
  m_Vect = vect;
  m_fDuration = m_fTimer = fDuration;
}

// -----------------------------------------------------------------
// Name : ~ConstantMovement3D
//  Destructor
// -----------------------------------------------------------------
ConstantMovement3D::~ConstantMovement3D()
{
}

// -----------------------------------------------------------------
// Name : applyMovement
// -----------------------------------------------------------------
void ConstantMovement3D::applyMovement(double delta, Coords * pCoords)
{
  Coords3D * pCoords3D = (Coords3D*) pCoords;
  m_fTimer -= delta;
  if (m_fTimer <= 0)
  {
    *pCoords3D += m_Vect * (delta+m_fTimer) / m_fDuration;
    setActive(false);
  }
  else
  {
    *pCoords3D += m_Vect * delta / m_fDuration;
  }
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
Movement * ConstantMovement3D::clone(u16 uMoveId)
{
  ConstantMovement3D * pClone = new ConstantMovement3D(uMoveId, m_fTimer, m_Vect);
  return pClone;
}
