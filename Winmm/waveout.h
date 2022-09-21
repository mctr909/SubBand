#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void waveout_open(
        int sampleRate,
        int bits,
        int channels,
        int bufferLength,
        int bufferCount,
        void (*fpWriteProc)(void*)
    );
    __declspec(dllexport) void waveout_close();
#ifdef __cplusplus
}
#endif
