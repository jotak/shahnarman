// -----------------------------------------------------------------
// TimeController
// -----------------------------------------------------------------

#include "TimeController.h"


// -----------------------------------------------------------------
// Name : TimeController
//  Constructor
// -----------------------------------------------------------------
TimeController::TimeController()
{
    m_State = TC_Stopped;
    m_fDelay = m_fTimer = 0.0f;
    m_uIterator = m_uMaxIterations = 0;
}

// -----------------------------------------------------------------
// Name : ~TimeController
//  Destructor
// -----------------------------------------------------------------
TimeController::~TimeController()
{
}

// -----------------------------------------------------------------
// Name : start
// -----------------------------------------------------------------
void TimeController::start(double fDelay)
{
    m_uMaxIterations = 0;
    m_fDelay = fDelay;
    m_fTimer = 0.0f;
    m_State = TC_Running;
}

// -----------------------------------------------------------------
// Name : start
// -----------------------------------------------------------------
void TimeController::start(double fDuration, double fDelay)
{
    m_uMaxIterations = (u32) ceil(fDuration / fDelay);
    m_uIterator = 0;
    m_fDelay = fDelay;
    m_fTimer = 0.0f;
    m_State = TC_Running;
}

// -----------------------------------------------------------------
// Name : start
// -----------------------------------------------------------------
void TimeController::start(u32 uMaxIterations, double fDelay)
{
    m_uMaxIterations = uMaxIterations;
    m_uIterator = 0;
    m_fDelay = fDelay;
    m_fTimer = 0.0f;
    m_State = TC_Running;
}

// -----------------------------------------------------------------
// Name : stop
// -----------------------------------------------------------------
void TimeController::stop()
{
    m_State = TC_Stopped;
}

// -----------------------------------------------------------------
// Name : update
// -----------------------------------------------------------------
TC_State TimeController::update(double fDelta)
{
    if (m_State != TC_Stopped)
    {
        if (m_State == TC_TimerOver)
        {
            m_State = TC_Stopped;
        }
        else
        {
            m_State = TC_Running;
            if (m_uMaxIterations > 0)
            {
                if (m_uIterator >= m_uMaxIterations)
                {
                    m_State = TC_TimerOver;
                }
                m_uIterator++;
            }
            else
            {
                m_fTimer += fDelta;
                if (m_fTimer >= m_fDelay)
                {
                    m_fTimer = 0;
                    m_State = TC_DelayReached;
                }
            }
        }
    }

    return m_State;
}
