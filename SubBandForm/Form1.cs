namespace SubBandForm {
    public partial class Form1 : Form {
        const int RANGE_DB = 100;
        const int DATA_N = 4096;
        
        ACF mAcfL1 = new ACF(DATA_N);
        double[] mAcf = new double[DATA_N];
        double[] mAcfSpec = new double[DATA_N];
        double[] mInput = new double[DATA_N];
        double mOscCount1 = 0.0;
        double mOscCount2 = 0.0;
        bool mSetSize = false;

        readonly Pen GRID_MAJOR = new Pen(Color.FromArgb(127, 0, 0), 1.0f);
        readonly Pen GRID_MINOR = new Pen(Color.FromArgb(63, 0, 0), 1.0f);

        public Form1() {
            InitializeComponent();
        }

        private void Form1_Load(object sender, EventArgs e) {
            mSetSize = true;
            timer1.Enabled = true;
            timer1.Interval = 16;
            timer1.Start();
        }

        private void Form1_SizeChanged(object sender, EventArgs e) {
            mSetSize = true;
        }

        private void timer1_Tick(object sender, EventArgs e) {
            if (mSetSize) {
                setSize();
            }
            calc();
            var g = Graphics.FromImage(pictureBox1.Image);
            g.Clear(Color.Black);
            var gheight = pictureBox1.Height / 2;
            drawWave(g, mAcf, 1.0, 1.0, gheight, 0);
            drawSpec(g, mAcfSpec, gheight, gheight);
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

        int getYdb(double v, int height, int offset) {
            if (0 < v) {
                v = 0;
            }
            if (v < -RANGE_DB) {
                v = -RANGE_DB;
            }
            return (int)(-v / RANGE_DB * height + offset);
        }

        void calc() {
            var oscAmp1 = Math.Pow(10, 0 / 20.0);
            var oscAmp2 = Math.Pow(10, -6 / 20.0);
            for (int i = 0; i < DATA_N; i++) {
                var tmp = 0.0;
                for (int o = 0; o < 16; o++) {
                    if (mOscCount1 < 0.5) {
                        tmp += oscAmp1;
                    } else {
                        tmp -= oscAmp1;
                    }
                    if (mOscCount2 < 0.5) {
                        tmp += oscAmp2;
                    } else {
                        tmp -= oscAmp2;
                    }
                    //tmp += Math.Sin(2 * Math.PI * mOscCount1) * oscAmp1;
                    //tmp += Math.Sin(2 * Math.PI * mOscCount2) * oscAmp2;
                    mOscCount1 += 100 / (44100 * 16.0);
                    mOscCount2 += 450 / (44100 * 16.0);
                    if (1.0 <= mOscCount1) {
                        mOscCount1 -= 1.0;
                    }
                    if (1.0 <= mOscCount2) {
                        mOscCount2 -= 1.0;
                    }
                }
                mInput[i] = tmp;
            }
            mAcfL1.ExecN(mInput, mAcf, 2);
            mAcfL1.Spec(mAcf, mAcfSpec);
        }

        void drawWave(Graphics g, double[] arr, double amp, double size, int height, int offset) {
            var width = pictureBox1.Width;
            var centerY = offset + height / 2;
            for (float yp = 0.0f, ym = 0.0f; yp < height / 2; yp += height / 20.0f, ym -= height / 20.0f) {
                if (yp == 0.0f) {
                    g.DrawLine(GRID_MAJOR, 0, yp + centerY, width - 1, yp + centerY);
                } else {
                    g.DrawLine(GRID_MINOR, 0, yp + centerY, width - 1, yp + centerY);
                    g.DrawLine(GRID_MINOR, 0, ym + centerY, width - 1, ym + centerY);
                }
            }
            var gp = (double)arr.Length / width * size;
            var x0 = 0;
            var y0 = getY(arr[0], amp, height, offset);
            int y1;
            var gx0 = 0.0;
            for (int x1 = 0; x1 < width; x1++) {
                var gx1 = x1 * gp;
                if (1.0 < gx1 - gx0) {
                    y1 = getY(arr[(int)gx0], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = (int)gx0; i <= gx1; i++) {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, getY(min, amp, height, offset), x1, getY(max, amp, height, offset));
                    y1 = getY(arr[(int)gx1], amp, height, offset);
                } else {
                    y1 = getY(arr[(int)gx1], amp, height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }

        void drawSpec(Graphics g, double[] arr, int height, int offset) {
            var width = pictureBox1.Width;
            var x0 = 0;
            var y0 = getYdb(arr[0], height, offset);
            int y1;
            var gx0 = 0.0;
            for (int db = 0; -RANGE_DB < db; db -= RANGE_DB / 5) {
                var py = getYdb(db, height, offset);
                g.DrawLine(GRID_MAJOR, 0, py, width - 1, py);
            }
            for (int x1 = 0; x1 < width; x1++) {
                var gx1 = Math.Pow(arr.Length / 10.0, (double)x1 / width - 1) * (arr.Length - 1);
                if (1.0 < gx1 - gx0) {
                    y1 = getYdb(arr[(int)gx0], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = (int)gx0; i <= gx1; i++) {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Green, x1, getYdb(min, height, offset), x1, getYdb(max, height, offset));
                    y1 = getYdb(arr[(int)gx1], height, offset);
                } else {
                    y1 = getYdb(arr[(int)gx1], height, offset);
                    g.DrawLine(Pens.Green, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }
    }
}
