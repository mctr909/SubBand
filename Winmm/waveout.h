#pragma once
#include <windows.h>

/******************************************************************************/
typedef struct WaveOutHandle {
    HWAVEOUT         handle = NULL;
    HANDLE           threadHandle;
    DWORD            threadId;
    CRITICAL_SECTION lock;
    WAVEFORMATEX     fmt = { 0 };
    WAVEHDR** ppHdr = NULL;
    int bufferCount = 0;
    int bufferLength = 0;
    int writeCount = 0;
    int writeIndex = 0;
    int addBufferIndex = 0;
    bool stop = false;
    bool stopped = true;
    void (*fpWriteProc)(void*) = NULL;
};

/******************************************************************************/
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
