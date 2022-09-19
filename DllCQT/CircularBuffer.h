#pragma once
#include <vector>
#include <cmath>
#include "Utils.h"

using namespace Cqt;

class CircularBuffer
{
public:
	CircularBuffer(size_t bufferSize = 128);
	~CircularBuffer() = default;

	inline size_t nextPowOfTwo(size_t size) { return (int)std::pow(2, std::ceil(std::log(size) / std::log(2))); };
	inline void resetReadPointer() { mReadPointer = mWritePointer; };
	size_t getBufferSize() { return mBufferSize; };

	void changeSize(size_t bufferSize);
	void pushSample(const BufferType value);
	void pushBlock(const BufferType* const data, const int blockSize);
	BufferType pullSample();
	void pullBlock(BufferType* const data, const int blockSize);
	void pullBlockAdd(BufferType* const data, const int blockSize);
	BufferType pullDelaySample(const int delay);
	void pullDelayBlock(BufferType* const data, const int delay, const int blockSize);
	void modulateDelayBlock(const BufferType* const data, const int delay, const int blockSize);
	size_t getWriteReadDistance();

protected:
	std::vector<BufferType> mBuffer;
	size_t mBufferSize{ 0 };
	size_t mBufferSizeMinOne{ 0 };
	size_t mWritePointer{ 0 };
	size_t mReadPointer{ 0 };
};
