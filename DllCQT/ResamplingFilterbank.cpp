#include "ResamplingFilterbank.h"

using namespace Cqt;

ResamplingFilterbank::ResamplingFilterbank(int stageNumber)
	: mInputResamplingHandler(stageNumber) {
	StageNumber = stageNumber;

	for (int i = 0; i < StageNumber - 1; i++) {
		mDownsamplingFilters.push_back(HalfBandLowpass(3));
		mDownsamplingBlockSizes.push_back(0);
		mUpsamplingFilters.push_back(HalfBandLowpass(3));
		mUpsamplingBlockSizes.push_back(0);
	}
	for (int i = 0; i < StageNumber; i++) {
		mStageInputBuffers.push_back(CircularBuffer());
		mStageOutputBuffers.push_back(CircularBuffer());
		mStageSamplerates.push_back(0.0);
	}
}

void ResamplingFilterbank::init(const double samplerate, const int blockSize, const int bufferSize) {
	// handling of input / output buffering
	mInputBuffer.changeSize(blockSize * 2);
	mOutputBuffer.changeSize(blockSize * 2);
	mInputData.resize(blockSize, 0.);
	mOutputData.resize(blockSize, 0.);
	mInputDataCounter = 0;
	mOutputDataCounter = blockSize;
	mExpectedBlockSize = blockSize;

	mOriginDownsampling = 0;
	int fsInt = static_cast<int>(samplerate);

	//assert(((fsInt % 44100) == 0) || ((fsInt % 48000) == 0));
	if ((fsInt % 44100) == 0) {
		mOriginDownsampling = (int)std::log2(fsInt / 44100);
		mOriginSamplerate = 44100.;
	} else if ((fsInt % 48000) == 0) {
		mOriginDownsampling = (int)std::log2(fsInt / 48000);
		mOriginSamplerate = 48000.;
	} else {
		mOriginDownsampling = (int)std::log2(fsInt / 44100);
		mOriginSamplerate = 44100.;
	}

	// init origin downsampling
	mInputResamplingHandler.init(mOriginDownsampling, blockSize, ProcessConfig::Block, DirectionConfig::DownUp);
	mOriginBlockSize = mInputResamplingHandler.getOutputBlockSizeDown();
	for (int i = 0; i < StageNumber; i++) {
		mStageSamplerates[i] = mOriginSamplerate / pow(2., i);
	}

	// configure internal stages
	for (int i = 0; i < (StageNumber - 1); i++) {
		mDownsamplingBlockSizes[i] = 0;
		mUpsamplingBlockSizes[i] = 0;
	}
	int originBlockSize = mOriginBlockSize;
	mBlockFilterNumber = 0;
	mSampleFilterNumber = 0;
	while ((originBlockSize > 1) && (mBlockFilterNumber < (StageNumber - 1)) && ((originBlockSize % 2) == 0)) {
		mDownsamplingBlockSizes[mBlockFilterNumber] = originBlockSize;
		originBlockSize /= 2;
		mUpsamplingBlockSizes[mBlockFilterNumber] = originBlockSize;
		mBlockFilterNumber++;
	}
	// init Filters
	for (int stage = 0; stage < (StageNumber - 1); stage++) {
		if (stage < mBlockFilterNumber) {
			mDownsamplingFilters[stage].init(mOriginBlockSize / (int)std::pow(2, stage), true, false, 0.02);
			mUpsamplingFilters[stage].init(mOriginBlockSize / (int)std::pow(2, stage + 1), false, false, 0.02);
		} else {
			mDownsamplingFilters[stage].init(mOriginBlockSize / (int)std::pow(2, stage), true, true, 0.02);
			mUpsamplingFilters[stage].init(mOriginBlockSize / (int)std::pow(2, stage + 1), false, true, 0.02);
		}
	}
	// buffers for samplebased / block based intermediate processing
	mIsSample.clear();
	mUpsamplingSampleBuffer.clear();
	if ((StageNumber - 1) > mBlockFilterNumber) {
		mSampleInputSize = originBlockSize;
		mUpsamplingSampleOutputBuffer.resize(originBlockSize, 0.);
		mSampleFilterNumber = (StageNumber - 1) - mBlockFilterNumber;
		mUpsamplingSampleBuffer.resize(mSampleFilterNumber, 0.);
		mIsSample.resize(originBlockSize);
		for (int i = 0; i < originBlockSize; i++) {
			mIsSample[i].resize(mSampleFilterNumber, false);
		}
	} else {
		mUpsamplingSampleOutputBuffer.resize(mUpsamplingFilters[mBlockFilterNumber - 1].getInputBlockSize(), 0.);
		mSampleInputSize = 0;
	}
	// init Buffers
	for (int stage = 0; stage < StageNumber; stage++) {
		mStageInputBuffers[stage].changeSize(bufferSize);
		mStageOutputBuffers[stage].changeSize(bufferSize);
	}
}

