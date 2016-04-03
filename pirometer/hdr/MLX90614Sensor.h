/*
 * MLX90614Device.h
 *
 *  Created on: 28 mar 2016
 *      Author: scott
 */

#ifndef HDR_MLX90614SENSOR_H_
#define HDR_MLX90614SENSOR_H_

#include <cstdint>
#include "I2CInterface.h"


#define MLX90614_RAM_TA  0x06   /// Address in RAM where ambient temperature is storing
#define MLX90614_RAM_TO1 0x07   /// Address in RAM where object temperature is storing

/** Commands which are understand by MLX90614 sensor
    * @note EEPROM_CMD and RAM_CMD must be combined with 5 LSB's bits of memory
    *       address.
    */
#define MLX90614_CMD_EEPROM     0x20
#define MLX90614_CMD_RAM        0x00
#define MLX90614_CMD_READ_FLAGS 0xF0
#define MLX90614_CMD_SLEEP      0xFF

// FORWARD DECLARATIONS

class MLX90614Sensor
{
public:
    MLX90614Sensor(I2CInterface & i2cDev);
    MLX90614Sensor(I2CInterface & i2cDev, const uint8_t snesorAddr);

    ~MLX90614Sensor() = default;

    uint16_t readAmbiendTemp();
    uint16_t readObjectTemp();

    uint16_t readRAM(const uint8_t addr);
    uint16_t readEEPROM(const uint8_t addr);

    // todo write methods


private:
    /// Sensor SMBus slave address
    uint8_t m_sensorAddr;

    /// Object to manage I2C communication with the sensor
    I2CInterface & m_dev;

   //const uint8_t m_buffSize;
    uint8_t m_buff[3];
};


#endif /* HDR_MLX90614SENSOR_H_ */
