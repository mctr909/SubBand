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

    public bool EliminateSlope = false;

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

    void exec(double[] input, double[] output) {
        var N = mRe.Length;
        Array.Copy(input, 0, mRe, 0, N);
        Array.Clear(mIm);
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

    public void ExecN(double[] input, double[] output, int n = 1) {
        var N = output.Length;
        exec(input, output);
        for (int i = 1; i < n; i++) {
            Array.Copy(output, 0, mRe2, 0, N);
            exec(mRe2, output);
        }
        mBase *= 1.0 - 1.0 / 20.0;
    }

    public void Spec(double[] input, double[] output) {
        Array.Clear(mRe2);
        Array.Clear(mIm2);
        Array.Copy(input, 0, mRe2, 0, input.Length);
        mSpec.Exec(mRe2, mIm2);
        var N = mRe2.Length / 2;
        if (EliminateSlope) {
            for (int i = 0; i < N; i++) {
                var re = mRe2[i];
                var im = mIm2[i];
                re = Math.Sqrt(re * re + im * im) / N;
                if (re < MIN) {
                    re = MIN;
                }
                mRe[i] = 10 * Math.Log10(re) + 12;
            }
            for (int i = 0; i < N; i++) {
                var w = 5 + (int)(15.0 * i / N);
                var max = double.MinValue;
                for (int j = i < w ? -i : -w; (i + j) < N && j <= w; j++) {
                    max = Math.Max(max, mRe[i + j]);
                }
                mRe2[i] = max;
            }
            for (int i = 0; i < N; i++) {
                if (mRe[i] < mRe2[i]) {
                    output[i] = -200;
                } else {
                    output[i] = mRe[i];
                }
            }
        } else {
            for (int i = 0; i < output.Length; i++) {
                var re = mRe2[i];
                var im = mIm2[i];
                re = Math.Sqrt(re * re + im * im) / output.Length;
                if (re < MIN) {
                    re = MIN;
                }
                output[i] = 10 * Math.Log10(re) + 12;
            }
        }
    }
}
