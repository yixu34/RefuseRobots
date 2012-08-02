using System;
using System.Drawing;

namespace MapEditor
{
	/// <summary>
	/// Summary description for tile.
	/// </summary>
	public class tile
	{
		public string path;
		public bool passable;
		public string name;

		public Image i;

		public string neighborNW, neighborNE, neighborSW, neighborSE;
		
		public const int tileSize = 48;
		
		public tile(string pName, string p, bool pass, string nw, string ne, string sw, string se)
		{
			name = pName;
			passable = pass;
			path = "images/"+p;
			i = new Bitmap(path);
			neighborNW = nw;
			neighborNE = ne;
			neighborSW = sw;
			neighborSE = se;
		}
	}
}
