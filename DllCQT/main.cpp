#include "ConstantQTransform.h"
#include "main.h"

#include <stdio.h>
#include <mmsystem.h>
#pragma comment (lib, "winmm.lib")

using namespace Cqt;

/******************************************************************************/
DWORD            gThreadId;
CRITICAL_SECTION gcsBufferLock;
HWAVEIN          ghWaveIn = NULL;
WAVEFORMATEX     gWaveFmt = { 0 };
WAVEHDR**        gppWaveHdr = NULL;

void (*gfpReadBuffer)(LPSTR) = NULL;
int gBufferCount = 0;
int gBufferLength = 0;
volatile int gReadCount = 0;
volatile int gReadIndex = 0;
volatile int gWriteIndex = 0;

ConstantQTransform* gQFT = NULL;
std::vector<BufferType> gInputBlock;
double* gOutputBlock = NULL;

/******************************************************************************/
BOOL open(
    int sampleRate,
    int bits,
    int channelCount,
    int bufferLength,
    int bufferCount,
    void(*fpWriteBufferProc)(LPSTR)
);
BOOL close();
void CALLBACK waveInProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam);
DWORD readBufferTask(LPVOID* param);
void read16(LPSTR pData);

/******************************************************************************/
void WINAPI wavein_open(
    int sampleRate,
    int bits,
    int bufferLength,
    int bufferCount
) {
    wavein_close();
    open(sampleRate, 16, 2, bufferLength, bufferCount, read16);
}

void WINAPI wavein_close() {
    close();
}

/******************************************************************************/
BOOL open(
    int sampleRate,
    int bits,
    int channelCount,
    int bufferLength,
    int bufferCount,
    void(*fpReadBufferProc)(LPSTR)
) {
    if (NULL != ghWaveIn) {
        if (!close()) {
            return FALSE;
        }
    }
    if (NULL == fpReadBufferProc) {
        return FALSE;
    }
    //
    gfpReadBuffer = fpReadBufferProc;
    gBufferCount = bufferCount;
    gBufferLength = bufferLength;
    gReadCount = 0;
    gReadIndex = 0;
    gWriteIndex = 0;
    if (NULL != gQFT) {
        delete gQFT;
    }
    gInputBlock.clear();
    for (int i = 0; i < bufferLength; i++) {
        gInputBlock.push_back(0);
    }
    if (NULL != gOutputBlock) {
        free(gOutputBlock);
    }
    gOutputBlock = (double*)calloc(12 * 8, sizeof(double));
    gQFT = new ConstantQTransform(12, 9);
    gQFT->init(256); // separate hop-sizes for each octave can be initialized using the .init(std::vector<int> octaveHopSizes) overload 
    gQFT->initFs(sampleRate, bufferLength);

    //
    gWaveFmt.wFormatTag = 32 == bits ? 3 : 1;
    gWaveFmt.nChannels = channelCount;
    gWaveFmt.wBitsPerSample = (WORD)bits;
    gWaveFmt.nSamplesPerSec = (DWORD)sampleRate;
    gWaveFmt.nBlockAlign = gWaveFmt.nChannels * gWaveFmt.wBitsPerSample / 8;
    gWaveFmt.nAvgBytesPerSec = gWaveFmt.nSamplesPerSec * gWaveFmt.nBlockAlign;
    //
    if (MMSYSERR_NOERROR != waveInOpen(
        &ghWaveIn,
        WAVE_MAPPER,
        &gWaveFmt,
        (DWORD_PTR)waveInProc,
        (DWORD_PTR)gppWaveHdr,
        CALLBACK_FUNCTION
    )) {
        return FALSE;
    }
    //
    InitializeCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
    //
    gppWaveHdr = (PWAVEHDR*)calloc(gBufferCount, sizeof(PWAVEHDR));
    for (int n = 0; n < gBufferCount; ++n) {
        gppWaveHdr[n] = (PWAVEHDR)calloc(1, sizeof(WAVEHDR));
        gppWaveHdr[n]->dwBufferLength = (DWORD)bufferLength * gWaveFmt.nBlockAlign;
        gppWaveHdr[n]->dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        gppWaveHdr[n]->dwLoops = 0;
        gppWaveHdr[n]->dwUser = 0;
        gppWaveHdr[n]->lpData = (LPSTR)calloc(bufferLength, gWaveFmt.nBlockAlign);
        waveInPrepareHeader(ghWaveIn, gppWaveHdr[n], sizeof(WAVEHDR));
        waveInAddBuffer(ghWaveIn, gppWaveHdr[n], sizeof(WAVEHDR));
    }
    //
    auto hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)readBufferTask, NULL, 0, &gThreadId);
    SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
    waveInStart(ghWaveIn);
    return TRUE;
}

BOOL close() {
    if (NULL == ghWaveIn) {
        return TRUE;
    }
    //
    for (int n = 0; n < gBufferCount; ++n) {
        waveInUnprepareHeader(ghWaveIn, gppWaveHdr[n], sizeof(WAVEHDR));
    }
    waveInReset(ghWaveIn);
    waveInClose(ghWaveIn);
    return TRUE;
}

void CALLBACK waveInProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD dwParam1, DWORD dwParam) {
    switch (uMsg) {
    case MM_WIM_OPEN:
        break;
    case MM_WIM_CLOSE:
        for (int b = 0; b < gBufferCount; ++b) {
            free(gppWaveHdr[b]->lpData);
            gppWaveHdr[b]->lpData = NULL;
        }
        break;
    case MM_WIM_DATA:
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
        if (gReadCount < 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
            return;
        }
        {
            waveInAddBuffer(ghWaveIn, gppWaveHdr[gWriteIndex], sizeof(WAVEHDR));
            gWriteIndex = (gWriteIndex + 1) % gBufferCount;
            gReadCount--;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
        break;
    default:
        break;
    }
}

DWORD readBufferTask(LPVOID* param) {
    while (TRUE) {
        if (NULL == gppWaveHdr || NULL == gppWaveHdr[0] || NULL == gppWaveHdr[0]->lpData) {
            Sleep(100);
            continue;
        }
        EnterCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
        if (gBufferCount <= gReadCount + 1) {
            LeaveCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
            continue;
        }
        {
            LPSTR pBuff = gppWaveHdr[gReadIndex]->lpData;
            memset(pBuff, 0, gWaveFmt.nBlockAlign * gBufferLength);
            gfpReadBuffer(pBuff);
            gReadIndex = (gReadIndex + 1) % gBufferCount;
            gReadCount++;
        }
        LeaveCriticalSection((LPCRITICAL_SECTION)&gcsBufferLock);
    }
    return 0;
}

void read16(LPSTR pData) {
    auto pBuff = (short*)pData;
    for (int i = 0; i < gBufferLength; i++) {
        gInputBlock[i] = pBuff[i] / 32768.0;
    }
    gQFT->inputBlock(gInputBlock.data(), gBufferLength);
    auto schedule = gQFT->getCqtSchedule();
    for (const auto& s : schedule) {
        gQFT->cqt(s);
        auto cqtDomainBuffer = gQFT->getOctaveCqtBuffer(s.octave);
    }
}
