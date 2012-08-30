#ifndef _GEOMETRY_QUADS_H
#define _GEOMETRY_QUADS_H

#include "Geometry.h"

class QuadData;

class GeometryQuads : public Geometry
{
public:
  GeometryQuads(int nQuads, QuadData ** pAllQuads, VBType type);
  GeometryQuads(QuadData * pQuad, VBType type);
  GeometryQuads(VBType type, DisplayEngine * pDisplay);
  ~GeometryQuads();

  void display(CoordsScreen pos, F_RGBA color);
  void display(Coords3D pos, F_RGBA color);
  void modify(int nQuads, QuadData ** pAllQuads);
  void modify(QuadData * pQuad) { modify(1, &pQuad); };
  int getTexture(int iQuad = 0);
  void setTexture(int iTexId, int iQuad = 0);
  virtual void reload();

protected:
  int m_iNbQuads;
  QuadData ** m_pAllQuads;
};


class QuadData
{
friend class GeometryQuads;
public:
  QuadData(int xstart, int xend, int ystart, int yend, wchar_t * texture, DisplayEngine * pDisplay);
  QuadData(int xstart, int xend, int ystart, int yend, int texture, DisplayEngine * pDisplay);
  QuadData(int xstart, int xend, int ystart, int yend, int ustart, int uend, int vstart, int vend, wchar_t * texture, DisplayEngine * pDisplay);
  QuadData(float xstart, float xend, float ystart, float yend, wchar_t * texture, DisplayEngine * pDisplay);
  QuadData(float xstart, float xend, float ystart, float yend, int ustart, int uend, int vstart, int vend, wchar_t * texture, DisplayEngine * pDisplay);
  QuadData(float xstart, float xend, float ystart, float yend, float ustart, float uend, float vstart, float vend, int texture, DisplayEngine * pDisplay);

  static void releaseQuads(int nQuads, QuadData ** pQuads);
  QuadData * clone();

  float m_fXStart;
  float m_fXEnd;
  float m_fYStart;
  float m_fYEnd;
  float m_fUStart;
  float m_fUEnd;
  float m_fVStart;
  float m_fVEnd;
  int m_iTex;
  DisplayEngine * m_pDisplay;
};

#endif
