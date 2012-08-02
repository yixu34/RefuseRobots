
using System;

using System.Drawing;
using System.IO;
using System.Collections;
using System.Windows.Forms;
namespace MapEditor
{
	public class tileset
	{
		ArrayList absTiles = new ArrayList();
		ArrayList edgeTiles = new ArrayList();
        ArrayList specialTiles = new ArrayList();

		public static System.Text.RegularExpressions.Regex regex = new System.Text.RegularExpressions.Regex("\\s+");
		
		public ArrayList getAbsTiles()
		{
			return absTiles;
		}
		
		public ArrayList getEdgeTiles()
		{
			return edgeTiles;
		}
		
		public ArrayList getManTiles()
		{
			ArrayList a = new ArrayList();
			a.AddRange(absTiles);
			a.AddRange(edgeTiles);
			return a;
		}
		
		public void fillListView(ListView lview)
		{
			fillListView(lview,0);
		}
		
		public void fillListView(ListView lview, int type)
		{
			ListView.ListViewItemCollection lv = lview.Items;
			lv.Clear();
			lview.LargeImageList = new ImageList();
			lview.LargeImageList.ImageSize = new Size(64,64);
			lview.LargeImageList.ColorDepth = System.Windows.Forms.ColorDepth.Depth32Bit;
			int i=0;
			ArrayList a = new ArrayList();
			if(type==0)
				a=(absTiles);
			else if(type==1)
			{
				a= getManTiles();
			}
            else if(type==2)
                a=specialTiles;
            foreach(tile t in a)
            {
                lv.Add(new ListViewItem(t.name,i++));
                lview.LargeImageList.Images.Add(t.i);
            }
        }
		
		public tileset(string filename)
		{
			StreamReader sr = new StreamReader(filename);
			ArrayList curTiles=absTiles;
			string line;
			
			do
			{
				line = sr.ReadLine();
				if(line==null)
					break;
				
				else if(line.StartsWith("[ABSOLUTE]"))
					curTiles = absTiles;
				else if(line.StartsWith("[EDGE]"))
					curTiles = edgeTiles;
				else if(line.StartsWith("[")) // Any other section is not-for-me
					curTiles = null;
				else if(line.Equals("") || line.StartsWith("#"))
					continue;
				else
				{
					
					string[] parts = regex.Split(line);
					
					if(curTiles==null)
						continue;
					
					tile newTile ;
					newTile = new tile(parts[0], parts[1], parts[2].Equals("1"), parts[3], parts[4], parts[5], parts[6]);
					
					curTiles.Add(newTile);
				}
			}
			while(line != null);
			
			sr.Close();

		}

		public tile getTile(string type)
		{
			foreach(tile t in absTiles)
				if(t.name.Equals(type))
					return t;
			foreach(tile t in edgeTiles)
				if(t.name.Equals(type))
					return t;
			System.Console.WriteLine("TILE does not exist");
			return null;
		}
		
		public tile mirrorHoriz(tile t)
		{
			string str = t.name;
			
			// Try to split this string into a type and direction suffix
			int ii = str.Length-1;
			while(ii>0 && str[ii]==str.ToUpper()[ii])
				ii--;
			if(ii+1 < str.Length) ii++;
			string start = str.Substring(0, ii),
			       suffix = str.Substring(ii, str.Length-ii);
			
			switch(suffix)
			{
				case "E" : suffix="W";  break;
				case "W" : suffix="E";  break;
				case "NE": suffix="NW"; break;
				case "NW": suffix="NE"; break;
				case "SE": suffix="SW"; break;
				case "SW": suffix="SE"; break;
			}
			
			tile ret = getTile(start+suffix);
			if(ret != null) return ret;
			else return t;
		}
		public tile mirrorVert(tile t)
		{
			string str = t.name;
			
			// Try to split this string into a type and direction suffix
			int ii = str.Length-1;
			while(ii>0 && str[ii]==str.ToUpper()[ii])
				ii--;
			if(ii+1 < str.Length) ii++;
			string start = str.Substring(0, ii),
			       suffix = str.Substring(ii, str.Length-ii);
			
			switch(suffix)
			{
				case "N" : suffix="S";  break;
				case "S" : suffix="N";  break;
				case "NE": suffix="SE"; break;
				case "NW": suffix="SW"; break;
				case "SE": suffix="NE"; break;
				case "SW": suffix="NW"; break;
			}
			
			tile ret = getTile(start+suffix);
			if(ret != null) return ret;
			else return t;
		}
	}
}
