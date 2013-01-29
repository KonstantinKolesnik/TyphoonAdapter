
namespace TyphoonAdapter.DCC
{
    public class DCC
    {
        // system addresses
        public const byte IdleAddress = 0xFF; // 255

        public const byte LocoBroadcastAddress = 0x00;
        public const byte LocoShortAddressMax = 0x7F;

        // operation mode loco instructions masks:
        public const byte DecoderControl = 0x00;                // 000 00000
        public const byte ConsistControl = 0x10;                // 000 10000
        public const byte AdvancedOperation = 0x20;             // 001 00000
        public const byte SpeedReverse = 0x40;                  // 010 00000
        public const byte SpeedForward = 0x60;                  // 011 00000
        public const byte FunctionGroup1 = 0x80;                // 100 00000
        public const byte FunctionGroup2 = 0xA0;                // 101 00000
        public const byte Reserved = 0xC0;                      // 110 00000
        public const byte CVAccess = 0xE0;                      // 111 00000

        // service mode instructions mask:
        public const byte ServiceMode = 0x70;                   // 01110000

        public const ushort BasicAccessoryBroadcastAddress = 0x1FF;   // 10[111111] 1[111]xxxx; =511 decimal
    }
}
