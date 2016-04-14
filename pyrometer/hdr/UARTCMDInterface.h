/*
 * UARTCMDInterface.h
 *
 *  Created on: 3 kwi 2016
 *      Author: scott
 */

#ifndef HDR_UARTCMDINTERFACE_H_
#define HDR_UARTCMDINTERFACE_H_


#include "UARTInterface.h"

class UARTCMDInterface : public virtual UARTInterface
{
public:
    UARTCMDInterface() {}
    virtual ~UARTCMDInterface() {}

    virtual bool isReadBufferEmpty() = 0;
};


#endif /* HDR_UARTCMDINTERFACE_H_ */
