using System;
using System.Runtime.InteropServices;

public abstract class WaveLib : IDisposable {
    [StructLayout(LayoutKind.Sequential)]
    private struct WAVEFORMATEX {
        public ushort wFormatTag;
        public ushort nChannels;
        public uint nSamplesPerSec;
        public uint nAvgBytesPerSec;
        public ushort nBlockAlign;
        public ushort wBitsPerSample;
        public ushort cbSize;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct WAVEHDR {
        public IntPtr lpData;
        public uint dwBufferLength;
        public uint dwBytesRecorded;
        public uint dwUser;
        public uint dwFlags;
        public uint dwLoops;
        public IntPtr lpNext;
        public uint reserved;
    }

    private enum MMRESULT {
        MMSYSERR_NOERROR = 0,
        MMSYSERR_ERROR = (MMSYSERR_NOERROR + 1),
        MMSYSERR_BADDEVICEID = (MMSYSERR_NOERROR + 2),
        MMSYSERR_NOTENABLED = (MMSYSERR_NOERROR + 3),
        MMSYSERR_ALLOCATED = (MMSYSERR_NOERROR + 4),
        MMSYSERR_INVALHANDLE = (MMSYSERR_NOERROR + 5),
        MMSYSERR_NODRIVER = (MMSYSERR_NOERROR + 6),
        MMSYSERR_NOMEM = (MMSYSERR_NOERROR + 7),
        MMSYSERR_NOTSUPPORTED = (MMSYSERR_NOERROR + 8),
        MMSYSERR_BADERRNUM = (MMSYSERR_NOERROR + 9),
        MMSYSERR_INVALFLAG = (MMSYSERR_NOERROR + 10),
        MMSYSERR_INVALPARAM = (MMSYSERR_NOERROR + 11),
        MMSYSERR_HANDLEBUSY = (MMSYSERR_NOERROR + 12),
        MMSYSERR_INVALIDALIAS = (MMSYSERR_NOERROR + 13),
        MMSYSERR_BADDB = (MMSYSERR_NOERROR + 14),
        MMSYSERR_KEYNOTFOUND = (MMSYSERR_NOERROR + 15),
        MMSYSERR_READERROR = (MMSYSERR_NOERROR + 16),
        MMSYSERR_WRITEERROR = (MMSYSERR_NOERROR + 17),
        MMSYSERR_DELETEERROR = (MMSYSERR_NOERROR + 18),
        MMSYSERR_VALNOTFOUND = (MMSYSERR_NOERROR + 19),
        MMSYSERR_NODRIVERCB = (MMSYSERR_NOERROR + 20),
        MMSYSERR_MOREDATA = (MMSYSERR_NOERROR + 21),
        MMSYSERR_LASTERROR = (MMSYSERR_NOERROR + 21)
    }

    private enum WaveOutMessage {
        Open = 0x3BB,
        Close = 0x3BC,
        Done = 0x3BD
    }

    private enum WaveInMessage {
        Open = 0x3BE,
        Close = 0x3BF,
        Data = 0x3C0
    }

    private const uint WAVE_MAPPER = unchecked((uint)(-1));

    private delegate void DOutCallback(IntPtr hdrvr, WaveOutMessage uMsg, int dwUser, IntPtr wavhdr, int dwParam2);

    private delegate void DInCallback(IntPtr hdrvr, WaveInMessage uMsg, int dwUser, IntPtr wavhdr, int dwParam2);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveOutOpen(ref IntPtr hwo, uint uDeviceID, ref WAVEFORMATEX lpFormat, DOutCallback dwCallback, IntPtr dwInstance, uint dwFlags = 0x00030000);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInOpen(ref IntPtr hwi, uint uDeviceID, ref WAVEFORMATEX lpFormat, DInCallback dwCallback, IntPtr dwInstance, uint dwFlags = 0x00030000);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveOutClose(IntPtr hwo);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInClose(IntPtr hwi);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveOutPrepareHeader(IntPtr hwo, IntPtr lpWaveOutHdr, int uSize);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInPrepareHeader(IntPtr hwi, IntPtr lpWaveInHdr, int uSize);

    [DllImport("winmm.dll")]
    private static extern MMRESULT waveOutUnprepareHeader(IntPtr hwo, IntPtr lpWaveOutHdr, int uSize);

    [DllImport("winmm.dll")]
    private static extern MMRESULT waveInUnprepareHeader(IntPtr hwi, IntPtr lpWaveInHdr, int uSize);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveOutReset(IntPtr hwo);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInReset(IntPtr hwi);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveOutWrite(IntPtr hwo, IntPtr pwh, uint cbwh);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInAddBuffer(IntPtr hwi, IntPtr pwh, uint cbwh);

