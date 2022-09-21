#include "wavein.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

/******************************************************************************/
DWORD            gWaveInThreadId;
CRITICAL_SECTION gcsWaveInLock;
HWAVEIN          ghWaveIn = NULL;
WAVEFORMATEX     gWaveInFmt = { 0 };
WAVEHDR**        gppWaveInHdr = NULL;

void (*gfpReadProc)(void*) = NULL;
int gWaveInBufferCount = 0;
int gWaveInBufferLength = 0;
int gWaveInReadCount = 0;
int gWaveInReadIndex = 0;
int gWaveInAddBufferIndex = 0;
bool gWaveInStop = false;
bool gWaveInStopped = true;

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
    //
    gWaveInBufferLength = bufferLength;
    gWaveInBufferCount = bufferCount;
    gfpReadProc = fpReadProc;
    if (NULL == gfpReadProc) {
        return;
    }
    //
    gWaveInReadCount = 0;
    gWaveInReadIndex = 0;
    gWaveInAddBufferIndex = 0;
    //
    gWaveInFmt.wFormatTag = 32 == bits ? 3 : 1;
    gWaveInFmt.nChannels = channels;
    gWaveInFmt.wBitsPerSample = (WORD)bits;
    gWaveInFmt.nSamplesPerSec = (DWORD)sampleRate;
    gWaveInFmt.nBlockAlign = gWaveInFmt.nChannels * gWaveInFmt.wBitsPerSample / 8;
    gWaveInFmt.nAvgBytesPerSec = gWaveInFmt.nSamplesPerSec * gWaveInFmt.nBlockAlign;
    //
    if (MMSYSERR_NOERROR != waveInOpen(
        &ghWaveIn,
        WAVE_MAPPER,
        &gWaveInFmt,
        (DWORD_PTR)wavein_proc,
        (DWORD_PTR)gppWaveInHdr,
        CALLBACK_FUNCTION
    )) {
        return;
    }
    //
    InitializeCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
    //
    gppWaveInHdr = (PWAVEHDR*)calloc(gWaveInBufferCount, sizeof(PWAVEHDR));
    for (int n = 0; n < gWaveInBufferCount; ++n) {
        gppWaveInHdr[n] = (PWAVEHDR)calloc(1, sizeof(WAVEHDR));
        gppWaveInHdr[n]->dwBufferLength = (DWORD)gWaveInBufferLength * gWaveInFmt.nBlockAlign;
        gppWaveInHdr[n]->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        gppWaveInHdr[n]->dwLoops = 0;
        gppWaveInHdr[n]->dwUser = 0;
        gppWaveInHdr[n]->lpData = (LPSTR)calloc(gWaveInBufferLength, gWaveInFmt.nBlockAlign);
        waveInPrepareHeader(ghWaveIn, gppWaveInHdr[n], sizeof(WAVEHDR));
        waveInAddBuffer(ghWaveIn, gppWaveInHdr[n], sizeof(WAVEHDR));
    }
    //
    gWaveInStop = false;
    gWaveInStopped = false;
    auto threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)wavein_task, NULL, 0, &gWaveInThreadId);
    SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
    waveInStart(ghWaveIn);
}

__declspec(dllexport) void wavein_close() {
    if (NULL == ghWaveIn) {
        return;
    }
    //
    gWaveInStop = true;
    while (!gWaveInStopped) {
        Sleep(100);
    }
    //
    waveInReset(ghWaveIn);
    for (int n = 0; n < gWaveInBufferCount; ++n) {
        waveInUnprepareHeader(ghWaveIn, gppWaveInHdr[n], sizeof(WAVEHDR));
    }
    waveInClose(ghWaveIn);
}

/******************************************************************************/
void CALLBACK wavein_proc(HWAVEIN hwi, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam) {
    switch (uMsg) {
    case MM_WIM_OPEN:
        break;
    case MM_WIM_CLOSE:
        for (int b = 0; b < gWaveInBufferCount; ++b) {
            free(gppWaveInHdr[b]->lpData);
            gppWaveInHdr[b]->lpData = NULL;
        }
        break;
    case MM_WIM_DATA:
        if (gWaveInStop) {
            return;
        }
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
        if (gWaveInReadCount < 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
            return;
        }
        {
            waveInAddBuffer(hwi, gppWaveInHdr[gWaveInAddBufferIndex++], sizeof(WAVEHDR));
            gWaveInAddBufferIndex %= gWaveInBufferCount;
            gWaveInReadCount--;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
        break;
    default:
        break;
    }
}

DWORD wavein_task(LPVOID* param) {
    while (!gWaveInStop) {
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
        if (gWaveInBufferCount <= gWaveInReadCount + 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
            Sleep(20);
            continue;
        }
        {
            auto pBuff = gppWaveInHdr[gWaveInReadIndex++]->lpData;
            gWaveInReadIndex %= gWaveInBufferCount;
            gfpReadProc(pBuff);
            gWaveInReadCount++;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveInLock);
    }
    gWaveInStopped = true;
    ExitThread(0);
    return 0;
}
