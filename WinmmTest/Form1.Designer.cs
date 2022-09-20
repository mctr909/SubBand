
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
            this.btnWaveIn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // btnWaveIn
            // 
            this.btnWaveIn.Location = new System.Drawing.Point(12, 12);
            this.btnWaveIn.Name = "btnWaveIn";
            this.btnWaveIn.Size = new System.Drawing.Size(75, 23);
            this.btnWaveIn.TabIndex = 0;
            this.btnWaveIn.Text = "WaveIn";
            this.btnWaveIn.UseVisualStyleBackColor = true;
            this.btnWaveIn.Click += new System.EventHandler(this.btnWaveIn_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(258, 232);
            this.Controls.Add(this.btnWaveIn);
            this.Name = "Form1";
            this.Text = "Form1";
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button btnWaveIn;
    }
}

