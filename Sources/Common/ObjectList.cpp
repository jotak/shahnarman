#include "ObjectList.h"

//---------------------------------------------------------------------------------
//*********************************************************************************
//    CLASS ListNode
//*********************************************************************************
//---------------------------------------------------------------------------------

// -----------------------------------------------------------------
// Name : ListNode
//  Constructor
// -----------------------------------------------------------------
ListNode::ListNode(ListNode * prev, ListNode * next, BaseObject * obj, long type)
{
    this->prev = prev;
    this->next = next;
    this->obj = obj;
    this->type = type;
}

// -----------------------------------------------------------------
// Name : ~ListNode
//  Destructor
// -----------------------------------------------------------------
ListNode::~ListNode()
{
#ifdef DBG_VERBOSE2
    printf("Begin destroy ListNode\n");
#endif
    if (obj != NULL)
        delete obj;
#ifdef DBG_VERBOSE2
    printf("End destroy ListNode\n");
#endif
}

//---------------------------------------------------------------------------------
//*********************************************************************************
//    CLASS ObjectList
//*********************************************************************************
//---------------------------------------------------------------------------------

// -----------------------------------------------------------------
// Name : ObjectList
//  Constructor
//  bEmbededMainInstance says if the embeded objects pointers will be the main (or only) pointers to these objects or not. If yes, they will be deleted on node deletion
// -----------------------------------------------------------------
ObjectList::ObjectList(bool bEmbedMainPointer)
{
    first = last = NULL;
    size = 0;
    m_bEmbedMainPointer = bEmbedMainPointer;
    for (int i = 0; i < MAX_ITERATORS; i++)
    {
        m_pIterators[i] = NULL;
        m_bFreeIterators[i] = true;
    }
    m_bFreeIterators[0] = false;    // Set iterator 0 as shared
}

// -----------------------------------------------------------------
// Name : ~ObjectList
//  Destructor
// -----------------------------------------------------------------
ObjectList::~ObjectList()
{
#ifdef DBG_VERBOSE2
    printf("Begin destroy ObjectList\n");
#endif
    deleteAll();
#ifdef DBG_VERBOSE2
    printf("End destroy ObjectList\n");
#endif
}

// -----------------------------------------------------------------
// Name : addFirst
// -----------------------------------------------------------------
void ObjectList::addFirst(BaseObject * object, long type)
{
    ListNode * newNode = new ListNode(NULL, first, object, type);
    if (first != NULL)
        first->prev = newNode;
    first = newNode;
    if (last == NULL)
        last = newNode;
    size++;
}

// -----------------------------------------------------------------
// Name : addLast
// -----------------------------------------------------------------
void ObjectList::addLast(BaseObject * object, long type)
{
    ListNode * newNode = new ListNode(last, NULL, object, type);
    if (last != NULL)
        last->next = newNode;
    last = newNode;
    if (first == NULL)
        first = newNode;
    size++;
}

// -----------------------------------------------------------------
// Name : deleteCurrent
//  return next (or prev) object
// -----------------------------------------------------------------
BaseObject * ObjectList::deleteCurrent(int _it, bool bSetToNext, bool bForceKeepMem)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);
    assert(m_pIterators[_it] != NULL);

    ListNode * nodeToDelete = m_pIterators[_it];
    // Connect prev and next together
    if (nodeToDelete->next != NULL)
        nodeToDelete->next->prev = nodeToDelete->prev;
    if (nodeToDelete->prev != NULL)
        nodeToDelete->prev->next = nodeToDelete->next;

    // If we want to delete "first", set its pointer to next
    if (first == nodeToDelete)
        first = nodeToDelete->next;

    // If we want to delete "last", set its pointer to prev
    if (last == nodeToDelete)
        last = nodeToDelete->prev;

    // If we want to delete a node pointed in iter, make iter points to prev
    for (int i = 0; i < MAX_ITERATORS; i++)
    {
        if (m_pIterators[i] == nodeToDelete)
            m_pIterators[i] = bSetToNext ? nodeToDelete->next : nodeToDelete->prev;
    }

    if (!m_bEmbedMainPointer || bForceKeepMem)
        nodeToDelete->obj = NULL;    // Set to NULL so that cListNode destructor won't try to free embeded object
    delete nodeToDelete;
    size--;

    return getCurrent(_it);
}

