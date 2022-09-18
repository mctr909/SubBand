#pragma once

namespace Cqt
{
    template<typename FloatType>
    static inline FloatType Pi()
    {
        return static_cast<FloatType>(3.14159265358979323846);
    }

    template<typename T>
    static inline T Clip(const T input, const T lowerBound, const T upperBound)
    {
        T y = input > upperBound ? upperBound : input;
        y = y < lowerBound ? lowerBound : y;
        return y;
    }
};
