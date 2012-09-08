#include "GeometryModifier.h"

// -----------------------------------------------------------------
// Name : GeometryModifier
//  Constructor
// -----------------------------------------------------------------
GeometryModifier::GeometryModifier(u16 uModId)
{
    m_uModId = uModId;
    m_bActive = m_bRunning = true;
}

// -----------------------------------------------------------------
// Name : ~GeometryModifier
//  Destructor
// -----------------------------------------------------------------
GeometryModifier::~GeometryModifier()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy GeometryModifier\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy GeometryModifier\n");
#endif
}
