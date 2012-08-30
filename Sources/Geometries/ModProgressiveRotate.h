#ifndef _MOD_PROGRESSIVE_ROTATE_H
#define _MOD_PROGRESSIVE_ROTATE_H

#include "GeometryModifier.h"

class ModProgressiveRotate : public GeometryModifier
{
public:
  ModProgressiveRotate(u16 uModId, float fSpeed, float fCenterX, float fCenterY);
  ~ModProgressiveRotate();

  virtual void doTransforms(F_RGBA * pColor);
  virtual void update(double delta);
  virtual GeometryModifier * clone(u16 uModId);

protected:
  float m_fAngle;
  float m_fSpeed; // degree per second
  float m_fRotateCenterX;
  float m_fRotateCenterY;
};

#endif
