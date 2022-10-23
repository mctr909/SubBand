using System;

class ACF {
    readonly double MIN;
    readonly int FFT_N;
    double[] mRe;
    double[] mIm;
    double[] mRe2;
    double[] mIm2;
    FFT mFFT;
    FFT mIFFT;
    FFT mSpec;
    double mBase;

    public ACF(int size) {
        MIN = Math.Pow(10, -200 / 20.0);
        FFT_N = size * 2;
        mRe = new double[size];
        mIm = new double[size];
        mFFT = new FFT(size);
        mIFFT = new FFT(size, true);
        mSpec = new FFT(FFT_N);
        mRe2 = new double[FFT_N];
        mIm2 = new double[FFT_N];
    }

    public void Exec(double[] input, double[] output) {
        var N = mRe.Length;
        Array.Copy(input, 0, mRe, 0, N);
        Array.Clear(mIm, 0, mIm.Length);
        mFFT.Exec(mRe, mIm);
        for (int i = 0; i < N; i++) {
            var re = (mRe[i] * mRe[i]) + (mIm[i] * mIm[i]);
            mIm[i] = -(mRe[i] * mIm[i]) + (mIm[i] * mRe[i]);
            output[i] = re;
        }
        mIFFT.Exec(output, mIm);
        if (mBase < output[0]) {
            mBase = output[0];
        }
        if (mBase < 1.0) {
            mBase = 1.0;
        }
        for (int i = 0, j = N / 2; i < N / 2; i++, j++) {
            output[j] = output[i] / mBase * (0.5 + 0.5 * Math.Cos(2 * Math.PI * i / N));
        }
        for (int i = N / 2 - 1, j = N / 2; j < N; i--, j++) {
            output[i] = output[j];
        }
    }

    public void Spec(double[] input, double[] output) {
        Array.Clear(mRe2, 0, mRe2.Length);
        Array.Clear(mIm2, 0, mIm2.Length);
        Array.Copy(input, 0, mRe2, 0, input.Length);
        mSpec.Exec(mRe2, mIm2);
        for (int i = 0; i < output.Length; i++) {
            var re = mRe2[i];
            var im = mIm2[i];
            re = Math.Sqrt(re * re + im * im) / output.Length;
            if (re < MIN) {
                re = MIN;
            }
            output[i] = 20 * Math.Log10(re);
        }
    }
}
