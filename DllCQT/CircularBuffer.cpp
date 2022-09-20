#include "CircularBuffer.h"

CircularBuffer::CircularBuffer(size_t bufferSize) {
	changeSize(bufferSize);
}

void CircularBuffer::changeSize(size_t bufferSize) {
	mBufferSize = nextPowOfTwo(bufferSize);
	mBufferSizeMinOne = mBufferSize - 1;
	mBuffer.resize(mBufferSize, static_cast<BufferType>(0.0));
	mWritePointer = 0;
	mReadPointer = 0;
}

void CircularBuffer::pushSample(const BufferType value) {
	mWritePointer += 1;
	mWritePointer = mWritePointer & mBufferSizeMinOne;
	mBuffer[mWritePointer] = value;
}

void CircularBuffer::pushBlock(const BufferType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mWritePointer += 1;
		mWritePointer = mWritePointer & mBufferSizeMinOne;
		mBuffer[mWritePointer] = data[i];
	}
}

BufferType CircularBuffer::pullDelaySample(const int delay) {
	auto tmp = (static_cast<int>(mWritePointer) - delay);
	size_t pos;
	if (tmp < 0) {
		pos = tmp + mBufferSize;
	} else {
		pos = tmp;
	}
	return mBuffer[pos];
}

void CircularBuffer::pullDelayBlock(BufferType* const data, const int delay, const int blockSize) {
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
		readPointer = readPointer & mBufferSizeMinOne;
	}
}

void CircularBuffer::modulateDelayBlock(const BufferType* const data, const int delay, const int blockSize) {
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
		readPointer = readPointer & mBufferSizeMinOne;
	}
}

BufferType CircularBuffer::pullSample() {
	mReadPointer += 1;
	mReadPointer = mReadPointer & mBufferSizeMinOne;
	return mBuffer[mReadPointer];
}

void CircularBuffer::pullBlock(BufferType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferSizeMinOne;
		data[i] = mBuffer[mReadPointer];
	}
}

void CircularBuffer::pullBlockAdd(BufferType* const data, const int blockSize) {
	for (int i = 0; i < blockSize; i++) {
		mReadPointer += 1;
		mReadPointer = mReadPointer & mBufferSizeMinOne;
		data[i] += mBuffer[mReadPointer];
	}
}

size_t CircularBuffer::getWriteReadDistance() {
	return  mWritePointer >= mReadPointer ? mWritePointer - mReadPointer : mBufferSize - mReadPointer + mWritePointer;
}
