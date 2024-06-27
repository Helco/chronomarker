namespace Chronomarker
{
    partial class StatusView
    {
        /// <summary> 
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary> 
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Component Designer generated code

        /// <summary> 
        /// Required method for Designer support - do not modify 
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            myProgressBar1 = new MyProgressBar();
            groupBox1 = new GroupBox();
            groupBox1.SuspendLayout();
            SuspendLayout();
            // 
            // myProgressBar1
            // 
            myProgressBar1.Location = new Point(6, 22);
            myProgressBar1.Name = "myProgressBar1";
            myProgressBar1.Size = new Size(394, 38);
            myProgressBar1.TabIndex = 0;
            myProgressBar1.Value = 50;
            myProgressBar1.Value2 = 30;
            // 
            // groupBox1
            // 
            groupBox1.Controls.Add(myProgressBar1);
            groupBox1.Location = new Point(3, 3);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new Size(406, 69);
            groupBox1.TabIndex = 1;
            groupBox1.TabStop = false;
            groupBox1.Text = "Health";
            // 
            // StatusView
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            Controls.Add(groupBox1);
            Name = "StatusView";
            Size = new Size(412, 418);
            groupBox1.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private MyProgressBar myProgressBar1;
        private GroupBox groupBox1;
    }
}
