#include "FasloMove.h"

// -----------------------------------------------------------------
// Name : FasloMove
//  Constructor
// -----------------------------------------------------------------
FasloMove::FasloMove(u16 uMoveId, Coords3D vector, float speedMod) : Movement(uMoveId)
{
    m_Vector = vector;
    m_fSpeedMod = speedMod;
    m_fPositionOnVector = 0.0f;
}

// -----------------------------------------------------------------
// Name : ~FasloMove
//  Destructor
// -----------------------------------------------------------------
FasloMove::~FasloMove()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy FasloMove\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy FasloMove\n");
#endif
}

// -----------------------------------------------------------------
// Name : applyMovement
// -----------------------------------------------------------------
void FasloMove::applyMovement(double delta, Coords * pCoords)
{
    float fPosOnVectDif = m_fPositionOnVector;
    float fSpeed = m_fSpeedMod * cos((float)M_PI * m_fPositionOnVector / 2.0f);
    m_fPositionOnVector += (float)(delta * fSpeed);
    if (m_fPositionOnVector >= 1.0f - EPSILON)
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
Movement * FasloMove::clone(u16 uMoveId)
{
    return new FasloMove(uMoveId, m_Vector, m_fSpeedMod);
}
