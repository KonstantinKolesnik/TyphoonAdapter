using System;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;

namespace TyphoonAdapter.HID
{
    class Kernel32
    {
        public const Int32 FILE_FLAG_OVERLAPPED = 0x40000000;
        public const Int32 FILE_SHARE_READ = 1;
        public const Int32 FILE_SHARE_WRITE = 2;
        public const UInt32 GENERIC_READ = 0x80000000;
        public const UInt32 GENERIC_WRITE = 0x40000000;
        public const Int32 INVALID_HANDLE_VALUE = -1;
        public const Int32 OPEN_EXISTING = 3;
        public const Int32 WAIT_TIMEOUT = 0x102;
        public const Int32 WAIT_OBJECT_0 = 0;

        [StructLayout(LayoutKind.Sequential)]
        public class SECURITY_ATTRIBUTES
        {
            internal Int32 nLength;
            internal Int32 lpSecurityDescriptor;
            internal Int32 bInheritHandle;
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern Int32 CancelIo(SafeFileHandle hFile);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern IntPtr CreateEvent(
            IntPtr SecurityAttributes,
            Boolean bManualReset,
            Boolean bInitialState,
            String lpName
            );

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern SafeFileHandle CreateFile(
            String lpFileName,
            UInt32 dwDesiredAccess,
            Int32 dwShareMode,
            IntPtr lpSecurityAttributes,
            Int32 dwCreationDisposition,
            Int32 dwFlagsAndAttributes,
            Int32 hTemplateFile
            );

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        public static extern Boolean GetOverlappedResult(
            SafeFileHandle hFile,
            IntPtr lpOverlapped,
            ref Int32 lpNumberOfBytesTransferred,
            Boolean bWait
            );

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern Boolean ReadFile(
            SafeFileHandle hFile,
            IntPtr lpBuffer,
            Int32 nNumberOfBytesToRead,
            ref Int32 lpNumberOfBytesRead,
            IntPtr lpOverlapped
            );

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern Int32 WaitForSingleObject(IntPtr hHandle, Int32 dwMilliseconds);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern Boolean WriteFile(
            SafeFileHandle hFile,
            Byte[] lpBuffer,
            Int32 nNumberOfBytesToWrite,
            ref Int32 lpNumberOfBytesWritten,
            IntPtr lpOverlapped
            );
    }
}
