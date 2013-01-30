using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using System.Windows.Forms;
using Microsoft.Win32.SafeHandles;

namespace TyphoonAdapter.HID
{
    public class GenericHIDDevice
    {
        private struct DeviceInformation
        {
            public UInt16 Vid;                // Our target device's VID
            public UInt16 Pid;                // Our target device's PID
            public SafeFileHandle HIDHandle;        // Handle used for communicating via hid.dll
            public String PathName;           // The device's path name
            public bool Attached;             // Device attachment state flag
            public NativeHID.HIDD_ATTRIBUTES Attributes;      // HID Attributes
            public NativeHID.HIDP_CAPS Capabilities;          // HID Capabilities
            //public SafeFileHandle ReadHandle;       // Read handle from the device
            //public SafeFileHandle WriteHandle;      // Write handle to the device
            public IntPtr NotificationHandle; // The device's notification handle
        }

        #region Fields
        /// <summary>
        /// contains the discovered attributes of the target USB device
        /// </summary>
        private DeviceInformation deviceInfo;
        private FileStream file = null;
        private BackgroundWorker reader = new BackgroundWorker();
        #endregion

        #region Properties
        public bool Attached
        {
            get { return deviceInfo.Attached; }
            private set { deviceInfo.Attached = value; }
        }
        #endregion

        #region Events
        public event EventHandler Connected;
        public event EventHandler Disconnected;
        public event DataEventHandler DataRecieved;
        #endregion

        #region Constructor/Destructor
        /// <summary>
        /// Constructor
        /// </summary>
        /// <remarks>This constructor method creates an object for HID communication and attempts to find
        /// (and bind to) the USB device indicated by the passed VID (Vendor ID) and PID (Product ID) which
        /// should both be passed as unsigned integers.</remarks>
        public GenericHIDDevice(int vid, int pid)
        {
            deviceInfo.Attached = false;
            deviceInfo.Vid = (UInt16)vid;
            deviceInfo.Pid = (UInt16)pid;

            reader.DoWork += new DoWorkEventHandler(ReadProc);
            //reader.RunWorkerCompleted += new RunWorkerCompletedEventHandler(backgroundWorker1_RunWorkerCompleted);
            //reader.ProgressChanged += new ProgressChangedEventHandler(backgroundWorker1_ProgressChanged);
            //reader.WorkerSupportsCancellation = true;
        }

        /// <summary>
        /// Destructor
        /// </summary>
        /// <remarks>This method closes any open connections to the USB device and clears up any resources
        /// that have been consumed over the lifetime of the object.</remarks>
        ~GenericHIDDevice()
        {
            DetachTargetDevice();
        }

        /// <summary>
        /// Detach the USB device
        /// </summary>
        /// <remarks>This method detaches the USB device and frees the read and write handles
        /// to the device.</remarks>
        public void DetachTargetDevice()
        {
            if (Attached)
            {
                Attached = false;
                // Close the readHandle, writeHandle and hidHandle
                if (!deviceInfo.HIDHandle.IsInvalid) deviceInfo.HIDHandle.Close();
                //if (!deviceInfo.ReadHandle.IsInvalid) deviceInfo.ReadHandle.Close();
                //if (!deviceInfo.WriteHandle.IsInvalid) deviceInfo.WriteHandle.Close();
            }
        }
        #endregion

