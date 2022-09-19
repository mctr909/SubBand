#pragma once

namespace Cqt
{
    typedef double BufferType;

    static inline BufferType Pi()
    {
        return static_cast<BufferType>(3.14159265358979323846);
    }

    template<typename T>
    static inline T Clip(const T input, const T lowerBound, const T upperBound)
    {
        T y = input > upperBound ? upperBound : input;
        y = y < lowerBound ? lowerBound : y;
        return y;
    }
};
