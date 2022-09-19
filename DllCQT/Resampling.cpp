#include "Resampling.h"

using namespace Cqt;

HalfBandLowpass::HalfBandLowpass(size_t allpassNumber)
{
	AllpassNumber = allpassNumber;
	mAllpassNumberTotal = AllpassNumber * 2;
	mFilterOrder = 2 * mAllpassNumberTotal + 1;
}

bool HalfBandLowpass::init(const int expectedBlockSize, bool isDownsampling, bool isSampleBased, double transitionBandwidth)
{
	// init filters
	mTransitionBandwidth = transitionBandwidth * 2. * Pi();
	mCoefficients.clear();
	mCoefficients = filterDesign();
	int filterCount = 0;
	for (size_t i = 0; i < mCoefficients.size(); i += 2)
	{
		mDirectPathFilters[filterCount].initCoeff(mCoefficients[i]);
		filterCount++;
	}
	filterCount = 0;
	for (size_t i = 1; i < mCoefficients.size(); i += 2)
	{
		mDelayPathFilters[filterCount].initCoeff(mCoefficients[i]);
		filterCount++;
	}
	// init buffers
	mDirectPathBuffer.clear();
	mDelayPathBuffer.clear();
	mOutputBlock.clear();
	if (!isSampleBased)
	{
		mInputBlockSize = expectedBlockSize;
		if (isDownsampling)
		{
			mFilterBufferSize = expectedBlockSize / 2;
			mTargetBlockSize = expectedBlockSize / 2;
		} else
		{
			mFilterBufferSize = expectedBlockSize;
			mTargetBlockSize = expectedBlockSize * 2;
		}
		mDirectPathBuffer.resize(mFilterBufferSize, static_cast<double>(0.));
		mDelayPathBuffer.resize(mFilterBufferSize, static_cast<double>(0.));
		mOutputBlock.resize(mTargetBlockSize, static_cast<double>(0.));
	}
	return true;
}

BufferType HalfBandLowpass::processSampleDown(const BufferType sample, bool& isSampleRet) {
	double output = 0.;
	mDelayPathInput = sample;
	mDirectPathInput = sample;
	if (mIsSample)
	{
		for (size_t i = 0; i < AllpassNumber; i++)
		{
			mDirectPathInput = mDirectPathFilters[i].processSample(mDirectPathInput);
			mDelayPathStorage = mDelayPathFilters[i].processSample(mDelayPathStorage);
		}
		output = (mDirectPathInput + mDelayPathStorage) * 0.5;
	}
	mDelayPathStorage = mDelayPathInput;
	isSampleRet = mIsSample;
	mIsSample = !mIsSample;
	return output;
};

BufferType HalfBandLowpass::processSampleUp(const BufferType sample) {
	double output = 0;
	if (mIsSample)
	{
		mDirectPathInput = sample;
		mDelayPathInput = sample;
		for (size_t i = 0; i < AllpassNumber; i++)
		{
			mDirectPathInput = mDirectPathFilters[i].processSample(mDirectPathInput);
		}
		output = mDirectPathInput;
	} else
	{
		for (size_t i = 0; i < AllpassNumber; i++)
		{
			mDelayPathInput = mDelayPathFilters[i].processSample(mDelayPathInput);
		}
		output = mDelayPathInput;
	}
	mIsSample = !mIsSample;
	return output;
};

BufferType* HalfBandLowpass::processBlockDown(BufferType* const inputBlock, const int inputBlockSize)
{
	int outCountDirect = 0;
	for (int i = 0; i < inputBlockSize; i += 2)
	{
		mDirectPathBuffer[outCountDirect] = inputBlock[i];
		outCountDirect++;
	}
	int outCountDelay = 0;
	for (int i = 1; i < inputBlockSize; i += 2)
	{
		mDelayPathBuffer[outCountDelay] = inputBlock[i];
		outCountDelay++;
	}
	mDelay.processBlock(mDelayPathBuffer.data(), mFilterBufferSize);
	for (size_t i = 0; i < AllpassNumber; i++)
	{
		mDirectPathFilters[i].processBlock(mDirectPathBuffer.data(), mFilterBufferSize);
		mDelayPathFilters[i].processBlock(mDelayPathBuffer.data(), mFilterBufferSize);
	}
	for (int i = 0; i < mTargetBlockSize; i++)
	{
		mOutputBlock[i] = static_cast<double>(0.5) * (mDirectPathBuffer[i] + mDelayPathBuffer[i]);
	}
	return mOutputBlock.data();
};

