#pragma once
#include <vector>
#include "Resampling.h"
#include "CircularBuffer.h"

namespace Cqt
{
    typedef CircularBuffer<double>* BufferPtr;

	template<int StageNumber>
	class ResamplingFilterbank {
	public:
		ResamplingFilterbank() = default;
		~ResamplingFilterbank() = default;

		double getOriginSamplerate() { return mOriginSamplerate; };
		BufferPtr getStageInputBuffer(const int stage) { return &mStageInputBuffers[stage]; };
		BufferPtr getStageOutputBuffer(const int stage) { return &mStageOutputBuffers[stage]; };
		int getOriginDownsampling() { return mOriginDownsampling; };

		void init(const double samplerate, const int blockSize, const int bufferSize);

		void inputBlock(double* const data, const int blockSize);
		double* outputBlock(const int blockSize);

	protected:
		ResamplingHandler<double, 3> mInputResamplingHandler;
		HalfBandLowpass<double, 3> mDownsamplingFilters[StageNumber - 1];
		HalfBandLowpass<double, 3> mUpsamplingFilters[StageNumber - 1];
		CircularBuffer<double> mStageInputBuffers[StageNumber];
		CircularBuffer<double> mStageOutputBuffers[StageNumber];

		double mOriginSamplerate;
		int mOriginBlockSize;
		int mOriginDownsampling;
		double mStageSamplerates[StageNumber];
		int mDownsamplingBlockSizes[StageNumber - 1];
		int mUpsamplingBlockSizes[StageNumber - 1];

		// block based
		int mBlockFilterNumber;

		// sample based
		int mSampleInputSize{ 0 };
		int mSampleFilterNumber;
		std::vector<std::vector<bool>> mIsSample;
		std::vector<double> mUpsamplingSampleBuffer;
		std::vector<double> mUpsamplingSampleOutputBuffer;

		// handling of input / output buffering
		CircularBuffer<double> mInputBuffer;
		CircularBuffer<double> mOutputBuffer;
		std::vector<double> mInputData;
		std::vector<double> mOutputData;
		size_t mInputDataCounter{ 0 };
		size_t mOutputDataCounter{ 0 };
		int mExpectedBlockSize{ 0 };
	};
};
