#pragma once
#include "Utils.h"
#include <cmath>
#include <vector>

namespace Cqt {
	template<typename FloatType>
	class Delay {
	public:
		Delay() = default;
		~Delay() = default;

		inline void processBlock(FloatType* const data, const int blockSize)
		{
			for (int i = 0; i < blockSize; i++)
			{
				const FloatType input = data[i];
				data[i] = mStorage;
				mStorage = input;
			}
		}
	private:
		FloatType mStorage{ 0. };
	};

	template <typename FloatType>
	class FirstOrderAllpass {
	public:
		FirstOrderAllpass() = default;
		~FirstOrderAllpass() = default;

		inline void initCoeff(FloatType ak)
		{
			mAk = static_cast<FloatType>(ak);
		};

		inline FloatType processSample(const FloatType sample)
		{
			const FloatType y = mAk * (sample - mYm1) + mXm1;
			mXm1 = sample;
			mYm1 = y;
			return y;
		};

		inline void processBlock(FloatType* const samples, const int blocksize)
		{
			for (int i = 0; i < blocksize; i++)
			{
				const FloatType sample = samples[i];
				samples[i] = mAk * (sample - mYm1) + mXm1;
				mXm1 = sample;
				mYm1 = samples[i];
			}
		};
	private:
		FloatType mAk{ 0. };
		FloatType mXm1{ 0. };
		FloatType mYm1{ 0. };
	};

	template <typename FloatType, size_t AllpassNumber>
	class HalfBandLowpass {
	public:
		HalfBandLowpass();
		~HalfBandLowpass() = default;

		int getOutputBlockSize() { return mTargetBlockSize; };
		int getInputBlockSize() { return mInputBlockSize; };

		/*
		Initialize the filters and allocate memory. Has to get called before processing starts.
		*/
		bool init(const int expectedBlockSize = 128, bool isDownsampling = true, bool isSampleBased = false, double transitionBandwidth = 0.02);
		/*
		Call with 2 * fs_target.
		*/
		FloatType processSampleDown(const FloatType sample, bool& isSampleRet);
		/*
		Call with 2 * fs_original.
		*/
		FloatType processSampleUp(const FloatType sample);
		FloatType* processBlockDown(FloatType* const inputBlock, const int inputBlockSize);
		FloatType* processBlockUp(FloatType* const inputBlock, const int inputBlockSize);

	private:
		bool mIsSample = false;
		double mTransitionBandwidth;
		size_t mAllpassNumberTotal;
		size_t mFilterOrder;
		std::vector<double> mCoefficients;

		FirstOrderAllpass<FloatType> mDirectPathFilters[AllpassNumber];
		FirstOrderAllpass<FloatType> mDelayPathFilters[AllpassNumber];
		Delay<FloatType> mDelay;

		int mTargetBlockSize{ 0 };
		int mInputBlockSize{ 1 };
		int mFilterBufferSize{ 0 };

		FloatType mDelayPathStorage = 0.;
		FloatType mDelayPathInput = 0.;
		FloatType mDirectPathInput = 0.;
		std::vector<FloatType> mDirectPathBuffer;
		std::vector<FloatType> mDelayPathBuffer;
		std::vector<FloatType> mOutputBlock;

		std::vector<double> filterDesign();
	};

	enum class ProcessConfig
	{
		Sample = 0,
		Block
	};
	enum class DirectionConfig
	{
		Up = 0,
		Down,
		UpDown,
		DownUp
	};

	template <typename FloatType, size_t AllpassNumber>
	class ResamplingHandler {
	public:
		ResamplingHandler(double transitionBandwidth = 0.02);
		~ResamplingHandler();

		int getOutputBlockSizeUp() { return mTargetBlockSizeUp; };
		int getOutputBlockSizeDown() { return mTargetBlockSizeDown; };

		/**
		Initialization of the ResamplingHandler
		The configuration gives the amount of memory that will be allocated.
		DirectionConfig has to match the order the processing functions will be called from outside.
		*/
		void init(const int powToExponent = 0, const int expectedBlockSize = 128, ProcessConfig sampleConfig = ProcessConfig::Sample, DirectionConfig directionConfig = DirectionConfig::UpDown);
		/*
		Called with 2*fs target. isSampleRet indicates whether sample is relevant.
		*/
		FloatType processSampleDown(FloatType sample, bool& isSampleRet);
		/*
		Called with target samplerate.
		*/
		FloatType processSampleUp(FloatType sample);
		FloatType* processBlockDown(FloatType* const  inputBlock);
		FloatType* processBlockUp(FloatType* const inputBlock);

	private:
		double mTransitionBandwidth;
		int mPowTwoFactor;
		int mResamplingFactor;
		int mInputBlockSizeDown{ 0 };
		int mInputBlockSizeUp{ 0 };
		int mTargetBlockSizeUp{ 0 };
		int mTargetBlockSizeDown{ 0 };
		std::vector<HalfBandLowpass<FloatType, AllpassNumber>*> mDownFilters;
		std::vector<HalfBandLowpass<FloatType, AllpassNumber>*> mUpFilters;
		// sample based storage
		std::vector<bool> mIsSample;
		std::vector<FloatType> mUpsamplingStorage;
	};
};
