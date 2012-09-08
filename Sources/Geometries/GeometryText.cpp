// -----------------------------------------------------------------
// GeometryText
// -----------------------------------------------------------------
#include "GeometryText.h"

// -----------------------------------------------------------------
// Name : GeometryText
//  Constructor
// -----------------------------------------------------------------
GeometryText::GeometryText(const char * sText, int iFontId, VBType type, DisplayEngine * pDisplay) : Geometry(type, pDisplay)
{
    m_VboId = 0;
    m_sText = NULL;
    m_fScale = -1;
    setText(sText, iFontId);
}

// -----------------------------------------------------------------
// Name : GeometryText
//  Constructor
// -----------------------------------------------------------------
GeometryText::GeometryText(const char * sText, int iFontId, float fFontHeight, VBType type, DisplayEngine * pDisplay) : Geometry(type, pDisplay)
{
    m_VboId = 0;
    m_sText = NULL;
    Font * pFont = m_pDisplay->getFontEngine()->getFont(iFontId);
    if (pFont)
    {
        Coords3D p = m_pDisplay->get3DCoords(CoordsScreen(0, pFont->getFontHeight()), DMS_2D);
        m_fScale = fFontHeight / p.y;
    }
    else
        m_fScale = -1;
    setText(sText, iFontId);
}

// -----------------------------------------------------------------
// Name : ~GeometryText
//  Destructor
// -----------------------------------------------------------------
GeometryText::~GeometryText()
{
    if (m_VboId > 0)
        glDeleteBuffers(1, &m_VboId);
    if (m_sText != NULL)
        delete[] m_sText;
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryText::display(CoordsScreen position, F_RGBA color)
{
    Coords3D d3Coords = m_pDisplay->get3DCoords(position, DMS_2D);
    display(d3Coords, color);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryText::display(Coords3D d3Coords, F_RGBA color)
{
    glEnable(GL_TEXTURE_2D);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
    glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(2 * sizeof(GLfloat)));
    if (F_RGBA_ISNULL(color))
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    else
        glColor4f(color.r, color.g, color.b, color.a);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, m_pDisplay->getTextureEngine()->getTexture(m_pDisplay->getFontEngine()->getFont(m_iFontId)->getTextureId())->m_uGlId);

    if (m_bShaderEnabled)
        glUseProgram(m_uShaderProgram);

    glPushMatrix();
    {
        glTranslatef(d3Coords.x, d3Coords.y, -d3Coords.z);
        glDrawArrays(GL_QUADS, 0, 4*m_iNbQuads);
    }

    if (m_bShaderEnabled)
        glUseProgram(0);

    glPopMatrix();
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_TEXTURE_2D);
}

