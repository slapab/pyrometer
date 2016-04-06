/*
 * UARTDevice.h
 *
 *  Created on: 29 mar 2016
 *      Author: scott
 */

#ifndef HDR_UARTDEVICE_H_
#define HDR_UARTDEVICE_H_

#include "RingBuffer.hpp"
#include "UARTCMDInterface.h"

/** Define BAUDRATE - values
*   For fCK = 48 MHz and over sampling = 16
*/
#define UART1_BAUD9600   0x1388
#define UART1_BAUD19200  0x9C4
#define UART1_BAUD57600  0x341
#define UART1_BAUD38400  0x4E2
#define UART1_BAUD115200 0x1A1

//Interrupt handler declaration in C/C++
#ifdef __cplusplus
extern "C" {
#endif
	 void USART1_IRQHandler();
#ifdef __cplusplus
}
#endif



class UARTDevice : public virtual UARTCMDInterface
{
public:
	UARTDevice(const UARTDevice &) 			   = delete;
	UARTDevice(UARTDevice &&) 				   = delete;
	UARTDevice & operator=(const UARTDevice &) = delete;
	UARTDevice & operator=(UARTDevice &&) 	   = delete;

	static UARTDevice & get();

    inline void enable(const bool enable);
    void init();
    bool send(const uint8_t data);
    bool read(uint8_t & ref);
    inline bool isCmdComplete();

    friend void USART1_IRQHandler();

protected:
    UARTDevice();
    virtual ~UARTDevice() {}

private:
    RingBuffer<uint8_t, 16> m_RcvBuff;
    RingBuffer<uint8_t, 16> m_SendBuff;

    volatile bool m_TxInProgress;
    volatile bool m_CmdComplete;
};


#endif /* HDR_UARTDEVICE_H_ */