// -----------------------------------------------------------------
// Name : deleteObject
// -----------------------------------------------------------------
int ObjectList::deleteObject(BaseObject * obj, bool bFirstOc, bool bForceKeepMem)
{
    ListNode * node = first;
    int nbDeleted = 0;

    while (node != NULL)
    {
        if (node->obj == obj)
        {
            ListNode * delNode = node;
            node = node->next;

            if (delNode->next != NULL)
                delNode->next->prev = delNode->prev;
            if (delNode->prev != NULL)
                delNode->prev->next = delNode->next;
            if (first == delNode)
                first = delNode->next;
            if (last == delNode)
                last = delNode->prev;
            for (int i = 0; i < MAX_ITERATORS; i++)
            {
                if (m_pIterators[i] == delNode)
                    m_pIterators[i] = delNode->prev;
            }

            if (!m_bEmbedMainPointer || bForceKeepMem)
                delNode->obj = NULL;    // Set to NULL so that cListNode destructor won't try to free embeded object
            delete delNode;
            nbDeleted++;
            size--;

            if (bFirstOc)
                return 1;
        }
        else
            node = node->next;
    }
    return nbDeleted;
}

// -----------------------------------------------------------------
// Name : deleteAll
// -----------------------------------------------------------------
void ObjectList::deleteAll(bool bForceKeepMem)
{
    while (first != NULL)
    {
        ListNode * tmp = first;
        first = first->next;
        if (!m_bEmbedMainPointer || bForceKeepMem)
            tmp->obj = NULL;    // Set to NULL so that cListNode destructor won't try to free embeded object
        delete tmp;
    }

    first = last = NULL;
    for (int i = 0; i < MAX_ITERATORS; i++)
        m_pIterators[i] = NULL;
    size = 0;
}

// -----------------------------------------------------------------
// Name : getIterator
// -----------------------------------------------------------------
int ObjectList::getIterator()
{
    for (int i = 0; i < MAX_ITERATORS; i++)
    {
        if (m_bFreeIterators[i])
        {
            m_bFreeIterators[i] = false;
            return i;
        }
    }
    assert(false); // assertion error : you asked for too many iterators at the same time!
    return -1;
}

// -----------------------------------------------------------------
// Name : releaseIterator
// -----------------------------------------------------------------
void ObjectList::releaseIterator(int _it)
{
    if (_it != 0)     // cannot free the shared iterator
        m_bFreeIterators[_it] = true;
}

// -----------------------------------------------------------------
// Name : getFirst
// -----------------------------------------------------------------
BaseObject * ObjectList::getFirst(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    m_pIterators[_it] = first;
    if (m_pIterators[_it] == NULL)
        return NULL;

    return m_pIterators[_it]->obj;
}

// -----------------------------------------------------------------
// Name : getNext
// -----------------------------------------------------------------
BaseObject * ObjectList::getNext(int _it, bool bLoop)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    if (m_pIterators[_it] == NULL) // it could happen on deletion
        return NULL;

    m_pIterators[_it] = m_pIterators[_it]->next;
    if (m_pIterators[_it] == NULL)
    {
        if (bLoop)
            return getFirst(_it);

        return NULL;
    }

    return m_pIterators[_it]->obj;
}

// -----------------------------------------------------------------
// Name : getLast
// -----------------------------------------------------------------
BaseObject * ObjectList::getLast(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    m_pIterators[_it] = last;
    if (m_pIterators[_it] == NULL)
        return NULL;

    return m_pIterators[_it]->obj;
}