// -----------------------------------------------------------------
// Name : setText
// -----------------------------------------------------------------
void GeometryText::setText(const char * sText, int iFontId)
{
    m_iTextLength = strlen(sText) + 1;
    if (m_sText != NULL)
        delete[] m_sText;
    m_sText = new char[m_iTextLength];
    wsafecpy(m_sText, m_iTextLength, sText);
    m_iFontId = iFontId;
    GLenum glType;
    switch (m_Type)
    {
    case VB_Static:
        glType = GL_STATIC_DRAW;
        break;
    case VB_Dynamic:
        glType = GL_DYNAMIC_DRAW;
        break;
    case VB_Stream:
    default:
        glType = GL_STREAM_DRAW;
        break;
    }
    if (glIsBuffer(m_VboId))
        glDeleteBuffers(1, &m_VboId);
    glGenBuffers(1, &m_VboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    m_pDisplay->setModeState(DMS_2D);

    Font * pFont = m_pDisplay->getFontEngine()->getFont(iFontId);
    float fTexWidth = (float) m_pDisplay->getTextureEngine()->getTexture(pFont->getTextureId())->m_iWidth;
    float fTexHeight = (float) m_pDisplay->getTextureEngine()->getTexture(pFont->getTextureId())->m_iHeight;
    int iXPxl = 0;
    int iYPxl = 0;
    int iLength = (int) strlen(sText);
    m_iNbQuads = 0;
    Vertex * vertices = new Vertex[iLength * 8];  // double number of vertice, because of acutes
    for (int i = 0; i < iLength; i++)
    {
        if (sText[i] == '\n')
        {
            iXPxl = 0;
            iYPxl += pFont->getFontHeight();
            continue;
        }
        CharDescriptor * charDesc = pFont->findCharDescriptor(sText[i]);
        if (charDesc == NULL)
            continue;
        CharDescriptor * acuteDesc = pFont->getLastAcuteDescriptor();

        Coords3D p0 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + charDesc->xoffset, iYPxl + charDesc->yoffset), DMS_2D);
        Coords3D p1 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + charDesc->xoffset + charDesc->width, iYPxl + charDesc->yoffset + charDesc->height), DMS_2D);
        if (m_fScale > 0)
        {
            p0.x *= m_fScale;
            p0.y *= m_fScale;
            p1.x *= m_fScale;
            p1.y *= m_fScale;
        }
        float fSrcLeft = (float)charDesc->x / fTexWidth;
        float fSrcRight = (float)(charDesc->x + charDesc->width) / fTexWidth;
        float fSrcTop = (float)charDesc->y / fTexHeight;
        float fSrcBottom = (float)(charDesc->y + charDesc->height) / fTexHeight;
        vertices[4*m_iNbQuads].set(p0.x, p0.y, 0.0f, fSrcLeft, fSrcTop);
        vertices[4*m_iNbQuads+1].set(p1.x, p0.y, 0.0f, fSrcRight, fSrcTop);
        vertices[4*m_iNbQuads+2].set(p1.x, p1.y, 0.0f, fSrcRight, fSrcBottom);
        vertices[4*m_iNbQuads+3].set(p0.x, p1.y, 0.0f, fSrcLeft, fSrcBottom);
        m_iNbQuads++;

        if (acuteDesc != NULL)  // acute?
        {
            Coords3D p0 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + acuteDesc->xoffset, iYPxl + acuteDesc->yoffset), DMS_2D);
            Coords3D p1 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + acuteDesc->xoffset + acuteDesc->width, iYPxl + acuteDesc->yoffset + acuteDesc->height), DMS_2D);
            float fSrcLeft = (float)acuteDesc->x / fTexWidth;
            float fSrcRight = (float)(acuteDesc->x + acuteDesc->width) / fTexWidth;
            float fSrcTop = (float)acuteDesc->y / fTexHeight;
            float fSrcBottom = (float)(acuteDesc->y + acuteDesc->height) / fTexHeight;
            vertices[4*m_iNbQuads].set(p0.x, p0.y, 0.0f, fSrcLeft, fSrcTop);
            vertices[4*m_iNbQuads+1].set(p1.x, p0.y, 0.0f, fSrcRight, fSrcTop);
            vertices[4*m_iNbQuads+2].set(p1.x, p1.y, 0.0f, fSrcRight, fSrcBottom);
            vertices[4*m_iNbQuads+3].set(p0.x, p1.y, 0.0f, fSrcLeft, fSrcBottom);
            m_iNbQuads++;
        }
        iXPxl += charDesc->xadvance;
    }
    glBufferData(GL_ARRAY_BUFFER, m_iNbQuads * 4 * sizeof(Vertex), vertices, glType);
    delete[] vertices;
}

