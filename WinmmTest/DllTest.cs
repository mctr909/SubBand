using System;
using System.Runtime.InteropServices;

namespace WinmmTest
{
    class DllTest
    {
        private delegate void DReadCallback(IntPtr buffer);
        private DReadCallback mReadCallback;
        public short[] ReadBuffer;

        [DllImport("sound.dll")]
        private static extern void wavein_open(
            int sampleRate,
            int bits,
            int channels,
            int bufferLength,
            int bufferCount,
            DReadCallback readCallback
        );
        [DllImport("sound.dll")]
        private static extern void wavein_close();

        public DllTest()
        {
            ReadBuffer = new short[1024];
        }

        public void WaveInOpen()
        {
            mReadCallback = new DReadCallback(Read16);
            wavein_open(44100, 16, 1, ReadBuffer.Length, 8, mReadCallback);
        }

        public void WaveInClose()
        {
            wavein_close();
        }

        private void Read16(IntPtr buffer)
        {
            Marshal.Copy(buffer, ReadBuffer, 0, ReadBuffer.Length);
        }
    }
}
