using Candle;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;

namespace BirdCAN
{
  public partial class Form1 : Form
  {
    Device selectedDevice;
    Channel ConnectedChannel;
    CancellationTokenSource cancel;
    Dictionary<uint, Tuple<DateTime, Frame>> RecentMessages = new Dictionary<uint, Tuple<DateTime, Frame>>();
    DataTable previewTable = new DataTable();
    Dictionary<uint, DataRow> rowFinder = new Dictionary<uint, DataRow>();
    DataColumn colTimeStamp;
    DataColumn colID;
    DataColumn colInterval;
    DataColumn colData;
    DataColumn colIntent;
    int CurrentSpeed = 0;
    bool UnlockBattery = false;
    bool RearLightOn = false;
    Dictionary<uint, Tuple<int, string>> names = new Dictionary<uint, Tuple<int, string>>();
    public Form1()
    {
      InitializeComponent();
      this.colTimeStamp = previewTable.Columns.Add("TimeStamp");
      this.colID = previewTable.Columns.Add("ID");
      this.colInterval = previewTable.Columns.Add("Interval");
      this.colData = previewTable.Columns.Add("Data");
      this.colIntent = previewTable.Columns.Add("Intent");
      colTimeStamp.DataType = colID.DataType = colData.DataType = colInterval.DataType = colInterval.DataType = typeof(string);

      this.dataGridView1.DataSource = previewTable;
      names[frmRearLightOff.Identifier] = new Tuple<int, string>(50, "Rear Light");
      names[frmBatteryLocked.Identifier] = new Tuple<int, string>(200, "Battery Output Enable");
      names[BatteryUnlockCodes[0].Identifier] = new Tuple<int, string>(1000, "Controller Uptime/Battery Keepalive");

      names[0x153] = new Tuple<int, string>(2, "Motor Control");
    }

    private void Form1_Load(object sender, EventArgs e)
    {
      var devices = Candle.Device.ListDevices().ToList<object>();
      devices.Insert(0, new { Path = "(None)" });
      cbCANAdapter.DataSource = devices;
      chkRearLight.Enabled = chkUnlockBattery.Enabled = tbMotorSpeed.Enabled = false;
    }

    private void btnConnect_Click(object sender, EventArgs e)
    {
      if (cbChannel.SelectedItem is Channel)
      {
        if (ConnectedChannel == null)
        {
          try
          {
            cbChannel.Enabled = cbCANAdapter.Enabled = false;
            chkRearLight.Enabled = chkUnlockBattery.Enabled = tbMotorSpeed.Enabled = true;
            tbMotorSpeed.Value = 0;
            batteryUnlockIndex = 0;
            ConnectedChannel = cbChannel.SelectedItem as Channel;
            ConnectedChannel.Start(500000);
            cancel = new CancellationTokenSource();
            ThreadPool.QueueUserWorkItem(TimerLoop);
            timer1.Enabled = true;
          }
          catch
          {
            cbChannel.Enabled = cbCANAdapter.Enabled = true;
            chkRearLight.Enabled = chkUnlockBattery.Enabled = tbMotorSpeed.Enabled = false;
          }
        }
        else
        {
          cancel.Cancel();
          Thread.Sleep(100); // give time for timer thread to finish
          // Already connected, and button is now `Disconnect` button
          ConnectedChannel.Stop();
          ConnectedChannel.Receive();
          ConnectedChannel = null;
          cbChannel.Enabled = cbCANAdapter.Enabled = true;
          chkRearLight.Enabled = chkUnlockBattery.Enabled = tbMotorSpeed.Enabled = false;
          timer1.Enabled = false;
        }
      }
      else
      {
        MessageBox.Show("Please select a device and channel");
      }
    }

    private void cbCANAdapter_SelectedIndexChanged(object sender, EventArgs e)
    {
      var device = cbCANAdapter.SelectedItem as Device;
      if (selectedDevice != null && selectedDevice != device)
        selectedDevice.Close();
      selectedDevice = device;
      if (device != null)
      {
        device.Open();
        cbChannel.DataSource = device.Channels.Select(x => x.Value).ToList();
      }
      else
      {
        cbChannel.DataSource = null;
      }
    }

