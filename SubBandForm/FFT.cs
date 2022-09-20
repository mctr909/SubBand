using System;

class FFT {
	readonly int SIZE;
	readonly int BITS;
	readonly double[] COS;
	readonly double[] SIN;
	public FFT(int n, bool inverse = false) {
		SIZE = n;
		BITS = (int)(Math.Log(SIZE) / Math.Log(2));
		COS = new double[SIZE / 2];
		SIN = new double[SIZE / 2];
		var dtheta = (inverse ? 2 : -2) * Math.PI / SIZE;
		for (int i = 0; i < COS.Length; i++) {
			COS[i] = Math.Cos(dtheta * i);
			SIN[i] = Math.Sin(dtheta * i);
		}
	}
	public void Exec(double[] real, double[] imag) {
		int j = 0;
		int n2 = SIZE / 2;
		for (int i = 1; i < SIZE - 1; i++) {
			int n1 = n2;
			while (j >= n1) {
				j -= n1;
				n1 /= 2;
			}
			j += n1;
			if (i < j) {
				var t1 = real[i];
				real[i] = real[j];
				real[j] = t1;
				t1 = imag[i];
				imag[i] = imag[j];
				imag[j] = t1;
			}
		}
		n2 = 1;
		for (int i = 0; i < BITS; i++) {
			int n1 = n2;
			n2 <<= 1;
			int w = 0;
			for (j = 0; j < n1; j++) {
				var c = COS[w];
				var s = SIN[w];
				w += 1 << (BITS - i - 1);
				for (int k = j; k < SIZE; k += n2) {
					var t = k + n1;
					var re = c * real[t] - s * imag[t];
					var im = s * real[t] + c * imag[t];
					real[k + n1] = real[k] - re;
					imag[k + n1] = imag[k] - im;
					real[k] += re;
					imag[k] += im;
				}
			}
		}
	}
}
