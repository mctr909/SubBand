using System;

class SubBand {
	readonly int SIZE;
	readonly int SIZE_H;
	readonly int SIZE_Q;

	double[] mInputBuff;
	FFT mFFT;
	FFT mIFFT;
	double[] mRe;
	double[] mIm;
	double[] mReL;
	double[] mImL;
	double[] mReH;
	double[] mImH;
	double[] mBkL;
	double[] mBkH;
	
	public double[] OutputL;
	public double[] OutputH;

	public SubBand(int size) {
		SIZE = size;
		SIZE_H = size / 2;
		SIZE_Q = size / 4;
		mInputBuff = new double[SIZE];
		mFFT = new FFT(SIZE);
		mIFFT = new FFT(SIZE_H, true);
		mRe = new double[SIZE];
		mIm = new double[SIZE];
		mReL = new double[SIZE_H];
		mImL = new double[SIZE_H];
		mReH = new double[SIZE_H];
		mImH = new double[SIZE_H];
		mBkL = new double[SIZE_Q];
		mBkH = new double[SIZE_Q];
		OutputL = new double[SIZE_Q];
		OutputH = new double[SIZE_Q];
	}

	public void Exec(double[] input) {
		for (int i = 0, j = SIZE_H; i < SIZE_H; i++, j++) {
			mInputBuff[i] = mInputBuff[j];
			mInputBuff[j] = input[i];
			var th = Math.PI * (i + 0.5) / SIZE;
			mRe[i] = mInputBuff[i] * Math.Sin(th) / SIZE;
			mRe[j] = mInputBuff[j] * Math.Cos(th) / SIZE;
			mIm[i] = mIm[j] = 0.0;
		}
		mFFT.Exec(mRe, mIm);
		for (int w0 = 0, w1 = SIZE_Q, w2 = SIZE_H, w3 = SIZE - SIZE_Q; w0 < SIZE_Q; w0++, w1++, w2++, w3++) {
			mReL[w0] = mRe[w0];
			mImL[w0] = mIm[w0];
			mReL[w1] = mRe[w3];
			mImL[w1] = mIm[w3];
			mReH[w0] = mRe[w1];
			mImH[w0] = mIm[w1];
			mReH[w1] = mRe[w2];
			mImH[w1] = mIm[w2];
		}
		mIFFT.Exec(mReL, mImL);
		mIFFT.Exec(mReH, mImH);
		for (int i = 0, j = SIZE_Q; i < SIZE_Q; i++, j++) {
			var th = 0.5 * Math.PI * (i + 0.5) / SIZE_Q;
			OutputL[i] = mReL[i] * Math.Sin(th) + mBkL[i] * Math.Cos(th);
			OutputH[i] = mReH[i] * Math.Sin(th) + mBkH[i] * Math.Cos(th);
			mBkL[i] = mReL[j];
			mBkH[i] = mReH[j];
		}
	}
}
