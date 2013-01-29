using System.Collections.Generic;

namespace TyphoonAdapter.DCC
{
    public class LocomotiveAddress
    {
        #region Fields
        private uint address = 3;
        private bool isLong = false;
        #endregion

        #region Properties
        public uint Address
        {
            get { return address; }
            set { address = value; }
        }
        public bool Long
        {
            get { return isLong; }
            set { isLong = value; }
        }
        #endregion

        #region Constructors
        public LocomotiveAddress()
        {
        }
        public LocomotiveAddress(uint address, bool isLong)
        {
            Address = address;
            Long = isLong;
        }
        #endregion

        #region Public methods
        public List<byte> GetBytes()
        {
            List<byte> bytes = new List<byte>();
            if (!isLong) // short address
            {
                if (address >= 1 && address <= DCC.LocoShortAddressMax)
                    bytes.Add((byte)(address & 0x7F)); // 7F = 01111111
            }
            else // long address
            {
                if (address >= 0 && address <= 10239)
                {
                    bytes.Add((byte)(192 + ((address / 256) & 0x3F))); // 192 = 11000000, 3F = 00111111
                    bytes.Add((byte)(address & 0xFF));
                }
            }
            return bytes;
        }
        #endregion
    }
}
