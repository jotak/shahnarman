#ifndef _STENCIL_GEOMETRY_H
#define _STENCIL_GEOMETRY_H

#include "Geometry.h"

class StencilGeometry : public Geometry
{
public:
  StencilGeometry(int iWidth, int iHeight, VBType type, DisplayEngine * pDisplay);
  ~StencilGeometry();

  void fillStencil(CoordsScreen position, bool bAdd);
  void resize(int iWidth, int iHeight);
  virtual void reload();

private:
  int m_iWidth, m_iHeight;
};

#endif
