#include "CircularBuffer.h"

template <typename T>
inline void CircularBuffer<T>::changeSize(size_t bufferSize)
{
	mBufferSize = nextPowOfTwo(bufferSize);
	mBufferSizeMinOne = mBufferSize - 1;
	mBuffer.resize(mBufferSize, static_cast<T>(0.));
	mWritePointer = 0;
	mReadPointer = 0;
}

template <typename T>
inline void CircularBuffer<T>::pushSample(const T value)
{
	mWritePointer += 1;
	mWritePointer = mWritePointer & mBufferSizeMinOne;
	mBuffer[mWritePointer] = value;
}

template <typename T>
inline void CircularBuffer<T>::pushBlock(const T* const data, const int blockSize)
{
	for (int i = 0; i < blockSize; i++)
	{
		mWritePointer += 1;
		mWritePointer = mWritePointer & mBufferSizeMinOne;
		mBuffer[mWritePointer] = data[i];
	}
}

template <typename T>
inline T CircularBuffer<T>::pullDelaySample(const int delay)
{
	int position = (static_cast<int>(mWritePointer) - delay);
	if (position < 0)
	{
		position += mBufferSize;
	}
	return mBuffer[position];
}

template <typename T>
inline void CircularBuffer<T>::pullDelayBlock(T* const data, const int delay, const int blockSize)
{
	int position = (static_cast<int>(mWritePointer) - delay);
	if (position < 0)
	{
		position += mBufferSize;
	}
	size_t readPointer = static_cast<size_t>(position);
	for (int i = 0; i < blockSize; i++)
	{
		data[i] = mBuffer[readPointer];
		readPointer += 1;
		readPointer = readPointer & mBufferSizeMinOne;
	}
}

template <typename T>
inline void CircularBuffer<T>::modulateDelayBlock(const T* const data, const int delay, const int blockSize)
{
	int position = (static_cast<int>(mWritePointer) - delay);
	if (position < 0)
	{
		position += mBufferSize;
	}
	size_t readPointer = static_cast<size_t>(position);
	for (int i = 0; i < blockSize; i++)
	{
		mBuffer[readPointer] *= data[i];
		readPointer += 1;
		readPointer = readPointer & mBufferSizeMinOne;
	}
}

template <typename T>
inline T CircularBuffer<T>::pullSample()
{
	mReadPointer += 1;
	mReadPointer = mReadPointer & mBufferSizeMinOne;
	return mBuffer[mReadPointer];
}

template <typename T>
inline void CircularBuffer<T>::pullBlock(T* const data, const int blockSize)
{
	for (int i = 0; i < blockSize; i++)
	{
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferSizeMinOne;
		data[i] = mBuffer[mReadPointer];
	}
}

template <typename T>
inline void CircularBuffer<T>::pullBlockAdd(T* const data, const int blockSize)
{
	for (int i = 0; i < blockSize; i++)
	{
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferSizeMinOne;
		data[i] += mBuffer[mReadPointer];
	}
}

template <typename T>
inline size_t CircularBuffer<T>::getWriteReadDistance()
{
	return  mWritePointer >= mReadPointer ? mWritePointer - mReadPointer : mBufferSize - mReadPointer + mWritePointer;
}
