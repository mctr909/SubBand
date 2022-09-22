#pragma once
#include <windows.h>

typedef struct WaveInHandle {
    DWORD            threadId;
    CRITICAL_SECTION lock;
    HWAVEIN          handle = NULL;
    WAVEFORMATEX     fmt = { 0 };
    WAVEHDR** hdr = NULL;
    void (*fpReadProc)(void*) = NULL;
    int bufferCount = 0;
    int bufferLength = 0;
    int readCount = 0;
    int readIndex = 0;
    int addBufferIndex = 0;
    bool stop = false;
    bool stopped = true;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) void wavein_open(
        int sampleRate,
        int bits,
        int channels,
        int bufferLength,
        int bufferCount,
        void (*fpReadProc)(void*)
    );
    __declspec(dllexport) void wavein_close();
#ifdef __cplusplus
}
#endif
