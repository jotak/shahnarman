#ifndef _SORT_INTERFACE_H
#define _SORT_INTERFACE_H

#include "ObjectList.h"

class SortInterface
{
public:
    SortInterface();
    ~SortInterface();

protected:
    virtual bool sortCompare(BaseObject * A, BaseObject * B) = 0;
    void sort(ObjectList * pList);

private:
    void qsort(ObjectList * pList, ListNode * first, int fidx, ListNode * last, int lidx);
    int qpart(ObjectList * pList, ListNode * first, int fidx, ListNode * last, int lidx, ListNode ** qnode);
};

#endif
