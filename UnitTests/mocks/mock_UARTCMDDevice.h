/*
 * mock_UARTCMDDevice.h
 *
 *  Created on: 3 kwi 2016
 *      Author: scott
 */

#ifndef MOCK_UARTCMDDEVICE_H_
#define MOCK_UARTCMDDEVICE_H_

#include <gmock/gmock.h>
#include "UARTCMDInterface.h"

class MockUARTCMDDevice : public UARTCMDInterface
{
public:
	MOCK_METHOD0(isReadBufferEmpty, bool());
	MOCK_METHOD1(enable, void(const bool));
	MOCK_METHOD0(init, void());
	MOCK_METHOD1(send, bool(const uint8_t));
	MOCK_METHOD1(read, bool(uint8_t &));
};



#endif /* MOCK_UARTCMDDEVICE_H_ */