BufferType* HalfBandLowpass::processBlockUp(BufferType* const inputBlock, const int inputBlockSize)
{
	for (int i = 0; i < inputBlockSize; i++)
	{
		mDirectPathBuffer[i] = inputBlock[i];
		mDelayPathBuffer[i] = inputBlock[i];
	}
	for (size_t i = 0; i < AllpassNumber; i++)
	{
		mDirectPathFilters[i].processBlock(mDirectPathBuffer.data(), mFilterBufferSize);
		mDelayPathFilters[i].processBlock(mDelayPathBuffer.data(), mFilterBufferSize);
	}
	int inCountDirect = 0;
	for (int i = 0; i < mTargetBlockSize; i += 2)
	{
		mOutputBlock[i] = mDirectPathBuffer[inCountDirect];
		inCountDirect += 1;
	}
	int inCountDelay = 0;
	for (int i = 1; i < mTargetBlockSize; i += 2)
	{
		mOutputBlock[i] = mDelayPathBuffer[inCountDelay];
		inCountDelay += 1;
	}
	return mOutputBlock.data();
};

inline std::vector<double> HalfBandLowpass::filterDesign()
{
	// step 1
	const double k = std::pow(std::tan((Pi() - mTransitionBandwidth) / 4.), 2);
	const double k_dash = std::sqrt(1. - std::pow(k, 2));
	const double e = (1. / 2.) * ((1. - std::sqrt(k_dash)) / (1. + std::sqrt(k_dash)));
	const double q = e + 2. * std::pow(e, 5) + 15. * std::pow(e, 9.) + 150. * std::pow(e, 13.);
	// step 2
	const size_t n = mFilterOrder;
	// step 3
	const double q_1 = std::pow(q, n);
	const double k_1 = 4. * std::sqrt(q_1);
	const double d_s = std::sqrt((k_1) / (1. + k_1));
	const double d_p = 1. - std::sqrt(1. - std::pow(d_s, 2));
	// step 4
	std::vector<double> w;
	std::vector<double> a_dash;
	for (size_t i = 1; i <= ((n - 1) / 2); i++)
	{
		// w_i
		double delta = 1.;
		double num = 0.;
		double m = 0.;
		while (delta > 1.e-100)
		{
			delta = std::pow((-1.), m) * std::pow(q, (m * (m + 1.))) * std::sin((2. * m + 1.) * Pi() * static_cast<double>(i) / static_cast<double>(n));
			num += delta;
			m += 1.;
		}
		num = 2. * std::pow(q, (1. / 4.)) * num;
		delta = 1.;
		double den = 0.;
		m = 1.;
		while (delta > 1.e-100)
		{
			delta = std::pow((-1.), m) * std::pow(q, std::pow(m, 2.)) * std::cos(2. * m * Pi() * static_cast<double>(i) / static_cast<double>(n));
			den += delta;
			m += 1.;
		}
		den = den * 2. + 1.;
		double w_i = num / den;
		w.push_back(w_i);
		// a'_i
		num = std::pow(((1. - std::pow(w_i, 2.) * k) * (1. - std::pow(w_i, 2.) / k)), (1. / 2.));
		den = 1. + std::pow(w_i, 2.);
		double a_dash_i = num / den;
		a_dash.push_back(a_dash_i);
	}
	// step 5
	std::vector<double> a;
	for (double a_dash_i : a_dash)
	{
		double a_i = (1. - a_dash_i) / (1. + a_dash_i);
		a.push_back(a_i);
	}
	return a;
};

ResamplingHandler::ResamplingHandler(int allpassNumber, double transitionBandwidth)
{
	AllpassNumber = allpassNumber;
	mTransitionBandwidth = transitionBandwidth;
}

ResamplingHandler::~ResamplingHandler()
{
	for (auto filter : mDownFilters)
	{
		delete filter;
	}
	for (auto filter : mUpFilters)
	{
		delete filter;
	}
}

