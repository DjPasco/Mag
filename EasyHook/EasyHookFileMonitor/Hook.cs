using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using EasyHook;
using System.Runtime.Remoting;
using System.Diagnostics;

namespace FileMon
{
    public class FileMonInterface : MarshalByRefObject
    {
        public void IsInstalled(Int32 InClientPID)
        {
            //Console.WriteLine("FileMon has been installed in target {0}.\r\n", InClientPID);
        }

        public void OnCreateFile(Int32 InClientPID, String InFileName)
        {
            //Console.WriteLine(InFileName);
        }

        public void ReportException(Exception InInfo)
        {
            //Console.WriteLine("The target process has reported an error:\r\n" + InInfo.ToString());
        }

        public void Ping()
        {
        }
    }

    class Hook
    {
        static String ChannelName = null;

        static public void CreateHook(string sExeName)
        {
            try
            {
                try
                {
                    Config.Register(
                        "A FileMon like demo application.",
                        "FileMon.exe",
                        "FileMonInject.dll");
                }
                catch (ApplicationException)
                {
                    Console.WriteLine("This is an administrative task!", "Permission denied...");
                    System.Diagnostics.Process.GetCurrentProcess().Kill();
                }

                RemoteHooking.IpcCreateServer<FileMonInterface>(ref ChannelName, WellKnownObjectMode.SingleCall);
                int nProcID;
                RemoteHooking.CreateAndInject(
                    sExeName,
                    "",
                    0,
                    "FileMonInject.dll",
                    "FileMonInject.dll",
                    out nProcID,
                    ChannelName);

                System.Diagnostics.Process.GetProcessById(nProcID).WaitForExit();
            }
            catch (Exception ExtInfo)
            {
                Console.WriteLine("There was an error while connecting to target:\r\n{0}", ExtInfo.ToString());
            }
         }
    }
}
