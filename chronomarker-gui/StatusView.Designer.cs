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
            splitContainer1 = new SplitContainer();
            label1 = new Label();
            groupBox2 = new GroupBox();
            splitContainer2 = new SplitContainer();
            label2 = new Label();
            myProgressBar2 = new MyProgressBar();
            label3 = new Label();
            checkBox1 = new CheckBox();
            checkBox2 = new CheckBox();
            label4 = new Label();
            groupBox1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer1).BeginInit();
            splitContainer1.Panel1.SuspendLayout();
            splitContainer1.Panel2.SuspendLayout();
            splitContainer1.SuspendLayout();
            groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)splitContainer2).BeginInit();
            splitContainer2.Panel1.SuspendLayout();
            splitContainer2.Panel2.SuspendLayout();
            splitContainer2.SuspendLayout();
            SuspendLayout();
            // 
            // myProgressBar1
            // 
            myProgressBar1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            myProgressBar1.Location = new Point(3, 3);
            myProgressBar1.Name = "myProgressBar1";
            myProgressBar1.Size = new Size(283, 30);
            myProgressBar1.TabIndex = 0;
            myProgressBar1.Value = 50;
            myProgressBar1.Value2 = 30;
            // 
            // groupBox1
            // 
            groupBox1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            groupBox1.Controls.Add(splitContainer1);
            groupBox1.Location = new Point(3, 3);
            groupBox1.Name = "groupBox1";
            groupBox1.Size = new Size(429, 65);
            groupBox1.TabIndex = 1;
            groupBox1.TabStop = false;
            groupBox1.Text = "Player";
            // 
            // splitContainer1
            // 
            splitContainer1.Dock = DockStyle.Fill;
            splitContainer1.FixedPanel = FixedPanel.Panel1;
            splitContainer1.IsSplitterFixed = true;
            splitContainer1.Location = new Point(3, 19);
            splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            splitContainer1.Panel1.Controls.Add(label1);
            // 
            // splitContainer1.Panel2
            // 
            splitContainer1.Panel2.Controls.Add(myProgressBar1);
            splitContainer1.Size = new Size(423, 43);
            splitContainer1.SplitterDistance = 130;
            splitContainer1.TabIndex = 1;
            splitContainer1.SplitterMoved += splitContainer1_SplitterMoved;
            // 
            // label1
            // 
            label1.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            label1.Font = new Font("Segoe UI", 12F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label1.Location = new Point(3, 3);
            label1.Name = "label1";
            label1.Size = new Size(124, 30);
            label1.TabIndex = 3;
            label1.Text = "O2 / CO2";
            label1.TextAlign = ContentAlignment.MiddleRight;
            // 
            // groupBox2
            // 
            groupBox2.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            groupBox2.Controls.Add(splitContainer2);
            groupBox2.Location = new Point(3, 74);
            groupBox2.Name = "groupBox2";
            groupBox2.Size = new Size(429, 211);
            groupBox2.TabIndex = 2;
            groupBox2.TabStop = false;
            groupBox2.Text = "Environment";
            // 
            // splitContainer2
            // 
            splitContainer2.Dock = DockStyle.Fill;
            splitContainer2.FixedPanel = FixedPanel.Panel1;
            splitContainer2.IsSplitterFixed = true;
            splitContainer2.Location = new Point(3, 19);
            splitContainer2.Name = "splitContainer2";
            // 
            // splitContainer2.Panel1
            // 
            splitContainer2.Panel1.Controls.Add(label4);
            splitContainer2.Panel1.Controls.Add(label3);
            splitContainer2.Panel1.Controls.Add(label2);
            // 
            // splitContainer2.Panel2
            // 
            splitContainer2.Panel2.Controls.Add(checkBox2);
            splitContainer2.Panel2.Controls.Add(checkBox1);
            splitContainer2.Panel2.Controls.Add(myProgressBar2);
            splitContainer2.Size = new Size(423, 189);
            splitContainer2.SplitterDistance = 130;
            splitContainer2.TabIndex = 1;
            // 
            // label2
            // 
            label2.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            label2.Font = new Font("Segoe UI", 12F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label2.Location = new Point(3, 3);
            label2.Name = "label2";
            label2.Size = new Size(124, 30);
            label2.TabIndex = 3;
            label2.Text = "Local time";
            label2.TextAlign = ContentAlignment.MiddleRight;
            // 
            // myProgressBar2
            // 
            myProgressBar2.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            myProgressBar2.Location = new Point(3, 3);
            myProgressBar2.Name = "myProgressBar2";
            myProgressBar2.Size = new Size(283, 30);
            myProgressBar2.TabIndex = 0;
            myProgressBar2.Value = 50;
            myProgressBar2.Value2 = 50;
            // 
            // label3
            // 
            label3.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            label3.Font = new Font("Segoe UI", 12F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label3.Location = new Point(3, 33);
            label3.Name = "label3";
            label3.Size = new Size(124, 30);
            label3.TabIndex = 4;
            label3.Text = "On land?";
            label3.TextAlign = ContentAlignment.MiddleRight;
            // 
            // checkBox1
            // 
            checkBox1.AutoCheck = false;
            checkBox1.CheckAlign = ContentAlignment.MiddleCenter;
            checkBox1.Checked = true;
            checkBox1.CheckState = CheckState.Indeterminate;
            checkBox1.Location = new Point(3, 39);
            checkBox1.Name = "checkBox1";
            checkBox1.Size = new Size(283, 24);
            checkBox1.TabIndex = 1;
            checkBox1.UseVisualStyleBackColor = true;
            // 
            // checkBox2
            // 
            checkBox2.AutoCheck = false;
            checkBox2.CheckAlign = ContentAlignment.MiddleCenter;
            checkBox2.Checked = true;
            checkBox2.CheckState = CheckState.Indeterminate;
            checkBox2.Location = new Point(3, 69);
            checkBox2.Name = "checkBox2";
            checkBox2.Size = new Size(283, 24);
            checkBox2.TabIndex = 2;
            checkBox2.UseVisualStyleBackColor = true;
            // 
            // label4
            // 
            label4.Anchor = AnchorStyles.Top | AnchorStyles.Left | AnchorStyles.Right;
            label4.Font = new Font("Segoe UI", 12F, FontStyle.Regular, GraphicsUnit.Point, 0);
            label4.Location = new Point(3, 63);
            label4.Name = "label4";
            label4.Size = new Size(124, 30);
            label4.TabIndex = 5;
            label4.Text = "Is scanning?";
            label4.TextAlign = ContentAlignment.MiddleRight;
            // 
            // StatusView
            // 
            AutoScaleDimensions = new SizeF(7F, 15F);
            AutoScaleMode = AutoScaleMode.Font;
            Controls.Add(groupBox2);
            Controls.Add(groupBox1);
            Name = "StatusView";
            Size = new Size(435, 389);
            groupBox1.ResumeLayout(false);
            splitContainer1.Panel1.ResumeLayout(false);
            splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer1).EndInit();
            splitContainer1.ResumeLayout(false);
            groupBox2.ResumeLayout(false);
            splitContainer2.Panel1.ResumeLayout(false);
            splitContainer2.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)splitContainer2).EndInit();
            splitContainer2.ResumeLayout(false);
            ResumeLayout(false);
        }

        #endregion

        private MyProgressBar myProgressBar1;
        private GroupBox groupBox1;
        private SplitContainer splitContainer1;
        private Label label1;
        private GroupBox groupBox2;
        private SplitContainer splitContainer2;
        private Label label3;
        private Label label2;
        private MyProgressBar myProgressBar2;
        private Label label4;
        private CheckBox checkBox2;
        private CheckBox checkBox1;
    }
}
