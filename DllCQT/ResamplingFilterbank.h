#pragma once
#include <vector>
#include "Resampling.h"
#include "CircularBuffer.h"

namespace Cqt
{
    typedef CircularBuffer* BufferPtr;

	class ResamplingFilterbank {
	public:
		ResamplingFilterbank(int stageNumber);
		~ResamplingFilterbank() = default;

		double getOriginSamplerate() { return mOriginSamplerate; };
		BufferPtr getStageInputBuffer(const int stage) { return &mStageInputBuffers[stage]; };
		BufferPtr getStageOutputBuffer(const int stage) { return &mStageOutputBuffers[stage]; };
		int getOriginDownsampling() { return mOriginDownsampling; };

		void init(const double samplerate, const int blockSize, const int bufferSize);

		void inputBlock(BufferType* const data, const int blockSize);
		BufferType* outputBlock(const int blockSize);

	protected:
		ResamplingHandler mInputResamplingHandler;
		std::vector<HalfBandLowpass> mDownsamplingFilters;
		std::vector<int> mDownsamplingBlockSizes;
		std::vector<HalfBandLowpass> mUpsamplingFilters;
		std::vector<int> mUpsamplingBlockSizes;
		std::vector<CircularBuffer> mStageInputBuffers;
		std::vector<CircularBuffer> mStageOutputBuffers;
		std::vector<double> mStageSamplerates;

		int StageNumber;
		double mOriginSamplerate;
		int mOriginBlockSize;
		int mOriginDownsampling;

		// block based
		int mBlockFilterNumber;

		// sample based
		int mSampleInputSize{ 0 };
		int mSampleFilterNumber;
		std::vector<std::vector<bool>> mIsSample;
		std::vector<BufferType> mUpsamplingSampleBuffer;
		std::vector<BufferType> mUpsamplingSampleOutputBuffer;

		// handling of input / output buffering
		CircularBuffer mInputBuffer;
		CircularBuffer mOutputBuffer;
		std::vector<BufferType> mInputData;
		std::vector<BufferType> mOutputData;
		size_t mInputDataCounter{ 0 };
		size_t mOutputDataCounter{ 0 };
		int mExpectedBlockSize{ 0 };
	};
};
