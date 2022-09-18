using System.Drawing.Imaging;

namespace CQT {
    public partial class Form1 : Form {
        readonly Pen GRID_MAJOR = new Pen(Color.FromArgb(127, 0, 0), 1.0f);
        readonly Pen GRID_MINOR1 = new Pen(Color.FromArgb(79, 0, 0), 1.0f) {
            DashPattern = new float[] { 3, 1 }
        };
        readonly Pen GRID_MINOR2 = new Pen(Color.FromArgb(79, 0, 0), 1.0f) {
            DashPattern = new float[] { 1, 2 }
        };

        int RANGE_DB = 72;
        bool mSetSize = false;
        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            mSetSize = true;
            timer1.Enabled = true;
            timer1.Interval = 33;
            timer1.Start();
        }

        private void Form1_SizeChanged(object sender, EventArgs e) {
            mSetSize = true;
        }

        private void timer1_Tick(object sender, EventArgs e) {
        }

        void setSize() {
            pictureBox1.Top = 0;
            pictureBox1.Left = 0;
            pictureBox1.Width = Width - 16;
            pictureBox1.Height = Height - 39;
            if (null != pictureBox1.Image) {
                pictureBox1.Image.Dispose();
                pictureBox1.Image = null;
            }
            pictureBox1.Image = new Bitmap(pictureBox1.Width, pictureBox1.Height, PixelFormat.Format32bppRgb);
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

        int linerToY(double amp, int height, int offset) {
            var min = 1e-100;
            if (amp < min) {
                amp = min;
            }
            amp = 20 * Math.Log10(amp) + 250;
            if (0 < amp) {
                amp = 0.0;
            }
            if (amp < -RANGE_DB) {
                amp = -RANGE_DB;
            }
            return (int)(-amp / RANGE_DB * height + offset);
        }

        void drawSpec(Graphics g, double[] arr, int octNum, int oct, int height, int offset) {
            var width = pictureBox1.Width;
            var gxOfs = width * oct / octNum;
            var gx0 = 0;
            var x0 = 0;
            var y0 = linerToY(arr[gx0], height, offset);
            int y1;
            for (int db = 0; -RANGE_DB < db; db -= 3) {
                var py = dbToY(db, height, offset);
                if (db % 12 == 0) {
                    g.DrawLine(GRID_MAJOR, 0, py, width - 1, py);
                } else if (db % 6 == 0) {
                    g.DrawLine(GRID_MINOR1, 0, py, width - 1, py);
                } else {
                    g.DrawLine(GRID_MINOR2, 0, py, width - 1, py);
                }
            }

            for (int x1 = gxOfs; x1 < gxOfs + width / octNum; x1++) {
                var gx = (double)(x1 - gxOfs) * octNum / width;
                var gx1 = (int)(arr.Length * gx);
                if (1 < gx1 - gx0) {
                    y1 = linerToY(arr[gx0], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = gx0; i <= gx1; i++) {
                        var v = arr[i % arr.Length];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, linerToY(min, height, offset), x1, linerToY(max, height, offset));
                    y1 = linerToY(arr[gx1], height, offset);
                } else {
                    y1 = linerToY(arr[gx1], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }
    }
}