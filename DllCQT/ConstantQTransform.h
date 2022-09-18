#pragma once
#include "CircularBuffer.h"
#include "ResamplingFilterbank.h"
#include "FFT.h"
#include "Utils.h"
#include <atomic>
#include <memory>
#include <complex>

#define SIMD_SZ 1
#define PFFFT_ENABLE_DOUBLE

namespace Cqt
{
    using namespace std::complex_literals;
    constexpr int Fft_Size{ 512 };
    constexpr int Fft_Domain_Size{ Fft_Size / 2 };
    constexpr double KernelThreshold{ 1.e-4 };
    constexpr double WindowEnergyLossCompensation{ 1.63 }; // Fixed for Hanning window as of now
    constexpr double WindowAmplitudeLossCompensation{ 2.0 }; // Fixed for Hanning window as of now

    typedef std::vector<std::complex<double>> CplxVector;
    typedef std::vector<double> RealVector;
    typedef RealVector TimeDataType;
    typedef CplxVector CqtBufferType;

	class FFT;

    /*
    Structure to schedule transformation timings.
    */
    struct ScheduleElement
    {
        int sample{ 0 }; // Position in actual block
        int octave{ 0 };
        int delayOctaveRate{ 0 }; // Delay of the transformation in samplesin the corresponding octave sample buffer
    };

	/*
	Handles the transformations of the single octaves.
	*/
	template <int B>
	class TransformationHandler {
	public:
		TransformationHandler();
		~TransformationHandler() = default;

		inline CqtBufferType* getCqtBuffer() { return &mCqtBuffer; };
		inline BufferPtr getOutputBuffer() { return mStageOutputBuffer; };

		void init(const int hopSize);
		void initBuffers(BufferPtr inputBuffer = nullptr, BufferPtr outputBuffer = nullptr);
		void initKernels(const CplxVector* const kernelArray, const CplxVector* const kernelArrayInverse, const std::vector<int>* const kernelMask, const std::vector<int>* const kernelMaskInv);
		void initFs(const int blockSize);

		void cqtTransform(const ScheduleElement schedule);
		void icqtTransform(const ScheduleElement schedule);

		static void calculateWindow(double* const windowData, const int size);
		static void calculateInverseWindow(double* const windowData, double* const invWindowData, const int size, const int hopSize);

	private:
		double mWindow[Fft_Size];
		double mInvWindow[Fft_Size];
		// kernel storage
		CplxVector mKernelArray[B]; // mB x Fft_Domain_Size
		CplxVector mKernelArrayInverse[B]; // mB x Fft_Domain_Size
		std::vector<int> mKernelMask[B]; // mB x Fft_Domain_Size
		std::vector<int> mKernelMaskInv[B]; // mB x Fft_Domain_Size

		// scaling 
		const double mFftScalingFactor{ 1. / std::sqrt(static_cast<double>(Fft_Size)) };

		FFT mFft;
		FFT mFftInv;
		RealVector mFftInputBuffer;
		CplxVector mFftBuffer;
		CplxVector mIfftInputBuffer;
		CqtBufferType mCqtBuffer; // mB
		RealVector mOutputBuffer;
		RealVector mIfftOutputBuffer;

		// input / output buffers
		BufferPtr mStageInputBuffer;
		BufferPtr mStageOutputBuffer;
	};

	/*
	Main CQT class
	*/
	template <int B, int OctaveNumber>
	class ConstantQTransform {
	public:
	 	ConstantQTransform();
		~ConstantQTransform() = default;

		inline std::vector<ScheduleElement>& getCqtSchedule() { return mCqtSchedule; };
		inline CqtBufferType* getOctaveCqtBuffer(const int octave) { return mTransformationHandlers[octave].getCqtBuffer(); };
		inline BufferPtr getOctaveOutputBuffer(const int octave) { return mTransformationHandlers[octave].getOutputBuffer(); };
		inline int getHopSize(const int octave) { return mHopSizes[octave]; };
		inline size_t getLatencySamples(const int octave) { return mLatencySamples[octave]; };
		inline double getLatencyMs(const int octave) { return mLatencyMs[octave]; };
		inline double getOctaveSampleRate(const int octave) { return mSampleRates[octave]; };
		inline std::vector<std::vector<double>>& getKernelFreqs() { return mKernelFreqs; };
		inline std::vector<std::vector<double>>& getKernelFreqsInv() { return mKernelFreqsInv; };
		inline void resetKernelFreqs() { initKernelFreqs(); };
		inline void recalculateKernels() { mNewKernels.store(true); };

		void init(int hopSize);
		void init(std::vector<int> octaveHopSizes);
		void initFs(double fs, const int blockSize);
		void setConcertPitch(double concertPitch);

		void inputBlock(double* const data, const int blockSize);
		double* outputBlock(const int blockSize);
		void cqt(const ScheduleElement schedule);
		void icqt(const ScheduleElement schedule);

	protected:
		void initKernelFreqs();
		void calculateKernels();

		double mConcertPitch{ 440. };
		int mBinNumber;
		int mOverlaps[OctaveNumber];
		double mLatencyMs[OctaveNumber];
		size_t mLatencySamples[OctaveNumber];
		int mHopSizes[OctaveNumber];
		double mFs;
		double mSampleRates[OctaveNumber];
		double mSampleRatesByOriginRate[OctaveNumber];

		TransformationHandler<B> mTransformationHandlers[OctaveNumber];
		ResamplingFilterbank<OctaveNumber> mFilterbank;
		size_t mSampleCounters[OctaveNumber];

		std::vector<ScheduleElement> mCqtSchedule;

		FFT mFft;
		CplxVector mFftTmpStorage;
		CplxVector mFftTmpStorageInv;

		CplxVector mKernelStorage[B];
		CplxVector mKernelStorageInv[B];
		std::vector<int> mKernelMask[B];
		std::vector<int> mKernelMaskInv[B];
		CplxVector mKernelStorageTime[B];
		CplxVector mKernelStorageTimeInv[B];
		RealVector window;
		std::atomic<bool> mNewKernels{ true };

		std::vector<std::vector<double>> mKernelFreqs;
		std::vector<std::vector<double>> mKernelFreqsInv;
	};
};
