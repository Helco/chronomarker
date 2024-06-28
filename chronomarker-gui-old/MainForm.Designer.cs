namespace Chronomarker
{
    partial class MainForm
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
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

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            statusView1 = new StatusView();
            textBox1 = new TextBox();
            SuspendLayout();
            // 
            // statusView1
            // 
            statusView1.Location = new Point(12, 12);
            statusView1.Name = "statusView1";
            statusView1.Size = new Size(563, 318);
            statusView1.TabIndex = 0;
            // 
            // textBox1
            // 
            textBox1.Location = new Point(12, 290);
            textBox1.Multiline = true;
            textBox1.Name = "textBox1";
            textBox1.ReadOnly = true;
            textBox1.Size = new Size(467, 245);
            textBox1.TabIndex = 1;
            // 
            // MainForm
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(587, 547);
            Controls.Add(textBox1);
            Controls.Add(statusView1);
            Margin = new Padding(2);
            Name = "MainForm";
            Text = "Form1";
            FormClosed += MainForm_FormClosed;
            Load += Form1_Load;
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private StatusView statusView1;
        private TextBox textBox1;
    }
}
