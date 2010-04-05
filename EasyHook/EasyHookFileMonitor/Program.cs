using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Text;

namespace FileMon
{
    class Program
    {
        static void Main(string[] args)
        {
            if (args.Length != 2)
            {
                return;
            }

            if ("hook" == args[0])
            {
                Hook.CreateHook(args[1]);
            }
            else if ("run" == args[0])
            {
                Process proc = new Process();
                proc.StartInfo.FileName = args[1];
                proc.StartInfo.UseShellExecute = false;
                if (proc.Start())
                {
                    proc.WaitForExit();
                }
            }

            return;
       }
    }
}