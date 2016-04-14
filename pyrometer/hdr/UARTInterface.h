/*
 * UARTInterface.h
 *
 *  Created on: 2 kwi 2016
 *      Author: scott
 */

#ifndef HDR_UARTINTERFACE_H_
#define HDR_UARTINTERFACE_H_

class UARTInterface
{
public:
    UARTInterface() {}
    virtual ~UARTInterface() {}

    virtual inline void enable(const bool enable) = 0;
    virtual void init() = 0;
    virtual bool send(const uint8_t data) = 0;
    virtual bool read(uint8_t & ref) = 0;
};



#endif /* HDR_UARTINTERFACE_H_ */
