using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace SubBandForm {
    public partial class Form1 : Form {
        int RANGE_DB = 90;
        bool mSetSize = false;
        WaveIn mWaveIn;
        byte[] mPix;

        readonly Pen GRID_MAJOR = new Pen(Color.FromArgb(127, 0, 0), 1.0f);
        readonly Pen GRID_MINOR1 = new Pen(Color.FromArgb(79, 0, 0), 1.0f);
        readonly Pen GRID_MINOR2 = new Pen(Color.FromArgb(79, 0, 0), 1.0f) {
            DashPattern = new float[] { 2, 3 }
        };

        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            mWaveIn = new WaveIn(44100, 256, 16384);
            tbWindowWidth_Scroll(sender, e);
            tbRange_Scroll(sender, e);
            mSetSize = true;
            timer1.Enabled = true;
            timer1.Interval = 1;
            timer1.Start();
        }

        private void Form1_SizeChanged(object sender, EventArgs e) {
            mSetSize = true;
        }

        private void tbWindowWidth_Scroll(object sender, EventArgs e) {
            mWaveIn.WindowWidth = Math.Pow(2.0, -1 - tbWindowWidth.Value / 12.0);
        }

        private void tbRange_Scroll(object sender, EventArgs e) {
            RANGE_DB = -tbRange.Value;
        }

        private void timer1_Tick(object sender, EventArgs e) {
            if (mSetSize) {
                setSize();
            }
            var g = Graphics.FromImage(pictureBox1.Image);
            g.Clear(Color.Black);
            var gheight = pictureBox1.Height / 2;
            mWaveIn.Read = true;
            drawSpec(g, mWaveIn.AcfSpec, gheight, 0);
            drawScrollSpec(mWaveIn.AcfSpec, gheight, gheight);
            pictureBox1.Image = pictureBox1.Image;
            g.Dispose();
            mSetSize = false;
        }

        void setSize() {
            tbWindowWidth.Top = 0;
            tbWindowWidth.Left = 0;
            pictureBox1.Top = tbWindowWidth.Bottom;
            pictureBox1.Left = 0;
            pictureBox1.Width = Width - 16;
            pictureBox1.Height = Height - tbWindowWidth.Height - 39;
            tbWindowWidth.Width = pictureBox1.Width / 2 - 4;
            tbRange.Top = 0;
            tbRange.Left = tbWindowWidth.Right + 4;
            tbRange.Width = tbWindowWidth.Width;
            if (null != pictureBox1.Image) {
                pictureBox1.Image.Dispose();
                pictureBox1.Image = null;
            }
            pictureBox1.Image = new Bitmap(pictureBox1.Width, pictureBox1.Height, PixelFormat.Format32bppRgb);
            mPix = new byte[4 * pictureBox1.Width * pictureBox1.Height];
        }

        int dbToY(double v, int height, int offset) {
            if (0 < v) {
                v = 0;
            }
            if (v < -RANGE_DB) {
                v = -RANGE_DB;
            }
            return (int)(-v / RANGE_DB * height + offset);
        }

        void drawSpec(Graphics g, double[] arr, int height, int offset) {
            var width = pictureBox1.Width;
            var gx0 = (int)(Math.Pow(arr.Length / 10.0, -1) * (arr.Length - 1));
            var x0 = 0;
            var y0 = dbToY(arr[gx0], height, offset);
            int y1;
            for (double db = 0; -RANGE_DB < db; db -= 1) {
                var py = dbToY(db, height, offset);
                if (db % 10 == 0) {
                    g.DrawLine(GRID_MAJOR, 0, py, width - 1, py);
                } else if (height > RANGE_DB && db % 5 == 0) {
                    g.DrawLine(GRID_MINOR1, 0, py, width - 1, py);
                } else if (height > 3 * RANGE_DB) {
                    g.DrawLine(GRID_MINOR2, 0, py, width - 1, py);
                }
            }
            for (int x1 = 0; x1 < width; x1++) {
                var gx1 = (int)(Math.Pow(arr.Length / 10.0, (double)x1 / width - 1) * (arr.Length - 1));
                if (1 < gx1 - gx0) {
                    y1 = dbToY(arr[gx0], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = gx0; i <= gx1; i++) {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, dbToY(min, height, offset), x1, dbToY(max, height, offset));
                    y1 = dbToY(arr[gx1], height, offset);
                } else {
                    y1 = dbToY(arr[gx1], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }

        void dbToHue(byte[] pix, int offset, double v) {
            v += 24;
            if (0 < v) {
                v = 0;
            }
            var range = RANGE_DB - 24;
            if (v < -range) {
                v = -range;
            }
            v = range + v;
            var g = (int)(v / range * 1279);
            if (g < 256) {
                pix[offset + 0] = (byte)g;
                pix[offset + 1] = 0;
                pix[offset + 2] = 0;
            }
            else if (g < 512) {
                g -= 256;
                pix[offset + 0] = 255;
                pix[offset + 1] = (byte)g;
                pix[offset + 2] = 0;
            }
            else if (g < 768) {
                g -= 512;
                pix[offset + 0] = (byte)(255 - g);
                pix[offset + 1] = 255;
                pix[offset + 2] = 0;
            }
            else if (g < 1024) {
                g -= 768;
                pix[offset + 0] = 0;
                pix[offset + 1] = 255;
                pix[offset + 2] = (byte)g;
            }
            else {
                g -= 1024;
                pix[offset + 0] = 0;
                pix[offset + 1] = (byte)(255 - g);
                pix[offset + 2] = 255;
            }
        }

        void drawScrollSpec(double[] arr, int height, int offset) {
            var bmp = (Bitmap)pictureBox1.Image;
            var bmpData = bmp.LockBits(new Rectangle(Point.Empty, bmp.Size), ImageLockMode.WriteOnly, bmp.PixelFormat);
            var size = 4 * bmp.Width * height;
            var offsetY = 4 * bmp.Width * offset;
            for (int i = 0; i < 4; i++) {
                Array.Copy(mPix, bmpData.Stride * i, mPix, bmpData.Stride * (i + 1), bmpData.Stride);
            }
            Array.Copy(mPix, 0, mPix, bmpData.Stride * 4, size - bmpData.Stride * 4);
            var gx0 = (int)(Math.Pow(arr.Length / 10.0, -1) * (arr.Length - 1));
            for (int s = 0, x = 0; x < bmp.Width; s += 4, x++) {
                var gx1 = (int)(Math.Pow(arr.Length / 10.0, (double)x / bmp.Width - 1) * (arr.Length - 1));
                if (1 < gx1 - gx0) {
                    dbToHue(mPix, s, arr[gx0]);
                    var max = double.MinValue;
                    for (var i = gx0; i <= gx1; i++) {
                        var v = arr[i];
                        max = Math.Max(max, v);
                    }
                    dbToHue(mPix, s, max);
                }
                else {
                    dbToHue(mPix, s, arr[gx1]);
                }
                gx0 = gx1;
            }
            Marshal.Copy(mPix, 0, bmpData.Scan0 + offsetY, size);
            bmp.UnlockBits(bmpData);
        }
    }
}
