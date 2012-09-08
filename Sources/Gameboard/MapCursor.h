#ifndef _MAPCURSOR_H
#define _MAPCURSOR_H

#include "../Physics/PhysicalObject.h"
#include "../Geometries/GeometryQuads.h"

class MapCursor : public PhysicalObject
{
public:
    MapCursor();
    ~MapCursor();

    void init(DisplayEngine * pDisplay);
    void update(double delta);
    void display();
    void moveTo(CoordsMap mapPos);
    bool isEnabled()
    {
        return m_bEnabled;
    };
    void setEnabled(bool b)
    {
        m_bEnabled = b;
    };
    void setColor(F_RGBA color)
    {
        m_Color = color;
    };

protected:
    GeometryQuads * m_pGeometry;
    F_RGBA m_Color;
    bool m_bEnabled;
};

#endif
