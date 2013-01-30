using System;
using System.Runtime.InteropServices;

namespace TyphoonAdapter.HID
{
    class User32
    {
        // from dbt.h
        public const Int32 DBT_DEVICEARRIVAL = 0x8000;
        public const Int32 DBT_DEVICEREMOVECOMPLETE = 0x8004;
        public const Int32 DBT_DEVTYP_DEVICEINTERFACE = 5;
        public const Int32 DBT_DEVTYP_HANDLE = 6;
        public const Int32 DEVICE_NOTIFY_ALL_INTERFACE_CLASSES = 4;
        public const Int32 DEVICE_NOTIFY_SERVICE_HANDLE = 1;
        public const Int32 DEVICE_NOTIFY_WINDOW_HANDLE = 0;
        public const Int32 WM_DEVICECHANGE = 0x219;

        [StructLayout(LayoutKind.Sequential)]
        public class DEV_BROADCAST_DEVICEINTERFACE
        {
            internal Int32 dbcc_size;
            internal Int32 dbcc_devicetype;
            internal Int32 dbcc_reserved;
            internal Guid dbcc_classguid;
            internal Int16 dbcc_name;
        }

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Auto)]
        public class DEV_BROADCAST_DEVICEINTERFACE_1
        {
            internal Int32 dbcc_size;
            internal Int32 dbcc_devicetype;
            internal Int32 dbcc_reserved;
            [MarshalAs(UnmanagedType.ByValArray, ArraySubType = UnmanagedType.U1, SizeConst = 16)]
            internal Byte[] dbcc_classguid;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 255)]
            internal Char[] dbcc_name;
        }

        [StructLayout(LayoutKind.Sequential)]
        public class DEV_BROADCAST_HDR
        {
            internal Int32 dbch_size;
            internal Int32 dbch_devicetype;
            internal Int32 dbch_reserved;
        }

        [DllImport("user32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr RegisterDeviceNotification(IntPtr hRecipient, IntPtr NotificationFilter, Int32 Flags);

        [DllImport("user32.dll", SetLastError = true)]
        public static extern Boolean UnregisterDeviceNotification(IntPtr Handle);
    }
}
