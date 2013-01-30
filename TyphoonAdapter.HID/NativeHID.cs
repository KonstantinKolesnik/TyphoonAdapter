using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace TyphoonAdapter.HID
{
    class NativeHID
    {
        // API declarations for hid.dll, taken from hidpi.h (part of the Windows Driver Kit (WDK))

        public const Int16 HidP_Input = 0;
        public const Int16 HidP_Output = 1;
        public const Int16 HidP_Feature = 2;

        [StructLayout(LayoutKind.Sequential)]
        public struct HIDD_ATTRIBUTES
        {
            public Int32 size;
            public UInt16 vendorId;
            public UInt16 productId;
            public UInt16 versionNumber;
        }

        public struct HIDP_CAPS
        {
            public Int16 usage;
            public Int16 usagePage;
            public Int16 inputReportByteLength;
            public Int16 outputReportByteLength;
            public Int16 featureReportByteLength;
            [MarshalAs(UnmanagedType.ByValArray, SizeConst = 17)]
            public Int16[] reserved;
            public Int16 numberLinkCollectionNodes;
            public Int16 numberInputButtonCaps;
            public Int16 numberInputValueCaps;
            public Int16 numberInputDataIndices;
            public Int16 numberOutputButtonCaps;
            public Int16 numberOutputValueCaps;
            public Int16 numberOutputDataIndices;
            public Int16 numberFeatureButtonCaps;
            public Int16 numberFeatureValueCaps;
            public Int16 numberFeatureDataIndices;
        }

        //public struct HidP_Value_Caps
        //    {
        //    internal Int16 usagePage;
        //    internal Byte reportID;
        //    internal Int32 isAlias;
        //    internal Int16 bitField;
        //    internal Int16 linkCollection;
        //    internal Int16 linkUsage;
        //    internal Int16 linkUsagePage;
        //    internal Int32 isRange;
        //    internal Int32 isStringRange;
        //    internal Int32 isDesignatorRange;
        //    internal Int32 isAbsolute;
        //    internal Int32 hasNull;
        //    internal Byte reserved;
        //    internal Int16 bitSize;
        //    internal Int16 reportCount;
        //    internal Int16 reserved2;
        //    internal Int16 reserved3;
        //    internal Int16 reserved4;
        //    internal Int16 reserved5;
        //    internal Int16 reserved6;
        //    internal Int32 logicalMin;
        //    internal Int32 logicalMax;
        //    internal Int32 physicalMin;
        //    internal Int32 physicalMax;
        //    internal Int16 usageMin;
        //    internal Int16 usageMax;
        //    internal Int16 stringMin;
        //    internal Int16 stringMax;
        //    internal Int16 designatorMin;
        //    internal Int16 designatorMax;
        //    internal Int16 dataIndexMin;
        //    internal Int16 dataIndexMax;
        //    }

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_FlushQueue(SafeFileHandle HidDeviceObject);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_FreePreparsedData(IntPtr PreparsedData);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_GetAttributes(SafeFileHandle HidDeviceObject, ref HIDD_ATTRIBUTES Attributes);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_GetFeature(SafeFileHandle HidDeviceObject, Byte[] lpReportBuffer, Int32 ReportBufferLength);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_GetInputReport(SafeFileHandle HidDeviceObject, Byte[] lpReportBuffer, Int32 ReportBufferLength);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern void HidD_GetHidGuid(ref Guid HidGuid);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_GetNumInputBuffers(SafeFileHandle HidDeviceObject, ref Int32 NumberBuffers);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_GetPreparsedData(SafeFileHandle HidDeviceObject, ref IntPtr PreparsedData);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_SetFeature(SafeFileHandle HidDeviceObject, Byte[] lpReportBuffer, Int32 ReportBufferLength);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_SetNumInputBuffers(SafeFileHandle HidDeviceObject, Int32 NumberBuffers);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Boolean HidD_SetOutputReport(SafeFileHandle HidDeviceObject, Byte[] lpReportBuffer, Int32 ReportBufferLength);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Int32 HidP_GetCaps(IntPtr PreparsedData, ref HIDP_CAPS Capabilities);

        [DllImport("hid.dll", SetLastError = true)]
        public static extern Int32 HidP_GetValueCaps(Int32 ReportType, Byte[] ValueCaps, ref Int32 ValueCapsLength, IntPtr PreparsedData);
    }
}
