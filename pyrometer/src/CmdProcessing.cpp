#include "CmdProcessing.h"

#include "UARTCMDInterface.h"
#include "I2CInterface.h"

#include "TimerCore.h"
#include "HandleTimer.h"

CmdProcessing::CmdProcessing(UARTCMDInterface & uartDevice, I2CInterface & i2cDev)
    : m_uartDev(uartDevice),
      m_processingData(),
      m_currentAlgorithm(nullptr),
      m_whichTempRead(ReadTempType::READ_OBJECT),
      m_NSamples(1),
      m_MDelay(10),
      m_NCnt(0),
      m_PrevTimePoint(0),
      m_Emissivity(0),
      m_IRTempSensor(MLX90614Sensor(i2cDev))
{
    m_uartDev.init();
    m_uartDev.enable(true);
}

void CmdProcessing::run()
{
    // Received byte?
    if (false == m_uartDev.isReadBufferEmpty())
    {
        parseCmd();
    }

    // todo move this to the execudeCmd? inline method?
    // Run the latest command action
    if (nullptr != m_currentAlgorithm)
    {
        // call member method of this class, the method is specified by parseCmd()
        (this->*m_currentAlgorithm)();
    }
}

void CmdProcessing::parseCmd()
{
    // Reading recognized command?
    if (false == m_processingData.readingCMDData)
    // each command must start form byte which defines command
    // if previously byte was not command byte or command has been read successfully
    // then check if the current one byte is the valid command
    {
        uint8_t data = 0;
        m_uartDev.read(data);
        CmdType cmdType = static_cast<CmdType>(data);

        if ((CmdType::CMD_ONEREAD == cmdType) ||
            (CmdType::CMD_MULTIPLE == cmdType) ||
            (CmdType::CMD_ALIVE == cmdType) ||
            (CmdType::CMD_STOP == cmdType) ||
            (CmdType::CMD_SAVE == cmdType))
        {
            // Command has been recognized
            m_processingData.cmdType = cmdType;
            m_processingData.readingCMDData = true;
            return;
        }
        else
        {
            // error, not recognized command byte, clear state and wait for another byte
            m_processingData.readingCMDData = false;
            return;
        }
    }
    // Command has been recognized -> need to process its parameters
    else
    {
        switch(m_processingData.cmdType)
        {
        case CmdType::CMD_ONEREAD:
            parseCmdOneRead();
            break;
        case CmdType::CMD_MULTIPLE:
            parseCmdMultipleRead();
            break;
        case CmdType::CMD_ALIVE:
            // Change algorithm directly
            m_currentAlgorithm = &CmdProcessing::HandleAliveCmd;
            break;
        case CmdType::CMD_STOP:
            // Change algorithm directly
            m_currentAlgorithm = &CmdProcessing::HandleStopCmd;
            break;
        case CmdType::CMD_SAVE:
            parseCmdSaveData();
            break;
        default:
            //fatal!
            break;
        }
    }
}

void CmdProcessing::parseCmdOneRead()
{
    // In this case that step is the last one, so reset flag.
    m_processingData.readingCMDData = false;

    // Read second byte of command from UART buffer
    uint8_t cmdWhichTemp = 0;
    m_uartDev.read(cmdWhichTemp);

    // Verify second byte - which temperature need to read?
    const ReadTempType readType = parseCmdRecognizeReadType(cmdWhichTemp);
    if(ReadTempType::NOT_RECOGNIZED == readType)
    {
        // Not recognized data. Do not apply any changes and return immediately
        return;
    }

    m_whichTempRead = readType;
    // Command has been parsed properly. Apply algorithm assigned to this command
    m_currentAlgorithm = &CmdProcessing::HandleReadOneTimeCmd;
}

