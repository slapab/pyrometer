#ifndef D_RING_BUFFER_H
#define D_RING_BUFFER_H

#include "BufferInterface.h"
#include "signal.h"	// for sig_atomic_t type

template <class BuffType, std::size_t SIZE>
class RingBuffer final : public virtual BufferInterface<BuffType, SIZE>
{
public:
	RingBuffer();
	virtual ~RingBuffer() {}

	virtual inline void reset() override;
	virtual bool append(const BuffType val) override;
	virtual bool get(BuffType & ref) override;
	virtual constexpr inline std::size_t size() override;
	virtual inline bool isFull() override;
	virtual inline bool isEmpty() override;

private:
	volatile BuffType mBuff[SIZE];

	volatile sig_atomic_t mHead;
	volatile sig_atomic_t mTail;
};


template <class BuffType, std::size_t SIZE>
RingBuffer<BuffType, SIZE>::RingBuffer()
	: mHead(0), mTail(0)
{
	// Check if SIZE of buffer is power of 2!
	static_assert((SIZE != 0) && !(SIZE & (SIZE - 1)), "Size of this RingBuffer must be power of 2!");
}



template <class BuffType, std::size_t SIZE>
inline void RingBuffer<BuffType, SIZE>::reset()
{
	mHead = mTail = 0U;
}



template <class BuffType, std::size_t SIZE>
bool RingBuffer<BuffType, SIZE>::append(const BuffType val)
{
    if ( true == isFull() )
	{
    	return false;
	}

    // calculate the index instead of modulo operation -> but the size must be power of 2
    sig_atomic_t head_idx = mHead & (SIZE-1);

    //todo is that operations order safe?

    // append to the buffer
    mBuff[ head_idx ] = val ;

    // update index
    ++mHead;

    return true ;
}



template <class BuffType, std::size_t SIZE>
bool RingBuffer<BuffType, SIZE>::get(BuffType & ref)
{
    if ( true == isEmpty() )
    {
        return false ;
    }

    sig_atomic_t tail_idx = mTail & (SIZE - 1);
    ref = mBuff[tail_idx] ;

    ++mTail ;

    return true ;
}



template <class BuffType, std::size_t SIZE>
constexpr inline std::size_t RingBuffer<BuffType, SIZE>::size()
{
	// Implementation of this buffer has size -1
	return SIZE-1;
}




template <class BuffType, std::size_t SIZE>
inline bool RingBuffer<BuffType, SIZE>::isFull()
{
	// add 1 to handle this implementation of ringBuffer - always real size is SIZE - 1!
	sig_atomic_t head_idx = (mHead + 1) & (SIZE - 1);
	sig_atomic_t tail_idx = mTail & (SIZE - 1);

	return ( head_idx == tail_idx ) ? true : false;
}



template <class BuffType, std::size_t SIZE>
inline bool RingBuffer<BuffType, SIZE>::isEmpty()
{
	return ( mHead == mTail )? true : false ;
}



#endif /* D_RING_BUFFER_H */


