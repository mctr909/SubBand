#pragma once
#include <vector>
#include <cmath>
#include "Utils.h"

class CircularBuffer
{
public:
	CircularBuffer(size_t bufferSize);
	~CircularBuffer();

	inline size_t nextPowOfTwo(size_t size) { return (int)std::pow(2, std::ceil(std::log(size) / std::log(2))); };
	inline void resetReadPointer() { mReadPointer = mWritePointer; };
	size_t getBufferSize() { return mBufferSize; };
	size_t getWriteReadDistance() {
		return  mWritePointer >= mReadPointer ? mWritePointer - mReadPointer : mBufferSize - mReadPointer + mWritePointer;
	}

	void pushSample(const FloatType value);
	void pushBlock(const FloatType* const data, const int blockSize);
	FloatType pullSample();
	void pullBlock(FloatType* const data, const int blockSize);
	void pullBlockAdd(FloatType* const data, const int blockSize);
	FloatType pullDelaySample(const int delay);
	void pullDelayBlock(FloatType* const data, const int delay, const int blockSize);
	void modulateDelayBlock(const FloatType* const data, const int delay, const int blockSize);

protected:
	size_t mWritePointer{ 0 };
	size_t mReadPointer{ 0 };
	FloatType* mBuffer = 0;
	size_t mBufferSize{ 0 };
	size_t mBufferLast{ 0 };
};
