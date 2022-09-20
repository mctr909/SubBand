#include "circular_buffer.h"
#include <string.h>

CircularBuffer::CircularBuffer(size_t bufferSize) {
	mWritePointer = 0;
	mReadPointer = 0;
	mBufferSize = nextPowOfTwo(bufferSize);
	mBufferLast = mBufferSize - 1;
	mBuffer = new FloatType[mBufferSize];
	memset(mBuffer, 0, sizeof(FloatType) * mBufferSize);
}

CircularBuffer::~CircularBuffer() {
	delete mBuffer;
}

void CircularBuffer::pushSample(const FloatType value) {
	mWritePointer += 1;
	mWritePointer = mWritePointer & mBufferLast;
	mBuffer[mWritePointer] = value;
}

void CircularBuffer::pushBlock(const FloatType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mWritePointer += 1;
		mWritePointer = mWritePointer & mBufferLast;
		mBuffer[mWritePointer] = data[i];
	}
}

FloatType CircularBuffer::pullDelaySample(const int delay) {
	auto tmp = (static_cast<int>(mWritePointer) - delay);
	size_t pos;
	if (tmp < 0) {
		pos = tmp + mBufferSize;
	} else {
		pos = tmp;
	}
	return mBuffer[pos];
}

void CircularBuffer::pullDelayBlock(FloatType* const data, const int delay, const int blockSize) {
	auto tmp = (static_cast<int>(mWritePointer) - delay);
	size_t pos;
	if (tmp < 0) {
		pos = tmp + mBufferSize;
	} else {
		pos = tmp;
	}
	auto readPointer = pos;
	for (int i = 0; i < blockSize; i++) {
		data[i] = mBuffer[readPointer];
		readPointer += 1;
		readPointer = readPointer & mBufferLast;
	}
}

void CircularBuffer::modulateDelayBlock(const FloatType* const data, const int delay, const int blockSize) {
	auto tmp = (static_cast<int>(mWritePointer) - delay);
	size_t pos;
	if (tmp < 0) {
		pos = tmp + mBufferSize;
	} else {
		pos = tmp;
	}
	auto readPointer = pos;
	for (int i = 0; i < blockSize; i++) {
		mBuffer[readPointer] *= data[i];
		readPointer += 1;
		readPointer = readPointer & mBufferLast;
	}
}

FloatType CircularBuffer::pullSample() {
	mReadPointer += 1;
	mReadPointer = mReadPointer & mBufferLast;
	return mBuffer[mReadPointer];
}

void CircularBuffer::pullBlock(FloatType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferLast;
		data[i] = mBuffer[mReadPointer];
	}
}

void CircularBuffer::pullBlockAdd(FloatType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferLast;
		data[i] += mBuffer[mReadPointer];
	}
}
