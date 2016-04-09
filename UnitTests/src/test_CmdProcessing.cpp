#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmdProcessing.h"

#include "mock_UARTCMDDevice.h"
#include "mock_I2CDevice.h"


using ::testing::_;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SetArgReferee;
using ::testing::InSequence;
using ::testing::Invoke;




class FTestCmdProcessing : public CmdProcessing
{
public:
	using AlgorithmTypeFT = CmdProcessing::AlgorithmType ;
	FTestCmdProcessing(UARTCMDInterface & uartDev, I2CInterface & i2cDev)
		: CmdProcessing(uartDev, i2cDev)
	{}

	// Bring to public:
	using CmdProcessing::parseCmd;
	using CmdProcessing::readOneTime;
	using CmdProcessing::readMultiple;
	using CmdProcessing::ProcessingTempaData;
	using CmdProcessing::m_processingData;
	using CmdProcessing::m_whichTempRead;
	using CmdProcessing::m_currentAlgorithm;

};



class FixtureCmdProcessing : public ::testing::Test
{
protected:
	FixtureCmdProcessing() : Test(), cmdProcessing(mockUartDev, mockI2CDev) {}

	MockI2CDevice mockI2CDev;
	MockUARTCMDDevice mockUartDev;
	FTestCmdProcessing cmdProcessing;
};


TEST_F(FixtureCmdProcessing, EmptyCMDBuffer_OnlyOneCallForData)
{
	EXPECT_CALL(mockUartDev, read(_)).Times(1);

	// test method
	cmdProcessing.parseCmd();

	// check state after method call
	EXPECT_EQ(cmdProcessing.m_currentAlgorithm, nullptr);
	EXPECT_EQ(cmdProcessing.m_whichTempRead, ReadTempType::READ_OBJECT);
}

TEST_F(FixtureCmdProcessing, OneReadCommandAmbientTemp_ValidateCommandCheck)
{
	const uint8_t CMDType = static_cast<uint8_t>(CmdType::CMD_ONEREAD);
	const uint8_t TempRead = static_cast<uint8_t>(ReadTempType::READ_AMBIENT);

	EXPECT_CALL(mockUartDev, read(_))
			.Times(2)	// exactly 3-times should be called
			.WillOnce(DoAll(SetArgReferee<0>(CMDType), Return(true)))	//After first read return CMDType value
			.WillOnce(DoAll(SetArgReferee<0>(TempRead), Return(true)))	//After 2nd read return TempRead value
			//.WillRepeatedly(Return(false))	// other calling should return false
			;

	// Call tested method - after this call should properly recognize command
	cmdProcessing.parseCmd();
	EXPECT_EQ(CmdType::CMD_ONEREAD, cmdProcessing.m_processingData.cmdType);
	EXPECT_TRUE(cmdProcessing.m_processingData.readingCMDData);

	// Call tested method second time - now should read byte with information which temp
	// want to read
	cmdProcessing.parseCmd();

	EXPECT_EQ(ReadTempType::READ_AMBIENT, cmdProcessing.m_whichTempRead);
	EXPECT_EQ(cmdProcessing.m_currentAlgorithm, &FTestCmdProcessing::readOneTime);
	EXPECT_FALSE(cmdProcessing.m_processingData.readingCMDData);
}

TEST_F(FixtureCmdProcessing, DISABLED_ContinousReadCommandAmbientTemp_ValidateCommandCheck)
{
	const uint8_t CMDType = static_cast<uint8_t>(CmdType::CMD_MULTIPLE);
	const uint8_t TempRead = static_cast<uint8_t>(ReadTempType::READ_AMBIENT);
	const uint8_t N_lowByte = 0x09;
	const uint8_t N_highByte = 0x00;
	const uint8_t M_lowByte = 0xFF;
	const uint8_t M_highByte = 0x00;

	const uint8_t EndByte = '\n';

	EXPECT_CALL(mockUartDev, read(_)).
			Times(7).	// exactly 3-times should be called
			WillOnce(DoAll(SetArgReferee<0>(CMDType), Return(true))).	//After first read return CMDType value
			WillOnce(DoAll(SetArgReferee<0>(TempRead), Return(true))).	//After 2nd read return TempRead value
			WillOnce(DoAll(SetArgReferee<0>(N_lowByte), Return(true))).	//After 3rd read return low byte of N
			WillOnce(DoAll(SetArgReferee<0>(N_highByte), Return(true))).//After 4rd read return high byte of N
			WillOnce(DoAll(SetArgReferee<0>(M_lowByte), Return(true))). //After 5rd read return low byte of M
			WillOnce(DoAll(SetArgReferee<0>(M_highByte), Return(true))).//After 6rd read return high byte of M
			WillOnce(DoAll(SetArgReferee<0>(EndByte), Return(true)))	//At the last return End Byte
			//WillRepeatedly(Return(false))	// other calling should return false
			;

	cmdProcessing.parseCmd();

	// Check internal state
	EXPECT_EQ(cmdProcessing.m_currentAlgorithm, &FTestCmdProcessing::readMultiple);
	EXPECT_EQ(cmdProcessing.m_whichTempRead, static_cast<ReadTempType>(TempRead));
}

TEST_F(FixtureCmdProcessing, OneReadAlgorithm_AmbiendTempRequestWithResponse)
{

	ReadTempType AMBIENT_TEMP = ReadTempType::READ_AMBIENT;
	const uint8_t START_BYTE = (static_cast<uint8_t>(CmdType::CMD_ONEREAD) & 0xF0) |
									(static_cast<uint8_t>(AMBIENT_TEMP) & 0x0F);

	// I2C data:
	const uint8_t AMBIENT_TEMP_LOW = 0xd8;
	const uint8_t AMBIENT_TEMP_HIGH = 0x38;

	// Set internal state which will inform that just received command to read AMBIENT TEMPERATURE
	cmdProcessing.m_whichTempRead = AMBIENT_TEMP;

	// I2C:
	EXPECT_CALL(mockI2CDev, read(_,_,_)).
			Times(1).
			WillOnce(Invoke([&](auto a1, auto array, auto a3) {
				array[0] = AMBIENT_TEMP_LOW;
				array[1] = AMBIENT_TEMP_HIGH;
			} ));


	// UART:
	{
		InSequence seq;

		EXPECT_CALL(mockUartDev, send(START_BYTE)).
				Times(1).WillOnce(Return(true));

		EXPECT_CALL(mockUartDev, send(AMBIENT_TEMP_LOW)).
					Times(1).WillOnce(Return(true));

		EXPECT_CALL(mockUartDev, send(AMBIENT_TEMP_HIGH)).
						Times(1).WillOnce(Return(true));
	}


	// Perform test: method call
	cmdProcessing.readOneTime();

	EXPECT_EQ(cmdProcessing.m_currentAlgorithm, nullptr);
}
