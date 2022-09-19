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

	class HalfBandLowpass {
	public:
		HalfBandLowpass(size_t allpassNumber);
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
		BufferType processSampleDown(const BufferType sample, bool& isSampleRet);
		/*
		Call with 2 * fs_original.
		*/
		BufferType processSampleUp(const BufferType sample);
		BufferType* processBlockDown(BufferType* const inputBlock, const int inputBlockSize);
		BufferType* processBlockUp(BufferType* const inputBlock, const int inputBlockSize);

	private:
		int AllpassNumber;
		bool mIsSample = false;
		double mTransitionBandwidth;
		size_t mAllpassNumberTotal;
		size_t mFilterOrder;
		std::vector<double> mCoefficients;

		std::vector<FirstOrderAllpass<double>> mDirectPathFilters; //[AllpassNumber];
		std::vector<FirstOrderAllpass<double>> mDelayPathFilters; //[AllpassNumber];
		Delay<double> mDelay;

		int mTargetBlockSize{ 0 };
		int mInputBlockSize{ 1 };
		int mFilterBufferSize{ 0 };

		double mDelayPathStorage = 0.;
		double mDelayPathInput = 0.;
		double mDirectPathInput = 0.;
		std::vector<double> mDirectPathBuffer;
		std::vector<double> mDelayPathBuffer;
		std::vector<BufferType> mOutputBlock;

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

	class ResamplingHandler {
	public:
		ResamplingHandler(int allpassNumber, double transitionBandwidth = 0.02);
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
		BufferType processSampleDown(BufferType sample, bool& isSampleRet);
		/*
		Called with target samplerate.
		*/
		BufferType processSampleUp(BufferType sample);
		BufferType* processBlockDown(BufferType* const  inputBlock);
		BufferType* processBlockUp(BufferType* const inputBlock);

	private:
		int AllpassNumber;
		double mTransitionBandwidth;
		int mPowTwoFactor;
		int mResamplingFactor;
		int mInputBlockSizeDown{ 0 };
		int mInputBlockSizeUp{ 0 };
		int mTargetBlockSizeUp{ 0 };
		int mTargetBlockSizeDown{ 0 };
		std::vector<HalfBandLowpass*> mDownFilters;
		std::vector<HalfBandLowpass*> mUpFilters;
		// sample based storage
		std::vector<bool> mIsSample;
		std::vector<double> mUpsamplingStorage;
	};
};
