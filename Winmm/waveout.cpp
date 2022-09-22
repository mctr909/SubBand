#include "waveout.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

WaveOutHandle ghWaveOut;

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
    if (NULL == ghWaveOut.fpWriteProc) {
        return;
    }
    //
    ghWaveOut.bufferLength = bufferLength;
    ghWaveOut.bufferCount = bufferCount;
    ghWaveOut.fpWriteProc = fpWriteProc;
    //
    ghWaveOut.writeCount = 0;
    ghWaveOut.writeIndex = 0;
    ghWaveOut.addBufferIndex = 0;
    //
    ghWaveOut.fmt.wFormatTag = 32 == bits ? 3 : 1;
    ghWaveOut.fmt.nChannels = channels;
    ghWaveOut.fmt.wBitsPerSample = (WORD)bits;
    ghWaveOut.fmt.nSamplesPerSec = (DWORD)sampleRate;
    ghWaveOut.fmt.nBlockAlign = ghWaveOut.fmt.nChannels * ghWaveOut.fmt.wBitsPerSample / 8;
    ghWaveOut.fmt.nAvgBytesPerSec = ghWaveOut.fmt.nSamplesPerSec * ghWaveOut.fmt.nBlockAlign;
    //
    if (MMSYSERR_NOERROR != waveOutOpen(
        &ghWaveOut.handle,
        WAVE_MAPPER,
        &ghWaveOut.fmt,
        (DWORD_PTR)waveout_proc,
        (DWORD_PTR)ghWaveOut.ppHdr,
        CALLBACK_FUNCTION
    )) {
        return;
    }
    //
    InitializeCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
    //
    ghWaveOut.ppHdr = (PWAVEHDR*)calloc(ghWaveOut.bufferCount, sizeof(PWAVEHDR));
    for (int n = 0; n < ghWaveOut.bufferCount; ++n) {
        ghWaveOut.ppHdr[n] = (PWAVEHDR)calloc(1, sizeof(WAVEHDR));
        ghWaveOut.ppHdr[n]->dwBufferLength = (DWORD)ghWaveOut.bufferLength * ghWaveOut.fmt.nBlockAlign;
        ghWaveOut.ppHdr[n]->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        ghWaveOut.ppHdr[n]->dwLoops = 0;
        ghWaveOut.ppHdr[n]->dwUser = 0;
        ghWaveOut.ppHdr[n]->lpData = (LPSTR)calloc(ghWaveOut.bufferLength, ghWaveOut.fmt.nBlockAlign);
        waveOutPrepareHeader(ghWaveOut.handle, ghWaveOut.ppHdr[n], sizeof(WAVEHDR));
        waveOutWrite(ghWaveOut.handle, ghWaveOut.ppHdr[n], sizeof(WAVEHDR));
    }
    //
    ghWaveOut.stop = false;
    ghWaveOut.stopped = false;
    ghWaveOut.threadHandle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)waveout_task, NULL, 0, &ghWaveOut.threadId);
    SetThreadPriority(ghWaveOut.threadHandle, THREAD_PRIORITY_HIGHEST);
}

__declspec(dllexport) void waveout_close() {
    if (NULL == ghWaveOut.handle) {
        return;
    }
    //
    ghWaveOut.stop = true;
    while (!ghWaveOut.stopped) {
        Sleep(100);
    }
    //
    waveOutReset(ghWaveOut.handle);
    for (int n = 0; n < ghWaveOut.bufferCount; ++n) {
        waveOutUnprepareHeader(ghWaveOut.handle, ghWaveOut.ppHdr[n], sizeof(WAVEHDR));
    }
    waveOutClose(ghWaveOut.handle);
}

/******************************************************************************/
void CALLBACK waveout_proc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam) {
    switch (uMsg) {
    case MM_WOM_OPEN:
        break;
    case MM_WOM_CLOSE:
        for (int b = 0; b < ghWaveOut.bufferCount; ++b) {
            free(ghWaveOut.ppHdr[b]->lpData);
            ghWaveOut.ppHdr[b]->lpData = NULL;
        }
        break;
    case MM_WOM_DONE:
        if (ghWaveOut.stop) {
            return;
        }
        EnterCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
        if (ghWaveOut.writeCount < 1) {
            waveOutWrite(hwo, ghWaveOut.ppHdr[ghWaveOut.addBufferIndex], sizeof(WAVEHDR));
            LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
            return;
        }
        {
            waveOutWrite(hwo, ghWaveOut.ppHdr[ghWaveOut.addBufferIndex++], sizeof(WAVEHDR));
            ghWaveOut.addBufferIndex %= ghWaveOut.bufferCount;
            ghWaveOut.writeCount--;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
        break;
    default:
        break;
    }
}

DWORD waveout_task(LPVOID* param) {
    while (!ghWaveOut.stop) {
        EnterCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
        if (ghWaveOut.bufferCount <= ghWaveOut.writeCount + 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
            Sleep(20);
            continue;
        }
        {
            auto pBuff = ghWaveOut.ppHdr[ghWaveOut.writeIndex++]->lpData;
            ghWaveOut.writeIndex %= ghWaveOut.bufferCount;
            memset(pBuff, 0, ghWaveOut.fmt.nBlockAlign * ghWaveOut.bufferLength);
            ghWaveOut.fpWriteProc(pBuff);
            ghWaveOut.writeCount++;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&ghWaveOut.lock);
    }
    ghWaveOut.stopped = true;
    ExitThread(0);
    return 0;
}
