using System.Windows.Forms;

namespace SubBandForm {
    partial class Form1 {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.timer1 = new System.Windows.Forms.Timer(this.components);
            this.tbWindowWidth = new System.Windows.Forms.TrackBar();
            this.tbRange = new System.Windows.Forms.TrackBar();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbWindowWidth)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbRange)).BeginInit();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Location = new System.Drawing.Point(10, 50);
            this.pictureBox1.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(129, 40);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            // 
            // timer1
            // 
            this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
            // 
            // tbWindowWidth
            // 
            this.tbWindowWidth.Location = new System.Drawing.Point(10, 10);
            this.tbWindowWidth.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.tbWindowWidth.Maximum = 24;
            this.tbWindowWidth.Name = "tbWindowWidth";
            this.tbWindowWidth.Size = new System.Drawing.Size(63, 45);
            this.tbWindowWidth.SmallChange = 3;
            this.tbWindowWidth.TabIndex = 1;
            this.tbWindowWidth.TickFrequency = 3;
            this.tbWindowWidth.Value = 9;
            this.tbWindowWidth.Scroll += new System.EventHandler(this.tbWindowWidth_Scroll);
            // 
            // tbRange
            // 
            this.tbRange.Location = new System.Drawing.Point(79, 10);
            this.tbRange.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.tbRange.Maximum = -40;
            this.tbRange.Minimum = -100;
            this.tbRange.Name = "tbRange";
            this.tbRange.Size = new System.Drawing.Size(63, 45);
            this.tbRange.SmallChange = 5;
            this.tbRange.TabIndex = 2;
            this.tbRange.TickFrequency = 10;
            this.tbRange.Value = -60;
            this.tbRange.Scroll += new System.EventHandler(this.tbRange_Scroll);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(150, 110);
            this.Controls.Add(this.tbRange);
            this.Controls.Add(this.tbWindowWidth);
            this.Controls.Add(this.pictureBox1);
            this.Margin = new System.Windows.Forms.Padding(3, 2, 3, 2);
            this.MinimumSize = new System.Drawing.Size(112, 110);
            this.Name = "Form1";
            this.Text = "Form1";
            this.Load += new System.EventHandler(this.Form1_Load);
            this.SizeChanged += new System.EventHandler(this.Form1_SizeChanged);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbWindowWidth)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.tbRange)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private PictureBox pictureBox1;
        private System.Windows.Forms.Timer timer1;
        private TrackBar tbWindowWidth;
        private TrackBar tbRange;
    }
}