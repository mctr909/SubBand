
namespace WinmmTest
{
    partial class Form1
    {
        /// <summary>
        /// 必要なデザイナー変数です。
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 使用中のリソースをすべてクリーンアップします。
        /// </summary>
        /// <param name="disposing">マネージド リソースを破棄する場合は true を指定し、その他の場合は false を指定します。</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows フォーム デザイナーで生成されたコード

        /// <summary>
        /// デザイナー サポートに必要なメソッドです。このメソッドの内容を
        /// コード エディターで変更しないでください。
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.btnWaveIn = new System.Windows.Forms.Button();
            this.picWaveIn = new System.Windows.Forms.PictureBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.picWaveIn)).BeginInit();
            this.SuspendLayout();
            // 
            // btnWaveIn
            // 
            this.btnWaveIn.Location = new System.Drawing.Point(12, 12);
            this.btnWaveIn.Name = "btnWaveIn";
            this.btnWaveIn.Size = new System.Drawing.Size(90, 26);
            this.btnWaveIn.TabIndex = 0;
            this.btnWaveIn.Text = "WaveInOpen";
            this.btnWaveIn.UseVisualStyleBackColor = true;
            this.btnWaveIn.Click += new System.EventHandler(this.btnWaveIn_Click);
            // 
            // picWaveIn
            // 
            this.picWaveIn.Location = new System.Drawing.Point(12, 44);
            this.picWaveIn.Name = "picWaveIn";
            this.picWaveIn.Size = new System.Drawing.Size(234, 135);
            this.picWaveIn.TabIndex = 1;
            this.picWaveIn.TabStop = false;
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(258, 232);
            this.Controls.Add(this.picWaveIn);
            this.Controls.Add(this.btnWaveIn);
            this.MinimumSize = new System.Drawing.Size(128, 128);
            this.Name = "Form1";
            this.Text = "Form1";
            this.SizeChanged += new System.EventHandler(this.Form1_SizeChanged);
            ((System.ComponentModel.ISupportInitialize)(this.picWaveIn)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnWaveIn;
        private System.Windows.Forms.PictureBox picWaveIn;
        private System.Windows.Forms.Timer timer1;
    }
}

