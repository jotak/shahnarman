#ifndef _SLOFASLO_MOVE_H
#define _SLOFASLO_MOVE_H

#include "Movement.h"

class SlofasloMove : public Movement
{
public:
    SlofasloMove(u16 uMoveId, Coords3D vector, float speedInit, float speedMod = 1.0f);
    ~SlofasloMove();

    virtual void applyMovement(double delta, Coords * pCoords);
    virtual Movement * clone(u16 uMoveId);

protected:
    Coords3D m_Vector;
    float m_fSpeedInit;
    float m_fSpeedMod;
    float m_fPositionOnVector;
};

#endif
