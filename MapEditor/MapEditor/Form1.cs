using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

namespace MapEditor
{

public class Form1 : System.Windows.Forms.Form
{
	newMap mapProp = new newMap();
	private System.ComponentModel.Container components = null;
	private System.Windows.Forms.Panel pMapView;
	Map map = null;
	private System.Windows.Forms.ListView lTileSet;
	float scale = 1f;

	private System.Windows.Forms.SaveFileDialog saveBox;
	private System.Windows.Forms.OpenFileDialog openBox;
	private System.Windows.Forms.MainMenu mainMenu1;
	private System.Windows.Forms.Panel toolGroup;
	private System.Windows.Forms.TabPage tAuto;
	private System.Windows.Forms.TabPage tMan;
	private System.Windows.Forms.TabPage tSpec;
	private System.Windows.Forms.ListView lManualTS;
	private System.Windows.Forms.Label label2;
	private System.Windows.Forms.RadioButton radioButton1;
	private System.Windows.Forms.RadioButton radioButton2;
	private System.Windows.Forms.NumericUpDown nOwner;
	private System.Windows.Forms.MenuItem menuFile;
	private System.Windows.Forms.MenuItem menuEdit;
	private System.Windows.Forms.MenuItem menuUndo;
	private System.Windows.Forms.MenuItem menuRedo;
	private System.Windows.Forms.MenuItem menuMirrorHoriz;
	private System.Windows.Forms.MenuItem menuMirrorVert;
	private System.Windows.Forms.MenuItem menuView;
	private System.Windows.Forms.MenuItem zoomIn;
	private System.Windows.Forms.MenuItem zoomOut;
	private System.Windows.Forms.MenuItem menuNew;
	private System.Windows.Forms.MenuItem menuSave;
	private System.Windows.Forms.MenuItem menuLoad;
	private System.Windows.Forms.MenuItem menuExit;
	private System.Windows.Forms.MenuItem viewRefresh;
	private System.Windows.Forms.Splitter splitter;
	private System.Windows.Forms.TabControl tileTabs;
	private System.Windows.Forms.RadioButton radioButton3;
	private System.Windows.Forms.GroupBox groupBox1;
	private System.Windows.Forms.MenuItem menuItem1;
	bool pressed = false;
	
	public Form1()
	{
		//
		// Required for Windows Form Designer support
		//
		InitializeComponent();
		
		//m= new Map("default_map.txt");
		//Map.getTileSet().fillListView(lTileSet);
		SetStyle(ControlStyles.DoubleBuffer | ControlStyles.AllPaintingInWmPaint , true);
		toolGroup.Location = new Point(Size.Width - 150,toolGroup.Location.Y);
		pMapView.Width = Size.Width - 200;
		pMapView.Height = Size.Height - 50;

		Map.parent = pMapView;
		/*
		SetStyle(ControlStyles.UserPaint, true);
		SetStyle(ControlStyles.AllPaintingInWmPaint, true);
		SetStyle(ControlStyles.DoubleBuffer, true);
		*/
	}

	/// <summary>
	/// Clean up any resources being used.
	/// </summary>
	protected override void Dispose( bool disposing )
	{
		if( disposing && components != null )
			components.Dispose();
		base.Dispose( disposing );
	}

