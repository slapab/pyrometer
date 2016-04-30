/*
 * TimerCore.h
 *
 *  Created on: 30 kwi 2016
 *      Author: scott
 */

#ifndef HDR_TIMERCORE_H_
#define HDR_TIMERCORE_H_

#include <csignal>
class TimerCore;

extern TimerCore SysTickTimerCore;

class TimerCore
{
public:
    using timer_type = sig_atomic_t;

    TimerCore();
    TimerCore & operator++();
    timer_type getValue() const;

protected:
    timer_type m_Value;
};



#endif /* HDR_TIMERCORE_H_ */
