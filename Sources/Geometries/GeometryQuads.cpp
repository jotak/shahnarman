// -----------------------------------------------------------------
// GeometryQuads
// -----------------------------------------------------------------
#include "GeometryQuads.h"

// -----------------------------------------------------------------
// Name : GeometryQuads
//  Constructor
// -----------------------------------------------------------------
GeometryQuads::GeometryQuads(int nQuads, QuadData ** pAllQuads, VBType type) : Geometry(type, pAllQuads[0]->m_pDisplay)
{
  m_VboId = 0;
  m_iNbQuads = 0;
  modify(nQuads, pAllQuads);
}

// -----------------------------------------------------------------
// Name : GeometryQuads
//  Constructor
// -----------------------------------------------------------------
GeometryQuads::GeometryQuads(QuadData * pQuad, VBType type) : Geometry(type, pQuad->m_pDisplay)
{
  m_VboId = 0;
  m_iNbQuads = 0;
  modify(1, &pQuad);
}

// -----------------------------------------------------------------
// Name : GeometryQuads
//  Constructor
// -----------------------------------------------------------------
GeometryQuads::GeometryQuads(VBType type, DisplayEngine * pDisplay) : Geometry(type, pDisplay)
{
  m_VboId = 0;
  m_iNbQuads = 0;
}

// -----------------------------------------------------------------
// Name : ~GeometryQuads
//  Destructor
// -----------------------------------------------------------------
GeometryQuads::~GeometryQuads()
{
  if (m_VboId > 0)
    glDeleteBuffers(1, &m_VboId);
  if (m_iNbQuads > 0)
  {
    for (int i = 0; i < m_iNbQuads; i++)
      delete m_pAllQuads[i];
    delete[] m_pAllQuads;
  }
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryQuads::display(CoordsScreen pos, F_RGBA color)
{
  if (m_iNbQuads == 0)
    return;

  Coords3D d3Coords = m_pDisplay->get3DCoords(pos, DMS_2D);
  display(d3Coords, color);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryQuads::display(Coords3D pos, F_RGBA color)
{
  if (m_iNbQuads == 0)
    return;

  glEnable(GL_TEXTURE_2D);
  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(2 * sizeof(GLfloat)));
  if (F_RGBA_ISNULL(color))
    color = rgba(1, 1, 1, 1);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glPushMatrix();
  glTranslatef(pos.x, pos.y, -pos.z);
  doModTransforms(&color);

  if (m_bShaderEnabled)
    glUseProgram(m_uShaderProgram);

  glColor4f(color.r, color.g, color.b, color.a);
  for (int i = 0; i < m_iNbQuads; i++)
  {
    glBindTexture(GL_TEXTURE_2D, m_pDisplay->getTextureEngine()->getTexture(m_pAllQuads[i]->m_iTex)->m_uGlId);
    glDrawArrays(GL_QUADS, 4*i, 4);
  }

  if (m_bShaderEnabled)
    glUseProgram(0);

  glPopMatrix();
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_TEXTURE_2D);
}