	#region Windows Form Designer generated code
	/// <summary>
	/// Required method for Designer support - do not modify
	/// the contents of this method with the code editor.
	/// </summary>
	private void InitializeComponent()
	{
		this.pMapView = new System.Windows.Forms.Panel();
		this.lTileSet = new System.Windows.Forms.ListView();
		this.saveBox = new System.Windows.Forms.SaveFileDialog();
		this.openBox = new System.Windows.Forms.OpenFileDialog();
		this.mainMenu1 = new System.Windows.Forms.MainMenu();
		this.menuFile = new System.Windows.Forms.MenuItem();
		this.menuNew = new System.Windows.Forms.MenuItem();
		this.menuLoad = new System.Windows.Forms.MenuItem();
		this.menuSave = new System.Windows.Forms.MenuItem();
		this.menuExit = new System.Windows.Forms.MenuItem();
		this.menuEdit = new System.Windows.Forms.MenuItem();
		this.menuUndo = new System.Windows.Forms.MenuItem();
		this.menuRedo = new System.Windows.Forms.MenuItem();
		this.menuMirrorHoriz = new System.Windows.Forms.MenuItem();
		this.menuMirrorVert = new System.Windows.Forms.MenuItem();
		this.menuItem1 = new System.Windows.Forms.MenuItem();
		this.menuView = new System.Windows.Forms.MenuItem();
		this.zoomIn = new System.Windows.Forms.MenuItem();
		this.zoomOut = new System.Windows.Forms.MenuItem();
		this.viewRefresh = new System.Windows.Forms.MenuItem();
		this.toolGroup = new System.Windows.Forms.Panel();
		this.tileTabs = new System.Windows.Forms.TabControl();
		this.tAuto = new System.Windows.Forms.TabPage();
		this.tMan = new System.Windows.Forms.TabPage();
		this.lManualTS = new System.Windows.Forms.ListView();
		this.tSpec = new System.Windows.Forms.TabPage();
		this.groupBox1 = new System.Windows.Forms.GroupBox();
		this.radioButton1 = new System.Windows.Forms.RadioButton();
		this.radioButton3 = new System.Windows.Forms.RadioButton();
		this.radioButton2 = new System.Windows.Forms.RadioButton();
		this.label2 = new System.Windows.Forms.Label();
		this.nOwner = new System.Windows.Forms.NumericUpDown();
		this.splitter = new System.Windows.Forms.Splitter();
		this.toolGroup.SuspendLayout();
		this.tileTabs.SuspendLayout();
		this.tAuto.SuspendLayout();
		this.tMan.SuspendLayout();
		this.tSpec.SuspendLayout();
		this.groupBox1.SuspendLayout();
		((System.ComponentModel.ISupportInitialize)(this.nOwner)).BeginInit();
		this.SuspendLayout();
		// 
		// pMapView
		// 
		this.pMapView.AutoScroll = true;
		this.pMapView.AutoScrollMinSize = new System.Drawing.Size(800, 600);
		this.pMapView.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
		this.pMapView.Dock = System.Windows.Forms.DockStyle.Left;
		this.pMapView.Location = new System.Drawing.Point(0, 0);
		this.pMapView.Name = "pMapView";
		this.pMapView.Size = new System.Drawing.Size(520, 614);
		this.pMapView.TabIndex = 1;
		this.pMapView.SizeChanged += new System.EventHandler(this.pMapView_SizeChanged);
		this.pMapView.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pMapView_MouseUp);
		this.pMapView.Paint += new System.Windows.Forms.PaintEventHandler(this.pMapView_Paint);
		this.pMapView.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pMapView_MouseMove);
		this.pMapView.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pMapView_MouseDown);
		// 
		// lTileSet
		// 
		this.lTileSet.Dock = System.Windows.Forms.DockStyle.Fill;
		this.lTileSet.Location = new System.Drawing.Point(0, 0);
		this.lTileSet.MultiSelect = false;
		this.lTileSet.Name = "lTileSet";
		this.lTileSet.Size = new System.Drawing.Size(216, 590);
		this.lTileSet.TabIndex = 3;
		// 
		// mainMenu1
		// 
		this.mainMenu1.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																				  this.menuFile,
																				  this.menuEdit,
																				  this.menuView});
		// 
		// menuFile
		// 
		this.menuFile.Index = 0;
		this.menuFile.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																				 this.menuNew,
																				 this.menuLoad,
																				 this.menuSave,
																				 this.menuExit});
		this.menuFile.Text = "&File";
		// 
		// menuNew
		// 
		this.menuNew.Index = 0;
		this.menuNew.Shortcut = System.Windows.Forms.Shortcut.CtrlN;
		this.menuNew.Text = "&New";
		this.menuNew.Click += new System.EventHandler(this.bNew_Click);
		// 
		// menuLoad
		// 
		this.menuLoad.Index = 1;
		this.menuLoad.Shortcut = System.Windows.Forms.Shortcut.CtrlO;
		this.menuLoad.Text = "&Open";
		this.menuLoad.Click += new System.EventHandler(this.bLoad_Click);
		// 
		// menuSave
		// 
		this.menuSave.Index = 2;
		this.menuSave.Shortcut = System.Windows.Forms.Shortcut.CtrlS;
		this.menuSave.Text = "&Save";
		this.menuSave.Click += new System.EventHandler(this.button1_Click);
		// 
		// menuExit
		// 
		this.menuExit.Index = 3;
		this.menuExit.Text = "E&xit";
		this.menuExit.Click += new System.EventHandler(this.menuExit_Click);
		// 
		// menuEdit
		// 
		this.menuEdit.Index = 1;
		this.menuEdit.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																				 this.menuUndo,
																				 this.menuRedo,
																				 this.menuMirrorHoriz,
																				 this.menuMirrorVert,
																				 this.menuItem1});
		this.menuEdit.Text = "&Edit";
		// 
		// menuUndo
		// 
		this.menuUndo.Enabled = false;
		this.menuUndo.Index = 0;
		this.menuUndo.Shortcut = System.Windows.Forms.Shortcut.CtrlZ;
		this.menuUndo.Text = "Undo";
		this.menuUndo.Click += new System.EventHandler(this.bUndo_Click);
		// 
		// menuRedo
		// 
		this.menuRedo.Enabled = false;
		this.menuRedo.Index = 1;
		this.menuRedo.Shortcut = System.Windows.Forms.Shortcut.CtrlY;
		this.menuRedo.Text = "Redo";
		this.menuRedo.Click += new System.EventHandler(this.bRedo_Click);
		// 
		// menuMirrorHoriz
		// 
		this.menuMirrorHoriz.Index = 2;
		this.menuMirrorHoriz.Text = "Mirror-duplicate horizontal";
		this.menuMirrorHoriz.Click += new System.EventHandler(this.menuMirrorHoriz_Click);
		// 
		// menuMirrorVert
		// 
		this.menuMirrorVert.Index = 3;
		this.menuMirrorVert.Text = "Mirror-duplicate vertical";
		this.menuMirrorVert.Click += new System.EventHandler(this.menuMirrorVert_Click);
		// 
		// menuItem1
		// 
		this.menuItem1.Index = 4;
		this.menuItem1.Text = "Properties";
		this.menuItem1.Click += new System.EventHandler(this.menuItem1_Click);
		// 
		// menuView
		// 
		this.menuView.Index = 2;
		this.menuView.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
																				 this.zoomIn,
																				 this.zoomOut,
																				 this.viewRefresh});
		this.menuView.Text = "&View";
		// 
		// zoomIn
		// 
		this.zoomIn.Index = 0;
		this.zoomIn.Text = "Zoom In";
		this.zoomIn.Click += new System.EventHandler(this.viewZoomIn_Click);
		// 
		// zoomOut
		// 
		this.zoomOut.Index = 1;
		this.zoomOut.Text = "Zoom Out";
		this.zoomOut.Click += new System.EventHandler(this.viewZoomOut_Click);
		// 
		// viewRefresh
		// 
		this.viewRefresh.Index = 2;
		this.viewRefresh.Shortcut = System.Windows.Forms.Shortcut.F5;
		this.viewRefresh.Text = "Refresh";
		this.viewRefresh.Click += new System.EventHandler(this.viewRefresh_Click);
		// 
		// toolGroup
		// 
		this.toolGroup.Controls.Add(this.tileTabs);
		this.toolGroup.Dock = System.Windows.Forms.DockStyle.Fill;
		this.toolGroup.Location = new System.Drawing.Point(520, 0);
		this.toolGroup.Name = "toolGroup";
		this.toolGroup.Size = new System.Drawing.Size(232, 614);
		this.toolGroup.TabIndex = 7;
		// 
		// tileTabs
		// 
		this.tileTabs.Controls.Add(this.tAuto);
		this.tileTabs.Controls.Add(this.tMan);
		this.tileTabs.Controls.Add(this.tSpec);
		this.tileTabs.Location = new System.Drawing.Point(8, 0);
		this.tileTabs.Name = "tileTabs";
		this.tileTabs.SelectedIndex = 0;
		this.tileTabs.Size = new System.Drawing.Size(224, 616);
		this.tileTabs.TabIndex = 5;
		// 
		// tAuto
		// 
		this.tAuto.Controls.Add(this.lTileSet);
		this.tAuto.Location = new System.Drawing.Point(4, 22);
		this.tAuto.Name = "tAuto";
		this.tAuto.Size = new System.Drawing.Size(216, 590);
		this.tAuto.TabIndex = 0;
		this.tAuto.Text = "Automatic";
		// 
		// tMan
		// 
		this.tMan.Controls.Add(this.lManualTS);
		this.tMan.Location = new System.Drawing.Point(4, 22);
		this.tMan.Name = "tMan";
		this.tMan.Size = new System.Drawing.Size(216, 590);
		this.tMan.TabIndex = 1;
		this.tMan.Text = "Manual";
		// 
		// lManualTS
		// 
		this.lManualTS.Dock = System.Windows.Forms.DockStyle.Fill;
		this.lManualTS.Location = new System.Drawing.Point(0, 0);
		this.lManualTS.MultiSelect = false;
		this.lManualTS.Name = "lManualTS";
		this.lManualTS.Size = new System.Drawing.Size(216, 590);
		this.lManualTS.TabIndex = 4;
		// 
		// tSpec
		// 
		this.tSpec.Controls.Add(this.groupBox1);
		this.tSpec.Controls.Add(this.label2);
		this.tSpec.Controls.Add(this.nOwner);
		this.tSpec.Location = new System.Drawing.Point(4, 22);
		this.tSpec.Name = "tSpec";
		this.tSpec.Size = new System.Drawing.Size(216, 590);
		this.tSpec.TabIndex = 2;
		this.tSpec.Text = "Special";
		// 
		// groupBox1
		// 
		this.groupBox1.Controls.Add(this.radioButton1);
		this.groupBox1.Controls.Add(this.radioButton3);
		this.groupBox1.Controls.Add(this.radioButton2);
		this.groupBox1.Location = new System.Drawing.Point(24, 8);
		this.groupBox1.Name = "groupBox1";
		this.groupBox1.Size = new System.Drawing.Size(152, 104);
		this.groupBox1.TabIndex = 3;
		this.groupBox1.TabStop = false;
		// 
		// radioButton1
		// 
		this.radioButton1.Location = new System.Drawing.Point(24, 72);
		this.radioButton1.Name = "radioButton1";
		this.radioButton1.Size = new System.Drawing.Size(64, 16);
		this.radioButton1.TabIndex = 3;
		this.radioButton1.Text = "Landfill";
		// 
		// radioButton3
		// 
		this.radioButton3.Checked = true;
		this.radioButton3.Location = new System.Drawing.Point(24, 24);
		this.radioButton3.Name = "radioButton3";
		this.radioButton3.Size = new System.Drawing.Size(72, 16);
		this.radioButton3.TabIndex = 1;
		this.radioButton3.TabStop = true;
		this.radioButton3.Text = "Edit";
		// 
		// radioButton2
		// 
		this.radioButton2.Location = new System.Drawing.Point(24, 48);
		this.radioButton2.Name = "radioButton2";
		this.radioButton2.Size = new System.Drawing.Size(72, 16);
		this.radioButton2.TabIndex = 2;
		this.radioButton2.Text = "Oil Well";
		// 
		// label2
		// 
		this.label2.Location = new System.Drawing.Point(32, 192);
		this.label2.Name = "label2";
		this.label2.Size = new System.Drawing.Size(48, 24);
		this.label2.TabIndex = 1;
		this.label2.Text = "Owner:";
		// 
		// nOwner
		// 
		this.nOwner.Location = new System.Drawing.Point(80, 192);
		this.nOwner.Maximum = new System.Decimal(new int[] {
															   16,
															   0,
															   0,
															   0});
		this.nOwner.Name = "nOwner";
		this.nOwner.Size = new System.Drawing.Size(48, 20);
		this.nOwner.TabIndex = 4;
		this.nOwner.Value = new System.Decimal(new int[] {
															 1,
															 0,
															 0,
															 0});
		this.nOwner.ValueChanged += new System.EventHandler(this.nOwner_ValueChanged);
		// 
		// splitter
		// 
		this.splitter.BackColor = System.Drawing.SystemColors.ControlDarkDark;
		this.splitter.Location = new System.Drawing.Point(520, 0);
		this.splitter.Name = "splitter";
		this.splitter.Size = new System.Drawing.Size(8, 614);
		this.splitter.TabIndex = 8;
		this.splitter.TabStop = false;
		this.splitter.SplitterMoved += new System.Windows.Forms.SplitterEventHandler(this.splitter_Moved);
		// 
		// Form1
		// 
		this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
		this.ClientSize = new System.Drawing.Size(752, 614);
		this.Controls.Add(this.splitter);
		this.Controls.Add(this.toolGroup);
		this.Controls.Add(this.pMapView);
		this.Menu = this.mainMenu1;
		this.Name = "Form1";
		this.Text = "Refuse Robots Level Editor";
		this.SizeChanged += new System.EventHandler(this.Form1_SizeChanged);
		this.toolGroup.ResumeLayout(false);
		this.tileTabs.ResumeLayout(false);
		this.tAuto.ResumeLayout(false);
		this.tMan.ResumeLayout(false);
		this.tSpec.ResumeLayout(false);
		this.groupBox1.ResumeLayout(false);
		((System.ComponentModel.ISupportInitialize)(this.nOwner)).EndInit();
		this.ResumeLayout(false);

	}
	#endregion

	[STAThread]
	static void Main() 
	{
		Application.Run(new Form1());
	}

	private void pMapView_Paint(object sender, System.Windows.Forms.PaintEventArgs e)
	{
		if(map==null) return;
		
		int x = (int)getScrollX();
		int y = (int)getScrollY();
		int width  = (int)Math.Ceiling( (double)pMapView.Size.Width  / getTileSize() )+1;
		int height = (int)Math.Ceiling( (double)pMapView.Size.Height / getTileSize() )+1;
		
		redrawTiles(x, y, x+width, y+height);
	}

	public int getTileSize() {
		return (int)(tile.tileSize*scale);
	}
	
	private double getScrollX() {
		return -(double)pMapView.AutoScrollPosition.X / getTileSize();
	}
	private double getScrollY() {
		return -(double)pMapView.AutoScrollPosition.Y / getTileSize();
	}
	
	private int mouseToWorldX(System.Windows.Forms.MouseEventArgs e) {
		return (int)Math.Floor(e.X/getTileSize() + getScrollX() + 0.5);
	}
	private int mouseToWorldY(System.Windows.Forms.MouseEventArgs e) {
		return (int)Math.Floor(e.Y/getTileSize() + getScrollY() + 0.5);
	}
	
	private void updateTile(System.Windows.Forms.MouseEventArgs e)
	{
		int x = mouseToWorldX(e);
		int y = mouseToWorldY(e);
		int isAuto = tileTabs.SelectedIndex;
		if( isAuto==0 && lTileSet.SelectedIndices.Count!=1) return;
		if( isAuto==1 && lManualTS.SelectedIndices.Count!=1) return;

		if(!pressed) return;	
	
		if(x<0 || y<0 || x >= map.width || y >= map.height)
			return;

		tile t = isAuto==0 ? (tile)Map.getTileSet().getAbsTiles()[lTileSet.SelectedIndices[0]]
			            : (tile)Map.getTileSet().getManTiles()[lManualTS.SelectedIndices[0]];
		
		if(e.Button == MouseButtons.Right)
		{
			if(map.fill(x,y,t))
				pMapView.Refresh();
		}

		if(map.setTile(x,y, t, isAuto==0) != t)
			redrawTiles(x-1, y-1, x+1, y+1);
	}
	
	private void redrawTiles(int minX, int minY, int maxX, int maxY)
	{
		map.draw(minX, minY, maxX, maxY, scale, pMapView.CreateGraphics(),
			(int)(getScrollX()*getTileSize()), (int)(getScrollY()*getTileSize()));
	}

	private void button1_Click(object sender, System.EventArgs e)
	{
		DialogResult r = saveBox.ShowDialog();
		if(r==DialogResult.OK)
		{
			try {
				map.write(saveBox.FileName);
			} catch(Exception) {
				map.write();
			}
		}
	}

	private void bLoad_Click(object sender, System.EventArgs e)
	{
		string c = System.IO.Directory.GetCurrentDirectory();
		DialogResult r = openBox.ShowDialog();
		if(r==DialogResult.OK)
		{
			System.IO.Directory.SetCurrentDirectory(c);
			// Parser p = new Parser(openBox.FileName);
			map = new Map(openBox.FileName);
			Map.getTileSet().fillListView(lTileSet); 
			Map.getTileSet().fillListView(lManualTS,1);
			updateSize();
		}

		pMapView.Refresh();
	}
	
	private void Form1_SizeChanged(object sender, System.EventArgs e)
	{
		updateLayout();
	}

	private void bNew_Click(object sender, System.EventArgs e)
	{
		mapProp.nTileX.Enabled = true;
		mapProp.nTileY.Enabled = true;
		mapProp.tPath.Enabled = true;

		DialogResult r = mapProp.ShowDialog();
		if(r != DialogResult.OK) return;
		map = new Map(mapProp.tPath.Text,(int)mapProp.nTileX.Value,(int)mapProp.nTileY.Value);
		map.desc = mapProp.tDesc.Text;
		map.numPlayers = (int)mapProp.nNumPlayers.Value;

		mapProp.nTileX.Enabled = false;
		mapProp.nTileY.Enabled = false;
		mapProp.tPath.Enabled = false;
		nOwner.Maximum = map.numPlayers;

		Map.getTileSet().fillListView(lTileSet);   
		Map.getTileSet().fillListView(lManualTS,1);
		updateSize();
	    
		pMapView.Refresh();
	}
	
	private void updateSize()
	{
		int sizew = (int)Math.Floor(map.width  * getTileSize());
		int sizeh = (int)Math.Floor(map.height * getTileSize());
		pMapView.AutoScrollMinSize = new Size(sizew,sizeh);
	}

	private void pMapView_MouseDown(object sender, System.Windows.Forms.MouseEventArgs e)
	{
		if( tileTabs.SelectedIndex==2) 
		{
			int x = mouseToWorldX(e);
			int y = mouseToWorldY(e);
			if(radioButton3.Checked)
			{
				int owner=map.getTileOwner(x,y);
				if(owner!=-1)
						nOwner.Value = owner;
			}
			else if(radioButton2.Checked)
				map.addOilWell(x,y,(int)nOwner.Value);
			else
				map.addLandfill(x,y,(int)nOwner.Value);
			
			redrawTiles(x-2, y-2, x+2, y+2); 
			return;
		}
		
		pressed = true;

		//  if(m!=null)
		//     m.pushState();     
		updateTile(e);
	}
	private void pMapView_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
	{
		if(pressed)
			pMapView_MouseDown(sender, e);
	}
	private void pMapView_MouseUp(object sender, System.Windows.Forms.MouseEventArgs e)
	{
		pressed = false;
	}

	private void bUndo_Click(object sender, System.EventArgs e)
	{
		//  m.undoState();
		//  pMapView2.Refresh();
	}

	private void bRedo_Click(object sender, System.EventArgs e)
	{
		// m.redoState();
		// pMapView2.Refresh();
	}


	private void splitter_Moved(object sender, System.Windows.Forms.SplitterEventArgs e) {
		updateLayout();
	}
	
	private void updateLayout()
	{
		toolGroup.Height = ClientRectangle.Height;
		
		tileTabs.Location = new Point(splitter.Width, tileTabs.Location.Y);
		tileTabs.Size = new Size(toolGroup.Width - tileTabs.Location.X,
		                         ClientRectangle.Height - tileTabs.Location.Y);
	}

	private void pMapView_SizeChanged(object sender, System.EventArgs e) {
		Refresh();
	}

	private void menuMirrorHoriz_Click(object sender, System.EventArgs e) {
		if(map != null) {
			map.makeHorizSymmetric();
			updateSize();
			Refresh();
		}
	}
	private void menuMirrorVert_Click(object sender, System.EventArgs e) {
		if(map != null) {
			map.makeVertSymmetric();
			updateSize();
			Refresh();
		}
	}
	
	private void viewRefresh_Click(object sender, System.EventArgs e) {
		pMapView.Refresh();
	}

	private void menuExit_Click(object sender, System.EventArgs e) {
		Dispose();
	}

	private void viewZoomIn_Click(object sender, System.EventArgs e) {
		scale *= 2.0f;
		updateSize();
	}
	private void viewZoomOut_Click(object sender, System.EventArgs e) {
		scale /= 2.0f;
		updateSize();
	}

	private void menuItem1_Click(object sender, System.EventArgs e)
	{
		DialogResult r = mapProp.ShowDialog();

		if(r!=DialogResult.OK) return;

		map.desc = mapProp.tDesc.Text;
		map.numPlayers = (int)mapProp.nNumPlayers.Value;
		nOwner.Maximum = map.numPlayers;
	}

	private void nOwner_ValueChanged(object sender, System.EventArgs e)
	{
		if(radioButton3.Checked)
		{
			map.setSelectedOwner((int)nOwner.Value);
		}
	}

}

}
