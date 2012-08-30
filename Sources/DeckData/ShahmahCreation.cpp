#include "ShahmahCreation.h"

// -----------------------------------------------------------------
// Name : ShahmahCreation
//  Constructor
// -----------------------------------------------------------------
ShahmahCreation::ShahmahCreation()
{
  m_pImages = new ObjectList(true); // will contain "new StringObject"
  m_pSkills = new ObjectList(true); // will contain "new Skill"
  m_pPeoples = new ObjectList(false); // will contain "findEthnicity"
}

// -----------------------------------------------------------------
// Name : ~ShahmahCreation
//  Destructor
// -----------------------------------------------------------------
ShahmahCreation::~ShahmahCreation()
{
  delete m_pImages;
  delete m_pSkills;
  delete m_pPeoples;
}
