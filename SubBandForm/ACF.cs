class ACF {
    double[] mRe;
    double[] mIm;
    FFT mFFT;
    FFT mIFFT;
    public ACF(int size) {
        mRe = new double[size];
        mIm = new double[size];
        mFFT = new FFT(size);
        mIFFT = new FFT(size, true);
    }

    public void Exec(double[] input, double[] output) {
        Array.Copy(input, 0, mRe, 0, mRe.Length);
        Array.Clear(mIm);
        mFFT.Exec(mRe, mIm);
        for (int i = 0; i < mRe.Length; i++) {
            output[i] =  (mRe[i] * mRe[i]) + (mIm[i] * mIm[i]);
            mIm[i]    = -(mRe[i] * mIm[i]) + (mIm[i] * mRe[i]);
        }
        mIFFT.Exec(output, mIm);
    }
}
