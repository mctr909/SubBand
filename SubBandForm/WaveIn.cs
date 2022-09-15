class WaveIn : WaveLib {
    readonly int FFT_N;
    readonly int READ_LEN;
    ACF mAcfL1;
    double[] mInput;
    double[] mFftBuff;
    public double[] Acf;
    public double[] AcfSpec;
    public bool Read;

    public WaveIn(int sampleRate, int fftSize, int bufferSize) : base(sampleRate, 1, bufferSize, 8, true) {
        READ_LEN = bufferSize;
        FFT_N = fftSize;
        mAcfL1 = new ACF(FFT_N);
        mInput = new double[FFT_N];
        mFftBuff = new double[FFT_N];
        Acf = new double[FFT_N];
        AcfSpec = new double[FFT_N];
    }

    protected override void SetData() {
        Array.Copy(mInput, READ_LEN, mInput, 0, FFT_N - READ_LEN);
        for (int i = 0, j = FFT_N - READ_LEN; i < READ_LEN; i++, j++) {
            mInput[j] = WaveBuffer[i] / 32768.0;
        }
        if (Read) {
            for (int i = 0; i < FFT_N; i++) {
                mFftBuff[i] = mInput[i] * (0.5 - 0.5 * Math.Cos(2 * Math.PI * i / FFT_N));
            }
            mAcfL1.ExecN(mFftBuff, Acf);
            for (int i = 0; i < FFT_N; i++) {
                var t = (2.0 * i / FFT_N - 1.0) * 2;
                Acf[i] *= Math.Pow(Math.E, -t * t);
            }
            mAcfL1.Spec(Acf, AcfSpec);
            Read = false;
        }
    }
}
