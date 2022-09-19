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

    typedef std::vector<std::complex<BufferType>> CplxVector;
    typedef std::vector<BufferType> RealVector;
    typedef RealVector TimeDataType;
    typedef CplxVector CqtBufferType;

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
	class TransformationHandler {
	public:
		TransformationHandler(int b);
		~TransformationHandler() = default;

		inline CqtBufferType* getCqtBuffer() { return &mCqtBuffer; };
		inline BufferPtr getOutputBuffer() { return mStageOutputBuffer; };

		void init(const int hopSize);
		void initBuffers(BufferPtr inputBuffer = nullptr, BufferPtr outputBuffer = nullptr);
		void initKernels(
			const std::vector<CplxVector> kernelArray,
			const std::vector<CplxVector> kernelArrayInverse,
			const std::vector<std::vector<int>> kernelMask,
			const std::vector<std::vector<int>> kernelMaskInv);
		void initFs(const int blockSize);

		void cqtTransform(const ScheduleElement schedule);
		void icqtTransform(const ScheduleElement schedule);

		static void calculateWindow(BufferType* const windowData, const int size);
		static void calculateInverseWindow(BufferType* const windowData, BufferType* const invWindowData, const int size, const int hopSize);

	private:
		int B;
		BufferType mWindow[Fft_Size];
		BufferType mInvWindow[Fft_Size];
		// kernel storage
		std::vector<CplxVector> mKernelArray;         // mB x Fft_Domain_Size
		std::vector<CplxVector> mKernelArrayInverse;  // mB x Fft_Domain_Size
		std::vector<std::vector<int>> mKernelMask;    // mB x Fft_Domain_Size
		std::vector<std::vector<int>> mKernelMaskInv; // mB x Fft_Domain_Size

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

#ifdef __cplusplus
	extern "C" {
#endif
	/*
	Main CQT class
	*/
	class ConstantQTransform {
	public:
	 	ConstantQTransform(int b, int octaveNumber);
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

		void inputBlock(BufferType* const data, const int blockSize);
		BufferType* outputBlock(const int blockSize);
		void cqt(const ScheduleElement schedule);
		void icqt(const ScheduleElement schedule);

	protected:
		void initKernelFreqs();
		void calculateKernels();

		int B;
		int OctaveNumber;
		double mConcertPitch{ 440. };
		int mBinNumber;
		double mFs;

		std::vector<int> mOverlaps;          //[OctaveNumber]
		std::vector<double> mLatencyMs;      //[OctaveNumber]
		std::vector<size_t> mLatencySamples; //[OctaveNumber]
		std::vector<int> mHopSizes;          //[OctaveNumber]
		std::vector<double> mSampleRates;             //[OctaveNumber]
		std::vector<double> mSampleRatesByOriginRate; //[OctaveNumber]
		std::vector<size_t> mSampleCounters;          //[OctaveNumber]
		std::vector<TransformationHandler> mTransformationHandlers; //[OctaveNumber]

		std::vector<CplxVector> mKernelStorage;        //[B]
		std::vector<CplxVector> mKernelStorageTime;    //[B]
		std::vector<CplxVector> mKernelStorageInv;     //[B]
		std::vector<CplxVector> mKernelStorageTimeInv; //[B]
		std::vector<std::vector<int>> mKernelMask;     //[B]
		std::vector<std::vector<int>> mKernelMaskInv;  //[B]

		ResamplingFilterbank mFilterbank;
		std::vector<ScheduleElement> mCqtSchedule;

		FFT mFft;
		CplxVector mFftTmpStorage;
		CplxVector mFftTmpStorageInv;
		RealVector mWindow;
		std::atomic<bool> mNewKernels{ true };
		std::vector<std::vector<double>> mKernelFreqs;
		std::vector<std::vector<double>> mKernelFreqsInv;
	};
};
#ifdef __cplusplus
}
#endif
