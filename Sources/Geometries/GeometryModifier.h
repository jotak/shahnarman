#ifndef _GEOMETRY_MODIFIER_H
#define _GEOMETRY_MODIFIER_H

#include "../Common/BaseObject.h"

class GeometryModifier : public BaseObject
{
public:
    GeometryModifier(u16 uModId);
    ~GeometryModifier();

    virtual void doTransforms(F_RGBA * pColor) {};
    virtual void update(double delta) {};
    virtual GeometryModifier * clone(u16 uModId) = 0;
    bool isActive()
    {
        return m_bActive;
    };
    void setActive(bool bActive)
    {
        m_bActive = bActive;
    };
    bool isRunning()
    {
        return m_bRunning;
    };
    void setRunning(bool bRunning)
    {
        m_bRunning = bRunning;
    };
    u16 getId()
    {
        return m_uModId;
    };

protected:
    u16 m_uModId;
    bool m_bActive;   // Not active = not displayed and not updated
    bool m_bRunning;  // Not running = not updated
};

#endif
