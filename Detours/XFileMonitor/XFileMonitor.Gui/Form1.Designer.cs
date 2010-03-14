namespace XFileMonitor.Gui
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.listOpenFiles = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.btnLaunch = new System.Windows.Forms.Button();
            this.textTargetApp = new System.Windows.Forms.TextBox();
            this.richFileOps = new System.Windows.Forms.RichTextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // listOpenFiles
            // 
            this.listOpenFiles.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listOpenFiles.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.listOpenFiles.FullRowSelect = true;
            this.listOpenFiles.Location = new System.Drawing.Point(13, 66);
            this.listOpenFiles.Name = "listOpenFiles";
            this.listOpenFiles.Size = new System.Drawing.Size(672, 279);
            this.listOpenFiles.Sorting = System.Windows.Forms.SortOrder.Ascending;
            this.listOpenFiles.TabIndex = 0;
            this.listOpenFiles.UseCompatibleStateImageBehavior = false;
            this.listOpenFiles.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "File Name";
            this.columnHeader1.Width = 300;
            // 
            // btnLaunch
            // 
            this.btnLaunch.Location = new System.Drawing.Point(13, 13);
            this.btnLaunch.Name = "btnLaunch";
            this.btnLaunch.Size = new System.Drawing.Size(110, 23);
            this.btnLaunch.TabIndex = 1;
            this.btnLaunch.Text = "Launch";
            this.btnLaunch.UseVisualStyleBackColor = true;
            this.btnLaunch.Click += new System.EventHandler(this.btnLaunch_Click);
            // 
            // textTargetApp
            // 
            this.textTargetApp.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.textTargetApp.Location = new System.Drawing.Point(129, 15);
            this.textTargetApp.Multiline = true;
            this.textTargetApp.Name = "textTargetApp";
            this.textTargetApp.ReadOnly = true;
            this.textTargetApp.Size = new System.Drawing.Size(555, 21);
            this.textTargetApp.TabIndex = 2;
            // 
            // richFileOps
            // 
            this.richFileOps.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.richFileOps.Location = new System.Drawing.Point(13, 378);
            this.richFileOps.Name = "richFileOps";
            this.richFileOps.Size = new System.Drawing.Size(672, 236);
            this.richFileOps.TabIndex = 3;
            this.richFileOps.Text = "";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 47);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(86, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "All Files Opened:";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(11, 360);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(114, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "File Writes and Reads:";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(696, 626);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.richFileOps);
            this.Controls.Add(this.textTargetApp);
            this.Controls.Add(this.btnLaunch);
            this.Controls.Add(this.listOpenFiles);
            this.Name = "Form1";
            this.Text = "Coding the Wheel - XFileMonitor v1.0";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView listOpenFiles;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button btnLaunch;
        private System.Windows.Forms.TextBox textTargetApp;
        private System.Windows.Forms.RichTextBox richFileOps;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
    }
}

