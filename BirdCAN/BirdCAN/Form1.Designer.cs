namespace BirdCAN
{
    partial class Form1
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

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
      this.components = new System.ComponentModel.Container();
      this.cbCANAdapter = new System.Windows.Forms.ComboBox();
      this.label1 = new System.Windows.Forms.Label();
      this.tbMotorSpeed = new System.Windows.Forms.TrackBar();
      this.chkRearLight = new System.Windows.Forms.CheckBox();
      this.btnConnect = new System.Windows.Forms.Button();
      this.dataGridView1 = new System.Windows.Forms.DataGridView();
      this.label2 = new System.Windows.Forms.Label();
      this.chkUnlockBattery = new System.Windows.Forms.CheckBox();
      this.label3 = new System.Windows.Forms.Label();
      this.cbChannel = new System.Windows.Forms.ComboBox();
      this.timer1 = new System.Windows.Forms.Timer(this.components);
      ((System.ComponentModel.ISupportInitialize)(this.tbMotorSpeed)).BeginInit();
      ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
      this.SuspendLayout();
      // 
      // cbCANAdapter
      // 
      this.cbCANAdapter.DisplayMember = "Path";
      this.cbCANAdapter.FormattingEnabled = true;
      this.cbCANAdapter.Location = new System.Drawing.Point(120, 16);
      this.cbCANAdapter.Margin = new System.Windows.Forms.Padding(4);
      this.cbCANAdapter.Name = "cbCANAdapter";
      this.cbCANAdapter.Size = new System.Drawing.Size(243, 24);
      this.cbCANAdapter.TabIndex = 1;
      this.cbCANAdapter.SelectedIndexChanged += new System.EventHandler(this.cbCANAdapter_SelectedIndexChanged);
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Location = new System.Drawing.Point(16, 21);
      this.label1.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(89, 16);
      this.label1.TabIndex = 0;
      this.label1.Text = "CAN Adapter:";
      // 
      // tbMotorSpeed
      // 
      this.tbMotorSpeed.Location = new System.Drawing.Point(16, 135);
      this.tbMotorSpeed.Margin = new System.Windows.Forms.Padding(4);
      this.tbMotorSpeed.Maximum = 400;
      this.tbMotorSpeed.Name = "tbMotorSpeed";
      this.tbMotorSpeed.Size = new System.Drawing.Size(399, 56);
      this.tbMotorSpeed.TabIndex = 7;
      this.tbMotorSpeed.TickFrequency = 413;
      this.tbMotorSpeed.Scroll += new System.EventHandler(this.tbMotorSpeed_Scroll);
      // 
      // chkRearLight
      // 
      this.chkRearLight.AutoSize = true;
      this.chkRearLight.Location = new System.Drawing.Point(16, 89);
      this.chkRearLight.Margin = new System.Windows.Forms.Padding(4);
      this.chkRearLight.Name = "chkRearLight";
      this.chkRearLight.Size = new System.Drawing.Size(90, 20);
      this.chkRearLight.TabIndex = 5;
      this.chkRearLight.Text = "Rear Light";
      this.chkRearLight.UseVisualStyleBackColor = true;
      this.chkRearLight.CheckedChanged += new System.EventHandler(this.chkRearLight_CheckedChanged);
      // 
      // btnConnect
      // 
      this.btnConnect.Location = new System.Drawing.Point(620, 15);
      this.btnConnect.Margin = new System.Windows.Forms.Padding(4);
      this.btnConnect.Name = "btnConnect";
      this.btnConnect.Size = new System.Drawing.Size(100, 28);
      this.btnConnect.TabIndex = 3;
      this.btnConnect.Text = "Connect";
      this.btnConnect.UseVisualStyleBackColor = true;
      this.btnConnect.Click += new System.EventHandler(this.btnConnect_Click);
      // 
      // dataGridView1
      // 
      this.dataGridView1.AllowUserToAddRows = false;
      this.dataGridView1.AllowUserToDeleteRows = false;
      this.dataGridView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.dataGridView1.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.AllCells;
      this.dataGridView1.ClipboardCopyMode = System.Windows.Forms.DataGridViewClipboardCopyMode.EnableAlwaysIncludeHeaderText;
      this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
      this.dataGridView1.Location = new System.Drawing.Point(16, 235);
      this.dataGridView1.Margin = new System.Windows.Forms.Padding(4);
      this.dataGridView1.Name = "dataGridView1";
      this.dataGridView1.RowHeadersVisible = false;
      this.dataGridView1.RowHeadersWidth = 51;
      this.dataGridView1.Size = new System.Drawing.Size(704, 288);
      this.dataGridView1.TabIndex = 9;
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(20, 215);
      this.label2.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(120, 16);
      this.label2.TabIndex = 8;
      this.label2.Text = "Recent Messages:";
      // 
      // chkUnlockBattery
      // 
      this.chkUnlockBattery.AutoSize = true;
      this.chkUnlockBattery.Location = new System.Drawing.Point(16, 60);
      this.chkUnlockBattery.Margin = new System.Windows.Forms.Padding(4);
      this.chkUnlockBattery.Name = "chkUnlockBattery";
      this.chkUnlockBattery.Size = new System.Drawing.Size(116, 20);
      this.chkUnlockBattery.TabIndex = 4;
      this.chkUnlockBattery.Text = "Unlock Battery";
      this.chkUnlockBattery.UseVisualStyleBackColor = true;
      this.chkUnlockBattery.CheckedChanged += new System.EventHandler(this.chkRearLight_CheckedChanged);
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(20, 116);
      this.label3.Margin = new System.Windows.Forms.Padding(4, 0, 4, 0);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(91, 16);
      this.label3.TabIndex = 6;
      this.label3.Text = "Motor Torque:";
      // 
      // cbChannel
      // 
      this.cbChannel.DisplayMember = "Index";
      this.cbChannel.FormattingEnabled = true;
      this.cbChannel.Location = new System.Drawing.Point(372, 16);
      this.cbChannel.Margin = new System.Windows.Forms.Padding(4);
      this.cbChannel.Name = "cbChannel";
      this.cbChannel.Size = new System.Drawing.Size(239, 24);
      this.cbChannel.TabIndex = 2;
      // 
      // timer1
      // 
      this.timer1.Tick += new System.EventHandler(this.timer1_Tick);
      // 
      // Form1
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(736, 538);
      this.Controls.Add(this.label3);
      this.Controls.Add(this.chkUnlockBattery);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.dataGridView1);
      this.Controls.Add(this.btnConnect);
      this.Controls.Add(this.chkRearLight);
      this.Controls.Add(this.tbMotorSpeed);
      this.Controls.Add(this.label1);
      this.Controls.Add(this.cbChannel);
      this.Controls.Add(this.cbCANAdapter);
      this.Margin = new System.Windows.Forms.Padding(4);
      this.Name = "Form1";
      this.Text = "Bird 3";
      this.FormClosed += new System.Windows.Forms.FormClosedEventHandler(this.Form1_FormClosed);
      this.Load += new System.EventHandler(this.Form1_Load);
      ((System.ComponentModel.ISupportInitialize)(this.tbMotorSpeed)).EndInit();
      ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
      this.ResumeLayout(false);
      this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbCANAdapter;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TrackBar tbMotorSpeed;
        private System.Windows.Forms.CheckBox chkRearLight;
        private System.Windows.Forms.Button btnConnect;
        private System.Windows.Forms.DataGridView dataGridView1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.CheckBox chkUnlockBattery;
        private System.Windows.Forms.Label label3;
    private System.Windows.Forms.ComboBox cbChannel;
    private System.Windows.Forms.Timer timer1;
  }
}