// -----------------------------------------------------------------
// Name : getPrev
// -----------------------------------------------------------------
BaseObject * ObjectList::getPrev(int _it, bool bLoop)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    if (m_pIterators[_it] == NULL) // it could happen on deletion
        return NULL;

    m_pIterators[_it] = m_pIterators[_it]->prev;
    if (m_pIterators[_it] == NULL)
    {
        if (bLoop)
            return getLast(_it);

        return NULL;
    }

    return m_pIterators[_it]->obj;
}

// -----------------------------------------------------------------
// Name : getCurrent
// -----------------------------------------------------------------
BaseObject * ObjectList::getCurrent(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    if (m_pIterators[_it] == NULL)
        return NULL;

    return m_pIterators[_it]->obj;
}

// -----------------------------------------------------------------
// Name : getCurrentType
// -----------------------------------------------------------------
long ObjectList::getCurrentType(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    if (m_pIterators[_it] == NULL)
        return -1;

    return m_pIterators[_it]->type;
}

// -----------------------------------------------------------------
// Name : goTo
// -----------------------------------------------------------------
bool ObjectList::goTo(int _it, BaseObject * obj)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);

    m_pIterators[_it] = first;
    while (m_pIterators[_it] != NULL)
    {
        if (m_pIterators[_it]->obj == obj)
            return true;

        m_pIterators[_it] = m_pIterators[_it]->next;
    }

    return false;
}

// -----------------------------------------------------------------
// Name : goTo
// -----------------------------------------------------------------
BaseObject * ObjectList::goTo(int _it, unsigned short idx)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);
    assert(idx < size);

    m_pIterators[_it] = first;
    while (m_pIterators[_it] != NULL)
    {
        if (idx == 0)
            return m_pIterators[_it]->obj;

        idx--;
        m_pIterators[_it] = m_pIterators[_it]->next;
    }

    return NULL;
}

// -----------------------------------------------------------------
// Name : moveCurrentToBegin
// -----------------------------------------------------------------
void ObjectList::moveCurrentToBegin(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);
    assert(m_pIterators[_it] != NULL);

    // If already at begining, nothing to do
    if (m_pIterators[_it]->prev == NULL)
        return;

    // If it's last, move "last" pointer to prev;
    if (last == m_pIterators[_it])
        last = last->prev;

    // Connect prev and next together
    if (m_pIterators[_it]->next != NULL)
        m_pIterators[_it]->next->prev = m_pIterators[_it]->prev;
    m_pIterators[_it]->prev->next = m_pIterators[_it]->next;

    // Set to begin
    m_pIterators[_it]->prev = NULL;
    m_pIterators[_it]->next = first;
    first->prev = m_pIterators[_it];

    first = m_pIterators[_it];
}

// -----------------------------------------------------------------
// Name : moveCurrentToEnd
// -----------------------------------------------------------------
void ObjectList::moveCurrentToEnd(int _it)
{
    assert(_it < MAX_ITERATORS);
    assert(!m_bFreeIterators[_it]);
    assert(m_pIterators[_it] != NULL);

    // If already at end, nothing to do
    if (m_pIterators[_it]->next == NULL)
        return;

    // If it's first, move "first" pointer to next;
    if (first == m_pIterators[_it])
        first = first->next;

    // Connect prev and next together
    m_pIterators[_it]->next->prev = m_pIterators[_it]->prev;
    if (m_pIterators[_it]->prev != NULL)
        m_pIterators[_it]->prev->next = m_pIterators[_it]->next;

    // Set to end
    m_pIterators[_it]->next = NULL;
    m_pIterators[_it]->prev = last;
    last->next = m_pIterators[_it];

    last = m_pIterators[_it];
}

// -----------------------------------------------------------------
// Name : operator []
// -----------------------------------------------------------------
BaseObject * ObjectList::operator [](unsigned short idx)
{
    if (idx >= size)
        return NULL;

    ListNode * tmpIter = this->first;
    for (int i = 0; i < idx; i++)
        tmpIter = tmpIter->next;

    return tmpIter->obj;
}