    [DllImport("winmm.dll", SetLastError = true, CharSet = CharSet.Auto)]
    private static extern MMRESULT waveInStart(IntPtr hwi);

    private IntPtr mWaveOutHandle;
    private IntPtr mWaveInHandle;
    private WAVEFORMATEX mWaveFormatEx;
    private WAVEHDR[] mWaveHeader;
    private IntPtr[] mWaveHeaderPtr;
    private DOutCallback mOutCallback;
    private DInCallback mInCallback;

    protected short[] WaveBuffer;
    private int mBufferIndex;
    private bool mIsPlay;

    public int SampleRate { get; }
    public int Channels { get; }
    public int BufferSize { get; }

    public WaveLib(int sampleRate = 44100, int channels = 2, int bufferSize = 4096, int bufferCount = 4, bool rec = false) {
        SampleRate = sampleRate;
        Channels = channels;
        BufferSize = bufferSize;
        mBufferIndex = 0;

        mWaveOutHandle = IntPtr.Zero;
        mWaveHeaderPtr = new IntPtr[bufferCount];
        mWaveHeader = new WAVEHDR[bufferCount];
        mOutCallback = new DOutCallback(OutCallback);
        mInCallback = new DInCallback(InCallback);
        WaveBuffer = new short[BufferSize];

        if (rec) {
            WaveInOpen();
        } else {
            WaveOutOpen();
        }
    }

    public void Dispose() {
        if (IntPtr.Zero != mWaveOutHandle) {
            WaveOutClose();
        }
        if (IntPtr.Zero != mWaveInHandle) {
            WaveInClose();
        }
    }

    private void WaveOutOpen() {
        if (IntPtr.Zero != mWaveOutHandle) {
            WaveOutClose();
        }

        mWaveFormatEx = new WAVEFORMATEX();
        mWaveFormatEx.wFormatTag = 1;
        mWaveFormatEx.nChannels = (ushort)Channels;
        mWaveFormatEx.nSamplesPerSec = (uint)SampleRate;
        mWaveFormatEx.nAvgBytesPerSec = (uint)(SampleRate * Channels * 16 >> 3);
        mWaveFormatEx.nBlockAlign = (ushort)(Channels * 16 >> 3);
        mWaveFormatEx.wBitsPerSample = 16;
        mWaveFormatEx.cbSize = 0;

        waveOutOpen(ref mWaveOutHandle, WAVE_MAPPER, ref mWaveFormatEx, mOutCallback, IntPtr.Zero);

        WaveBuffer = new short[BufferSize];

        for (int i = 0; i < mWaveHeader.Length; ++i) {
            mWaveHeaderPtr[i] = Marshal.AllocHGlobal(Marshal.SizeOf(mWaveHeader[i]));
            mWaveHeader[i].dwBufferLength = (uint)(WaveBuffer.Length * 16 >> 3);
            mWaveHeader[i].lpData = Marshal.AllocHGlobal((int)mWaveHeader[i].dwBufferLength);
            mWaveHeader[i].dwFlags = 0;
            Marshal.Copy(WaveBuffer, 0, mWaveHeader[i].lpData, WaveBuffer.Length);
            Marshal.StructureToPtr(mWaveHeader[i], mWaveHeaderPtr[i], true);

            waveOutPrepareHeader(mWaveOutHandle, mWaveHeaderPtr[i], Marshal.SizeOf(typeof(WAVEHDR)));
            waveOutWrite(mWaveOutHandle, mWaveHeaderPtr[i], (uint)Marshal.SizeOf(typeof(WAVEHDR)));
        }
    }

