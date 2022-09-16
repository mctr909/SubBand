class WaveIn : WaveLib {
    readonly int FFT_N;
    readonly int READ_LEN;
    double mSigma;
    ACF mAcfL1;
    double[] mInput;
    double[] mFftBuff;
    public double[] Acf;
    public double[] AcfSpec;
    public bool Read;

    public double WindowWidth {
        set { mSigma = Math.Sqrt(Math.PI) / value; }
        get { return Math.Sqrt(Math.PI) / mSigma; }
    }

    public double Gate {
        set { mAcfL1.Gate = value; }
        get { return mAcfL1.Gate; }
    }

    public WaveIn(int sampleRate, int bufferSize, int fftSize) : base(sampleRate, 1, bufferSize, 8, true) {
        READ_LEN = bufferSize;
        FFT_N = fftSize;
        WindowWidth = 1.0;
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
                var ts = (2.0 * i / FFT_N - 1.0) * mSigma;
                mFftBuff[i] = mInput[i] * Math.Pow(Math.E, -ts * ts) * mSigma;
            }
            mAcfL1.ExecN(mFftBuff, Acf);
            mAcfL1.Spec(Acf, AcfSpec);
            Read = false;
        }
    }
}