void CmdProcessing::parseCmdMultipleRead()
{
    // Read byte from UART buffer
    uint8_t data = 0;
    m_uartDev.read(data);

    if (0u == m_processingData.seq)
    {
        // Verify second byte - which temperature need to read?
        m_processingData.readType = parseCmdRecognizeReadType(data);
        if(ReadTempType::NOT_RECOGNIZED == m_processingData.readType)
        {
            // Not recognized data. Do not apply any changes,
            // reset internal state and return immediately
            m_processingData.readingCMDData = false;
            m_processingData.seq = 0;
            return;
        }

        // inform that next bytes will are command parameters
        ++m_processingData.seq;
    }
    // reading command parameters
    else
    {
        // append byte to the temporary place - until all parameters byte will be read
        m_processingData.paramData[m_processingData.seq-1] = data;
        ++m_processingData.seq;

        // if 4 bytes of parameters were read
        if (4u <= (m_processingData.seq-1u))
        {
            // convert 4B of parameters data:
            m_NSamples = (m_processingData.paramData[1] << 8) | m_processingData.paramData[0] ;
            m_MDelay   = (m_processingData.paramData[3] << 8) | m_processingData.paramData[2] ;

            // switch active command to that just read - switch algorithm
            m_currentAlgorithm = &CmdProcessing::HandleReadMultipleCmd;
            // save information which temperature need to read
            m_whichTempRead = m_processingData.readType;
            // get current time point and substrate the duration -> force first read as soon as possible
            m_PrevTimePoint = SysTickTimerCore.GetTimePoint() - m_MDelay - 1 ;
            m_NCnt = 0;

            // reset internal state of processing command
            m_processingData.seq = 0;
            m_processingData.readingCMDData = false;
        }
    }

}


void CmdProcessing::parseCmdSaveData()
{
    // Read byte from UART buffer
    uint8_t data = 0;
    m_uartDev.read(data);

    m_processingData.paramData[m_processingData.seq] = data;
    ++m_processingData.seq;

    if (2u >= m_processingData.seq)
    {
        // save data to variable
        m_Emissivity = (m_processingData.paramData[1] << 8u) | m_processingData.paramData[0];
        // change command algorithm
        m_currentAlgorithm = &CmdProcessing::HandleSaveEmissivityCmd;

        // reset internal state of processing command
        m_processingData.seq = 0;
        m_processingData.readingCMDData = false;
    }
}


// ALOGRITHMS WHICH HANDLES SUPPORTED COMMANDS

void CmdProcessing::HandleReadOneTimeCmd()
{
    // Create Start Byte:
    const uint8_t startByte = (static_cast<uint8_t>(CmdType::CMD_ONEREAD) & 0xF0) |
                               (static_cast<uint8_t>(m_whichTempRead) & 0x0F);

    // Read temperatures from the sensor
    auto temperatures = ReadTemperatures(m_whichTempRead);

    // Transmit data to the terminal via UART interface
    SendTemperaturesFrame(startByte, temperatures, m_whichTempRead);

    // The end - disable an algorithm
    m_currentAlgorithm = nullptr;
}


void CmdProcessing::HandleReadMultipleCmd()
{
    auto now = SysTickTimerCore.GetTimePoint();

    // Read data after each MDelay [ms] or all the time with max speed if MDelay is 0 [ms]
    if ((now - m_PrevTimePoint) >= m_MDelay)
    {
        // - store new time point
        m_PrevTimePoint = now;

        // - read data from the sensor
        auto temperatures = ReadTemperatures(m_whichTempRead);

        // - - create Start Byte:
        const uint8_t startByte = (static_cast<uint8_t>(CmdType::CMD_MULTIPLE) & 0xF0) |
                                  (static_cast<uint8_t>(m_whichTempRead) & 0x0F);
        // - send temperatures:
        SendTemperaturesFrame(startByte, temperatures, m_whichTempRead);

        // - keep watching if samples already were sent. If NSamples is 0 then send continuously.
        ++m_NCnt;
        if ((m_NSamples <= m_NCnt) && (0 < m_NSamples))
        {   // Already were sent all required samples so everything is done
            m_currentAlgorithm = nullptr;
        }
    }
}

