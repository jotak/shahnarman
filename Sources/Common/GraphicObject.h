#ifndef _GRAPHIC_OBJECT_H
#define _GRAPHIC_OBJECT_H

#include "BaseObject.h"
#include "../Geometries/Geometry.h"

#define TYPE_MAX_CHARS      64

#define GOTYPE_NOTHING      0x00000000
#define GOTYPE_ALL          0xffffffff
#define GOTYPE_BASE         0x00000001
#define GOTYPE_COMPONENT    0x00000002
#define GOTYPE_CONTAINER    0x00000004
#define GOTYPE_FRAME        0x00000008
#define GOTYPE_TBDFRAME     0x00000010
#define GOTYPE_BUTTON       0x00000020
#define GOTYPE_LABEL        0x00000040
#define GOTYPE_TOGGLEBUTTON 0x00000080
#define GOTYPE_DOCUMENT     0x00000100
//#define GOTYPE_PERSISTANT   0x00000200
#define GOTYPE_EDITBOX      0x00000800
#define GOTYPE_SMARTSLIDER  0x00001000

#define GOTYPE_MAPOBJECT    0x00010000
#define GOTYPE_UNIT         0x00020000
#define GOTYPE_DEAD_UNIT    0x00040000
#define GOTYPE_MAPTILE      0x00080000
#define GOTYPE_TOWN         0x00100000
#define GOTYPE_TEMPLE       0x00200000
#define GOTYPE_REMOVED_UNIT 0x00400000
#define GOTYPE_OWNED_OBJECT 0x01000000
#define GOTYPE_FOE_OBJECT   0x02000000

class GraphicObject : public BaseObject // Includes both map objects and GUI objects
{
public:
    GraphicObject()
    {
        m_pGeometry = NULL;
    };
    ~GraphicObject()
    {
        if (m_pGeometry != NULL) delete m_pGeometry;
    };

    virtual u32 getType()
    {
        return GOTYPE_BASE;
    };
    virtual DisplayEngine * getDisplay()
    {
        if (m_pGeometry != NULL) return m_pGeometry->getDisplay();
        else return NULL;
    };
    virtual Geometry * getGeometry()
    {
        return m_pGeometry;
    };

protected:
    Geometry * m_pGeometry;
};

#endif
