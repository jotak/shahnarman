#ifndef _GEOMETRYTEXT_ENGINE_H
#define _GEOMETRYTEXT_ENGINE_H

#include "Geometry.h"

class GeometryText : public Geometry
{
public:
    GeometryText(const char * sText, int iFontId, VBType type, DisplayEngine * pDisplay);
    GeometryText(const char * sText, int iFontId, float fFontHeight, VBType type, DisplayEngine * pDisplay);
    ~GeometryText();

    void display(CoordsScreen pos, F_RGBA color);
    void display(Coords3D pos, F_RGBA color);
    void setText(const char * sText, int iFontId);
    virtual void reload();

protected:
    int m_iNbQuads;
    char * m_sText;
    int m_iTextLength;
    int m_iFontId;
    float m_fScale;
};

#endif
