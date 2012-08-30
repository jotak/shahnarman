// -----------------------------------------------------------------
// StencilGeometry
// -----------------------------------------------------------------
#include "StencilGeometry.h"

// -----------------------------------------------------------------
// Name : StencilGeometry
//  Constructor
// -----------------------------------------------------------------
StencilGeometry::StencilGeometry(int iWidth, int iHeight, VBType type, DisplayEngine * pDisplay) : Geometry(type, pDisplay)
{
  m_iWidth = iWidth;
  m_iHeight = iHeight;
  GLenum glType;
  switch (type)
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
  Coords3D d3Dim = pDisplay->get3DCoords(CoordsScreen(iWidth, iHeight), DMS_2D);
  Vertex vertices[4];
  vertices[0].set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[1].set(d3Dim.x, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[2].set(d3Dim.x, d3Dim.y, 0.0f, 1.0f, 1.0f);
  vertices[3].set(0.0f, d3Dim.y, 0.0f, 0.0f, 1.0f);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), vertices, glType);
}

// -----------------------------------------------------------------
// Name : ~StencilGeometry
//  Destructor
// -----------------------------------------------------------------
StencilGeometry::~StencilGeometry()
{
  glDeleteBuffers(1, &m_VboId);
}

// -----------------------------------------------------------------
// Name : fillStencil
// -----------------------------------------------------------------
void StencilGeometry::fillStencil(CoordsScreen position, bool bAdd)
{
  // Stencil operations
  int iPreviousState = m_pDisplay->setStencilState(bAdd ? 1 : 3);

  // Draw in stencil
  Coords3D d3Coords = m_pDisplay->get3DCoords(position, DMS_2D);
  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  glVertexPointer(3, GL_FLOAT, sizeof(Vertex), BUFFER_OFFSET(2 * sizeof(GLfloat)));
  glEnableClientState(GL_VERTEX_ARRAY);

  glPushMatrix();
  {
    glTranslatef(d3Coords.x, d3Coords.y, -d3Coords.z);
    glDrawArrays(GL_QUADS, 0, 4);
  }
  glPopMatrix();
  glDisableClientState(GL_VERTEX_ARRAY);

  // Restore previous state
  m_pDisplay->setStencilState(iPreviousState);
}

// -----------------------------------------------------------------
// Name : resize
// -----------------------------------------------------------------
void StencilGeometry::resize(int iWidth, int iHeight)
{
  m_iWidth = iWidth;
  m_iHeight = iHeight;
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
  if (glIsBuffer(m_VboId))
    glDeleteBuffers(1, &m_VboId);
  glGenBuffers(1, &m_VboId);
  glBindBuffer(GL_ARRAY_BUFFER, m_VboId);
  Coords3D d3Dim = m_pDisplay->get3DCoords(CoordsScreen(iWidth, iHeight), DMS_2D);
  Vertex vertices[4];
  vertices[0].set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[1].set(d3Dim.x, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[2].set(d3Dim.x, d3Dim.y, 0.0f, 1.0f, 1.0f);
  vertices[3].set(0.0f, d3Dim.y, 0.0f, 0.0f, 1.0f);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), vertices, glType);
}

// -----------------------------------------------------------------
// Name : reload
// -----------------------------------------------------------------
void StencilGeometry::reload()
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
  Coords3D d3Dim = m_pDisplay->get3DCoords(CoordsScreen(m_iWidth, m_iHeight), DMS_2D);
  Vertex vertices[4];
  vertices[0].set(0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  vertices[1].set(d3Dim.x, 0.0f, 0.0f, 1.0f, 0.0f);
  vertices[2].set(d3Dim.x, d3Dim.y, 0.0f, 1.0f, 1.0f);
  vertices[3].set(0.0f, d3Dim.y, 0.0f, 0.0f, 1.0f);
  glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(Vertex), vertices, glType);
}