        #region Device discovery
        public void FindTargetDevice()
        {
            String[] listOfDevicePathNames = new String[128]; // 128 is the maximum number of USB devices allowed on a single host
            int deviceIdx = 0;
            bool attributesOK = false;
            bool isDeviceDetected = false;
            int numberOfDevicesFound = 0;

            try
            {
                // Get all the devices with the correct HID GUID
                if (QueryHIDDevices(ref listOfDevicePathNames, ref numberOfDevicesFound))
                {
                    deviceIdx = 0;
                    do
                    {
                        //deviceInfo.HIDHandle = Kernel32.CreateFile(
                        //    listOfDevicePathNames[deviceIdx],
                        //    Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,//0,
                        //    Kernel32.FILE_SHARE_READ | Kernel32.FILE_SHARE_WRITE,
                        //    IntPtr.Zero,
                        //    Kernel32.OPEN_EXISTING,
                        //    Kernel32.FILE_FLAG_OVERLAPPED,//0,
                        //    0);

                        deviceInfo.HIDHandle = Kernel32.CreateFile(
                            listOfDevicePathNames[deviceIdx],
                            0,//Kernel32.GENERIC_READ | Kernel32.GENERIC_WRITE,
                            Kernel32.FILE_SHARE_READ | Kernel32.FILE_SHARE_WRITE,
                            IntPtr.Zero,
                            Kernel32.OPEN_EXISTING,
                            0,
                            0);



                        if (!deviceInfo.HIDHandle.IsInvalid)
                        {
                            deviceInfo.Attributes.size = Marshal.SizeOf(deviceInfo.Attributes);
                            attributesOK = NativeHID.HidD_GetAttributes(deviceInfo.HIDHandle, ref deviceInfo.Attributes);
                            if (attributesOK)
                            {
                                if (deviceInfo.Attributes.vendorId == deviceInfo.Vid && deviceInfo.Attributes.productId == deviceInfo.Pid)
                                {
                                    isDeviceDetected = true;
                                    deviceInfo.PathName = listOfDevicePathNames[deviceIdx];
                                }
                                else
                                {
                                    isDeviceDetected = false;
                                    deviceInfo.HIDHandle.Close();
                                }
                            }
                            else
                            {
                                isDeviceDetected = false;
                                deviceInfo.HIDHandle.Close();
                            }
                        }
                        deviceIdx++;
                    } while (!(isDeviceDetected || (deviceIdx == numberOfDevicesFound + 1)));
                }

                // If we found a matching device then we need discover more details about the attached device
                // and then open read and write handles to the device to allow communication
                if (isDeviceDetected)
                {
                    // Query the HID device's capabilities (primarily we are only really interested in the 
                    // input and output report byte lengths as this allows us to validate information sent
                    // to and from the device does not exceed the devices capabilities.
                    //
                    // We could determine the 'type' of HID device here too, but since this class is only
                    // for generic HID communication we don't care...
                    QueryHIDDeviceCapabilities();
                    if (attributesOK)
                    {
                        Attached = true;
                        if (Connected != null)
                            Connected(this, EventArgs.Empty);

                        //file = new FileStream(deviceInfo.HIDHandle, FileAccess.ReadWrite, deviceInfo.Capabilities.inputReportByteLength, true);
                        //BeginAsyncRead();
                        //reader.RunWorkerAsync();

                        /*
                        deviceInfo.ReadHandle = Kernel32.CreateFile(
                            deviceInfo.PathName,
                            Kernel32.GENERIC_READ,
                            Kernel32.FILE_SHARE_READ | Kernel32.FILE_SHARE_WRITE,
                            IntPtr.Zero,
                            Kernel32.OPEN_EXISTING,
                            Kernel32.FILE_FLAG_OVERLAPPED,
                            0);

                        deviceInfo.WriteHandle = Kernel32.CreateFile(
                            deviceInfo.PathName,
                            Kernel32.GENERIC_WRITE,
                            Kernel32.FILE_SHARE_READ | Kernel32.FILE_SHARE_WRITE,
                            IntPtr.Zero,
                            Kernel32.OPEN_EXISTING,
                            0,
                            0);

                        if (deviceInfo.ReadHandle.IsInvalid)
                            deviceInfo.ReadHandle.Close();
                        else if (deviceInfo.WriteHandle.IsInvalid)
                            deviceInfo.WriteHandle.Close();
                        else
                        {
                            Attached = true;
                            if (Connected != null)
                                Connected(this, EventArgs.Empty);
                        }
                        */
                    }
                }
            }
            catch (Exception)
            {
                isDeviceDetected = false;
            }
        }

