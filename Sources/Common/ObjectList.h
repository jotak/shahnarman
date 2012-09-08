#ifndef _OBJECT_LIST_H
#define _OBJECT_LIST_H

#include "BaseObject.h"

#define MAX_ITERATORS       1024

class ListNode
{
public:
    ListNode(ListNode * prev, ListNode * next, BaseObject * obj, long type);
    ~ListNode();

    BaseObject * obj;
    long type;
    ListNode * prev;
    ListNode * next;
};

class ObjectList
{
public:
    ObjectList(bool bEmbedMainPointer);
    ~ObjectList();

    // overloaded operator
    BaseObject * operator[](unsigned short idx);

    // Add / remove items
    void addFirst(BaseObject * object, long type = -1);
    void addLast(BaseObject * object, long type = -1);
    BaseObject * deleteCurrent(int _it, bool bSetToNext, bool bForceKeepMem = false);
    int deleteObject(BaseObject * obj, bool bFirstOc, bool bForceKeepMem = false);
    void deleteAll(bool bForceKeepMem = false);

    // Iterator functions
    int getIterator();
    void releaseIterator(int _it);
    BaseObject * getFirst(int _it);
    BaseObject * getNext(int _it, bool bLoop = false);
    BaseObject * getLast(int _it);
    BaseObject * getPrev(int _it, bool bLoop = false);
    BaseObject * getCurrent(int _it);
    long getCurrentType(int _it);
    bool goTo(int _it, BaseObject * obj);
    BaseObject * goTo(int _it, unsigned short idx);

    // List item position
    void moveCurrentToBegin(int _it);
    void moveCurrentToEnd(int _it);

    int size;
    ListNode * first;
    ListNode * last;

private:
    bool m_bEmbedMainPointer;
    ListNode * m_pIterators[MAX_ITERATORS];
    bool m_bFreeIterators[MAX_ITERATORS];
};

class MetaObjectList : public ObjectList, public BaseObject
{
public:
    MetaObjectList(bool bEmbedMainPointer) : ObjectList(bEmbedMainPointer) {};
};

#endif
