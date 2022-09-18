#pragma once
#include <vector>
#include <complex>
#include "ConstantQTransform.h"

namespace Cqt {
	class FFT {
	public:
		FFT() {}
		FFT(int n, bool inverse = false);

		std::vector<std::complex<double>> spectrumVector() {
			auto ret = std::vector<std::complex<double>>();
			for (int i = 0; i < SIZE / 2; i++) {
				ret.push_back(std::complex<double>(0, 0));
			}
			return ret;
		}
		std::vector<std::complex<double>> valueVector() {
			auto ret = std::vector<std::complex<double>>();
			for (int i = 0; i < SIZE; i++) {
				ret.push_back(std::complex<double>(0, 0));
			}
			return ret;
		}

		void Exec(std::vector<std::complex<double>> io);
		void Exec(std::vector<std::complex<double>> input, std::vector<std::complex<double>> output);
		void Exec(std::vector<std::complex<double>> input, double* output);
		void Exec(double* input, std::vector<std::complex<double>> output);

	private:
		int SIZE;
		int BITS;
		double* COS = 0;
		double* SIN = 0;
	};
};
