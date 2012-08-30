#ifndef _STACKGROUP_INTERFACE_H
#define _STACKGROUP_INTERFACE_H

#include "../Common/ObjectList.h"

class StackGroupInterface
{
public:
  virtual MetaObjectList * resetCurrentGroup(BaseObject * pItem, MetaObjectList * pCurrentGroup) = 0;
  virtual MetaObjectList * onClickOnGroupItem(BaseObject * pItem, bool bClickState, MetaObjectList * pCurrentGroup) = 0;
};

#endif