void ResamplingHandler::init(const int powToExponent, const int expectedBlockSize, ProcessConfig sampleConfig, DirectionConfig directionConfig)
{
	for (auto filter : mDownFilters)
	{
		delete filter;
	}
	for (auto filter : mUpFilters)
	{
		delete filter;
	}
	mDownFilters.clear();
	mUpFilters.clear();
	mPowTwoFactor = powToExponent;
	mResamplingFactor = (int)std::pow(2, powToExponent);
	for (int i = 0; i < mPowTwoFactor; i++)
	{
		if (sampleConfig == ProcessConfig::Sample)
		{
			mIsSample.push_back(false);
			mUpsamplingStorage.push_back(0.);
			mDownFilters.push_back(new HalfBandLowpass(AllpassNumber));
			mUpFilters.push_back(new HalfBandLowpass(AllpassNumber));
			mDownFilters[i]->init(1, true, true, mTransitionBandwidth);
			mUpFilters[i]->init(1, false, true, mTransitionBandwidth);
		} else if (sampleConfig == ProcessConfig::Block)
		{
			if (directionConfig == DirectionConfig::Down)
			{
				mDownFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mDownFilters[i]->init(expectedBlockSize / (int)std::pow(2, i), true, false, mTransitionBandwidth);
				mInputBlockSizeDown = expectedBlockSize;
			} else if (directionConfig == DirectionConfig::Up)
			{
				mUpFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mUpFilters[i]->init(expectedBlockSize * (int)std::pow(2, i), false, false, mTransitionBandwidth);
				mInputBlockSizeUp = expectedBlockSize;
			} else if (directionConfig == DirectionConfig::DownUp)
			{
				mDownFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mUpFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mDownFilters[i]->init(expectedBlockSize / (int)std::pow(2, i), true, false, mTransitionBandwidth);
				mUpFilters[i]->init(expectedBlockSize / (int)std::pow(2, mPowTwoFactor - i), false, false, mTransitionBandwidth);
				mInputBlockSizeDown = expectedBlockSize;
				mInputBlockSizeUp = expectedBlockSize / (int)std::pow(2, mPowTwoFactor);
			} else if (directionConfig == DirectionConfig::UpDown)
			{
				mDownFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mUpFilters.push_back(new HalfBandLowpass(AllpassNumber));
				mUpFilters[i]->init(expectedBlockSize * (int)std::pow(2, i), false, false, mTransitionBandwidth);
				mDownFilters[i]->init(expectedBlockSize * (int)std::pow(2, mPowTwoFactor - i), true, false, mTransitionBandwidth);
				mInputBlockSizeDown = expectedBlockSize * (int)std::pow(2, mPowTwoFactor);
				mInputBlockSizeUp = expectedBlockSize;
			}
		}
	}
	if (sampleConfig == ProcessConfig::Block)
	{
		if (mDownFilters.size() > 0)
		{
			mTargetBlockSizeDown = mDownFilters[mPowTwoFactor - 1]->getOutputBlockSize();

		} else
		{
			mTargetBlockSizeDown = expectedBlockSize;
		}
		if (mUpFilters.size() > 0)
		{
			mTargetBlockSizeUp = mUpFilters[mPowTwoFactor - 1]->getOutputBlockSize();
		} else
		{
			mTargetBlockSizeUp = expectedBlockSize;
		}
	}
};

BufferType ResamplingHandler::processSampleDown(BufferType sample, bool& isSampleRet) {
	if (mPowTwoFactor > 0) {
		for (int i = 0; i < mPowTwoFactor; i++)
		{
			mIsSample[i] = false;
		}
		for (int i = 0; i < mPowTwoFactor; i++)
		{
			bool isSampleTmp = mIsSample[i];
			sample = mDownFilters[i]->processSampleDown(sample, isSampleTmp);
			mIsSample[i] = isSampleTmp;
			if (!mIsSample[i])
			{
				break;
			}
		}
		isSampleRet = mIsSample[mPowTwoFactor - 1];
		return sample;
	} else
	{
		isSampleRet = true;
		return sample;
	}
};

BufferType ResamplingHandler::processSampleUp(BufferType sample) {
	if (mPowTwoFactor > 0) {
		if (mIsSample[mPowTwoFactor - 1])
		{
			mUpsamplingStorage[mPowTwoFactor - 1] = sample;
		}
		for (int i = (mPowTwoFactor - 1); i > 0; i -= 1)
		{
			if (mIsSample[i - 1])
			{
				mUpsamplingStorage[i - 1] = mUpFilters[i]->processSampleUp(mUpsamplingStorage[i]);
			}
		}
		sample = mUpFilters[0]->processSampleUp(mUpsamplingStorage[0]);
		return sample;
	} else
	{
		return sample;
	}
};

BufferType* ResamplingHandler::processBlockDown(BufferType* const  inputBlock)
{
	auto inBlock = inputBlock;
	auto outBlock = inputBlock;
	int inputBlockSize = mInputBlockSizeDown;
	for (int i = 0; i < mPowTwoFactor; i++)
	{
		outBlock = mDownFilters[i]->processBlockDown(inBlock, inputBlockSize);
		inputBlockSize = mDownFilters[i]->getOutputBlockSize();
		inBlock = outBlock;
	}
	return outBlock;
};

BufferType* ResamplingHandler::processBlockUp(BufferType* const  inputBlock)
{
	auto inBlock = inputBlock;
	auto outBlock = inputBlock;
	int inputBlockSize = mInputBlockSizeUp;
	for (int i = 0; i < mPowTwoFactor; i++)
	{
		outBlock = mUpFilters[i]->processBlockUp(inBlock, inputBlockSize);
		inputBlockSize = mUpFilters[i]->getOutputBlockSize();
		inBlock = outBlock;
	}
	return outBlock;
};
