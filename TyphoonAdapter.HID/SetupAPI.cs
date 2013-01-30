using System;
using System.Runtime.InteropServices;

namespace TyphoonAdapter.HID
{
    class SetupAPI
    {
        // from setupapi.h
        public const Int32 DIGCF_PRESENT = 2;
        public const Int32 DIGCF_DEVICEINTERFACE = 0X10;

        public struct SP_DEVICE_INTERFACE_DATA
        {
            public Int32 cbSize;
            public Guid InterfaceClassGuid;
            public Int32 Flags;
            public IntPtr Reserved;
        }

        //public struct SP_DEVICE_INTERFACE_DETAIL_DATA
        //{
        //    public Int32 cbSize;
        //    public String DevicePath;
        //}

        //public struct SP_DEVINFO_DATA
        //{
        //    public Int32 cbSize;
        //    public Guid ClassGuid;
        //    public Int32 DevInst;
        //    public Int32 Reserved;
        //}

        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern Int32 SetupDiCreateDeviceInfoList(ref Guid ClassGuid, Int32 hwndParent);

        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern Int32 SetupDiDestroyDeviceInfoList(IntPtr DeviceInfoSet);

        [DllImport("setupapi.dll", SetLastError = true)]
        public static extern Boolean SetupDiEnumDeviceInterfaces(IntPtr DeviceInfoSet, IntPtr DeviceInfoData, ref Guid InterfaceClassGuid, Int32 MemberIndex, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData);

        [DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern IntPtr SetupDiGetClassDevs(ref Guid ClassGuid, IntPtr Enumerator, IntPtr hwndParent, Int32 Flags);

        [DllImport("setupapi.dll", SetLastError = true, CharSet = CharSet.Auto)]
        public static extern Boolean SetupDiGetDeviceInterfaceDetail(IntPtr DeviceInfoSet, ref SP_DEVICE_INTERFACE_DATA DeviceInterfaceData, IntPtr DeviceInterfaceDetailData, Int32 DeviceInterfaceDetailDataSize, ref Int32 RequiredSize, IntPtr DeviceInfoData);
    }
}