    void TimerLoop(object state)
    {
      if (ConnectedChannel == null)
      {
        return;
      }
      var timerCounter = 0;
      var timerIntervalMs = 2;
      int timerCounterMax = 5000;
      PrecisionRepeatActionOnInterval(new TimeSpan(timerIntervalMs * TimeSpan.TicksPerMillisecond), cancel.Token, () =>
      {
        if (cancel.Token.IsCancellationRequested)
        {
          return;
        }


        // no longer in startup. Time to start sending motor speed
        var speed = (uint)CurrentSpeed;
        var bytes = BitConverter.GetBytes(speed);
        var x = bytes[1];
        if (speed == 0)
        {
          bytes[3] = 1;
        }
        //bytes[1] = bytes[0];
        //bytes[0] = x;
        var frame = new Frame { Identifier = 0x153, Data = bytes };
        RecentMessages[frame.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, frame);
        ConnectedChannel.Send(frame);
        if (timerCounter % 200 == 0)
        {
          if (UnlockBattery)
          {
            RecentMessages[frmBatteryUnlocked.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, frmBatteryUnlocked);
            ConnectedChannel.Send(frmBatteryUnlocked);
          }
          else
          {
            RecentMessages[frmBatteryLocked.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, frmBatteryLocked);
            ConnectedChannel.Send(frmBatteryLocked);
          }
        }


        if (timerCounter % 50 == 0)
        {
          if (RearLightOn)
          {
            RecentMessages[frmRearLightOn.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, frmRearLightOn);
            ConnectedChannel.Send(frmRearLightOn);
          }
          else
          {
            RecentMessages[frmRearLightOff.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, frmRearLightOff);
            ConnectedChannel.Send(frmRearLightOff);
          }
        }


        if ((timerCounter + 1000) % 1000 == 0)
        { // Every one second
          var batteryMessageFrame = BatteryUnlockCodes[batteryUnlockIndex];
          RecentMessages[batteryMessageFrame.Identifier] = new Tuple<DateTime, Frame>(DateTime.Now, batteryMessageFrame);
          ConnectedChannel.Send(batteryMessageFrame);

          batteryUnlockIndex++;
          if (batteryUnlockIndex >= BatteryUnlockCodes.Count)
            batteryUnlockIndex = firstBatteryUnlockLoopIndex;
        }
        timerCounter += timerIntervalMs;
        if (timerCounter == timerCounterMax)
          timerCounter = 0;

      });
    }
    Frame frmBatteryUnlocked = new Frame { Identifier = 0x140, Data = new byte[] { 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
    Frame frmBatteryLocked = new Frame { Identifier = 0x140, Data = new byte[] { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } };
    Frame frmRearLightOn = new Frame { Identifier = 0x500, Data = new byte[] { 0x01, 0x00, 0x00 } };
    Frame frmRearLightOff = new Frame { Identifier = 0x500, Data = new byte[] { 0x00, 0x00, 0x00 } };

    int batteryUnlockIndex = 0;
    int firstBatteryUnlockLoopIndex = 5;
    List<Frame> BatteryUnlockCodes = new List<Frame>(new[]
    {
      new Frame { Identifier = 0x742,  Data = new byte[]{0x01, 0x00, 0xdd, 0x00, 0xcc, 0x35, 0xe0, 0x26}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x02, 0x00, 0xdd, 0x43, 0xb3, 0x02, 0x40, 0x8b}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x03, 0x00, 0xdd, 0x43, 0x05, 0x46, 0x0f, 0x6c}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x04, 0x00, 0xdd, 0x43, 0x34, 0xe5, 0x5e, 0x8d}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x05, 0x00, 0xdd, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x06, 0x00, 0x83, 0x43, 0xb1, 0x49, 0xc5, 0xa3}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x07, 0x00, 0x5f, 0x43, 0x66, 0xc8, 0x35, 0xf4}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x08, 0x00, 0x59, 0x43, 0xe1, 0xfd, 0x31, 0xfa}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x09, 0x00, 0xc4, 0x43, 0x76, 0xc7, 0x18, 0xe0}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0a, 0x00, 0x64, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0b, 0x00, 0x13, 0x43, 0x01, 0x2a, 0x41, 0xa7}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0c, 0x00, 0x44, 0x43, 0x14, 0x5b, 0x5e, 0x3a}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0d, 0x00, 0x07, 0x43, 0xba, 0x86, 0xf7, 0x96}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0e, 0x00, 0xee, 0x43, 0x11, 0x3c, 0x37, 0x6a}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x0f, 0x00, 0xd0, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x10, 0x00, 0xc5, 0x43, 0xd6, 0x30, 0x20, 0x92}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x11, 0x00, 0xb6, 0x43, 0x4c, 0xa9, 0xf2, 0x89}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x12, 0x00, 0x70, 0x43, 0xf2, 0x1d, 0x15, 0x2a}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x13, 0x00, 0x23, 0x43, 0x05, 0x87, 0xa9, 0x46}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x14, 0x00, 0x71, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x15, 0x00, 0x9c, 0x43, 0x79, 0x0a, 0x38, 0x91}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x16, 0x00, 0xb6, 0x43, 0xa4, 0xb7, 0xf0, 0x76}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x17, 0x00, 0x80, 0x43, 0xe4, 0xaf, 0x27, 0xfb}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x18, 0x00, 0x75, 0x43, 0x5c, 0x80, 0x47, 0xa3}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x19, 0x00, 0x0f, 0x43, 0x03, 0x57, 0x43, 0x41}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1a, 0x00, 0x2f, 0x43, 0x8a, 0xa3, 0x4c, 0x6c}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1b, 0x00, 0x21, 0x43, 0x36, 0xe7, 0x21, 0xc3}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1c, 0x00, 0x0f, 0x43, 0x79, 0xc0, 0xab, 0xba}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1d, 0x00, 0x29, 0x43, 0xb6, 0x44, 0x3b, 0xce}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1e, 0x00, 0x6a, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x1f, 0x00, 0xfc, 0x43, 0x66, 0x48, 0x45, 0xc7}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x20, 0x00, 0xb5, 0x43, 0xc9, 0xfa, 0xdd, 0x9f}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x21, 0x00, 0xbf, 0x43, 0x68, 0x9b, 0x32, 0x4c}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x22, 0x00, 0x9b, 0x43, 0xeb, 0x43, 0xdf, 0x90}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x23, 0x00, 0xf7, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x24, 0x00, 0x4c, 0x43, 0x96, 0x3c, 0x96, 0x04}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x25, 0x00, 0xe2, 0x43, 0x48, 0x2f, 0x34, 0xb5}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x26, 0x00, 0xdc, 0x43, 0x27, 0xfc, 0x06, 0xb0}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x27, 0x00, 0xf5, 0x43, 0xd7, 0x21, 0x01, 0x97}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x28, 0x00, 0xd7, 0x43, 0x28, 0x00, 0x2a, 0x00}},
      new Frame { Identifier = 0x742,  Data = new byte[]{0x29, 0x00, 0x39, 0x43, 0xcd, 0x3c, 0x36, 0x2c } }
    });

    public static void PrecisionRepeatActionOnInterval(TimeSpan interval, CancellationToken? ct, Action action)
    {
      long stage1Delayms = 20;
      long stage2Delayms = 5 * TimeSpan.TicksPerMillisecond;
      bool USE_SLEEP0 = true;

      DateTime target = DateTime.Now + new TimeSpan(0, 0, 0, 0, (int)stage1Delayms + 2);
      bool warmup = true;
      while (ct == null || !ct.Value.IsCancellationRequested)
      {
        // Getting closer to 'target' - Lets do the less precise but least cpu intesive wait
        var timeLeft = target - DateTime.Now;
        if (timeLeft.TotalMilliseconds >= stage1Delayms)
        {
          try
          {
            Thread.Sleep((int)(timeLeft.TotalMilliseconds - stage1Delayms));
          }
          catch (TaskCanceledException) when (ct != null)
          {
            return;
          }
        }

        // Getting closer to 'target' - Lets do the semi-precise but mild cpu intensive wait - Thread.Sleep(0)
        // Note: Thread.Sleep(0) is removed below because it is sometimes looked down on and also said not good to mix 'Thread.Sleep(0)' with Tasks.
        //       However, Thread.Sleep(0) does have a quicker and more reliable turn around time then Task.Yield() so to 
        //       make up for this a longer (and more expensive) Thread.SpinWait(1) would be needed.
        if (USE_SLEEP0)
        {
          while (DateTime.Now < target - new TimeSpan(stage2Delayms / 8))
          {
            Thread.Sleep(0);
          }
        }

        // Extreamlly close to 'target' - Lets do the most precise but very cpu/battery intesive 
        while (DateTime.Now < target)
        {
          Thread.SpinWait(64);
        }

        if (!warmup)
        {
          action();
          target += interval;
        }
        else
        {
          long start1 = DateTime.Now.Ticks + ((long)interval.TotalMilliseconds * TimeSpan.TicksPerMillisecond);
          long alignVal = start1 - (start1 % ((long)interval.TotalMilliseconds * TimeSpan.TicksPerMillisecond));
          target = new DateTime(alignVal);
          warmup = false;
        }
      }
    }

    private void timer1_Tick(object sender, EventArgs e)
    {
      var messages = RecentMessages.Values.ToList();
      foreach (var message in messages)
      {
        DataRow dataRow;
        if (rowFinder.ContainsKey(message.Item2.Identifier))
        {
          dataRow = rowFinder[message.Item2.Identifier];
        }
        else
        {
          dataRow = this.previewTable.NewRow();
          dataRow[colID] = "0x" + Convert.ToString(message.Item2.Identifier, 16);
          if (names.ContainsKey(message.Item2.Identifier))
          {
            dataRow[colIntent] = names[message.Item2.Identifier].Item2;
            dataRow[colInterval] = names[message.Item2.Identifier].Item1.ToString() + " ms";
          }
          rowFinder[message.Item2.Identifier] = dataRow;
          this.previewTable.Rows.Add(dataRow);
        }
        dataRow[colTimeStamp] = message.Item1.ToString("hh:mm:ss.fff");
        dataRow[colData] = string.Join(", ", message.Item2.Data.Select(x => "0x" + Convert.ToString(x, 16).PadLeft(2, '0')));
      }
    }

    private void tbMotorSpeed_Scroll(object sender, EventArgs e)
    {
      CurrentSpeed = tbMotorSpeed.Value;
    }

    private void Form1_FormClosed(object sender, FormClosedEventArgs e)
    {
      btnConnect_Click(null, null);
      selectedDevice?.Close();
    }

    private void chkRearLight_CheckedChanged(object sender, EventArgs e)
    {
      UnlockBattery = chkUnlockBattery.Checked;
      RearLightOn = chkRearLight.Checked;
    }
  }
}
