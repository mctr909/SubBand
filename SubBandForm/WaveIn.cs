class WaveIn : WaveLib {
    readonly int FFT_N;
    readonly int READ_LEN;
    ACF mAcfL1;
    public double[] Input;
    public double[] Acf;
    public double[] AcfSpec;
    public bool Read;

    public WaveIn(int sampleRate, int fftSize, int bufferSize) : base(sampleRate, 1, bufferSize, 16, true) {
        READ_LEN = bufferSize;
        FFT_N = fftSize;
        mAcfL1 = new ACF(FFT_N);
        Input = new double[FFT_N];
        Acf = new double[FFT_N];
        AcfSpec = new double[FFT_N];
    }

    protected override void SetData() {
        Array.Copy(Input, READ_LEN, Input, 0, FFT_N - READ_LEN);
        for (int i = 0, j = FFT_N - READ_LEN; i < READ_LEN; i++, j++) {
            Input[j] = WaveBuffer[i] / 32768.0;
        }
        if (Read) {
            for (int i = 0; i < FFT_N; i++) {
                var t = (2.0 * i / FFT_N - 1.0) * 2;
                Input[i] *= Math.Pow(Math.E, -t * t) * Math.Sin(Math.PI * i / FFT_N); 
            }
            mAcfL1.ExecN(Input, Acf, 1);
            mAcfL1.Spec(Acf, AcfSpec);
            Read = false;
        }
    }
}
