/*
 * I2CInterface.cpp
 *
 *  Created on: 22 mar 2016
 *      Author: scott
 */

#ifndef D_I2C_INTERFACE_H
#define D_I2C_INTERFACE_H

#include <cstddef>
#include <cstdint>

class I2CInterface
{
public:
    I2CInterface() {}
    virtual ~I2CInterface() {}

    virtual void init() = 0;
    virtual void deinit() = 0;
    virtual void enable(const bool en) = 0;

    virtual void send(const uint8_t, const uint8_t * const, const size_t ) = 0;
    virtual void read(const uint8_t, uint8_t * const, const size_t) = 0;
};



#endif
