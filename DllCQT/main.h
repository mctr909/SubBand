#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void wavein_open(
        int sampleRate,
        int bits,
        int bufferLength,
        int bufferCount
    );
    __declspec(dllexport) void wavein_close();
#ifdef __cplusplus
}
#endif
