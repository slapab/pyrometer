/*
 * TimerCore.cpp
 *
 *  Created on: 30 kwi 2016
 *      Author: scott
 */

#include "TimerCore.h"

TimerCore SysTickTimerCore;

TimerCore::TimerCore()
    : m_Value(0)
{}

TimerCore::timer_type TimerCore::getValue() const
{
    return m_Value;
}

TimerCore & TimerCore::operator++()
{
    ++m_Value;
    return *this;
}


