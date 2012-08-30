#include "SlofasloMove.h"

// -----------------------------------------------------------------
// Name : SlofasloMove
//  Constructor
// -----------------------------------------------------------------
SlofasloMove::SlofasloMove(u16 uMoveId, Coords3D vector, float speedInit, float speedMod) : Movement(uMoveId)
{
  m_Vector = vector;
  m_fSpeedInit = speedInit;
  m_fSpeedMod = speedMod;
  m_fPositionOnVector = 0.0f;
}

// -----------------------------------------------------------------
// Name : ~SlofasloMove
//  Destructor
// -----------------------------------------------------------------
SlofasloMove::~SlofasloMove()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy SlofasloMove\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy SlofasloMove\n");
#endif
}

// -----------------------------------------------------------------
// Name : applyMovement
// -----------------------------------------------------------------
void SlofasloMove::applyMovement(double delta, Coords * pCoords)
{
  float fPosOnVectDif = m_fPositionOnVector;
  float fSpeed = m_fSpeedInit + m_fSpeedMod * sin((float)M_PI * m_fPositionOnVector);
  m_fPositionOnVector += (float)(delta * fSpeed);
  if (m_fPositionOnVector >= 1.0f)
  {
    m_fPositionOnVector = 1.0f;
    setActive(false);
  }
  fPosOnVectDif = m_fPositionOnVector - fPosOnVectDif;
  *((Coords3D*)pCoords) += m_Vector * fPosOnVectDif;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
Movement * SlofasloMove::clone(u16 uMoveId)
{
  return new SlofasloMove(uMoveId, m_Vector, m_fSpeedInit, m_fSpeedMod);
}
