/*
 * BufferInterface.h
 *
 *  Created on: 29 mar 2016
 *      Author: scott
 */

#ifndef HDR_BUFFERINTERFACE_H_
#define HDR_BUFFERINTERFACE_H_

#include <cstdint>
#include <cstddef>

template<class BuffType, std::size_t SIZE>
class BufferInterface
{
public:
    BufferInterface() {}
    virtual ~BufferInterface() {}

    virtual void reset() = 0;
    virtual bool append(const BuffType val) = 0;
    virtual bool get(BuffType & ref) = 0;
    virtual constexpr inline std::size_t size() = 0;
    virtual bool isFull() = 0;
    virtual bool isEmpty() = 0;
};




#endif /* HDR_BUFFERINTERFACE_H_ */