// -----------------------------------------------------------------
// Name : reload
// -----------------------------------------------------------------
void GeometryText::reload()
{
    GLenum glType;
    switch (m_Type)
    {
    case VB_Static:
        glType = GL_STATIC_DRAW;
        break;
    case VB_Dynamic:
        glType = GL_DYNAMIC_DRAW;
        break;
    case VB_Stream:
    default:
        glType = GL_STREAM_DRAW;
        break;
    }
    glGenBuffers(1, &m_VboId);
    glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
    m_pDisplay->setModeState(DMS_2D);

    Font * pFont = m_pDisplay->getFontEngine()->getFont(m_iFontId);
    float fTexWidth = (float) m_pDisplay->getTextureEngine()->getTexture(pFont->getTextureId())->m_iWidth;
    float fTexHeight = (float) m_pDisplay->getTextureEngine()->getTexture(pFont->getTextureId())->m_iHeight;
    int iXPxl = 0;
    int iYPxl = 0;
    int iLength = (int) strlen(m_sText);
    m_iNbQuads = 0;
    Vertex * vertices = new Vertex[iLength * 8];  // double number of vertice, because of acutes
    for (int i = 0; i < iLength; i++)
    {
        if (m_sText[i] == '\n')
        {
            iXPxl = 0;
            iYPxl += pFont->getFontHeight();
            continue;
        }
        CharDescriptor * charDesc = pFont->findCharDescriptor(m_sText[i]);
        if (charDesc == NULL)
            continue;
        CharDescriptor * acuteDesc = pFont->getLastAcuteDescriptor();

        Coords3D p0 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + charDesc->xoffset, iYPxl + charDesc->yoffset), DMS_2D);
        Coords3D p1 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + charDesc->xoffset + charDesc->width, iYPxl + charDesc->yoffset + charDesc->height), DMS_2D);
        float fSrcLeft = (float)charDesc->x / fTexWidth;
        float fSrcRight = (float)(charDesc->x + charDesc->width) / fTexWidth;
        float fSrcTop = (float)charDesc->y / fTexHeight;
        float fSrcBottom = (float)(charDesc->y + charDesc->height) / fTexHeight;
        vertices[4*m_iNbQuads].set(p0.x, p0.y, 0.0f, fSrcLeft, fSrcTop);
        vertices[4*m_iNbQuads+1].set(p1.x, p0.y, 0.0f, fSrcRight, fSrcTop);
        vertices[4*m_iNbQuads+2].set(p1.x, p1.y, 0.0f, fSrcRight, fSrcBottom);
        vertices[4*m_iNbQuads+3].set(p0.x, p1.y, 0.0f, fSrcLeft, fSrcBottom);
        m_iNbQuads++;

        if (acuteDesc != NULL)  // acute?
        {
            Coords3D p0 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + acuteDesc->xoffset, iYPxl + acuteDesc->yoffset), DMS_2D);
            Coords3D p1 = m_pDisplay->get3DCoords(CoordsScreen(iXPxl + acuteDesc->xoffset + acuteDesc->width, iYPxl + acuteDesc->yoffset + acuteDesc->height), DMS_2D);
            float fSrcLeft = (float)acuteDesc->x / fTexWidth;
            float fSrcRight = (float)(acuteDesc->x + acuteDesc->width) / fTexWidth;
            float fSrcTop = (float)acuteDesc->y / fTexHeight;
            float fSrcBottom = (float)(acuteDesc->y + acuteDesc->height) / fTexHeight;
            vertices[4*m_iNbQuads].set(p0.x, p0.y, 0.0f, fSrcLeft, fSrcTop);
            vertices[4*m_iNbQuads+1].set(p1.x, p0.y, 0.0f, fSrcRight, fSrcTop);
            vertices[4*m_iNbQuads+2].set(p1.x, p1.y, 0.0f, fSrcRight, fSrcBottom);
            vertices[4*m_iNbQuads+3].set(p0.x, p1.y, 0.0f, fSrcLeft, fSrcBottom);
            m_iNbQuads++;
        }
        iXPxl += charDesc->xadvance;
    }
    glBufferData(GL_ARRAY_BUFFER, m_iNbQuads * 4 * sizeof(Vertex), vertices, glType);
    delete[] vertices;
}
