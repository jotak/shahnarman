#include "Movement.h"

// -----------------------------------------------------------------
// Name : Movement
//  Constructor
// -----------------------------------------------------------------
Movement::Movement(u16 uMoveId)
{
  m_uMoveId = uMoveId;
  m_bActive = true;
}

// -----------------------------------------------------------------
// Name : ~Movement
//  Destructor
// -----------------------------------------------------------------
Movement::~Movement()
{
#ifdef DBG_VERBOSE1
  printf("Begin destroy Movement\n");
#endif
#ifdef DBG_VERBOSE1
  printf("End destroy Movement\n");
#endif
}
