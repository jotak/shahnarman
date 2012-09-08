#ifndef _BASE_OBJECT_H
#define _BASE_OBJECT_H

#include "../utils.h"

#define NAME_MAX_CHARS          64
#define DESCRIPTION_MAX_CHARS   1024

class BaseObject
{
public:
    BaseObject()
    {
        m_pAttachment = NULL;
    };
    virtual ~BaseObject() {};
    void setAttachment(BaseObject * pObj)
    {
        m_pAttachment = pObj;
    };
    BaseObject * getAttachment()
    {
        return m_pAttachment;
    };

protected:
    BaseObject * m_pAttachment;
};

class StringObject : public BaseObject
{
public:
    StringObject(char * sStr)
    {
        wsafecpy(m_sString, 256, sStr);
    };
    char m_sString[256];
};

class CoordsObject : public BaseObject
{
public:
    CoordsObject(Coords3D c)
    {
        m_Coords = c;
    };
    CoordsObject(CoordsScreen c)
    {
        m_Coords.x = c.x;
        m_Coords.y = c.y;
        m_Coords.z = c.z;
    };
    CoordsObject(CoordsMap c)
    {
        m_Coords.x = c.x;
        m_Coords.y = c.y;
    };

    Coords3D getCoords3D()
    {
        return m_Coords;
    };
    CoordsScreen getCoordsScreen()
    {
        return CoordsScreen((int)m_Coords.x, (int)m_Coords.y, m_Coords.z);
    };
    CoordsMap getCoordsMap()
    {
        return CoordsMap((int)m_Coords.x, (int)m_Coords.y);
    };

    Coords3D m_Coords;
};

#endif
