#include "wavein.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

WaveInHandle ghWaveIn;

/******************************************************************************/
void CALLBACK wavein_proc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam);
DWORD wavein_task(LPVOID* param);

/******************************************************************************/
__declspec(dllexport) void wavein_open(
    int sampleRate,
    int bits,
    int channels,
    int bufferLength,
    int bufferCount,
    void (*fpReadProc)(void*)
) {
    wavein_close();
    if (NULL == fpReadProc) {
        return;
    }
    //
    ghWaveIn.bufferLength = bufferLength;
    ghWaveIn.bufferCount = bufferCount;
    ghWaveIn.fpReadProc = fpReadProc;
    //
    ghWaveIn.readCount = 0;
    ghWaveIn.readIndex = 0;
    ghWaveIn.addBufferIndex = 0;
    //
    ghWaveIn.fmt.wFormatTag = 32 == bits ? 3 : 1;
    ghWaveIn.fmt.nChannels = channels;
    ghWaveIn.fmt.wBitsPerSample = (WORD)bits;
    ghWaveIn.fmt.nSamplesPerSec = (DWORD)sampleRate;
    ghWaveIn.fmt.nBlockAlign = ghWaveIn.fmt.nChannels * ghWaveIn.fmt.wBitsPerSample / 8;
    ghWaveIn.fmt.nAvgBytesPerSec = ghWaveIn.fmt.nSamplesPerSec * ghWaveIn.fmt.nBlockAlign;
    //
    if (MMSYSERR_NOERROR != waveInOpen(
        &ghWaveIn.handle,
        WAVE_MAPPER,
        &ghWaveIn.fmt,
        (DWORD_PTR)wavein_proc,
        (DWORD_PTR)ghWaveIn.hdr,
        CALLBACK_FUNCTION
    )) {
        return;
    }
    //
    InitializeCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
    //
    ghWaveIn.hdr = (PWAVEHDR*)calloc(bufferCount, sizeof(PWAVEHDR));
    for (int n = 0; n < bufferCount; ++n) {
        ghWaveIn.hdr[n] = (PWAVEHDR)calloc(1, sizeof(WAVEHDR));
        ghWaveIn.hdr[n]->dwBufferLength = (DWORD)bufferLength * ghWaveIn.fmt.nBlockAlign;
        ghWaveIn.hdr[n]->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        ghWaveIn.hdr[n]->dwLoops = 0;
        ghWaveIn.hdr[n]->dwUser = 0;
        ghWaveIn.hdr[n]->lpData = (LPSTR)calloc(bufferLength, ghWaveIn.fmt.nBlockAlign);
        waveInPrepareHeader(ghWaveIn.handle, ghWaveIn.hdr[n], sizeof(WAVEHDR));
        waveInAddBuffer(ghWaveIn.handle, ghWaveIn.hdr[n], sizeof(WAVEHDR));
    }
    //
    ghWaveIn.stop = false;
    ghWaveIn.stopped = false;
    auto threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wavein_task, NULL, 0, &ghWaveIn.threadId);
    SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
    waveInStart(ghWaveIn.handle);
}

__declspec(dllexport) void wavein_close() {
    if (NULL == ghWaveIn.handle) {
        return;
    }
    //
    ghWaveIn.stop = true;
    while (!ghWaveIn.stopped) {
        Sleep(100);
    }
    //
    waveInReset(ghWaveIn.handle);
    for (int n = 0; n < ghWaveIn.bufferCount; ++n) {
        waveInUnprepareHeader(ghWaveIn.handle, ghWaveIn.hdr[n], sizeof(WAVEHDR));
    }
    waveInClose(ghWaveIn.handle);
}

/******************************************************************************/
void CALLBACK wavein_proc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam) {
    switch (uMsg) {
    case MM_WIM_OPEN:
        break;
    case MM_WIM_CLOSE:
        for (int b = 0; b < ghWaveIn.bufferCount; ++b) {
            free(ghWaveIn.hdr[b]->lpData);
            ghWaveIn.hdr[b]->lpData = NULL;
        }
        break;
    case MM_WIM_DATA:
        if (ghWaveIn.stop) {
            return;
        }
        EnterCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
        if (ghWaveIn.readCount < 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
            return;
        }
        {
            waveInAddBuffer(hwi, ghWaveIn.hdr[ghWaveIn.addBufferIndex++], sizeof(WAVEHDR));
            ghWaveIn.addBufferIndex %= ghWaveIn.bufferCount;
            ghWaveIn.readCount--;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
        break;
    default:
        break;
    }
}

DWORD wavein_task(LPVOID* param) {
    while (!ghWaveIn.stop) {
        EnterCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
        if (ghWaveIn.bufferCount <= ghWaveIn.readCount + 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
            Sleep(20);
            continue;
        }
        {
            auto pBuff = ghWaveIn.hdr[ghWaveIn.readIndex++]->lpData;
            ghWaveIn.readIndex %= ghWaveIn.bufferCount;
            ghWaveIn.fpReadProc(pBuff);
            ghWaveIn.readCount++;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveIn.lock);
    }
    ghWaveIn.stopped = true;
    ExitThread(0);
    return 0;
}
