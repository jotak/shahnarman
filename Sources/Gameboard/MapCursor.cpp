// -----------------------------------------------------------------
// MAP CURSOR
// -----------------------------------------------------------------
#include "MapCursor.h"
//#include "../Geometries/ModProgressiveScaling.h"
#include "../Geometries/ModProgressiveRotate.h"

// -----------------------------------------------------------------
// Name : MapCursor
// -----------------------------------------------------------------
MapCursor::MapCursor()
{
  m_pGeometry = NULL;
  m_Color = F_RGBA_NULL;
  m_bEnabled = false;
}

// -----------------------------------------------------------------
// Name : ~MapCursor
// -----------------------------------------------------------------
MapCursor::~MapCursor()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy MapCursor\n");
#endif
  FREE(m_pGeometry);
#ifdef DBG_VERBOSE1
  printf("End destroy MapCursor\n");
#endif
}

// -----------------------------------------------------------------
// Name : init
// -----------------------------------------------------------------
void MapCursor::init(DisplayEngine * pDisplay)
{
  QuadData quad(0.0f, 1.0f, 0.0f, 1.0f, "selection_circle", pDisplay);
  m_pGeometry = new GeometryQuads(&quad, VB_Static);
//  ModProgressiveScaling * pMod = new ModProgressiveScaling(0, 0.8f, 1.0f, 1.0f, -0.3f, 0.5f, 0.5f, PSB_ForthAndBack);
//  m_pGeometry->bindModifier(pMod);
  ModProgressiveRotate * pRotMod = new ModProgressiveRotate(0, 100, 0.5f, 0.5f);
  m_pGeometry->bindModifier(pRotMod);
  m_bEnabled = false;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
void MapCursor::update(double delta)
{
  if (m_bEnabled)
  {
    PhysicalObject::update(delta);
    m_pGeometry->update(delta);
  }
}

// -----------------------------------------------------------------
// Name : display
// -----------------------------------------------------------------
void MapCursor::display()
{
  if (m_bEnabled)
    m_pGeometry->display(m_3DPosition, m_Color);
}

// -----------------------------------------------------------------
// Name : moveTo
// -----------------------------------------------------------------
void MapCursor::moveTo(CoordsMap mapPos)
{
  PhysicalObject::moveTo(m_pGeometry->getDisplay()->get3DCoords(mapPos, BOARDPLANE));
//  PhysicalObject::moveTo(m_pGeometry->getDisplay()->get3DCoords(mapPos, FARPLANE-EPSILON));
}