        private bool QueryHIDDevices(ref String[] listOfDevicePathNames, ref int numberOfDevicesFound)
        {
            if (Attached)
                DetachTargetDevice();

            // Initialise the internal variables required for performing the search
            Int32 bufferSize = 0;
            IntPtr detailDataBuffer = IntPtr.Zero;
            Boolean deviceFound;
            IntPtr deviceInfoSet = new System.IntPtr();
            Boolean lastDevice = false;
            Int32 listIndex = 0;
            SetupAPI.SP_DEVICE_INTERFACE_DATA deviceInterfaceData = new SetupAPI.SP_DEVICE_INTERFACE_DATA();
            Boolean success;

            // Get the required GUID
            System.Guid systemHidGuid = new Guid();
            NativeHID.HidD_GetHidGuid(ref systemHidGuid);
            try
            {
                // Here we populate a list of plugged-in devices matching our class GUID (DIGCF_PRESENT specifies that the list should only contain devices which are plugged in)
                deviceInfoSet = SetupAPI.SetupDiGetClassDevs(ref systemHidGuid, IntPtr.Zero, IntPtr.Zero, SetupAPI.DIGCF_PRESENT | SetupAPI.DIGCF_DEVICEINTERFACE);
                deviceFound = false;
                listIndex = 0;

                deviceInterfaceData.cbSize = Marshal.SizeOf(deviceInterfaceData);

                // Look through the retrieved list of class GUIDs looking for a match on our interface GUID
                do
                {
                    success = SetupAPI.SetupDiEnumDeviceInterfaces(deviceInfoSet, IntPtr.Zero, ref systemHidGuid, listIndex, ref deviceInterfaceData);
                    if (!success)
                        lastDevice = true;
                    else
                    {
                        // The target device has been found, now we need to retrieve the device path so we can open
                        // the read and write handles required for USB communication

                        // First call is just to get the required buffer size for the real request
                        success = SetupAPI.SetupDiGetDeviceInterfaceDetail(deviceInfoSet, ref deviceInterfaceData, IntPtr.Zero, 0, ref bufferSize, IntPtr.Zero);

                        // Allocate some memory for the buffer
                        detailDataBuffer = Marshal.AllocHGlobal(bufferSize);
                        Marshal.WriteInt32(detailDataBuffer, (IntPtr.Size == 4) ? (4 + Marshal.SystemDefaultCharSize) : 8);

                        // Second call gets the detailed data buffer
                        //Debug.WriteLine("usbGenericHidCommunication:findHidDevices() -> Getting details of the device");
                        success = SetupAPI.SetupDiGetDeviceInterfaceDetail(deviceInfoSet, ref deviceInterfaceData, detailDataBuffer, bufferSize, ref bufferSize, IntPtr.Zero);

                        // Skip over cbsize (4 bytes) to get the address of the devicePathName.
                        IntPtr pDevicePathName = new IntPtr(detailDataBuffer.ToInt32() + 4);

                        // Get the String containing the devicePathName.
                        listOfDevicePathNames[listIndex] = Marshal.PtrToStringAuto(pDevicePathName);

                        //Debug.WriteLine(string.Format("usbGenericHidCommunication:findHidDevices() -> Found matching device (memberIndex {0})", memberIndex));
                        deviceFound = true;
                    }
                    listIndex = listIndex + 1;
                }
                while (!((lastDevice == true)));
            }
            catch (Exception)
            {
                // Something went badly wrong... output some debug and return false to indicated device discovery failure
                Debug.WriteLine("usbGenericHidCommunication:findHidDevices() -> EXCEPTION: Something went south whilst trying to get devices with matching GUIDs - giving up!");
                return false;
            }
            finally
            {
                // Clean up the unmanaged memory allocations
                if (detailDataBuffer != IntPtr.Zero)
                {
                    // Free the memory allocated previously by AllocHGlobal.
                    Marshal.FreeHGlobal(detailDataBuffer);
                }

                if (deviceInfoSet != IntPtr.Zero)
                    SetupAPI.SetupDiDestroyDeviceInfoList(deviceInfoSet);
            }

            if (deviceFound)
            {
                Debug.WriteLine(string.Format("usbGenericHidCommunication:findHidDevices() -> Found {0} devices with matching GUID", listIndex - 1));
                numberOfDevicesFound = listIndex - 2;
            }
            else
                Debug.WriteLine("usbGenericHidCommunication:findHidDevices() -> No matching devices found");

            return deviceFound;
        }
        private void QueryHIDDeviceCapabilities()
        {
            IntPtr preparsedData = new IntPtr();
            try
            {
                // Get the preparsed data from the HID driver
                NativeHID.HidD_GetPreparsedData(deviceInfo.HIDHandle, ref preparsedData);
                // Get the HID device's capabilities
                NativeHID.HidP_GetCaps(preparsedData, ref deviceInfo.Capabilities);
            }
            catch (Exception ex)
            {
                throw ex;
            }
            finally
            {
                if (preparsedData != IntPtr.Zero)
                    NativeHID.HidD_FreePreparsedData(preparsedData);
            }
        }
        #endregion

