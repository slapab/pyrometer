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
    auto currValue = m_TimerCore.getValue();

    while ((m_TimerCore.getValue() - currValue) <= duration)
    {
        __asm__("");
    }
}

