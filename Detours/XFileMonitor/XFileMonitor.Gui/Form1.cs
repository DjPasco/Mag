using System;
using System.ComponentModel;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace XFileMonitor.Gui
{
    public partial class Form1 : Form
    {
        // Managed version of Win32 COPYDATASTRUCT
        public struct COPYDATASTRUCT
        {
            public int dwData;
            public int cbData;
            public IntPtr lpData;
        };

        // Struct used to store file i/o context. Defined
        // as native structure in XFileMonitor.Hook.cpp
        public struct FILECONTEXT
        {
            public IntPtr Handle;
            public System.Int32 OriginalAPI;
        }

        // File operations enumeration. Defined as native
        // enum in XFileMonitor.Hook.cpp.
        enum FileAction
        {
            File_Unknown = 0,
            File_CreateAlways,
            File_CreateNew,
            File_OpenAlways,
            File_OpenExisting,
            File_TruncateExisting,

            File_Write,
            File_Read,
            File_Close
        };

        // WM_COPYDATA msg value
        const int WM_COPYDATA = 0x4A;

        // Here's the C-language export we need to call
        [DllImport("XFileMonitor.Hook.dll", CharSet=CharSet.Auto)]
        static extern Boolean InstallHook(string targetApplication);

        public Form1()
        {
            InitializeComponent();

            _boldFont = new Font(richFileOps.SelectionFont, FontStyle.Bold);
        }

        private Font _boldFont;

        private void OnCopyData(ref Message m)
        {
            // Get the COPYDATASTRUCT
            COPYDATASTRUCT cds = new COPYDATASTRUCT();
            cds = (COPYDATASTRUCT)Marshal.PtrToStructure(m.LParam, typeof(COPYDATASTRUCT));
            
            // The lpData member of COPYDATASTRUCT points to an (unmanaged) FILECONTEXT struct
            FILECONTEXT ht = (FILECONTEXT)Marshal.PtrToStructure(cds.lpData, typeof(FILECONTEXT));

            string strTemp;

            // If the target application opened, created, wrote to, or read from a file...
            if (cds.dwData >= (int)FileAction.File_CreateAlways && cds.dwData <= (int) FileAction.File_Read)
            {
                int unManagedSize = Marshal.SizeOf(typeof(FILECONTEXT));
                unsafe
                {
                    byte* pString = (byte*)cds.lpData.ToPointer();
                    pString += unManagedSize;
                    IntPtr pManString = new IntPtr((void*)pString);
                    strTemp = Marshal.PtrToStringUni(pManString);
                }

                if (cds.dwData <= 5)
                {
                    ListViewItem[] lvs = listOpenFiles.Items.Find(strTemp, true);
                    if (lvs == null || lvs.Length == 0)
                    {
                        ListViewItem lv = listOpenFiles.Items.Insert(listOpenFiles.Items.Count, strTemp);
                        lv.Tag = ht.Handle;
                        lv.Name = strTemp;
                    }
                }
                else
                {
                    int nIndex = strTemp.IndexOf('\n');
                    if (nIndex > -1)
                    {
                        string Intro = strTemp.Substring(0, nIndex);
                        string Other = strTemp.Substring(nIndex);
                        Font oldFont = richFileOps.SelectionFont;
                        richFileOps.SelectionFont = _boldFont;
                        richFileOps.SelectedText = Intro;
                        richFileOps.SelectionFont = oldFont;
                        richFileOps.Select(richFileOps.TextLength, 0);
                        richFileOps.SelectedText = Other;
                    }
                }
                m.Result = (IntPtr)1;
            }
            // If the target application CLOSED a file
            else if (cds.dwData == (int)FileAction.File_Close)
            {
                // TODO: remove the file from the GUI
            }
        }

        protected override void WndProc(ref Message m)
        {
            if (m.Msg == WM_COPYDATA)
                OnCopyData(ref m);

            base.WndProc(ref m);
        }

        private void btnLaunch_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "Executable files (*.exe)|*.exe||";
            dlg.RestoreDirectory = true;
            if (DialogResult.OK == dlg.ShowDialog())
            {
                textTargetApp.Text = dlg.FileName;
                InstallHook(dlg.FileName);
            }
        }
    }
}