/*
 * mock_I2CDevice.h
 *
 *  Created on: 3 kwi 2016
 *      Author: scott
 */

#ifndef MOCK_I2CDEVICE_H_
#define MOCK_I2CDEVICE_H_

#include <gmock/gmock.h>

#include "I2CInterface.h"

class MockI2CDevice : public I2CInterface
{
public:
	MOCK_METHOD0(init, void());
	MOCK_METHOD0(deinit, void());
	MOCK_METHOD1(enable, void(const bool));
	MOCK_METHOD3(send, void(const uint8_t, const uint8_t * const, const size_t));
	MOCK_METHOD3(read, void(const uint8_t, uint8_t * const, const size_t));
};


#endif /* MOCK_I2CDEVICE_H_ */
