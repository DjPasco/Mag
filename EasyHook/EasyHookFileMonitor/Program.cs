using System;
using System.Collections.Generic;
using System.Runtime.Remoting;
using Microsoft.Win32;
using System.Text;
using System.IO;
using EasyHook;

namespace FileMon
{
    public class FileMonInterface : MarshalByRefObject
    {
        public void IsInstalled(Int32 InClientPID)
        {
            Console.WriteLine("FileMon has been installed in target {0}.\r\n", InClientPID);
        }

        public void OnCreateFile(Int32 InClientPID, String InFileName)
        {
            //Console.WriteLine(InFileName);
        }

        public void ReportException(Exception InInfo)
        {
            Console.WriteLine("The target process has reported an error:\r\n" + InInfo.ToString());
        }

        public void Ping()
        {
        }
    }

    class Program
    {
        static String ChannelName = null;

        static void Main(string[] args)
        {
            //Int32 TargetPID = 0;

            //if ((args.Length != 1) || !Int32.TryParse(args[0], out TargetPID))
            //{
            //    Console.WriteLine();
            //    Console.WriteLine("Usage: FileMon %PID%");
            //    Console.WriteLine();

            //    return;
            //}

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
                    
                }

                RemoteHooking.IpcCreateServer<FileMonInterface>(ref ChannelName, WellKnownObjectMode.SingleCall);
                int nProcID;
                RemoteHooking.CreateAndInject(
                    "FileUsage.exe", 
                    "",
                    0,
                    "FileMonInject.dll",
                    "FileMonInject.dll",
                    out nProcID,
                    ChannelName);

                Console.ReadLine();
            }
            catch (Exception ExtInfo)
            {
                Console.WriteLine("There was an error while connecting to target:\r\n{0}", ExtInfo.ToString());
            }
        }
    }
}