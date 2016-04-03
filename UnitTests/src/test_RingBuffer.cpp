#include "gmock/gmock.h"
#include "gtest/gtest.h"


#include "RingBuffer.hpp"

using ::testing::Test;

// Define the size of buffer
#define SIZE 8

class TestingRingBufferFixture : public Test
{
protected:
	using BuffersElementType = uint8_t;

	RingBuffer<BuffersElementType, SIZE> mRingBuffer;
};


TEST_F(TestingRingBufferFixture, CheckReturnedSize)
{
	const std::size_t expectedSize = SIZE - 1; // implementation of this buffer has real size SIZE -1
	EXPECT_EQ(expectedSize, mRingBuffer.size());
}

TEST_F(TestingRingBufferFixture, TrueAppendingDataToEmptyBuffer)
{
	const BuffersElementType data = 32;

	EXPECT_TRUE(mRingBuffer.append(data));
}

TEST_F(TestingRingBufferFixture, FalseIfAppendDataToFullBuffer)
{
	RingBuffer<uint8_t, 2> buffer;

	// fill the buffer;
	EXPECT_TRUE(buffer.append(1));

	// try add another element while buffer is full
	EXPECT_FALSE(buffer.append(102));
}

TEST_F(TestingRingBufferFixture, FalseIfGettingDataFromEmptyBuffer)
{
	BuffersElementType data = 0;	// to store data;

	// trying to get data from empty buffer should return false
	EXPECT_FALSE(mRingBuffer.get(data));
}

TEST_F(TestingRingBufferFixture, TrueForIsEmpty)
{
	EXPECT_TRUE(mRingBuffer.isEmpty());
	EXPECT_FALSE(mRingBuffer.isFull());
}

TEST_F(TestingRingBufferFixture, TrueForIsFull)
{
	RingBuffer<uint8_t, 2> buffer;

	//Fill whole buffer
	EXPECT_TRUE(buffer.append(100));

	EXPECT_TRUE(buffer.isFull());
	EXPECT_FALSE(buffer.isEmpty());
}

TEST_F(TestingRingBufferFixture, FillFull)
{

	// Fill multiple times
	uint8_t val = 1;
	RingBuffer<uint8_t, 8> buffer;

	while( true == buffer.append(val) )
	{
		++val;
	}

	EXPECT_EQ(static_cast<std::size_t>(val-1), buffer.size());
	EXPECT_TRUE(buffer.isFull());
}

TEST_F(TestingRingBufferFixture, FillThenEmptyAndCheckValues)
{
	// Fill
	BuffersElementType val = 1;
	while( true == mRingBuffer.append(val) )
	{
		++val;
	}

	BuffersElementType data = 0;
	val = 1;

	// Read all data from buffer
	while( true == mRingBuffer.get(data) )
	{
		// compare values: saved with just read
		EXPECT_EQ(val, data);

		++val;
	}

	// test if has been read the same times what real size is
	EXPECT_EQ(static_cast<std::size_t>(val-1), mRingBuffer.size());
}
