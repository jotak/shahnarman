// -----------------------------------------------------------------
// Geometry and Vertex
// -----------------------------------------------------------------
#include "Geometry.h"

// -----------------------------------------------------------------
// Name : Vertex::set
// -----------------------------------------------------------------
void Vertex::set(GLfloat x, GLfloat y, GLfloat z, GLfloat u, GLfloat v)
{
    this->x = x;
    this->y = y;
    this->z = z;
    this->u = u;
    this->v = v;
}

// -----------------------------------------------------------------
// Name : Geometry
//  Constructor
// -----------------------------------------------------------------
Geometry::Geometry(VBType type, DisplayEngine * pDisplay)
{
    m_Type = type;
    m_pDisplay = pDisplay;
    m_pModsList = new ObjectList(true);
    m_pDisplay->registerGeometry(this);
    m_uShaderProgram = 0;
    m_uPxShader = m_uVxShader = 0;
    m_bShaderEnabled = false;
    m_bShaderLoaded = false;
}

// -----------------------------------------------------------------
// Name : ~Geometry
//  Destructor
// -----------------------------------------------------------------
Geometry::~Geometry()
{
    if (m_uShaderProgram != 0)
        glDeleteProgram(m_uShaderProgram);
    if (m_uPxShader != 0)
        glDeleteShader(m_uPxShader);
    if (m_uVxShader != 0)
        glDeleteShader(m_uVxShader);
    m_pDisplay->unregisterGeometry(this);
    delete m_pModsList;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void Geometry::update(double delta)
{
    // Update display mods
    GeometryModifier * pMod = (GeometryModifier*) m_pModsList->getFirst(0);
    while (pMod != NULL)
    {
        if (pMod->isActive() && pMod->isRunning())
            pMod->update(delta);
        pMod = (GeometryModifier*) m_pModsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : getModifier
// -----------------------------------------------------------------
GeometryModifier * Geometry::getModifier(u16 uModId)
{
    GeometryModifier * pMod = (GeometryModifier*) m_pModsList->getFirst(0);
    while (pMod != NULL)
    {
        if (pMod->getId() == uModId)
            return pMod;
        pMod = (GeometryModifier*) m_pModsList->getNext(0);
    }
    return NULL;
}

// -----------------------------------------------------------------
// Name : bindModifier
// -----------------------------------------------------------------
void Geometry::bindModifier(GeometryModifier * pMod)
{
    m_pModsList->addFirst(pMod);
}

// -----------------------------------------------------------------
// Name : unbindModifier
// -----------------------------------------------------------------
void Geometry::unbindModifier(u16 uModId, bool bAll, bool bDelete)
{
    GeometryModifier * pMod = (GeometryModifier*) m_pModsList->getFirst(0);
    while (pMod != NULL)
    {
        if (pMod->getId() == uModId)
        {
            m_pModsList->deleteCurrent(0, false, !bDelete);
            if (!bAll)
                return;
        }
        pMod = (GeometryModifier*) m_pModsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : doModTransforms
// -----------------------------------------------------------------
void Geometry::doModTransforms(F_RGBA * pColor)
{
    GeometryModifier * pMod = (GeometryModifier*) m_pModsList->getFirst(0);
    while (pMod != NULL)
    {
        if (pMod->isActive())
            pMod->doTransforms(pColor);
        pMod = (GeometryModifier*) m_pModsList->getNext(0);
    }
}

// -----------------------------------------------------------------
// Name : bindShader
// -----------------------------------------------------------------
bool Geometry::bindShader(const char * sVertexShader, const char * sPixelShader)
{
    m_bShaderLoaded = false;
    m_bShaderEnabled = false;
    if (m_uShaderProgram != 0)
    {
        glDeleteProgram(m_uShaderProgram);
        m_uShaderProgram = 0;
    }
    if (m_uVxShader != 0)
    {
        glDeleteShader(m_uVxShader);
        m_uVxShader = 0;
    }
    if (m_uPxShader != 0)
    {
        glDeleteShader(m_uPxShader);
        m_uPxShader = 0;
    }
    if (sVertexShader != NULL)
    {
        if (!m_pDisplay->loadShader(&m_uVxShader, GL_VERTEX_SHADER, sVertexShader))
            return false;
    }
    if (sPixelShader != NULL)
    {
        if (!m_pDisplay->loadShader(&m_uPxShader, GL_FRAGMENT_SHADER, sPixelShader))
            return false;
    }
    m_bShaderLoaded = m_pDisplay->linkShaders(&m_uShaderProgram, m_uVxShader, m_uPxShader);
    return m_bShaderLoaded;
}

// -----------------------------------------------------------------
// Name : activateShader
// -----------------------------------------------------------------
void Geometry::activateShader()
{
    if (m_bShaderLoaded)
        m_bShaderEnabled = true;
}

// -----------------------------------------------------------------
// Name : deactivateShader
// -----------------------------------------------------------------
void Geometry::deactivateShader()
{
    m_bShaderEnabled = false;
}

// -----------------------------------------------------------------
// Name : registerShaderVariable
// -----------------------------------------------------------------
int Geometry::registerShaderVariable(const char * name)
{
    if (m_bShaderLoaded)
        return (int) glGetUniformLocation(m_uShaderProgram, name);
    return 0;
}

// -----------------------------------------------------------------
// Name : beginSetShaderVariables
// -----------------------------------------------------------------
void Geometry::beginSetShaderVariables()
{
    if (m_bShaderLoaded)
        glUseProgram(m_uShaderProgram);
}

// -----------------------------------------------------------------
// Name : endSetShaderVariables
// -----------------------------------------------------------------
void Geometry::endSetShaderVariables()
{
    if (m_bShaderLoaded)
        glUseProgram(0);
}

// -----------------------------------------------------------------
// Name : setShaderInt
// -----------------------------------------------------------------
void Geometry::setShaderInt(int iVarId, int val)
{
    if (m_bShaderLoaded)
        glUniform1i((GLint) iVarId, val);
}

// -----------------------------------------------------------------
// Name : setShaderFloat
// -----------------------------------------------------------------
void Geometry::setShaderFloat(int iVarId, float val)
{
    if (m_bShaderLoaded)
        glUniform1f((GLint) iVarId, val);
}

// -----------------------------------------------------------------
// Name : setShaderIvec2
// -----------------------------------------------------------------
void Geometry::setShaderIvec2(int iVarId, CoordsScreen val)
{
    if (m_bShaderLoaded)
        glUniform2i((GLint) iVarId, val.x, val.y);
}

// -----------------------------------------------------------------
// Name : setShaderVec3
// -----------------------------------------------------------------
void Geometry::setShaderVec3(int iVarId, Coords3D val)
{
    if (m_bShaderLoaded)
        glUniform3f((GLint) iVarId, val.x, val.y, val.z);
}
