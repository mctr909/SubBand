#pragma once
#include <vector>
#include <complex>
#include "Utils.h"

namespace Cqt {
	class FFT {
	public:
		FFT() = default;
		FFT(int n, bool inverse);

		void spectrumVector(std::vector<std::complex<BufferType>> list) {
			for (int i = 0; i < SIZE / 2; i++) {
				list.push_back(std::complex<BufferType>(0, 0));
			}
		}
		void valueVector(std::vector<std::complex<BufferType>> list) {
			list.clear();
			for (int i = 0; i < SIZE; i++) {
				list.push_back(std::complex<BufferType>(0, 0));
			}
		}
		void valueVector(std::vector<BufferType> list) {
			list.clear();
			for (int i = 0; i < SIZE; i++) {
				list.push_back(0);
			}
		}

		void Exec(std::vector<std::complex<BufferType>> io);
		void Exec(std::vector<std::complex<BufferType>> input, std::vector<std::complex<BufferType>> output);
		void Exec(std::vector<std::complex<BufferType>> input, std::vector<BufferType> output);
		void Exec(std::vector<BufferType> input, std::vector<std::complex<BufferType>> output);

	private:
		int SIZE = 0;
		int BITS = 0;
		double* COS = 0;
		double* SIN = 0;
	};
};
