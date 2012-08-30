#ifndef _GEOMETRYTEXT_ENGINE_H
#define _GEOMETRYTEXT_ENGINE_H

#include "Geometry.h"

class GeometryText : public Geometry
{
public:
  GeometryText(const wchar_t * sText, int iFontId, VBType type, DisplayEngine * pDisplay);
  GeometryText(const wchar_t * sText, int iFontId, float fFontHeight, VBType type, DisplayEngine * pDisplay);
  ~GeometryText();

  void display(CoordsScreen pos, F_RGBA color);
  void display(Coords3D pos, F_RGBA color);
  void setText(const wchar_t * sText, int iFontId);
  virtual void reload();

protected:
  int m_iNbQuads;
  wchar_t * m_sText;
  int m_iTextLength;
  int m_iFontId;
  float m_fScale;
};

#endif