    private void WaveInOpen() {
        if (IntPtr.Zero != mWaveInHandle) {
            WaveInClose();
        }

        mWaveFormatEx = new WAVEFORMATEX();
        mWaveFormatEx.wFormatTag = 1;
        mWaveFormatEx.nChannels = (ushort)Channels;
        mWaveFormatEx.nSamplesPerSec = (uint)SampleRate;
        mWaveFormatEx.nAvgBytesPerSec = (uint)(SampleRate * Channels * 16 >> 3);
        mWaveFormatEx.nBlockAlign = (ushort)(Channels * 16 >> 3);
        mWaveFormatEx.wBitsPerSample = 16;
        mWaveFormatEx.cbSize = 0;

        waveInOpen(ref mWaveInHandle, WAVE_MAPPER, ref mWaveFormatEx, mInCallback, IntPtr.Zero);

        WaveBuffer = new short[BufferSize];

        for (int i = 0; i < mWaveHeader.Length; ++i) {
            mWaveHeaderPtr[i] = Marshal.AllocHGlobal(Marshal.SizeOf(mWaveHeader[i]));
            mWaveHeader[i].dwBufferLength = (uint)(WaveBuffer.Length * 16 >> 3);
            mWaveHeader[i].lpData = Marshal.AllocHGlobal((int)mWaveHeader[i].dwBufferLength);
            mWaveHeader[i].dwFlags = 0;
            Marshal.Copy(WaveBuffer, 0, mWaveHeader[i].lpData, WaveBuffer.Length);
            Marshal.StructureToPtr(mWaveHeader[i], mWaveHeaderPtr[i], true);

            waveInPrepareHeader(mWaveInHandle, mWaveHeaderPtr[i], Marshal.SizeOf(typeof(WAVEHDR)));
            waveInAddBuffer(mWaveInHandle, mWaveHeaderPtr[i], (uint)Marshal.SizeOf(typeof(WAVEHDR)));
        }
        waveInStart(mWaveInHandle);
    }

    private void WaveOutClose() {
        if (IntPtr.Zero == mWaveOutHandle) {
            return;
        }

        mIsPlay = false;

        waveOutReset(mWaveOutHandle);
        for (int i = 0; i < mWaveHeader.Length; ++i) {
            waveOutUnprepareHeader(mWaveHeaderPtr[i], mWaveOutHandle, Marshal.SizeOf<WAVEHDR>());
            Marshal.FreeHGlobal(mWaveHeader[i].lpData);
            Marshal.FreeHGlobal(mWaveHeaderPtr[i]);
            mWaveHeader[i].lpData = IntPtr.Zero;
            mWaveHeaderPtr[i] = IntPtr.Zero;
        }
        waveOutClose(mWaveOutHandle);
        mWaveOutHandle = IntPtr.Zero;
    }

    private void WaveInClose() {
        if (IntPtr.Zero == mWaveInHandle) {
            return;
        }

        waveInReset(mWaveInHandle);
        for (int i = 0; i < mWaveHeader.Length; ++i) {
            waveInUnprepareHeader(mWaveHeaderPtr[i], mWaveInHandle, Marshal.SizeOf<WAVEHDR>());
            Marshal.FreeHGlobal(mWaveHeader[i].lpData);
            Marshal.FreeHGlobal(mWaveHeaderPtr[i]);
            mWaveHeader[i].lpData = IntPtr.Zero;
            mWaveHeaderPtr[i] = IntPtr.Zero;
        }
        waveInClose(mWaveInHandle);
        mWaveInHandle = IntPtr.Zero;
    }

    private void OutCallback(IntPtr hdrvr, WaveOutMessage uMsg, int dwUser, IntPtr waveHdr, int dwParam2) {
        switch (uMsg) {
        case WaveOutMessage.Open:
            mIsPlay = true;
            break;
        case WaveOutMessage.Close:
            break;
        case WaveOutMessage.Done:
            if (!mIsPlay) {
                break;
            }

            waveOutWrite(mWaveOutHandle, waveHdr, (uint)Marshal.SizeOf(typeof(WAVEHDR)));

            for (mBufferIndex = 0; mBufferIndex < mWaveHeader.Length; ++mBufferIndex) {
                if (mWaveHeaderPtr[mBufferIndex] == waveHdr) {
                    SetData();
                    mWaveHeader[mBufferIndex] = Marshal.PtrToStructure<WAVEHDR>(mWaveHeaderPtr[mBufferIndex]);
                    Marshal.Copy(WaveBuffer, 0, mWaveHeader[mBufferIndex].lpData, WaveBuffer.Length);
                    Marshal.StructureToPtr(mWaveHeader[mBufferIndex], mWaveHeaderPtr[mBufferIndex], true);
                }
            }
            break;
        }
    }

    private void InCallback(IntPtr hdrvr, WaveInMessage uMsg, int dwUser, IntPtr waveHdr, int dwParam2) {
        switch (uMsg) {
        case WaveInMessage.Open:
            break;
        case WaveInMessage.Close:
            break;
        case WaveInMessage.Data:
            var hdr = Marshal.PtrToStructure<WAVEHDR>(waveHdr);
            Marshal.Copy(hdr.lpData, WaveBuffer, 0, WaveBuffer.Length);
            SetData();
            waveInAddBuffer(mWaveInHandle, waveHdr, (uint)Marshal.SizeOf(typeof(WAVEHDR)));
            break;
        }
    }

    protected virtual void SetData() { }
}
