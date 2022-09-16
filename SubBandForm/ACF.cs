class ACF {
    readonly double MIN;
    double mGate;
    double[] mRe;
    double[] mIm;
    double[] mRe2;
    double[] mIm2;
    FFT mFFT;
    FFT mIFFT;
    FFT mSpec;
    double mBase;

    public double Gate {
        set { mGate = Math.Pow(10, value / 20.0); }
        get { return 20 * Math.Log10(mGate); }
    }

    public ACF(int size) {
        MIN = Math.Pow(10, -200 / 20.0);
        Gate = 0.0;
        mRe = new double[size];
        mIm = new double[size];
        mFFT = new FFT(size);
        mIFFT = new FFT(size, true);
        mSpec = new FFT(size * 2);
        mRe2 = new double[size * 2];
        mIm2 = new double[size * 2];
    }

    void exec(double[] input, double[] output) {
        var N = mRe.Length;
        Array.Copy(input, 0, mRe, 0, N);
        Array.Clear(mIm);
        mFFT.Exec(mRe, mIm);
        for (int i = 0; i < N; i++) {
            var re =  (mRe[i] * mRe[i]) + (mIm[i] * mIm[i]);
            mIm[i] = -(mRe[i] * mIm[i]) + (mIm[i] * mRe[i]);
            output[i] = re;
        }
        mIFFT.Exec(output, mIm);
        mBase = output[0];
        if (mBase < mGate) {
            mBase = mGate;
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
    }

    public void Spec(double[] input, double[] output) {
        Array.Clear(mRe2);
        Array.Clear(mIm2);
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
