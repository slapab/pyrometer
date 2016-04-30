/*
 * HandleTimer.h
 *
 *  Created on: 30 kwi 2016
 *      Author: scott
 */

#ifndef HDR_HANDLETIMER_H_
#define HDR_HANDLETIMER_H_

#include <csignal>
#include <cstdint>

class TimerCore;

class HandleTimer
{
public:
    using timer_type = sig_atomic_t;

    HandleTimer() = delete;
    HandleTimer(const TimerCore & timerCore);

    void Sleep_ms(uint32_t duration);

protected:
    const TimerCore & m_TimerCore;
};



#endif /* HDR_HANDLETIMER_H_ */
