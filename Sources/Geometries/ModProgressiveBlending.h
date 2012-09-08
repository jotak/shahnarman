#ifndef _MOD_PROGRESSIVE_BLENDING_H
#define _MOD_PROGRESSIVE_BLENDING_H

#include "GeometryModifier.h"

class ModProgressiveBlending : public GeometryModifier
{
public:
    ModProgressiveBlending(u16 uModId, F_RGBA startColor, F_RGBA endColor, float fPeriod);
    ~ModProgressiveBlending();

    virtual void doTransforms(F_RGBA * pColor);
    virtual void update(double delta);
    virtual GeometryModifier * clone(u16 uModId);

protected:
    F_RGBA m_StartColor, m_EndColor;
    float m_fPeriod;
    float m_fTimer;
};

#endif
