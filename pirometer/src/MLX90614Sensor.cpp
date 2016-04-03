#include <MLX90614Sensor.h>
#include "I2CDevice.h"

MLX90614Sensor::MLX90614Sensor(I2CInterface & i2cDev)
    : m_sensorAddr(0x5A),   // default sensor's address
      m_dev(i2cDev)   // assign sensor's address to the communication object
{
    m_dev.init();
}

//todo what about address now?
MLX90614Sensor::MLX90614Sensor(I2CInterface & i2cDev, const uint8_t sensorAddr)
    : m_sensorAddr(sensorAddr), // User specified address
      m_dev(i2cDev)			    // assign sensor's address to the communication object
{
    m_dev.init();
}

uint16_t MLX90614Sensor::readAmbiendTemp()
{
    constexpr const uint8_t command = MLX90614_CMD_RAM | MLX90614_RAM_TA;
    m_dev.read(command, &m_buff[0], 2);

    //todo error checking

    return static_cast<uint16_t>((m_buff[1] << 8) | m_buff[0]);
}

uint16_t MLX90614Sensor::readObjectTemp()
{
    constexpr const uint8_t command = MLX90614_CMD_RAM | MLX90614_RAM_TO1;
    m_dev.read(command, &m_buff[0], 2);

    // todo error checking
    return static_cast<uint16_t>((m_buff[1] << 8) | m_buff[0]);
}

uint16_t MLX90614Sensor::readRAM(const uint8_t addr)
{
    return 0;
}

uint16_t MLX90614Sensor::readEEPROM(const uint8_t addr)
{
    return 0;
}

