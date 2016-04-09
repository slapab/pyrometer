#include "CmdProcessing.h"

#include "UARTCMDInterface.h"
#include "I2CInterface.h"

CmdProcessing::CmdProcessing(UARTCMDInterface & uartDevice, I2CInterface & i2cDev)
	: m_uartDev(uartDevice),
	  m_currentAlgorithm(nullptr),
	  m_whichTempRead(ReadTempType::READ_OBJECT),
	  m_NSamples(1),
	  m_MDelay(10),
	  m_IRTempSensor(i2cDev)
{}

void CmdProcessing::run()
{
	// Received new command?
	// todo remove isCmdComplete() method check receive buffer instead
	if ( m_uartDev.isCmdComplete() )
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
	uint8_t cmdType = 0;
	uint8_t cmdWhichTemp = 0;
	ReadTempType tmp_whichTemp;

	// Reading recognized command?
	if (false == m_processingData.readingCMDData)
	// each command must start form byte which defines command
	// if previously byte was not command byte or command has been read successfully
	// then check if the current one byte is the valid command
	{
		uint8_t data = 0;
		m_uartDev.read(data);	// todo check return val?
		CmdType cmdType = static_cast<CmdType>(data);

		if ((CmdType::CMD_ONEREAD == cmdType) ||
			(CmdType::CMD_MULTIPLE == cmdType) ||
			(CmdType::CMD_ALIVE == cmdType) ||
			(CmdType::CMD_STOP == cmdType))
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
			break;
		case CmdType::CMD_STOP:
			break;
		default:
			//fatal!
			break;
		}
	}




//	// This lambda function just flushes currently parsing command
//	auto _flushLatestCMD = [&] ()
//		{
//			uint8_t data = 0;
//			while( true == m_uartDev.read(data))
//			{
//				if ( '\n' == data )
//				{
//					break;
//				}
//			}
//		};
//
//	// Assign right algorithm due to received command
//	if (CmdType::CMD_ONEREAD == static_cast<CmdType>(cmdType))
//	{
//		// Read last command byte -> should be END byte
//		_flushLatestCMD();
//
//		// Apply algorithm assigned this command
//		m_currentAlgorithm = &CmdProcessing::readOneTime;
//	}
//	else if (CmdType::CMD_MULTIPLE == static_cast<CmdType>(cmdType))
//	{
//		uint8_t lowByte, highByte;
//		bool checkReturn = true;
//
//		// Read now the N - samples and the M [ms] delay between data capturing
//		checkReturn &= m_uartDev.read(lowByte);
//		checkReturn &= m_uartDev.read(highByte);
//
//		// If data can not be read it means that data are corrupted and need to discard this parsing procedure
//		if (false == checkReturn)	return;
//
//		// store N value
//		m_NSamples = (highByte << 8) | lowByte;
//
//		// Read the M [ms] time delay between reading samples from sensor
//		checkReturn = true;
//		checkReturn &= m_uartDev.read(lowByte);
//		checkReturn &= m_uartDev.read(highByte);
//
//		// If data can not be read it means that data are corrupted and need to discard this parsing procedure
//		if (false == checkReturn)	return;
//
//		// store M value
//		m_MDelay = (highByte << 8) | lowByte;
//
//		// Read last command byte -> should be END byte
//		_flushLatestCMD();
//
//		// Apply algorithm assigned this command
//		m_currentAlgorithm = &CmdProcessing::readMultiple;
//	}
//	else if (CmdType::CMD_STOP == static_cast<CmdType>(cmdType))
//	{
//		// disable any action:
//		m_currentAlgorithm = nullptr;
//
//		// second byte should be STOP byte but need to check this
//		if ('\n' != cmdWhichTemp)
//		{
//			_flushLatestCMD();
//		}
//
//		return;
//	}
//	// Cannot apply any changes if command is not know
//	else
//	{
//
//		// todo what need to do here
//		_flushLatestCMD();
//		return;
//	}
//
//	// command was parsed properly apply changes
//	m_whichTempRead = tmp_whichTemp;
}

void CmdProcessing::parseCmdOneRead()
{
	// In this case that step is the last one, so reset flag.
	m_processingData.readingCMDData = false;

	// Read second byte of command from UART buffer
	uint8_t cmdWhichTemp = 0;
	m_uartDev.read(cmdWhichTemp);

	// Verify second byte - which temperature need to read?
	switch (static_cast<ReadTempType>(cmdWhichTemp))
	{
	case ReadTempType::READ_AMBIENT :
		m_whichTempRead = ReadTempType::READ_AMBIENT;
		break ;
	case ReadTempType::READ_OBJECT :
		m_whichTempRead = ReadTempType::READ_OBJECT;
		break ;
	case ReadTempType::READ_BOTH :
		m_whichTempRead = ReadTempType::READ_BOTH;
		break ;
	default :
		// Not recognized data. Do not apply any changes and return immediately
		return;
	}

	// Command has been parsed properly. Apply algorithm assigned to this command
	m_currentAlgorithm = &CmdProcessing::readOneTime;
}


void CmdProcessing::parseCmdMultipleRead()
{

}

void CmdProcessing::parseCmdSaveData()
{

}


// Algorithms assigned to commands:


void CmdProcessing::readOneTime()
{
	uint16_t temp, temp_second;

	// Create Start Byte:
	const uint8_t START_BYTE = (static_cast<uint8_t>(CmdType::CMD_ONEREAD) & 0xF0) |
								(static_cast<uint8_t>(m_whichTempRead) & 0x0F);


	if ( ReadTempType::READ_AMBIENT == m_whichTempRead )
	{
		temp = m_IRTempSensor.readAmbiendTemp();
	}
	else if (ReadTempType::READ_OBJECT == m_whichTempRead )
	{
		temp = m_IRTempSensor.readObjectTemp();
	}
	else if (ReadTempType::READ_BOTH == m_whichTempRead)
	{
		temp = m_IRTempSensor.readAmbiendTemp();
		temp_second = m_IRTempSensor.readObjectTemp();
	}

	// Append data to the UART sending buffer
	m_uartDev.send(START_BYTE);
	m_uartDev.send(static_cast<uint8_t>(temp));    // transmit low byte first
	m_uartDev.send(static_cast<uint8_t>(temp>>8)); // transmit high byte as second byte

	// if Both need to send then temp_second is the Object temp.
	if (ReadTempType::READ_BOTH == m_whichTempRead)
	{
		m_uartDev.send(static_cast<uint8_t>(temp_second));    // transmit low byte first
		m_uartDev.send(static_cast<uint8_t>(temp_second>>8)); // transmit high byte as second byte
	}

	// disable algorithm
	m_currentAlgorithm = nullptr;
}


void CmdProcessing::readMultiple()
{

}
