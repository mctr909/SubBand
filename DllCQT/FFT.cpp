#include "FFT.h"
#include "Utils.h"
#include <math.h>
#include <stdlib.h>

using namespace Cqt;

FFT::FFT(int n, bool inverse) {
	SIZE = n;
	BITS = (int)(log(SIZE) / log(2));
	COS = new double[SIZE / 2];
	SIN = new double[SIZE / 2];
	auto dtheta = (inverse ? 2 : -2) * Pi<double>() / SIZE;
	for (int i = 0; i < SIZE / 2; i++) {
		COS[i] = cos(dtheta * i);
		SIN[i] = sin(dtheta * i);
	}
}
void
FFT::Exec(std::vector<std::complex<double>> io) {
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
void
FFT::Exec(std::vector<std::complex<double>> input, std::vector<std::complex<double>> output) {
	for (int i = 0; i < SIZE; i++) {
		output[i] = input[i];
	}
	Exec(output);
}
void
FFT::Exec(std::vector<std::complex<double>> input, double* output) {
	//TODO: FFT::Exec(CplxVector input, double* output)
}
void
FFT::Exec(double* input, std::vector<std::complex<double>> output) {
	for (int i = 0; i < SIZE; i++) {
		output[i].real(input[i]);
		output[i].imag(0.0);
	}
	Exec(output);
}
