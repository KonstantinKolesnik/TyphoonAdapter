using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using TyphoonAdapter.DCC;
using TyphoonAdapter.HID;

namespace TyphoonAdapter.USBPipeline
{
    public class AdapterUSB
    {
        class MsgDispatcher : Form
        {
            private GenericHIDDevice device = null;

            internal MsgDispatcher(GenericHIDDevice device)
            {
                this.device = device;
            }

            protected override void OnHandleCreated(EventArgs e)
            {
                base.OnHandleCreated(e);
                device.RegisterForDeviceNotifications(Handle);
            }
            protected override void OnClosing(System.ComponentModel.CancelEventArgs e)
            {
                device.UnregisterFromDeviceNotification(Handle);
                base.OnClosing(e);
            }
            protected override void WndProc(ref Message m)
            {
                device.HandleDeviceNotificationMessages(m);
                base.WndProc(ref m);
            }
        }

        #region Fields
        private MsgDispatcher frm = null;
        private GenericHIDDevice device = null;
        #endregion

        #region Properties
        public bool IsConnected
        {
            get { return device.Attached; }
        }
        #endregion

        #region Events
        public event EventHandler Connected;
        public event EventHandler Disconnected;
        #endregion

        #region Constructor
        public AdapterUSB()
        {
            device = new GenericHIDDevice(5824, 1503);
            device.Connected += OnConnected;
            device.Disconnected += OnDisconnected;

            frm = new MsgDispatcher(device);
            frm.WindowState = FormWindowState.Minimized;
            frm.Show();
            frm.Hide();
        }
        #endregion

        #region Event handlers
        private void OnConnected(object sender, EventArgs args)
        {
            if (Connected != null)
                Connected(this, args);
        }
        private void OnDisconnected(object sender, EventArgs args)
        {
            if (Disconnected != null)
                Disconnected(this, args);
        }
        #endregion

        #region Public methods
        public void Connect()
        {
            device.FindTargetDevice();
        }
        public void Disconnect()
        {
            device.DetachTargetDevice();
        }
        public AdapterStatus GetStatus()
        {
            byte[] data = device.ReadInputReport();
            if (data != null && data[0] == 'S')
            {
                return new AdapterStatus()
                {
                    MainTrackActive = data[1] != 0,
                    ProgramTrackActive = data[2] != 0,
                    MainTrackShortCircuitBlocked = data[3] != 0,
                    ProgramTrackShortCircuitBlocked = data[4] != 0,
                    RailcomActive = data[5] != 0,
                    AckOn = data[6] != 0
                };
            }
            return null;
        }
        public void Send(object data)
        {
            if (!IsConnected || data == null)
                return;

            byte[] bb = null;

            if (data is string)
            {
                string s = data as string;
                if (!String.IsNullOrEmpty(s))
                    bb = Encoding.ASCII.GetBytes(s);
            }
            else if (data is DCCCommand)
            {
                DCCCommand cmd = data as DCCCommand;
                if (cmd != null && cmd.Data.Count != 0 && cmd.Repeats != 0)
                {
                    List<byte> list = new List<byte>();
                    list.Add((byte)'D');                // dcc command type
                    list.Add(cmd.Type == DCCCommandType.Service ? (byte)'P' : (byte)'O'); // operation or service?
                    list.Add((byte)cmd.Repeats);        // repeats count
                    list.Add((byte)cmd.Data.Count);     // dcc command bytes count
                    list.AddRange(cmd.Data);            // dcc command bytes

                    bb = list.ToArray();
                }
            }

            if (bb != null && bb.Length != 0)
                device.WriteOutputReport(bb);
        }
        #endregion
    }
}