        #region Device Notification
        public Boolean RegisterForDeviceNotifications(IntPtr windowHandle)
        {
            Debug.WriteLine("usbGenericHidCommunication:registerForDeviceNotifications() -> Method called");

            // A DEV_BROADCAST_DEVICEINTERFACE header holds information about the request.
            User32.DEV_BROADCAST_DEVICEINTERFACE devBroadcastDeviceInterface = new User32.DEV_BROADCAST_DEVICEINTERFACE();
            IntPtr devBroadcastDeviceInterfaceBuffer = IntPtr.Zero;
            Int32 size = 0;

            // Get the required GUID
            Guid systemHidGuid = new Guid();
            NativeHID.HidD_GetHidGuid(ref systemHidGuid);

            try
            {
                // Set the parameters in the DEV_BROADCAST_DEVICEINTERFACE structure.
                size = Marshal.SizeOf(devBroadcastDeviceInterface);
                devBroadcastDeviceInterface.dbcc_size = size;
                devBroadcastDeviceInterface.dbcc_devicetype = User32.DBT_DEVTYP_DEVICEINTERFACE;
                devBroadcastDeviceInterface.dbcc_reserved = 0;
                devBroadcastDeviceInterface.dbcc_classguid = systemHidGuid;

                devBroadcastDeviceInterfaceBuffer = Marshal.AllocHGlobal(size);
                Marshal.StructureToPtr(devBroadcastDeviceInterface, devBroadcastDeviceInterfaceBuffer, true);

                // Register for notifications and store the returned handle
                deviceInfo.NotificationHandle = User32.RegisterDeviceNotification(windowHandle, devBroadcastDeviceInterfaceBuffer, User32.DEVICE_NOTIFY_WINDOW_HANDLE);
                Marshal.PtrToStructure(devBroadcastDeviceInterfaceBuffer, devBroadcastDeviceInterface);

                if ((deviceInfo.NotificationHandle.ToInt32() == IntPtr.Zero.ToInt32()))
                {
                    Debug.WriteLine("usbGenericHidCommunication:registerForDeviceNotifications() -> Notification registration failed");
                    return false;
                }
                else
                {
                    Debug.WriteLine("usbGenericHidCommunication:registerForDeviceNotifications() -> Notification registration succeded");
                    return true;
                }
            }
            catch (Exception)
            {
                Debug.WriteLine("usbGenericHidCommunication:registerForDeviceNotifications() -> EXCEPTION: An unknown exception has occured!");
            }
            finally
            {
                // Free the memory allocated previously by AllocHGlobal.
                if (devBroadcastDeviceInterfaceBuffer != IntPtr.Zero)
                    Marshal.FreeHGlobal(devBroadcastDeviceInterfaceBuffer);
            }
            return false;
        }
        public Boolean UnregisterFromDeviceNotification(IntPtr windowHandle)
        {
            return User32.UnregisterDeviceNotification(windowHandle);
        }
        public void HandleDeviceNotificationMessages(Message m)
        {
            if (m.Msg == User32.WM_DEVICECHANGE)
            {
                try
                {
                    switch (m.WParam.ToInt32())
                    {
                        case User32.DBT_DEVICEARRIVAL:
                            if (!Attached)
                            {
                                FindTargetDevice();
                                if (Attached && Connected != null)
                                    Connected(this, EventArgs.Empty);
                            }
                            break;
                        case User32.DBT_DEVICEREMOVECOMPLETE:
                            if (IsNotificationForTargetDevice(m))
                            {
                                DetachTargetDevice();
                                if (Disconnected != null)
                                    Disconnected(this, EventArgs.Empty);
                            }
                            break;
                    }
                }
                catch (Exception)
                {
                }
            }

        }
        private Boolean IsNotificationForTargetDevice(Message m)
        {
            Int32 stringSize;
            try
            {
                User32.DEV_BROADCAST_DEVICEINTERFACE_1 devBroadcastDeviceInterface = new User32.DEV_BROADCAST_DEVICEINTERFACE_1();
                User32.DEV_BROADCAST_HDR devBroadcastHeader = new User32.DEV_BROADCAST_HDR();

                Marshal.PtrToStructure(m.LParam, devBroadcastHeader);

                // Is the notification event concerning a device interface?
                if (devBroadcastHeader.dbch_devicetype == User32.DBT_DEVTYP_DEVICEINTERFACE)
                {
                    // Get the device path name of the affected device
                    stringSize = System.Convert.ToInt32((devBroadcastHeader.dbch_size - 32) / 2);
                    devBroadcastDeviceInterface.dbcc_name = new Char[stringSize + 1];
                    Marshal.PtrToStructure(m.LParam, devBroadcastDeviceInterface);
                    String deviceNameString = new String(devBroadcastDeviceInterface.dbcc_name, 0, stringSize);

                    // Compare the device name with our target device's pathname (strings are moved to lower case using en-US to ensure case insensitivity accross different regions)
                    return String.Compare(deviceNameString.ToLower(new CultureInfo("en-US")), deviceInfo.PathName.ToLower(new CultureInfo("en-US")), true) == 0;
                }
            }
            catch (Exception)
            {
                return false;
            }
            return false;
        }
        #endregion

