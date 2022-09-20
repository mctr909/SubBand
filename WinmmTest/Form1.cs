using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WinmmTest
{
    public partial class Form1 : Form
    {
        readonly Pen GRID_MAJOR = new Pen(Color.FromArgb(127, 0, 0), 1.0f);
        readonly Pen GRID_MINOR1 = new Pen(Color.FromArgb(79, 0, 0), 1.0f)
        {
            DashPattern = new float[] { 3, 1 }
        };
        readonly Pen GRID_MINOR2 = new Pen(Color.FromArgb(79, 0, 0), 1.0f)
        {
            DashPattern = new float[] { 1, 2 }
        };

        DllTest mDll;
        bool mSizeChange = false;

        public Form1()
        {
            InitializeComponent();
            mSizeChange = true;
            mDll = new DllTest();
            timer1.Enabled = true;
            timer1.Interval = 1;
            timer1.Start();
        }

        private void btnWaveIn_Click(object sender, EventArgs e)
        {
            switch (btnWaveIn.Text)
            {
                case "WaveInOpen":
                    mDll.WaveInOpen();
                    btnWaveIn.Text = "WaveInClose";
                    break;
                case "WaveInClose":
                    mDll.WaveInClose();
                    btnWaveIn.Text = "WaveInOpen";
                    break;
            }
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            if (mSizeChange)
            {
                sizeChange();
            }

            var g = Graphics.FromImage(picWaveIn.Image);
            g.Clear(Color.Black);

            drawWave(g, mDll.ReadBuffer, 4, 0, 1, picWaveIn.Width, picWaveIn.Height, 0);

            picWaveIn.Image = picWaveIn.Image;
            g.Dispose();
            mSizeChange = false;
        }

        private void Form1_SizeChanged(object sender, EventArgs e)
        {
            mSizeChange = true;
        }

        void sizeChange()
        {
            picWaveIn.Left = 0;
            picWaveIn.Width = Width - 16;
            if (null != picWaveIn.Image)
            {
                picWaveIn.Image.Dispose();
            }
            picWaveIn.Image = new Bitmap(picWaveIn.Width, picWaveIn.Height);
        }

        int dbToLiner(double v, int height)
        {
            if (0 < v)
            {
                v = 0;
            }
            return (int)(-Math.Pow(10, v / 20.0) * height);
        }

        int getY(short v, double amp, int height, int offset)
        {
            amp = 0.5 - (v / 32768.0) * amp * 0.5;
            if (amp < 0)
            {
                amp = 0;
            }
            if (1 < amp)
            {
                amp = 1;
            }
            return (int)(amp * height + offset);
        }

        void drawWave(Graphics g, short[] arr, double amp, double begin, double end, int width, int height, int offset)
        {
            var centerY = height / 2;
            g.DrawLine(GRID_MAJOR, width / 2 + 1, offset, width / 2 + 1, offset + height);
            g.DrawLine(GRID_MAJOR, 0, centerY, width - 1, centerY);
            for (float ydb = -3.0f; -24 <= ydb; ydb -= 3.0f)
            {
                var y = dbToLiner(ydb, height) / 2;
                var yp = centerY + offset + y;
                var ym = centerY + offset - y;
                g.DrawLine(GRID_MINOR1, 0, yp, width - 1, yp);
                g.DrawLine(GRID_MINOR1, 0, ym, width - 1, ym);
            }
            var gPitch = (double)arr.Length / width;
            var gBegin = begin * width * gPitch;
            gPitch *= end - begin;
            var gx0 = (int)gBegin;
            var x0 = 0;
            var y0 = getY(arr[gx0], amp, height, offset);
            int y1;
            for (int x1 = 0; x1 < width; x1++)
            {
                var gx1 = (int)(x1 * gPitch + gBegin);
                if (1 < gx1 - gx0)
                {
                    y1 = getY(arr[gx0], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = short.MinValue;
                    var min = short.MaxValue;
                    for (var i = gx0; i <= gx1; i++)
                    {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, getY(min, amp, height, offset), x1, getY(max, amp, height, offset));
                    y1 = getY(arr[gx1], amp, height, offset);
                }
                else
                {
                    y1 = getY(arr[gx1], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }
    }
}
