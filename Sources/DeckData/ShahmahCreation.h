#ifndef _SHAHMAH_CREATION_H
#define _SHAHMAH_CREATION_H

#include "../Common/ObjectList.h"

class ShahmahCreation : public BaseObject
{
public:
    ShahmahCreation();
    ~ShahmahCreation();

    ObjectList * m_pSkills;
    ObjectList * m_pPeoples;
    ObjectList * m_pImages;
};

#endif
