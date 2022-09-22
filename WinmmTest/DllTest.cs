using System;
using System.Runtime.InteropServices;

namespace WinmmTest
{
    class DllTest : IDisposable
    {
        delegate void DCallback(IntPtr buffer);
        DCallback mReadCallback;
        DCallback mWriteCallback;
        double mWriteCount;

        public short[] ReadBuffer;
        public short[] WriteBuffer;

        [DllImport("sound.dll")]
        private static extern void wavein_setconfig(
            int sampleRate,
            int bits,
            int channels,
            int bufferLength,
            int bufferCount
        );
        [DllImport("sound.dll")]
        private static extern void wavein_setcallback(DCallback readCallback);
        [DllImport("sound.dll")]
        private static extern void wavein_open();
        [DllImport("sound.dll")]
        private static extern void wavein_close();

        [DllImport("sound.dll")]
        private static extern void waveout_setconfig(
            int sampleRate,
            int bits,
            int channels,
            int bufferLength,
            int bufferCount
        );
        [DllImport("sound.dll")]
        private static extern void waveout_setcallback(DCallback writeCallback);
        [DllImport("sound.dll")]
        private static extern void waveout_open();
        
        [DllImport("sound.dll")]
        private static extern void waveout_close();

        public DllTest()
        {
            ReadBuffer = new short[1024];
            WriteBuffer = new short[1024];
        }

        public void Dispose() {
            wavein_close();
            waveout_close();
        }

        public void WaveInOpen()
        {
            mReadCallback = new DCallback(Read16);
            wavein_setconfig(44100, 16, 1, ReadBuffer.Length, 8);
            wavein_setcallback(mReadCallback);
            wavein_open();
        }

        public void WaveOutOpen()
        {
            mWriteCallback = new DCallback(Write16);
            waveout_setconfig(44100, 16, 1, WriteBuffer.Length, 8);
            waveout_setcallback(mWriteCallback);
            waveout_open();
        }

        public void WaveInClose()
        {
            wavein_close();
        }

        public void WaveOutClose()
        {
            waveout_close();
        }

        private void Read16(IntPtr buffer)
        {
            Marshal.Copy(buffer, ReadBuffer, 0, ReadBuffer.Length);
        }

        private void Write16(IntPtr buffer)
        {
            for (int i = 0; i < WriteBuffer.Length; i++)
            {
                WriteBuffer[i] = (short)(256 * Math.Sin(2 * Math.PI * mWriteCount));
                mWriteCount += 440 / 44100.0;
                if (1.0 <= mWriteCount)
                {
                    mWriteCount -= 1.0;
                }
            }
            Marshal.Copy(WriteBuffer, 0, buffer, WriteBuffer.Length);
        }
    }
}