// -----------------------------------------------------------------
// Name : modify
// -----------------------------------------------------------------
void GeometryQuads::modify(int nQuads, QuadData ** pAllQuads)
{
  GLenum glType;
  switch (m_Type)
  {
  case VB_Static: glType = GL_STATIC_DRAW;
    break;
  case VB_Dynamic: glType = GL_DYNAMIC_DRAW;
    break;
  case VB_Stream:
  default: glType = GL_STREAM_DRAW;
    break;
  }
  if (m_iNbQuads > 0)
  {
    for (int i = 0; i < m_iNbQuads; i++)
      delete m_pAllQuads[i];
    delete[] m_pAllQuads;
  }
  if (!glIsBuffer(m_VboId))
    glGenBuffers(1, &m_VboId);
  else if (m_iNbQuads != nQuads)
  {
    glDeleteBuffers(1, &m_VboId);
    glGenBuffers(1, &m_VboId);
  }
  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  m_iNbQuads = nQuads;
  if (m_iNbQuads == 0)
  {
    m_pAllQuads = NULL;
    return;
  }

  m_pAllQuads = new QuadData*[m_iNbQuads];
  Vertex * vertices = new Vertex[m_iNbQuads*4];
  for (int i = 0; i < m_iNbQuads; i++)
  {
    // QuadData
    m_pAllQuads[i] = pAllQuads[i]->clone();

    // Vertex
    vertices[4*i+0].set(pAllQuads[i]->m_fXStart, pAllQuads[i]->m_fYStart, 0.0f, pAllQuads[i]->m_fUStart, pAllQuads[i]->m_fVStart);
    vertices[4*i+1].set(pAllQuads[i]->m_fXEnd  , pAllQuads[i]->m_fYStart, 0.0f, pAllQuads[i]->m_fUEnd  , pAllQuads[i]->m_fVStart);
    vertices[4*i+2].set(pAllQuads[i]->m_fXEnd  , pAllQuads[i]->m_fYEnd  , 0.0f, pAllQuads[i]->m_fUEnd  , pAllQuads[i]->m_fVEnd  );
    vertices[4*i+3].set(pAllQuads[i]->m_fXStart, pAllQuads[i]->m_fYEnd  , 0.0f, pAllQuads[i]->m_fUStart, pAllQuads[i]->m_fVEnd  );
  }
  glBufferData(GL_ARRAY_BUFFER, m_iNbQuads * 4 * sizeof(Vertex), vertices, glType);
  delete[] vertices;
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
int GeometryQuads::getTexture(int iQuad)
{
  return m_pAllQuads[iQuad]->m_iTex;
}

// -----------------------------------------------------------------
// Name : setTexture
// -----------------------------------------------------------------
void GeometryQuads::setTexture(int iTexId, int iQuad)
{
  m_pAllQuads[iQuad]->m_iTex = iTexId;
}

// -----------------------------------------------------------------
// Name : reload
// -----------------------------------------------------------------
void GeometryQuads::reload()
{
  GLenum glType;
  switch (m_Type)
  {
  case VB_Static: glType = GL_STATIC_DRAW;
    break;
  case VB_Dynamic: glType = GL_DYNAMIC_DRAW;
    break;
  case VB_Stream:
  default: glType = GL_STREAM_DRAW;
    break;
  }
  glGenBuffers(1, &m_VboId);
  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  if (m_iNbQuads == 0)
    return;

  Vertex * vertices = new Vertex[m_iNbQuads*4];
  for (int i = 0; i < m_iNbQuads; i++)
  {
    // Vertex
    vertices[4*i+0].set(m_pAllQuads[i]->m_fXStart, m_pAllQuads[i]->m_fYStart, 0.0f, m_pAllQuads[i]->m_fUStart, m_pAllQuads[i]->m_fVStart);
    vertices[4*i+1].set(m_pAllQuads[i]->m_fXEnd  , m_pAllQuads[i]->m_fYStart, 0.0f, m_pAllQuads[i]->m_fUEnd  , m_pAllQuads[i]->m_fVStart);
    vertices[4*i+2].set(m_pAllQuads[i]->m_fXEnd  , m_pAllQuads[i]->m_fYEnd  , 0.0f, m_pAllQuads[i]->m_fUEnd  , m_pAllQuads[i]->m_fVEnd  );
    vertices[4*i+3].set(m_pAllQuads[i]->m_fXStart, m_pAllQuads[i]->m_fYEnd  , 0.0f, m_pAllQuads[i]->m_fUStart, m_pAllQuads[i]->m_fVEnd  );
  }
  glBufferData(GL_ARRAY_BUFFER, m_iNbQuads * 4 * sizeof(Vertex), vertices, glType);
  delete[] vertices;
}


// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(int xstart, int xend, int ystart, int yend, const wchar_t * texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  Coords3D d3start = pDisplay->get3DCoords(CoordsScreen(xstart, ystart), DMS_2D);
  Coords3D d3end = pDisplay->get3DCoords(CoordsScreen(xend, yend), DMS_2D);
  m_fXStart = d3start.x;
  m_fYStart = d3start.y;
  m_fXEnd = d3end.x;
  m_fYEnd = d3end.y;

  m_iTex = pDisplay->getTextureEngine()->loadTexture(texture);
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(m_iTex);
  m_fUStart = pTex->m_fU0;
  m_fUEnd = pTex->m_fU1;
  m_fVStart = pTex->m_fV0;
  m_fVEnd = pTex->m_fV1;
}

// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(int xstart, int xend, int ystart, int yend, int texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  Coords3D d3start = pDisplay->get3DCoords(CoordsScreen(xstart, ystart), DMS_2D);
  Coords3D d3end = pDisplay->get3DCoords(CoordsScreen(xend, yend), DMS_2D);
  m_fXStart = d3start.x;
  m_fYStart = d3start.y;
  m_fXEnd = d3end.x;
  m_fYEnd = d3end.y;

  m_iTex = texture;
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(m_iTex);
  m_fUStart = pTex->m_fU0;
  m_fUEnd = pTex->m_fU1;
  m_fVStart = pTex->m_fV0;
  m_fVEnd = pTex->m_fV1;
}

// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(int xstart, int xend, int ystart, int yend, int ustart, int uend, int vstart, int vend, const wchar_t * texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  Coords3D d3start = pDisplay->get3DCoords(CoordsScreen(xstart, ystart), DMS_2D);
  Coords3D d3end = pDisplay->get3DCoords(CoordsScreen(xend, yend), DMS_2D);
  m_fXStart = d3start.x;
  m_fYStart = d3start.y;
  m_fXEnd = d3end.x;
  m_fYEnd = d3end.y;

  m_iTex = pDisplay->getTextureEngine()->loadTexture(texture, false, ustart, uend, vstart, vend);
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(m_iTex);
  m_fUStart = pTex->m_fU0;
  m_fUEnd = pTex->m_fU1;
  m_fVStart = pTex->m_fV0;
  m_fVEnd = pTex->m_fV1;
}

// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(float xstart, float xend, float ystart, float yend, const wchar_t * texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  m_fXStart = xstart;
  m_fYStart = ystart;
  m_fXEnd = xend;
  m_fYEnd = yend;

  m_iTex = pDisplay->getTextureEngine()->loadTexture(texture, true);
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(m_iTex);
  m_fUStart = pTex->m_fU0;
  m_fUEnd = pTex->m_fU1;
  m_fVStart = pTex->m_fV0;
  m_fVEnd = pTex->m_fV1;
}

// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(float xstart, float xend, float ystart, float yend, int ustart, int uend, int vstart, int vend, const wchar_t * texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  m_fXStart = xstart;
  m_fYStart = ystart;
  m_fXEnd = xend;
  m_fYEnd = yend;

  m_iTex = pDisplay->getTextureEngine()->loadTexture(texture, true, ustart, uend, vstart, vend);
  Texture * pTex = pDisplay->getTextureEngine()->getTexture(m_iTex);
  m_fUStart = pTex->m_fU0;
  m_fUEnd = pTex->m_fU1;
  m_fVStart = pTex->m_fV0;
  m_fVEnd = pTex->m_fV1;
}

// -----------------------------------------------------------------
// Name : QuadData
//  Constructor
// -----------------------------------------------------------------
QuadData::QuadData(float xstart, float xend, float ystart, float yend, float ustart, float uend, float vstart, float vend, int texture, DisplayEngine * pDisplay)
{
  m_pDisplay = pDisplay;
  m_fXStart = xstart;
  m_fYStart = ystart;
  m_fXEnd = xend;
  m_fYEnd = yend;
  m_iTex = texture;
  m_fUStart = ustart;
  m_fUEnd = uend;
  m_fVStart = vstart;
  m_fVEnd = vend;
}

// -----------------------------------------------------------------
// Name : clone
// -----------------------------------------------------------------
QuadData * QuadData::clone()
{
  return new QuadData(m_fXStart, m_fXEnd, m_fYStart, m_fYEnd, m_fUStart, m_fUEnd, m_fVStart, m_fVEnd, m_iTex, m_pDisplay);
}

// -----------------------------------------------------------------
// Name : releaseQuads
//  Static function
// -----------------------------------------------------------------
void QuadData::releaseQuads(int nQuads, QuadData ** pQuads)
{
  for (int i = 0; i < nQuads; i++)
    delete pQuads[i];
  delete[] pQuads;
}
