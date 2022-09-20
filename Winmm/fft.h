#pragma once
#include <vector>
#include <complex>
#include <math.h>
#include "Utils.h"

class FFT {
private:
	FFT() {}

public:
	FFT(int size, bool inverse) {
		SIZE = size;
		BITS = (int)(log(SIZE) / log(2));
		COS = new double[SIZE / 2];
		SIN = new double[SIZE / 2];
		auto dtheta = (inverse ? 2 : -2) * Pi() / SIZE;
		for (int i = 0; i < SIZE / 2; i++) {
			COS[i] = cos(dtheta * i);
			SIN[i] = sin(dtheta * i);
		}
	}
	~FFT() {
		delete COS;
		delete SIN;
	}
	void exec(std::complex<FloatType>* io) {
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
				auto t1 = io[i];
				io[i] = io[j];
				io[j] = t1;
			}
		}
		n2 = 1;
		for (int i = 0; i < BITS; i++) {
			int n1 = n2;
			n2 <<= 1;
			int w = 0;
			for (j = 0; j < n1; j++) {
				auto c = COS[w];
				auto s = SIN[w];
				w += 1 << (BITS - i - 1);
				for (int k = j; k < SIZE; k += n2) {
					auto k1 = io[k + n1];
					auto re = c * k1.real() - s * k1.imag();
					auto im = s * k1.real() + c * k1.imag();
					auto k0 = io[k];
					io[k + n1].real(k0.real() - re);
					io[k + n1].imag(k0.imag() - im);
					io[k].real(io[k].real() + re);
					io[k].real(io[k].imag() + im);
				}
			}
		}
	}

private:
	int SIZE = 0;
	int BITS = 0;
	double* COS = 0;
	double* SIN = 0;
};
