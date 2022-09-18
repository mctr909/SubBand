#pragma once
#include <vector>
#include <cmath>

template <typename T>
class CircularBuffer
{
public:
	CircularBuffer(size_t bufferSize = 128) { changeSize(bufferSize); };
	~CircularBuffer() = default;

	inline size_t nextPowOfTwo(size_t size) { return std::pow(2, std::ceil(std::log(size) / std::log(2))); };
	inline void resetReadPointer() { mReadPointer = mWritePointer; };
	size_t getBufferSize() { return mBufferSize; };

	void changeSize(size_t bufferSize);
	void pushSample(const T value);
	void pushBlock(const T* const data, const int blockSize);
	T pullSample();
	void pullBlock(T* const data, const int blockSize);
	void pullBlockAdd(T* const data, const int blockSize);
	T pullDelaySample(const int delay);
	void pullDelayBlock(T* const data, const int delay, const int blockSize);
	void modulateDelayBlock(const T* const data, const int delay, const int blockSize);
	size_t getWriteReadDistance();

protected:
	std::vector<T> mBuffer;
	size_t mBufferSize{ 0 };
	size_t mBufferSizeMinOne{ 0 };
	size_t mWritePointer{ 0 };
	size_t mReadPointer{ 0 };
};
