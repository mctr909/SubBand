#pragma once
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void wavein_setconfig(
        int sampleRate,
        int bits,
        int channels,
        int bufferLength,
        int bufferCount
    );
    __declspec(dllexport) void wavein_setcallback(void (*fpReadProc)(void*));
    __declspec(dllexport) void wavein_open();
    __declspec(dllexport) void wavein_close();
#ifdef __cplusplus
}
#endif
