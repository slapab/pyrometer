/*
 * HandleTimer.cpp
 *
 *  Created on: 30 kwi 2016
 *      Author: scott
 */

#include "HandleTimer.h"
#include "TimerCore.h"

HandleTimer::HandleTimer(const TimerCore & timerCore)
    : m_TimerCore(timerCore)
{}


void HandleTimer::Sleep_ms(uint32_t duration)
{
    auto currValue = m_TimerCore.GetTimePoint();

    while ((m_TimerCore.GetTimePoint() - currValue) <= duration)
    {
        __asm__("");
    }
}

