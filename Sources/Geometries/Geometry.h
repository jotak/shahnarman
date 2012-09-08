#ifndef _GEOMETRY_ENGINE_H
#define _GEOMETRY_ENGINE_H

#include "../Display/Texture.h"
#include "../Display/DisplayEngine.h"
#include "GeometryModifier.h"

#pragma pack(push, 1)
class Vertex
{
public:
    void set(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v);

    GLfloat u, v;
    GLfloat x, y, z;
};
#pragma pack(pop)

enum VBType
{
    VB_Static = 0,
    VB_Stream,
    VB_Dynamic
};

class Geometry : public BaseObject
{
public:
    Geometry(VBType type, DisplayEngine * pDisplay);
    virtual ~Geometry();

    // Update function
    virtual void update(double delta);

    // Display functions
    virtual void display(CoordsScreen pos, F_RGBA color) {};
    virtual void display(Coords3D pos, F_RGBA color) {};
    DisplayEngine * getDisplay()
    {
        return m_pDisplay;
    };

    // Mods functions
    GeometryModifier * getModifier(u16 uModId);
    void bindModifier(GeometryModifier * pMod);
    void unbindModifier(u16 uModId, bool bAll, bool bDelete);

    // Shaders
    bool bindShader(const char * sVertexShader, const char * sPixelShader);
    bool isShaderActive()
    {
        return m_bShaderEnabled;
    };
    void activateShader();
    void deactivateShader();
    int registerShaderVariable(const char * name);
    void beginSetShaderVariables();
    void endSetShaderVariables();
    void setShaderInt(int iVarId, int val);
    void setShaderFloat(int iVarId, float val);
    void setShaderIvec2(int iVarId, CoordsScreen val);
    void setShaderVec3(int iVarId, Coords3D val);

    // Misc.
    virtual void reload() = 0;

protected:
    void doModTransforms(F_RGBA * pColor);

    GLuint m_VboId;
    DisplayEngine * m_pDisplay;
    VBType m_Type;
    ObjectList * m_pModsList;
    GLuint m_uShaderProgram;
    GLuint m_uPxShader;
    GLuint m_uVxShader;
    bool m_bShaderEnabled;
    bool m_bShaderLoaded;
};

#endif
