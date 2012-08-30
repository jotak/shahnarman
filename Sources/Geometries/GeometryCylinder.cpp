// -----------------------------------------------------------------
// GeometryCylinder
// -----------------------------------------------------------------
#include "GeometryCylinder.h"

// -----------------------------------------------------------------
// Name : GeometryCylinder
//  Constructor
// -----------------------------------------------------------------
GeometryCylinder::GeometryCylinder(float fDiameter, float fHeight, u16 uSlices, int iTex, VBType type, DisplayEngine * pDisplay) : Geometry(type, pDisplay)
{
  m_VboId = m_BordersVboId = 0;
  m_pQuadric = gluNewQuadric();
  gluQuadricTexture(m_pQuadric, GL_TRUE);
  m_iTopTex = pDisplay->getTextureEngine()->loadTexture(L"pastille_top", true);
  m_iRoundTex = pDisplay->getTextureEngine()->loadTexture(L"pastille_tour", true);
  modify(fDiameter, fHeight, uSlices, iTex);
}

// -----------------------------------------------------------------
// Name : ~GeometryCylinder
//  Destructor
// -----------------------------------------------------------------
GeometryCylinder::~GeometryCylinder()
{
  if (m_VboId > 0)
    glDeleteBuffers(1, &m_VboId);
  if (m_BordersVboId > 0)
    glDeleteBuffers(1, &m_BordersVboId);
  gluDeleteQuadric(m_pQuadric);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryCylinder::display(CoordsScreen pos, F_RGBA color, F_RGBA borderColor)
{
  Coords3D d3Coords = m_pDisplay->get3DCoords(pos, DMS_2D);
  display(d3Coords, color, borderColor);
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void GeometryCylinder::display(Coords3D pos, F_RGBA color, F_RGBA borderColor)
{
  if (F_RGBA_ISNULL(color))
    color = rgba(1, 1, 1, 1);
  if (F_RGBA_ISNULL(borderColor))
    borderColor = rgba(1, 1, 1, 1);
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  glPushMatrix();
  glTranslatef(pos.x, pos.y, -pos.z);
  doModTransforms(&color);
  glTranslatef(m_fDiameter / 2.0f, m_fDiameter / 2.0f, 0);

  if (m_bShaderEnabled)
    glUseProgram(m_uShaderProgram);

  glEnable(GL_TEXTURE_2D);

  borderColor = F_RGBA_MULTIPLY(borderColor, color);
  glColor4f(borderColor.r, borderColor.g, borderColor.b, borderColor.a);
  glBindBuffer(GL_ARRAY_BUFFER, m_BordersVboId);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(2 * sizeof(GLfloat)));
  glBindTexture(GL_TEXTURE_2D, m_pDisplay->getTextureEngine()->getTexture(m_iRoundTex)->m_uGlId);
  glDrawArrays(GL_QUAD_STRIP, 0, 2*m_uSlices+4);

  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(0));
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(2 * sizeof(GLfloat)));
  glBindTexture(GL_TEXTURE_2D, m_pDisplay->getTextureEngine()->getTexture(m_iTopTex)->m_uGlId);
  glTranslatef(0, 0, m_fHeight);
  glDrawArrays(GL_TRIANGLE_FAN, 0, m_uSlices+2);
  glColor4f(color.r, color.g, color.b, color.a);
  glBindTexture(GL_TEXTURE_2D, m_pDisplay->getTextureEngine()->getTexture(m_iTex)->m_uGlId);
  glDrawArrays(GL_TRIANGLE_FAN, 0, m_uSlices+2);
//  glTranslatef(0, 0, -m_fHeight);

  glPopMatrix();
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

// -----------------------------------------------------------------
// Name : modify
// -----------------------------------------------------------------
void GeometryCylinder::modify(float fDiameter, float fHeight, u16 uSlices, int iTex)
{
  m_fDiameter = fDiameter;
  m_fHeight = fHeight;
  m_uSlices = uSlices;
  m_iTex = iTex;
  reload();
}

// -----------------------------------------------------------------
// Name : getTexture
// -----------------------------------------------------------------
int GeometryCylinder::getTexture()
{
  return m_iTex;
}

// -----------------------------------------------------------------
// Name : setTexture
// -----------------------------------------------------------------
void GeometryCylinder::setTexture(int iTexId)
{
  m_iTex = iTexId;
}

// -----------------------------------------------------------------
// Name : reload
// -----------------------------------------------------------------
void GeometryCylinder::reload()
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

  Vertex * pBase = new Vertex[m_uSlices+2];  // triangle fan
  float fAlpha = 2.0f*PI/(float)m_uSlices;
  float fRadius = m_fDiameter / 2.0f;
  // Set origin vertex for triangle fan
  pBase[0].set(0, 0, 0, 0.5f, 0.5f);
  // Next vertice
  float fCos, fSin;
  for (int i = 0; i < m_uSlices; i++)
  {
    fCos = cos(i*fAlpha);
    fSin = sin(i*fAlpha);
    pBase[i+1].set(fRadius*fCos, fRadius*fSin, 0.0f, (1.0f + fCos) / 2.0f, (1.0f + fSin) / 2.0f);
  }
  // Close shape
  pBase[m_uSlices+1].set(fRadius, 0.0f, 0.0f, 1.0f, 0.5f);

  glBufferData(GL_ARRAY_BUFFER, (m_uSlices+2) * sizeof(Vertex), pBase, glType);

  // Now do quad strip for borders
  glGenBuffers(1, &m_BordersVboId);
  glBindBuffer(GL_ARRAY_BUFFER, m_BordersVboId);
  Vertex * pBorders = new Vertex[2*m_uSlices+4];  // quad strip

  // Set the 2 first vertice for quad strip
  float u = 0.0f;
  pBorders[0].set(fRadius, 0, 0, u, 1);
  pBorders[1].set(fRadius, 0, m_fHeight, u, 0);
  // Next vertice
  for (int i = 0; i < m_uSlices; i++)
  {
    u = 1.0f - u;
    pBorders[2*i+2].set(fRadius*cos(i*fAlpha), fRadius*sin(i*fAlpha), 0.0f, u, 1);
    pBorders[2*i+3].set(fRadius*cos(i*fAlpha), fRadius*sin(i*fAlpha), m_fHeight, u, 0);
  }
  // Close shape
  u = 1.0f - u;
  pBorders[2*m_uSlices+2].set(fRadius, 0, 0, u, 1);
  pBorders[2*m_uSlices+3].set(fRadius, 0, m_fHeight, u, 0);

  glBufferData(GL_ARRAY_BUFFER, (2*m_uSlices+4) * sizeof(Vertex), pBorders, glType);

  delete[] pBase;
  delete[] pBorders;
}
