// -----------------------------------------------------------------
// SortInterface
// -----------------------------------------------------------------

#include "SortInterface.h"


// -----------------------------------------------------------------
// Name : SortInterface
//  Constructor
// -----------------------------------------------------------------
SortInterface::SortInterface()
{
}

// -----------------------------------------------------------------
// Name : ~SortInterface
//  Destructor
// -----------------------------------------------------------------
SortInterface::~SortInterface()
{
}

// -----------------------------------------------------------------
// Name : sort
// -----------------------------------------------------------------
void SortInterface::sort(ObjectList * pList)
{
    if (pList == NULL || pList->size <= 1)
        return;
    qsort(pList, pList->first, 0, pList->last, pList->size - 1);
}

// -----------------------------------------------------------------
// Name : qsort (private)
// -----------------------------------------------------------------
void SortInterface::qsort(ObjectList * pList, ListNode * first, int fidx, ListNode * last, int lidx)
{
    if (fidx < lidx)
    {
        ListNode * qnode;
        int q = qpart(pList, first, fidx, last, lidx, &qnode);
        qsort(pList, first, fidx, qnode, q);
        qsort(pList, qnode->next, q+1, last, lidx);
    }
}

// -----------------------------------------------------------------
// Name : qpart (private)
// -----------------------------------------------------------------
int SortInterface::qpart(ObjectList * pList, ListNode * first, int fidx, ListNode * last, int lidx, ListNode ** qnode)
{
    BaseObject * pivot = first->obj;
    int i = fidx;
    int j = lidx;
    ListNode * inode = first;
    ListNode * jnode = last;
    BaseObject * tmp;
    while (true)
    {
        while (sortCompare(jnode->obj, pivot))
        {
            j--;
            jnode = jnode->prev;
        }
        while (sortCompare(pivot, inode->obj))
        {
            i++;
            inode = inode->next;
        }
        if (i < j)
        {
            tmp = inode->obj;
            inode->obj = jnode->obj;
            jnode->obj = tmp;
            //tmp = inode->prev;
            //inode->prev = jnode->prev;
            //jnode->prev = tmp;
            //tmp = inode->next;
            //inode->next = jnode->next;
            //jnode->next = tmp;
            //if (inode->next != NULL)
            //  inode->next->prev = inode;
            //else
            //  pList->last = inode;
            //if (inode->prev != NULL)
            //  inode->prev->next = inode;
            //else
            //  pList->first = inode;
            //if (jnode->next != NULL)
            //  jnode->next->prev = jnode;
            //else
            //  pList->last = jnode;
            //if (jnode->prev != NULL)
            //  jnode->prev->next = jnode;
            //else
            //  pList->first = jnode;
        }
        else
        {
            *qnode = jnode;
            return j;
        }
        j--;
        jnode = jnode->prev;
        i++;
        inode = inode->next;
    }
    *qnode = jnode;
    return j;
}
