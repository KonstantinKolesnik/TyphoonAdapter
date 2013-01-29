using System.Collections.Generic;

namespace TyphoonAdapter.DCC
{
    public class AccessoryAddress
    {
        #region Fields
        private ushort decoderAddress = 1;
        private byte decoderOutput = 0;
        #endregion

        #region Properties
        public ushort DecoderAddress
        {
            get { return decoderAddress; }
            set { decoderAddress = value; }
        }
        public byte DecoderOutput
        {
            get { return decoderOutput; }
            set { decoderOutput = value; }
        }
        public ushort AccessoryNumber
        {
            get { return (ushort)((decoderAddress - 1) * 4 + (decoderOutput + 1)); }
            set
            {
                ushort n = (ushort)(value - 1);
                DecoderAddress = (ushort)(n / 4 + 1);
                DecoderOutput = (byte)(n % 4);
            }
        }
        #endregion

        #region Constructors
        // decoderAddress: [1...510]; 511 is acc broadcast address; 511 total addresses, 510 for decoders
        // decoderOutput: [0...3]
        public AccessoryAddress(ushort decoderAddress, byte decoderOutput)
        {
            DecoderAddress = decoderAddress;
            DecoderOutput = decoderOutput;
        }

        // accessoryNumber: [1...2040]
        public AccessoryAddress(ushort accessoryNumber)
        {
            ushort n = (ushort)(accessoryNumber - 1);
            DecoderAddress = (ushort)(n / 4 + 1);
            DecoderOutput = (byte)(n % 4);
        }
        #endregion

        #region Public methods
        public List<byte> GetBytes()
        {
            byte addressLSBMasked = (byte)(0x80 | (decoderAddress & 0x3F)); // 0...5 LSB bits; = CV1
            byte addressMSBMasked = (byte)(0x80 | (((decoderAddress / 0x40) ^ 0x07) * 0x10)); // 6...8 MSB bits; inverted; = CV9
            byte outputMasked = (byte)((decoderOutput << 1) & 0x06);

            List<byte> bytes = new List<byte>();
            bytes.Add(addressLSBMasked);
            bytes.Add((byte)(addressMSBMasked | outputMasked));

            return bytes;
        }
        #endregion
    }
}
