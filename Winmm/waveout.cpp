#include "waveout.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

/******************************************************************************/
DWORD            gWaveOutThreadId;
CRITICAL_SECTION gcsWaveOutLock;
HWAVEOUT         ghWaveOut = NULL;
WAVEFORMATEX     gWaveOutFmt = { 0 };
WAVEHDR**        gppWaveOutHdr = NULL;

void (*gfpWriteProc)(void*) = NULL;
int gWaveOutBufferCount = 0;
int gWaveOutBufferLength = 0;
int gWaveOutWriteCount = 0;
int gWaveOutWriteIndex = 0;
int gWaveOutAddBufferIndex = 0;
bool gWaveOutStop = false;
bool gWaveOutStopped = true;

/******************************************************************************/
void CALLBACK waveout_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam);
DWORD waveout_task(LPVOID* param);

/******************************************************************************/
__declspec(dllexport) void waveout_open(
    int sampleRate,
    int bits,
    int channels,
    int bufferLength,
    int bufferCount,
    void (*fpWriteProc)(void*)
) {
    waveout_close();
    //
    gWaveOutBufferLength = bufferLength;
    gWaveOutBufferCount = bufferCount;
    gfpWriteProc = fpWriteProc;
    if (NULL == gfpWriteProc) {
        return;
    }
    //
    gWaveOutWriteCount = 0;
    gWaveOutWriteIndex = 0;
    gWaveOutAddBufferIndex = 0;
    //
    gWaveOutFmt.wFormatTag = 32 == bits ? 3 : 1;
    gWaveOutFmt.nChannels = channels;
    gWaveOutFmt.wBitsPerSample = (WORD)bits;
    gWaveOutFmt.nSamplesPerSec = (DWORD)sampleRate;
    gWaveOutFmt.nBlockAlign = gWaveOutFmt.nChannels * gWaveOutFmt.wBitsPerSample / 8;
    gWaveOutFmt.nAvgBytesPerSec = gWaveOutFmt.nSamplesPerSec * gWaveOutFmt.nBlockAlign;
    //
    if (MMSYSERR_NOERROR != waveOutOpen(
        &ghWaveOut,
        WAVE_MAPPER,
        &gWaveOutFmt,
        (DWORD_PTR)waveout_proc,
        (DWORD_PTR)gppWaveOutHdr,
        CALLBACK_FUNCTION
    )) {
        return;
    }
    //
    InitializeCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
    //
    gppWaveOutHdr = (PWAVEHDR*)calloc(gWaveOutBufferCount, sizeof(PWAVEHDR));
    for (int n = 0; n < gWaveOutBufferCount; ++n) {
        gppWaveOutHdr[n] = (PWAVEHDR)calloc(1, sizeof(WAVEHDR));
        gppWaveOutHdr[n]->dwBufferLength = (DWORD)gWaveOutBufferLength * gWaveOutFmt.nBlockAlign;
        gppWaveOutHdr[n]->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        gppWaveOutHdr[n]->dwLoops = 0;
        gppWaveOutHdr[n]->dwUser = 0;
        gppWaveOutHdr[n]->lpData = (LPSTR)calloc(gWaveOutBufferLength, gWaveOutFmt.nBlockAlign);
        waveOutPrepareHeader(ghWaveOut, gppWaveOutHdr[n], sizeof(WAVEHDR));
        waveOutWrite(ghWaveOut, gppWaveOutHdr[n], sizeof(WAVEHDR));
    }
    //
    gWaveOutStop = false;
    gWaveOutStopped = false;
    auto threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)waveout_task, NULL, 0, &gWaveOutThreadId);
    SetThreadPriority(threadHandle, THREAD_PRIORITY_HIGHEST);
}

__declspec(dllexport) void waveout_close() {
    if (NULL == ghWaveOut) {
        return;
    }
    //
    gWaveOutStop = true;
    while (!gWaveOutStopped) {
        Sleep(100);
    }
    //
    waveOutReset(ghWaveOut);
    for (int n = 0; n < gWaveOutBufferCount; ++n) {
        waveOutUnprepareHeader(ghWaveOut, gppWaveOutHdr[n], sizeof(WAVEHDR));
    }
    waveOutClose(ghWaveOut);
}

/******************************************************************************/
void CALLBACK waveout_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam) {
    switch (uMsg) {
    case MM_WOM_OPEN:
        break;
    case MM_WOM_CLOSE:
        for (int b = 0; b < gWaveOutBufferCount; ++b) {
            free(gppWaveOutHdr[b]->lpData);
            gppWaveOutHdr[b]->lpData = NULL;
        }
        break;
    case MM_WOM_DONE:
        if (gWaveOutStop) {
            return;
        }
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
        if (gWaveOutWriteCount < 1) {
            waveOutWrite(hwo, gppWaveOutHdr[gWaveOutAddBufferIndex], sizeof(WAVEHDR));
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
            return;
        }
        {
            waveOutWrite(hwo, gppWaveOutHdr[gWaveOutAddBufferIndex++], sizeof(WAVEHDR));
            gWaveOutAddBufferIndex %= gWaveOutBufferCount;
            gWaveOutWriteCount--;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
        break;
    default:
        break;
    }
}

DWORD waveout_task(LPVOID* param) {
    while (!gWaveOutStop) {
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
        if (gWaveOutBufferCount <= gWaveOutWriteCount + 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
            Sleep(20);
            continue;
        }
        {
            auto pBuff = gppWaveOutHdr[gWaveOutWriteIndex++]->lpData;
            gWaveOutWriteIndex %= gWaveOutBufferCount;
            memset(pBuff, 0, gWaveOutFmt.nBlockAlign * gWaveOutBufferLength);
            gfpWriteProc(pBuff);
            gWaveOutWriteCount++;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsWaveOutLock);
    }
    gWaveOutStopped = true;
    ExitThread(0);
    return 0;
}