        #region Write
        public bool WriteFile(Byte[] data)
        {
            if (!Attached)
                return false;

            Byte[] outputReportBuffer = PrepareOutputReport(data);
            try
            {
                file.Write(outputReportBuffer, 0, outputReportBuffer.Length);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
        public bool WriteOutputReport(Byte[] data)
        {
            if (!Attached)
                return false;

            Byte[] outputReportBuffer = PrepareOutputReport(data);
            try
            {
                bool retval = false;
                //while (!retval)
                //    retval = NativeHID.HidD_SetOutputReport(deviceInfo.HIDHandle, outputReportBuffer, outputReportBuffer.Length);

                do
                {
                    retval = NativeHID.HidD_SetOutputReport(deviceInfo.HIDHandle, outputReportBuffer, outputReportBuffer.Length);
                }
                while (!retval);

                return retval;
            }
            catch (Exception)
            {
                return false;
            }
        }

        private Byte[] PrepareOutputReport(Byte[] data)
        {
            // The first byte must always be zero, or the write will fail!
            Byte[] outputReportBuffer = new byte[deviceInfo.Capabilities.outputReportByteLength];
            outputReportBuffer[0] = 0;
            int n = Math.Min(outputReportBuffer.Length - 1, data.Length);
            for (int i = 1; i <= n; i++)
                outputReportBuffer[i] = data[i - 1];
            return outputReportBuffer;
        }
        #endregion

        #region Read
        #region FileStream
        private void BeginAsyncRead()
        {
            byte[] inputReport = new byte[deviceInfo.Capabilities.inputReportByteLength];
            file.BeginRead(inputReport, 0, inputReport.Length, new AsyncCallback(ReadCompleted), inputReport);
        }
        private void ReadCompleted(IAsyncResult result)
        {
            byte[] inputReport = (byte[])result.AsyncState;	// retrieve the read buffer
            try
            {
                file.EndRead(result);	// this throws any exceptions that happened during the read
                try
                {
                    if (DataRecieved != null)
                        DataRecieved(this, new DataEventArgs(inputReport));
                }
                finally
                {
                    BeginAsyncRead();	// when all that is done, kick off another read for the next report
                }
            }
            catch (IOException)	// if we got an IO exception, the device was removed
            {
                //if (DeviceRemoved != null)
                //    DeviceRemoved(this, new EventArgs());
                //Dispose();
            }
        }
        #endregion

        #region HID Report
        public Byte[] ReadInputReport()
        {
            if (!Attached)
                return null;

            Byte[] inputReportBuffer = new byte[deviceInfo.Capabilities.inputReportByteLength];
            inputReportBuffer[0] = 0;
            try
            {
                NativeHID.HidD_GetInputReport(deviceInfo.HIDHandle, inputReportBuffer, inputReportBuffer.Length);
                return ParseInputReport(ref inputReportBuffer);
            }
            catch (Exception)
            {
                return null;
            }
        }

        private void ReadProc(object sender, DoWorkEventArgs e)
        {
            Byte[] inputReportBuffer = new byte[deviceInfo.Capabilities.inputReportByteLength];
            inputReportBuffer[0] = 0;
            while (deviceInfo.Attached)
            {
                try
                {
                    NativeHID.HidD_GetInputReport(deviceInfo.HIDHandle, inputReportBuffer, inputReportBuffer.Length);
                    if (DataRecieved != null)
                        DataRecieved.BeginInvoke(this, new DataEventArgs(ParseInputReport(ref inputReportBuffer)), null, null);
                }
                catch (Exception)
                {
                }
            }
        }
        private Byte[] ParseInputReport(ref Byte[] inputReportBuffer)
        {
            byte[] data = new byte[inputReportBuffer.Length - 1];
            for (int i = 1; i < inputReportBuffer.Length; i++)
                data[i - 1] = inputReportBuffer[i];
            return data;
        }
        #endregion

        #region WinAPI FILE
        public bool ReadSingleReportFromDevice(ref Byte[] inputReportBuffer)
        {
            int numberOfBytesRead = 0;
            if (inputReportBuffer.Length != (int)deviceInfo.Capabilities.inputReportByteLength)
                return false;
            else
                return ReadRawReportFromDevice(ref inputReportBuffer, ref numberOfBytesRead);
        }
        /// <summary>
        /// Attempts to retrieve multiple reports from the device in 
        /// a single read. This action can block the form execution if you request too much data.
        /// If you need more data from the device and want to avoid any blocking you will have to make
        /// multiple commands to the device and deal with the multiple requests and responses in your
        /// application.
        /// </summary>
        public bool ReadMultipleReportsFromDevice(ref Byte[] inputReportBuffer, int numberOfReports)
        {
            if (numberOfReports == 0 || numberOfReports > 128)
                return false;
            if (inputReportBuffer.Length != ((int)deviceInfo.Capabilities.inputReportByteLength * numberOfReports))
                return false;

            bool success = false;
            int numberOfBytesRead = 0;
            long pointerToBuffer = 0;
            Byte[] temp = new Byte[inputReportBuffer.Length];
            while (pointerToBuffer != ((int)deviceInfo.Capabilities.inputReportByteLength * numberOfReports))
            {
                success = ReadRawReportFromDevice(ref temp, ref numberOfBytesRead);
                if (!success)
                    return false;
                Array.Copy(temp, 0, inputReportBuffer, pointerToBuffer, (long)numberOfBytesRead);
                pointerToBuffer += (long)numberOfBytesRead;
            }
            return success;
        }

        /// <summary>
        /// Reads a raw report from the device with timeout handling
        /// Note: This method performs no checking on the buffer.  The first byte returned is
        /// usually zero.
        /// The maximum report size will be determind by the length of the inputReportBuffer.
        /// </summary>
        private bool ReadRawReportFromDevice(ref Byte[] inputReportBuffer, ref int numberOfBytesRead)
        {
            if (!Attached)
                return false;

            bool success = false;

            try
            {
                // Prepare an event object for the overlapped ReadFile
                IntPtr eventObject = Kernel32.CreateEvent(IntPtr.Zero, false, false, "");

                NativeOverlapped hidOverlapped = new NativeOverlapped();
                hidOverlapped.OffsetLow = 0;
                hidOverlapped.OffsetHigh = 0;
                hidOverlapped.EventHandle = eventObject;

                // Allocate memory for the unmanaged input buffer and overlap structure.
                IntPtr nonManagedBuffer = Marshal.AllocHGlobal(inputReportBuffer.Length);
                IntPtr nonManagedOverlapped = Marshal.AllocHGlobal(Marshal.SizeOf(hidOverlapped));
                Marshal.StructureToPtr(hidOverlapped, nonManagedOverlapped, false);

                // Read the input report buffer
                //success = Kernel32.ReadFile(
                //    deviceInfo.ReadHandle,
                //    nonManagedBuffer,
                //    inputReportBuffer.Length,
                //    ref numberOfBytesRead,
                //    nonManagedOverlapped);

                ////int err = Marshal.GetLastWin32Error();
                //if (!success)
                //{
                //    // Wait a maximum of 3 seconds for the FileRead to complete
                //    switch (Kernel32.WaitForSingleObject(eventObject, 3000))
                //    {
                //        case (System.Int32)Kernel32.WAIT_OBJECT_0:
                //            success = true;
                //            // Get the number of bytes transferred
                //            Kernel32.GetOverlappedResult(deviceInfo.ReadHandle, nonManagedOverlapped, ref numberOfBytesRead, false);
                //            break;
                //        case Kernel32.WAIT_TIMEOUT:
                //            Kernel32.CancelIo(deviceInfo.ReadHandle);
                //            if (!deviceInfo.HIDHandle.IsInvalid) deviceInfo.HIDHandle.Close();
                //            if (!deviceInfo.ReadHandle.IsInvalid) deviceInfo.ReadHandle.Close();
                //            if (!deviceInfo.WriteHandle.IsInvalid) deviceInfo.WriteHandle.Close();

                //            success = false;
                //            DetachTargetDevice();
                //            break;
                //        default:
                //            success = false;
                //            DetachTargetDevice();
                //            break;
                //    }
                //}
                if (success)
                    Marshal.Copy(nonManagedBuffer, inputReportBuffer, 0, numberOfBytesRead);
            }
            catch (Exception)
            {
                return false;
            }
            return success;
        }
        #endregion
        #endregion
    }
}
