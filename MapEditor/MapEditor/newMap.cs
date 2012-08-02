using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace MapEditor
{
	/// <summary>
	/// Summary description for newMap.
	/// </summary>
	public class newMap : System.Windows.Forms.Form
	{
        public System.Windows.Forms.TextBox tPath;
        public System.Windows.Forms.NumericUpDown nTileY;
        public System.Windows.Forms.NumericUpDown nTileX;
        private System.Windows.Forms.Button bBrowse;
        private System.Windows.Forms.OpenFileDialog openDialog;
        private System.Windows.Forms.Button bOK;
        private System.Windows.Forms.Button bCancel;
		private System.Windows.Forms.Label label1;
		private System.Windows.Forms.Label label2;
		private System.Windows.Forms.Label label3;
		private System.Windows.Forms.Label label4;
		private System.Windows.Forms.Label label7;
		public System.Windows.Forms.NumericUpDown nNumPlayers;
		public System.Windows.Forms.TextBox tDesc;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public newMap()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			this.tPath = new System.Windows.Forms.TextBox();
			this.nTileY = new System.Windows.Forms.NumericUpDown();
			this.nTileX = new System.Windows.Forms.NumericUpDown();
			this.bBrowse = new System.Windows.Forms.Button();
			this.openDialog = new System.Windows.Forms.OpenFileDialog();
			this.bOK = new System.Windows.Forms.Button();
			this.bCancel = new System.Windows.Forms.Button();
			this.label1 = new System.Windows.Forms.Label();
			this.label2 = new System.Windows.Forms.Label();
			this.label3 = new System.Windows.Forms.Label();
			this.label4 = new System.Windows.Forms.Label();
			this.tDesc = new System.Windows.Forms.TextBox();
			this.label7 = new System.Windows.Forms.Label();
			this.nNumPlayers = new System.Windows.Forms.NumericUpDown();
			((System.ComponentModel.ISupportInitialize)(this.nTileY)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nTileX)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.nNumPlayers)).BeginInit();
			this.SuspendLayout();
			// 
			// tPath
			// 
			this.tPath.Location = new System.Drawing.Point(64, 72);
			this.tPath.Name = "tPath";
			this.tPath.Size = new System.Drawing.Size(136, 20);
			this.tPath.TabIndex = 2;
			this.tPath.Text = "tileset_default.txt";
			// 
			// nTileY
			// 
			this.nTileY.Location = new System.Drawing.Point(144, 32);
			this.nTileY.Maximum = new System.Decimal(new int[] {
																   65535,
																   0,
																   0,
																   0});
			this.nTileY.Name = "nTileY";
			this.nTileY.Size = new System.Drawing.Size(56, 20);
			this.nTileY.TabIndex = 1;
			this.nTileY.Value = new System.Decimal(new int[] {
																 256,
																 0,
																 0,
																 0});
			// 
			// nTileX
			// 
			this.nTileX.Location = new System.Drawing.Point(144, 8);
			this.nTileX.Maximum = new System.Decimal(new int[] {
																   65535,
																   0,
																   0,
																   0});
			this.nTileX.Name = "nTileX";
			this.nTileX.Size = new System.Drawing.Size(56, 20);
			this.nTileX.TabIndex = 0;
			this.nTileX.Value = new System.Decimal(new int[] {
																 256,
																 0,
																 0,
																 0});
			// 
			// bBrowse
			// 
			this.bBrowse.Location = new System.Drawing.Point(208, 72);
			this.bBrowse.Name = "bBrowse";
			this.bBrowse.Size = new System.Drawing.Size(64, 24);
			this.bBrowse.TabIndex = 3;
			this.bBrowse.Text = "Browse...";
			this.bBrowse.Click += new System.EventHandler(this.bBrowse_Click);
			// 
			// bOK
			// 
			this.bOK.Location = new System.Drawing.Point(208, 176);
			this.bOK.Name = "bOK";
			this.bOK.Size = new System.Drawing.Size(64, 24);
			this.bOK.TabIndex = 4;
			this.bOK.Text = "OK";
			this.bOK.Click += new System.EventHandler(this.bOK_Click);
			// 
			// bCancel
			// 
			this.bCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
			this.bCancel.Location = new System.Drawing.Point(136, 176);
			this.bCancel.Name = "bCancel";
			this.bCancel.Size = new System.Drawing.Size(64, 24);
			this.bCancel.TabIndex = 5;
			this.bCancel.Text = "Cancel";
			this.bCancel.Click += new System.EventHandler(this.bCancel_Click);
			// 
			// label1
			// 
			this.label1.Location = new System.Drawing.Point(8, 8);
			this.label1.Name = "label1";
			this.label1.Size = new System.Drawing.Size(112, 16);
			this.label1.TabIndex = 6;
			this.label1.Text = "Width";
			// 
			// label2
			// 
			this.label2.Location = new System.Drawing.Point(8, 32);
			this.label2.Name = "label2";
			this.label2.Size = new System.Drawing.Size(120, 16);
			this.label2.TabIndex = 7;
			this.label2.Text = "Height";
			// 
			// label3
			// 
			this.label3.Location = new System.Drawing.Point(8, 72);
			this.label3.Name = "label3";
			this.label3.Size = new System.Drawing.Size(56, 16);
			this.label3.TabIndex = 8;
			this.label3.Text = "Tileset";
			// 
			// label4
			// 
			this.label4.Location = new System.Drawing.Point(8, 112);
			this.label4.Name = "label4";
			this.label4.Size = new System.Drawing.Size(80, 16);
			this.label4.TabIndex = 9;
			this.label4.Text = "Description";
			// 
			// tDesc
			// 
			this.tDesc.Location = new System.Drawing.Point(136, 104);
			this.tDesc.Name = "tDesc";
			this.tDesc.Size = new System.Drawing.Size(136, 20);
			this.tDesc.TabIndex = 2;
			this.tDesc.Text = "";
			// 
			// label7
			// 
			this.label7.Location = new System.Drawing.Point(8, 136);
			this.label7.Name = "label7";
			this.label7.Size = new System.Drawing.Size(104, 16);
			this.label7.TabIndex = 9;
			this.label7.Text = "Number of Players";
			// 
			// nNumPlayers
			// 
			this.nNumPlayers.Location = new System.Drawing.Point(136, 136);
			this.nNumPlayers.Maximum = new System.Decimal(new int[] {
																		65535,
																		0,
																		0,
																		0});
			this.nNumPlayers.Minimum = new System.Decimal(new int[] {
																		2,
																		0,
																		0,
																		0});
			this.nNumPlayers.Name = "nNumPlayers";
			this.nNumPlayers.Size = new System.Drawing.Size(56, 20);
			this.nNumPlayers.TabIndex = 1;
			this.nNumPlayers.Value = new System.Decimal(new int[] {
																	  2,
																	  0,
																	  0,
																	  0});
			// 
			// newMap
			// 
			this.AcceptButton = this.bOK;
			this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
			this.CancelButton = this.bCancel;
			this.ClientSize = new System.Drawing.Size(280, 216);
			this.Controls.Add(this.label4);
			this.Controls.Add(this.label3);
			this.Controls.Add(this.label2);
			this.Controls.Add(this.label1);
			this.Controls.Add(this.bOK);
			this.Controls.Add(this.bBrowse);
			this.Controls.Add(this.nTileY);
			this.Controls.Add(this.tPath);
			this.Controls.Add(this.nTileX);
			this.Controls.Add(this.bCancel);
			this.Controls.Add(this.tDesc);
			this.Controls.Add(this.label7);
			this.Controls.Add(this.nNumPlayers);
			this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
			this.Name = "newMap";
			this.Text = "Map Properties";
			((System.ComponentModel.ISupportInitialize)(this.nTileY)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nTileX)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.nNumPlayers)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

        private void bBrowse_Click(object sender, System.EventArgs e)
        {
            DialogResult r = openDialog.ShowDialog();
            if(r==DialogResult.OK)
                tPath.Text = openDialog.FileName.Substring(openDialog.FileName.LastIndexOf("\\")+1);
        }

        private void bOK_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.OK;
            Close();
           
        }

        private void bCancel_Click(object sender, System.EventArgs e)
        {
            DialogResult = DialogResult.Cancel;
            Close();
        }
	}
}
