#ifndef _MOVEMENT_H
#define _MOVEMENT_H

#include "../Common/BaseObject.h"

class Movement : public BaseObject
{
public:
    Movement(u16 uMoveId);
    ~Movement();

    virtual void applyMovement(double delta, Coords * pCoords) = 0;
    virtual Movement * clone(u16 uMoveId) = 0;
    bool isActive()
    {
        return m_bActive;
    };
    void setActive(bool bActive)
    {
        m_bActive = bActive;
    };
    u16 getId()
    {
        return m_uMoveId;
    };

protected:
    u16 m_uMoveId;
    bool m_bActive;
};

#endif
