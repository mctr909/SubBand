namespace SubBandForm {
    public partial class Form1 : Form {
        const int DATA_N = 4096;
        const int DFT_N = 256;
        const int DFT_NH = DFT_N / 2;
        const int DFT_NQ1 = DFT_N / 4;
        SubBand mSb1 = new SubBand(DFT_N);
        ACF mAcfL = new ACF(DATA_N);
        ACF mAcfH = new ACF(DATA_N);
        double[] mOutAcfL = new double[DATA_N];
        double[] mOutAcfH = new double[DATA_N];
        double[] mOutL = new double[DATA_N];
        double[] mOutH = new double[DATA_N];
        double[] mInput = new double[DFT_NH];
        double mOscCount = 0.0;
        double mOscPitch = 4.0;
        bool mSetSize = false;
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
            g.Clear(Color.Transparent);
            draw(g, mOutAcfL, 0.5, pictureBox1.Height / 2, 0);
            draw(g, mOutAcfH, 0.5, pictureBox1.Height / 2, pictureBox1.Height / 2);
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

        int getY(double v, int height, int offset) {
            return (int)((0.5 - 0.375 * v) * height + offset);
        }

        void calc() {
            for (int i = 0, k = 0; i < DATA_N; i += DFT_NH, k += DFT_NQ1) {
                for (int j = 0; j < DFT_NH; j++) {
                    var tmp = 0.0;
                    for (int o = 0; o < 16; o++) {
                        if (mOscCount < 0.5) {
                            tmp += 1;
                        } else {
                            tmp -= 1;
                        }
                        mOscPitch = 4.0 + 12.0 * Math.Sin(mOscCount * 0.02);
                        mOscCount += mOscPitch / (DATA_N * 16.0);
                        if (1.0 <= mOscCount) {
                            mOscCount -= 1.0;
                        }
                    }
                    mInput[j] = tmp / 16.0;
                }
                mSb1.execute(ref mInput);
                for (int j = 0; j < DFT_NQ1; j++) {
                    mOutL[k + j] = mSb1.OutputL[j];
                    mOutH[k + j] = mSb1.OutputH[j];
                }
            }
            mAcfL.Exec(mOutL, mOutAcfL);
            mAcfH.Exec(mOutH, mOutAcfH);
            var baseL = mOutAcfL[0];
            var baseH = mOutAcfH[0];
            for (int j = 0; j < DATA_N; j++) {
                mOutAcfL[j] /= baseL;
                mOutAcfH[j] /= baseH;
            }
        }

        void draw(Graphics g, double[] arr, double size, int height, int offset) {
            var gp = (double)arr.Length / pictureBox1.Width * size;
            var x0 = 0;
            var y0 = 0;
            int y1;
            var gx0 = 0.0;
            for (int x1 = 0; x1 < pictureBox1.Width; x1++) {
                var gx1 = x1 * gp;
                if (1.0 < gx1 - gx0) {
                    y1 = getY(arr[(int)gx0], height, offset);
                    g.DrawLine(Pens.Navy, x0, y0, x1, y1);
                    var max = double.MinValue;
                    var min = double.MaxValue;
                    for (var i = (int)gx0; i <= gx1; i++) {
                        var v = arr[i];
                        min = Math.Min(min, v);
                        max = Math.Max(max, v);
                    }
                    g.DrawLine(Pens.Navy, x1, getY(min, height, offset), x1, getY(max, height, offset));
                    y1 = getY(arr[(int)gx1], height, offset);
                } else {
                    y1 = getY(arr[(int)gx1], height, offset);
                    g.DrawLine(Pens.Navy, x0, y0, x1, y1);
                }
                x0 = x1;
                y0 = y1;
                gx0 = gx1;
            }
        }
    }
}
