namespace SubBandForm {
    public partial class Form1 : Form {
        const int RANGE_DB = 24;
        bool mSetSize = false;
        WaveIn mWaveIn;

        readonly Pen GRID_MAJOR = new Pen(Color.FromArgb(127, 0, 0), 1.0f);
        readonly Pen GRID_MINOR = new Pen(Color.FromArgb(79, 0, 0), 1.0f) {
            DashPattern = new float[] { 3, 2 }
        };

        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            mWaveIn = new WaveIn(44100, 8192, 512);
            mSetSize = true;
            timer1.Enabled = true;
            timer1.Interval = 1;
            timer1.Start();
        }

        private void Form1_SizeChanged(object sender, EventArgs e) {
            mSetSize = true;
        }

        private void timer1_Tick(object sender, EventArgs e) {
            if (mSetSize) {
                setSize();
            }
            var g = Graphics.FromImage(pictureBox1.Image);
            g.Clear(Color.Black);
            var gheight = pictureBox1.Height / 2;
            mWaveIn.Read = true;
            drawWave(g, mWaveIn.Acf, 1.0, 3 / 8.0, 5 / 8.0, gheight, 0);
            drawSpec(g, mWaveIn.AcfSpec, 0.25, gheight, gheight);
            pictureBox1.Image = pictureBox1.Image;
            g.Dispose();
            mSetSize = false;
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
            pictureBox1.Image = new Bitmap(pictureBox1.Width, pictureBox1.Height);
        }

        int getY(double v, double amp, int height, int offset) {
            return (int)((0.5 - v * amp * 0.5) * height + offset);
        }

        int dbToLiner(double v, int height) {
            if (0 < v) {
                v = 0;
            }
            return (int)(-Math.Pow(10, v / 20.0) * height);
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

        void drawWave(Graphics g, double[] arr, double amp, double begin, double end, int height, int offset) {
            var width = pictureBox1.Width;
            var centerY = height / 2;
            g.DrawLine(GRID_MAJOR, width / 2 + 1, offset, width / 2 + 1, offset + height);
            g.DrawLine(GRID_MAJOR, 0, centerY, width - 1, centerY);
            for (float ydb = -3.0f; -12 <= ydb; ydb -= 3.0f) {
                var y = dbToLiner(ydb, height) / 2;
                var yp = centerY + offset + y;
                var ym = centerY + offset - y;
                g.DrawLine(GRID_MINOR, 0, yp, width - 1, yp);
                g.DrawLine(GRID_MINOR, 0, ym, width - 1, ym);
            }
            var gPitch = (double)arr.Length / width;
            var gBegin = begin * width * gPitch;
            gPitch *= end - begin;
            var gx0 = (int)gBegin;
            var x0 = 0;
            var y0 = getY(arr[gx0], amp, height, offset);
            int y1;
            for (int x1 = 0; x1 < width; x1++) {
                var gx1 = (int)(x1 * gPitch + gBegin);
                if (1 < gx1 - gx0) {
                    y1 = getY(arr[gx0], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = gx0; i <= gx1; i++) {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, getY(min, amp, height, offset), x1, getY(max, amp, height, offset));
                    y1 = getY(arr[gx1], amp, height, offset);
                } else {
                    y1 = getY(arr[gx1], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }

        void drawSpec(Graphics g, double[] arr, double scale, int height, int offset) {
            var width = pictureBox1.Width;
            var gx0 = (int)(Math.Pow(arr.Length / 10.0, -1) * (arr.Length - 1));
            var x0 = 0;
            var y0 = dbToY(arr[gx0] * scale, height, offset);
            int y1;
            for (int db = 0; -RANGE_DB < db; db -= 3) {
                var py = dbToY(db, height, offset);
                if (db % 12 == 0) {
                    g.DrawLine(GRID_MAJOR, 0, py, width - 1, py);
                } else {
                    g.DrawLine(GRID_MINOR, 0, py, width - 1, py);
                }
            }
            for (int x1 = 0; x1 < width; x1++) {
                var gx1 = (int)(Math.Pow(arr.Length / 10.0, (double)x1 / width - 1) * (arr.Length - 1));
                if (1 < gx1 - gx0) {
                    y1 = dbToY(arr[gx0] * scale, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = gx0; i <= gx1; i++) {
                        var v = arr[i] * scale;
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, dbToY(min, height, offset), x1, dbToY(max, height, offset));
                    y1 = dbToY(arr[gx1] * scale, height, offset);
                } else {
                    y1 = dbToY(arr[gx1] * scale, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }
    }
}
