/*
 * CmdProcessing.h
 *
 *  Created on: 2 kwi 2016
 *      Author: scott
 */

#ifndef HDR_CMDPROCESSING_H_
#define HDR_CMDPROCESSING_H_

#include <utility>      // std::move
#include <cstdint>
#include <csignal>
#include "MLX90614Sensor.h"

// FORWARD DECLARATIONS:
class UARTCMDInterface;
class I2CInterface;

enum class CmdType : uint8_t
{
    CMD_ONEREAD  = 0xAA,
    CMD_MULTIPLE = 0xBB,
    CMD_STOP     = 0xE8,
    CMD_ALIVE    = 0xEE,
    CMD_SAVE     = 0x55
};

enum class ReadTempType : uint8_t
{
    NOT_RECOGNIZED = 0x00,
    READ_AMBIENT   = 0x11,
    READ_OBJECT    = 0x12,
    READ_BOTH      = 0x13
};

enum class CmdResponseType : uint8_t
{
	RESP_ERROR     = 0x0D,
	RESP_SAVE_DONE = 0x55,
	RESP_ALIVE     = 0xEE
};


class CmdProcessing
{
public:
    CmdProcessing(UARTCMDInterface & uartDev, I2CInterface & i2cDev );
    ~CmdProcessing() {}

    void run();

protected:
    // Command algorithm's function/method type
    using AlgorithmType = void (CmdProcessing::*) ();

    struct ProcessingTempaData
    {
        ProcessingTempaData()
            : cmdType(CmdType::CMD_STOP),
              readType(ReadTempType::NOT_RECOGNIZED),
              readingCMDData(false),
              seq(0) {}

        // temporary place for storing command parameters data
        uint8_t paramData[4];
        // recognized command type
        CmdType cmdType;
        // recognized read type
        ReadTempType readType;
        // true if command type was recognized and still reading / parsing command
        bool readingCMDData;
        // if command has multiple parameters to read this variable is using to
        // tracking the order of received bytes
        uint8_t seq;
    };

    // PROTECTED METHODS:
    //void execudeCmd();
    void parseCmd();
    // - parts of parsing command - each method is parsing one specific command
    void parseCmdOneRead();
    void parseCmdMultipleRead();
    void parseCmdSaveData();

    // - all supported command algorithms:
    void HandleReadOneTimeCmd();
    void HandleReadMultipleCmd();
    void HandleSaveEmissivityCmd();
    void HandleStopCmd();
    void HandleAliveCmd();

    // - helper methods
    std::pair<uint16_t, uint16_t> ReadTemperatures(const ReadTempType readTempType);
    void SendTemperaturesFrame(const uint8_t startByte,
                               const std::pair<uint16_t, uint16_t> & temperatures,
                               const ReadTempType readTempType);

    // PROTECTED FIELDS:

    UARTCMDInterface &  m_uartDev;
    MLX90614Sensor m_IRTempSensor;

    ProcessingTempaData m_processingData;
    AlgorithmType       m_currentAlgorithm;

    // Data for reading commands
    ReadTempType m_whichTempRead;
    uint16_t     m_NSamples;        // the N value from command frame
    uint16_t     m_MDelay;          // the M value from command frame
    uint16_t     m_NCnt;            // it is using to count how many times data were sent
    sig_atomic_t m_PrevTimePoint;   // storing previous time point

    // Data for save emissivity command
    uint16_t     m_Emissivity;  // value which need to be write to IR sensor

private:
    ReadTempType parseCmdRecognizeReadType(const uint8_t data);
};


#endif /* HDR_CMDPROCESSING_H_ */
