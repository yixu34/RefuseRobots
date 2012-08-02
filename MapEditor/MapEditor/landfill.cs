using System;
using System.Drawing;

namespace MapEditor
{
	/// <summary>
	/// Summary description for landfill.
	/// </summary>
	/// 
	
	public class landfill
	{
		public int owner;
		public Point center;
		protected Size size;

		public landfill(int own, int x, int y, int sizeX, int sizeY)
		{
			owner = own;
			center = new Point(x,y);
			size = new Size(sizeX,sizeY);
		}

		public virtual string write()
		{
			return "landfill "+owner+" "+center.X+" "+center.Y+" "+size.Width+" "+size.Height;
		}

		public Rectangle area()
		{	
			Rectangle r = new Rectangle(center.X-1,center.Y-1,size.Width/2,size.Height/2);
			return r;
		}

	}
}