#ifndef _EVENT_LISTENER_H
#define _EVENT_LISTENER_H

#include "../utils.h"

#define LISTENER_MAX_PRIORITY   10
class ButtonAction;

// Abstract class EventListener
class EventListener
{
    friend class InputEngine;
public:
    EventListener(int iPriority)
    {
        assert(iPriority >= 0 && iPriority <= LISTENER_MAX_PRIORITY);
        m_iEventListenerPriority = iPriority;
    }; // higher is better
    virtual bool onCatchButtonEvent(ButtonAction * pEvent)
    {
        return false;
    };
    virtual bool onCursorMoveEvent(int xPxl, int yPxl)
    {
        return false;
    };

protected:
    int m_iEventListenerPriority;
};

#endif