void CmdProcessing::HandleStopCmd()
{
    // reset internal state of processing command
    m_processingData.seq = 0;
    m_processingData.readingCMDData = false;
    // Disable any algorithm which is working right now
    m_currentAlgorithm = nullptr;
}


void CmdProcessing::HandleAliveCmd()
{
    // reset internal state of processing command
    m_processingData.seq = 0;
    m_processingData.readingCMDData = false;
    // Disable any algorithm which is working right now
    m_currentAlgorithm = nullptr;
    // Send the response byte
    m_uartDev.send(static_cast<uint8_t>(CmdResponseType::RESP_ALIVE));
}


void CmdProcessing::HandleSaveEmissivityCmd()
{
	constexpr const uint8_t cmdEmissivity = MLX90614_CMD_EEPROM | MLX90614_EPPROM_EMISSIVITY;
	uint16_t emValue = 0;

	// save the emissivity to the IR sensor
    m_IRTempSensor.writeEmissivity(m_Emissivity);

    // restart the IR sensor
	m_IRTempSensor.restart();

    // Verify if value of emissivity was write properly into EEPROM
    emValue = m_IRTempSensor.readEEPROM(cmdEmissivity);

    // If value has been read properly then send ack via UART
    if (m_Emissivity == emValue)
    {
    	m_uartDev.send(static_cast<uint8_t>(CmdResponseType::RESP_SAVE_DONE));
    }
    else
    {
    	m_uartDev.send(static_cast<uint8_t>(CmdResponseType::RESP_ERROR));
    }

    // Disable algorithm after writing data to the IR sensor
    m_currentAlgorithm = nullptr;
}

// PRIVATE and PROTECTED - HELPING METHODS:

ReadTempType CmdProcessing::parseCmdRecognizeReadType(const uint8_t data)
{
    switch (static_cast<ReadTempType>(data))
    {
    case ReadTempType::READ_AMBIENT :
        return ReadTempType::READ_AMBIENT;
    case ReadTempType::READ_OBJECT :
        return ReadTempType::READ_OBJECT;
    case ReadTempType::READ_BOTH :
        return ReadTempType::READ_BOTH;
    default :
        // Not recognized data. Do not apply any changes.
        return ReadTempType::NOT_RECOGNIZED;
    }
}


std::pair<uint16_t, uint16_t> CmdProcessing::ReadTemperatures(const ReadTempType tempType)
{
    uint16_t temp, temp_second;

    if ( ReadTempType::READ_AMBIENT == tempType )
    {
        temp = m_IRTempSensor.readAmbiendTemp();
    }
    else if (ReadTempType::READ_OBJECT == tempType )
    {
        temp = m_IRTempSensor.readObjectTemp();
    }
    else if (ReadTempType::READ_BOTH == tempType)
    {
        temp = m_IRTempSensor.readAmbiendTemp();
        temp_second = m_IRTempSensor.readObjectTemp();
    }

    return std::pair<uint16_t, uint16_t>(temp, temp_second);
}


void CmdProcessing::SendTemperaturesFrame(const uint8_t startByte,
                                          const std::pair<uint16_t, uint16_t> & temperatures,
                                          const ReadTempType readTempType)
{
    // - send the START_BYTE first
    m_uartDev.send(startByte);

    // transmit the first temperature
    m_uartDev.send(static_cast<uint8_t>(temperatures.first));    // transmit low byte first
    m_uartDev.send(static_cast<uint8_t>(temperatures.first>>8)); // transmit high byte as second byte

    // if Both need to send then temp_second is the Object temperature.
    if (ReadTempType::READ_BOTH == readTempType)
    {
        m_uartDev.send(static_cast<uint8_t>(temperatures.second));    // transmit low byte first
        m_uartDev.send(static_cast<uint8_t>(temperatures.second>>8)); // transmit high byte as second byte
    }
}
