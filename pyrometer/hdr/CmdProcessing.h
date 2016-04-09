/*
 * CmdProcessing.h
 *
 *  Created on: 2 kwi 2016
 *      Author: scott
 */

#ifndef HDR_CMDPROCESSING_H_
#define HDR_CMDPROCESSING_H_

#include <cstdint>
#include "MLX90614Sensor.h"

// FORWARD DECLARATIONS:
class UARTCMDInterface;
class I2CInterface;

enum class CmdType : uint8_t
{
	CMD_ONEREAD  = 0xAA,
	CMD_MULTIPLE = 0xBB,
	CMD_STOP 	 = 0xE8,
	CMD_ALIVE 	 = 0xEE
};

enum class ReadTempType : uint8_t
{
	READ_AMBIENT = 0x11,
	READ_OBJECT  = 0x12,
	READ_BOTH  	 = 0x13
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
		CmdType cmdType;
		bool readingCMDData;
	};

	// PROTECTED METHODS:
	//void execudeCmd();
	void parseCmd();
	// parts of parsing command - each method is parsing one specific command
	void parseCmdOneRead();
	void parseCmdMultipleRead();
	void parseCmdSaveData();

	// - all supported command algorithms:
	void readOneTime();
	void readMultiple();

	// PROTECTED FIELDS:
	UARTCMDInterface & m_uartDev;

	ProcessingTempaData m_processingData;
	AlgorithmType m_currentAlgorithm;

	ReadTempType m_whichTempRead;

	uint16_t m_NSamples;	// the N value from command frame
	uint16_t m_MDelay;		// the M value from command frame

	MLX90614Sensor m_IRTempSensor;
};


#endif /* HDR_CMDPROCESSING_H_ */
