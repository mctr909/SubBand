using System;
using System.Runtime.InteropServices;

namespace WinmmTest
{
    class DllTest
    {
        delegate void DCallback(IntPtr buffer);
        DCallback mReadCallback;
        DCallback mWriteCallback;
        double mWriteCount;

        public short[] ReadBuffer;
        public short[] WriteBuffer;
        
        [DllImport("sound.dll")]
        private static extern void wavein_open(
            int sampleRate,
            int bits,
            int channels,
            int bufferLength,
            int bufferCount,
            DCallback readCallback
        );
        [DllImport("sound.dll")]
        private static extern void waveout_open(
            int sampleRate,
            int bits,
            int channels,
            int bufferLength,
            int bufferCount,
            DCallback writeCallback
        );
        [DllImport("sound.dll")]
        private static extern void wavein_close();
        [DllImport("sound.dll")]
        private static extern void waveout_close();

        public DllTest()
        {
            ReadBuffer = new short[1024];
            WriteBuffer = new short[1024];
        }

        public void WaveInOpen()
        {
            mReadCallback = new DCallback(Read16);
            wavein_open(44100, 16, 1, ReadBuffer.Length, 8, mReadCallback);
        }

        public void WaveOutOpen()
        {
            mWriteCallback = new DCallback(Write16);
            waveout_open(44100, 16, 1, WriteBuffer.Length, 8, mWriteCallback);
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
