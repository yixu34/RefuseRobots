using System;
using System.Drawing;

namespace MapEditor
{
	/// <summary>
	/// Summary description for oilwell.
	/// </summary>
	public class oilwell :landfill
	{
		public oilwell( int x, int y, int sizeX, int sizeY)
		:base(0,x,y,sizeX,sizeY)
		{
		
		}

		public override string write()
		{
			return "oil "+" "+center.X+" "+center.Y+" "+size.Width+" "+size.Height;
		}
	}
}
