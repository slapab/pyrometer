/*
 * I2CDevice.cpp
 *
 *  Created on: 22 mar 2016
 *      Author: scott
 */

#ifndef D_I2C_DEVICE_H
#define D_I2C_DEVICE_H

#include "I2CInterface.h"


class I2CDevice : public virtual I2CInterface
{
public:

    I2CDevice() = delete;
    I2CDevice(const uint8_t slaveAddr);

    void init() override ;
    void deinit() override;
    void enable(const bool en) override;

    void send(const uint8_t addr, const uint8_t * const pBuff, const size_t size) override;
    void read(const uint8_t addr, uint8_t * const pBuff, const size_t len) override;


private:
    void initSMBusMode();
    void clockEnable(const bool);

    void setSlaveAddr(const uint8_t slaveAddr = 0);
    inline void setTransferBytes(size_t bytes);

    // private members
    uint8_t m_slaveAddres;

};


#endif
