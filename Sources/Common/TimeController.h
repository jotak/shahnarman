#ifndef _TIME_CONTROLLER_H
#define _TIME_CONTROLLER_H

#include "BaseObject.h"

enum TC_State
{
    TC_Running = 0,
    TC_Stopped,
    TC_DelayReached,
    TC_TimerOver
};

class TimeController : public BaseObject
{
public:
    TimeController();
    ~TimeController();

    virtual void start(double fDelay);
    virtual void start(double fDuration, double fDelay);
    virtual void start(u32 uMaxIterations, double fDelay);
    virtual void stop();
    virtual TC_State update(double fDelta);
    virtual TC_State getState()
    {
        return m_State;
    };
    virtual double getTimer()
    {
        return m_fTimer;
    };

private:
    TC_State m_State;
    u32 m_uMaxIterations;
    double m_fDelay;
    double m_fTimer;
    u32 m_uIterator;
};

#endif