void ResamplingFilterbank::inputBlock(BufferType* const data, const int blockSize) {
	mInputBuffer.pushBlock(data, blockSize);
	mInputDataCounter += blockSize;
	if (mInputDataCounter >= mExpectedBlockSize) {
		mInputDataCounter -= mExpectedBlockSize;
		mInputBuffer.pullDelayBlock(mInputData.data(), mExpectedBlockSize + mInputDataCounter - 1, mExpectedBlockSize);
		// Filterbank
		int stageIdx = 0;
		int filterIdx = 0;
		// input Resampler to FsOrigin
		auto dataIn = mInputResamplingHandler.processBlockDown(mInputData.data());
		int blockSize = mInputResamplingHandler.getOutputBlockSizeDown();
		// save origin data
		mStageInputBuffers[stageIdx].pushBlock(dataIn, blockSize);
		stageIdx++;
		// process block based stages
		for (int i = 0; i < mBlockFilterNumber; i++) {
			dataIn = mDownsamplingFilters[filterIdx].processBlockDown(dataIn, blockSize);
			blockSize = mDownsamplingFilters[filterIdx].getOutputBlockSize();
			// save stage data
			mStageInputBuffers[stageIdx].pushBlock(dataIn, blockSize);
			stageIdx++;
			filterIdx++;
		}

		// handle sample based stages, where block based is not possible anymore
		int stageIdxSave = stageIdx;
		int filterIdxSave = filterIdx;
		for (int s = 0; s < mSampleInputSize; s++) {
			stageIdx = stageIdxSave;
			filterIdx = filterIdxSave;
			double sampleIn = dataIn[s];
			std::fill(mIsSample[s].begin(), mIsSample[s].end(), false);
			for (int i = 0; i < mSampleFilterNumber; i++) {
				bool isSampleTmp = mIsSample[s][i];
				sampleIn = mDownsamplingFilters[filterIdx].processSampleDown(sampleIn, isSampleTmp);
				mIsSample[s][i] = isSampleTmp;
				if (mIsSample[s][i]) {
					mStageInputBuffers[stageIdx].pushSample(sampleIn);
				}
				if (!mIsSample[s][i]) {
					break;
				}
				filterIdx++;
				stageIdx++;
			}
		}
	}
}

BufferType* ResamplingFilterbank::outputBlock(const int blockSize) {
	mOutputDataCounter += blockSize;
	if (mOutputDataCounter >= mExpectedBlockSize) {
		mOutputDataCounter -= mExpectedBlockSize;
		//Filterbank
		int stageIdx = StageNumber - 1;
		int filterIdx = StageNumber - 2;
		// handle sample based stages
		for (int s = 0; s < mSampleInputSize; s++) {
			stageIdx = StageNumber - 1;
			filterIdx = StageNumber - 2;
			// lowest Stage
			if (mSampleFilterNumber > 0) {
				if (mIsSample[s][(mSampleFilterNumber - 1)]) {
					mUpsamplingSampleBuffer[(mSampleFilterNumber - 1)] = mStageOutputBuffers[stageIdx].pullSample();
				}
				stageIdx--;
			}
			// rest of stages
			for (int i = (mSampleFilterNumber - 2); i >= 0; i -= 1) {
				if (mIsSample[s][i]) {
					// pull own sample and store - add together with upsampled lower level
					mUpsamplingSampleBuffer[i] = mStageOutputBuffers[stageIdx].pullSample() + mUpsamplingFilters[filterIdx].processSampleUp(mUpsamplingSampleBuffer[i + 1]);
				}
				stageIdx--;
				filterIdx--;
			}
			// highest sample based stage
			if (mSampleFilterNumber > 0) {
				mUpsamplingSampleOutputBuffer[s] = mStageOutputBuffers[stageIdx].pullSample() + mUpsamplingFilters[filterIdx].processSampleUp(mUpsamplingSampleBuffer[0]);
				stageIdx--;
				filterIdx--;
			}
		}

		// get output of last sample based stage
		int inputBlockSize = mUpsamplingFilters[filterIdx].getInputBlockSize();
		auto inBlock = mUpsamplingSampleOutputBuffer.data();
		if (mSampleFilterNumber <= 0) {
			mStageOutputBuffers[stageIdx].pullBlock(mUpsamplingSampleOutputBuffer.data(), inputBlockSize);
			stageIdx--;
		}
		// handle block based stages 
		for (int i = 0; i < mBlockFilterNumber; i++) {
			inBlock = mUpsamplingFilters[filterIdx].processBlockUp(inBlock, inputBlockSize);
			inputBlockSize = mUpsamplingFilters[filterIdx].getOutputBlockSize();
			filterIdx--;
			// combine upsampled data
			mStageOutputBuffers[stageIdx].pullBlockAdd(inBlock, inputBlockSize);
			stageIdx--;
		}

		// input Resampler to FsOrigin
		auto dataOut = mInputResamplingHandler.processBlockUp(inBlock);
		// store output
		mOutputBuffer.pushBlock(dataOut, mExpectedBlockSize);
	}
	mOutputBuffer.pullBlock(mOutputData.data(), blockSize);
	return mOutputData.data();
}
