using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Runtime.InteropServices;
using EasyHook;

namespace FileMonInject
{
    public class Main : EasyHook.IEntryPoint
    {
        FileMon.FileMonInterface Interface;
        LocalHook CreateFileHook;
        Stack<String> Queue = new Stack<String>();

        public Main(RemoteHooking.IContext InContext,
                    String InChannelName)
        {
            // connect to host...
            Interface = RemoteHooking.IpcConnectClient<FileMon.FileMonInterface>(InChannelName);
        }

        public void Run(RemoteHooking.IContext InContext,
                        String InChannelName)
        {
            // install hook...
            try
            {
                CreateFileHook = LocalHook.Create(LocalHook.GetProcAddress("kernel32.dll", "CreateFileW"),
                                                  new DCreateFile(CreateFile_Hooked),
                                                  this);

                CreateFileHook.ThreadACL.SetExclusiveACL(new Int32[] { 0 });
                RemoteHooking.WakeUpProcess();
            }
            catch (Exception ExtInfo)
            {
                Interface.ReportException(ExtInfo);
                return;
            }

            Interface.IsInstalled(RemoteHooking.GetCurrentProcessId());

            try
            {
                while (true)
                {
                    Interface.Ping();
                }
            }
            catch
            {
                //
            }
        }

        [UnmanagedFunctionPointer(CallingConvention.StdCall,
            CharSet = CharSet.Unicode,
            SetLastError = true)]
        delegate IntPtr DCreateFile(
            String InFileName,
            UInt32 InDesiredAccess,
            UInt32 InShareMode,
            IntPtr InSecurityAttributes,
            UInt32 InCreationDisposition,
            UInt32 InFlagsAndAttributes,
            IntPtr InTemplateFile);

        // just use a P-Invoke implementation to get native API access from C# (this step is not necessary for C++.NET)
        [DllImport("kernel32.dll",
            CharSet = CharSet.Unicode,
            SetLastError = true,
            CallingConvention = CallingConvention.StdCall)]
        static extern IntPtr CreateFile(
            String InFileName,
            UInt32 InDesiredAccess,
            UInt32 InShareMode,
            IntPtr InSecurityAttributes,
            UInt32 InCreationDisposition,
            UInt32 InFlagsAndAttributes,
            IntPtr InTemplateFile);

        // this is where we are intercepting all file accesses!
        static IntPtr CreateFile_Hooked(
            String InFileName,
            UInt32 InDesiredAccess,
            UInt32 InShareMode,
            IntPtr InSecurityAttributes,
            UInt32 InCreationDisposition,
            UInt32 InFlagsAndAttributes,
            IntPtr InTemplateFile)
        {
            // call original API...
            return CreateFile(
                InFileName,
                InDesiredAccess,
                InShareMode,
                InSecurityAttributes,
                InCreationDisposition,
                InFlagsAndAttributes,
                InTemplateFile);
        }
    }
}
