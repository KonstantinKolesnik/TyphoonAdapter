using System;

namespace TyphoonAdapter.HID
{
    public class DataEventArgs : EventArgs
    {
        public readonly byte[] Data;

        public DataEventArgs(byte[] data)
        {
            Data = data;
        }
    }
}
