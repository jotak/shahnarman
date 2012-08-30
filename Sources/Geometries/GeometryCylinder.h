#ifndef _GEOMETRY_CYLINDER_H
#define _GEOMETRY_CYLINDER_H

#include "Geometry.h"

class GLUQuadric;

class GeometryCylinder : public Geometry
{
public:
  GeometryCylinder(float fDiameter, float fHeight, u16 uSlices, int iTex, VBType type, DisplayEngine * pDisplay);
  ~GeometryCylinder();

  void display(CoordsScreen pos, F_RGBA color) { display(pos, color, F_RGBA_NULL); };
  void display(Coords3D pos, F_RGBA color) { display(pos, color, F_RGBA_NULL); };
  void display(CoordsScreen pos, F_RGBA color, F_RGBA borderColor);
  void display(Coords3D pos, F_RGBA color, F_RGBA borderColor);
  void modify(float fDiameter, float fHeight, u16 uSlices, int iTex);
  int getTexture();
  void setTexture(int iTexId);
  virtual void reload();

protected:
  int m_iTex;
  int m_iTopTex;
  int m_iRoundTex;
  GLuint m_BordersVboId;
  GLUquadric * m_pQuadric;
  float m_fDiameter;
  float m_fHeight;
  u16 m_uSlices;
};

#endif
