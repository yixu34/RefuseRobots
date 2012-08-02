using System;
using System.Collections;
using System.IO;
using System.Drawing;
using System.Security.Permissions;

namespace MapEditor
{
	public class Map
	{
		static tileset tiles;
		Stack undoStack = new Stack();
		Stack redoStack = new Stack();
		Parser parser;
    
		static tile_t [,] m;
		static ArrayList landfills = new ArrayList();
		public int height,width;
	
		string tilesetName;
		string description;
		int nPlayers;
		Point selected;
		public int numPlayers
		{
			get { return nPlayers; }
			set { nPlayers = value; }
		}
		public string desc
		{
			get { return description; }
			set { description = value; }
		}

		public static System.Windows.Forms.Panel parent;
    
		class tile_t
		{
			tile tile_p;
			public tile myTile
			{
				get 
				{
					return tile_p;
				}
				set
				{
				
					if(landf!=null)
						removeLandfill(landf);
					tile_p = value;
				}
			}
			public bool blanked;
			public landfill landf;

		

			public tile_t(tile_t old) 
			{
				landf = old.landf;
				tile_p = old.myTile;
			}
			public tile_t(string type) 
			{
				tile_p = tiles.getTile(type);
				landf=null;
			}
			public tile_t(tile t) 
			{
				tile_p=t;
				landf=null;
			}

			public tile_t(tile t, landfill i)
			{
				tile_p = t;
				landf = i;
			}

			public Image getImage() { return myTile.i;    }
			public string getType() { return myTile.name; }
		}
	
		public bool isInBounds(int x, int y)
		{
			return x>=0 && y>=0 && x<width && y<height;
		}
		public tile setTile(int x, int y, tile t, bool checkBorder)
		{
			if(!isInBounds(x, y))
				return null;
		
			tile oldTile = m[x,y].myTile;
			m[x,y].myTile = t;

		
			if(!checkBorder)
				return oldTile;
		
			for(int yi=-1; yi<=1; yi++)
				for(int xi=-1; xi<=1; xi++)
				{
					if(xi!=0 || yi!=0)
						blankTile(x+xi, y+yi, -xi, -yi);
				}
		
			bool repeat = false;
		
			do 
			{
				repeat = false;
			
				// Because of certain corner cases, we have to visit tiles in a
				// particular order (horizontally-adjacent first, then diagonally-
				// adjacent).
				int[] xs = { -1, 0, 1,  0, -1, -1,  1, 1 };
				int[] ys = { 0,  1, 0, -1, -1,  1, -1, 1 };
			
				//for(int yi=-1; yi<=1; yi++)
				//for(int xi=-1; xi<=1; xi++)
				for(int ii=0; ii<8; ii++)
				{
					int xi = x+xs[ii], yi = y+ys[ii];
				
					if(isInBounds(xi, yi) && m[xi,yi].blanked)
					{
						if(matchTile(xi, yi)) 
						{
							repeat = true;
							unblankTile(xi, yi);
						}
					}
				}
			} while(repeat);
		
			for(int yi=-1; yi<=1; yi++)
				for(int xi=-1; xi<=1; xi++)
					unblankTile(x+xi, y+yi);
		
			return oldTile;
		}
		// Determine the tile type for the corner at the top-left of tile (x,y) by
		// looking at that corner's neighbors. Blanked tiles and tiles which don't
		// care what this corner is aren't counted. If none of the four surrounding
		// tiles produce a constraint, returns "*". If tiles disagree, they vote
		// (ties broken arbitrarily).
		protected string getCorner(int x, int y)
		{
			int numResults = 0;
			string[] results = new string[4];
		
			// Look at NW neighbor
			if(x>0 && y>0 && !m[x-1,y-1].blanked)
				results[numResults++] = m[x-1, y-1].myTile.neighborSE;
		
			// Look at NE neighbor
			if(x<width && y>0 && !m[x,y-1].blanked)
				results[numResults++] = m[x, y-1].myTile.neighborSW;
		
			// Look at SW neighbor
			if(x>0 && y<height && !m[x-1,y].blanked)
				results[numResults++] = m[x-1, y].myTile.neighborNE;
		
			// Look at SE neighbor
			if(x<width && y<height && !m[x,y].blanked)
				results[numResults++] = m[x, y].myTile.neighborNW;
		
			switch(numResults)
			{
				default:
				case 0: return "*";
				case 1: return results[0];
				case 2: return results[0];
				case 3:
					if(results[1]==results[0] || results[2]==results[0])
						return results[0];
					else
						return results[1];
				case 4:
					if(results[1]==results[0] || results[2]==results[0] || results[3]==results[0])
						return results[0];
					else if(results[1]==results[2] || results[1]==results[3])
						return results[1];
					else
						return results[2];
			}
		}
		protected bool matchCorners(string a, string b)
		{
			return a=="*" || b=="*" || a==b;
		}
		// If tile (x,y) has a conflict on some of its corners, blank it out.
		// Consider only conflicts with one particular tile, the location of which
		// is (x+dx, y+dy)
		protected void blankTile(int x, int y, int dx, int dy)
		{
			if(!isInBounds(x, y))
				return;
			tile t = m[x,y].myTile;
			tile neighbor = m[x+dx, y+dy].myTile;
		
			if( (dx>=0 && dy>=0 && !matchCorners(t.neighborSE, neighbor.neighborNW))
				|| (dx<=0 && dy>=0 && !matchCorners(t.neighborSW, neighbor.neighborNE))
				|| (dx>=0 && dy<=0 && !matchCorners(t.neighborNE, neighbor.neighborSW))
				|| (dx<=0 && dy<=0 && !matchCorners(t.neighborNW, neighbor.neighborSE)) )
			{
				m[x,y].blanked = true;
			} 
		}
		protected bool matchTile(int x, int y)
		{
			if(!isInBounds(x, y))
				return false;
		
			string constraintNW = getCorner(x,   y),
				constraintNE = getCorner(x+1, y),
				constraintSW = getCorner(x,   y+1),
				constraintSE = getCorner(x+1, y+1);
		
			object[] edges = tiles.getEdgeTiles().ToArray();
			object[] bases = tiles.getAbsTiles().ToArray();
		
			foreach(tile t in edges)
			{
				if( matchCorners(t.neighborNE, constraintNE)
					&& matchCorners(t.neighborNW, constraintNW)
					&& matchCorners(t.neighborSW, constraintSW)
					&& matchCorners(t.neighborSE, constraintSE) )
				{
					m[x,y].myTile = t;
					return true;
				}
			}
			foreach(tile t in bases)
			{
				if( matchCorners(t.neighborNE, constraintNE)
					&& matchCorners(t.neighborNW, constraintNW)
					&& matchCorners(t.neighborSW, constraintSW)
					&& matchCorners(t.neighborSE, constraintSE) )
				{
					m[x,y].myTile = t;
					return true;
				}
			}
			return false;
		}
		protected void unblankTile(int x, int y) 
		{
			if(!isInBounds(x, y))
				return;
			m[x,y].blanked = false;
		}
	
		public static tileset getTileSet()
		{
			return tiles;
		}
	
		public Map(string tilePath, int numX, int numY)
		{
			landfills = new ArrayList();
			selected = new Point(-1,-1);
			tilesetName = tilePath;
			width = numX;
			height = numY;
			tiles = new tileset(tilesetName);
			parser = new Parser();
			m = new tile_t[width,height];
		
			tile defaultTile = (tile)tiles.getAbsTiles()[0];

			for(int y=0;y<height;y++)
				for(int x=0;x<width;x++)
					m[x,y] = new tile_t(defaultTile);
		}

		public bool fill(int x, int y, tile t)
		{
			tile current = m[x,y].myTile;
			if(t==current) return false;
			int cx=x;
			int cy=y;
			Queue q = new Queue();
			q.Enqueue(new Point(x,y));
			m[x,y].myTile = t;
			while(q.Count!=0)
			{
				Point p = (Point)q.Dequeue();
				if(p.X+1<width && m[p.X+1,p.Y].myTile == current)
				{
	            
					m[p.X+1,p.Y].myTile = t;
					q.Enqueue(new Point(p.X+1,p.Y));
				}
				if(p.X-1>=0 && m[p.X-1,p.Y].myTile == current)
				{
					m[p.X-1,p.Y].myTile = t;
					q.Enqueue(new Point(p.X-1,p.Y));
				}
				if(p.Y+1<height && m[p.X,p.Y+1].myTile == current)
				{
					m[p.X,p.Y+1].myTile = t;
					q.Enqueue(new Point(p.X,p.Y+1));
				}
				if(p.Y-1>=0 && m[p.X,p.Y-1].myTile == current)
				{
					m[p.X,p.Y-1].myTile = t;
					q.Enqueue(new Point(p.X,p.Y-1));
				}
	        
			}
			return true;
		}
		public Map(string path)
		{
			parser = new Parser(path);

			ArrayList current;
			landfills = new ArrayList();
			selected = new Point(-1,-1);
			current = parser.getSection("ATTRIBUTES");
			foreach (string[] line in current)
			{
				if(line[0].Equals("tileset"))
				{
					tilesetName = line[1];
					tiles = new tileset(tilesetName);
				}
				else if(line[0].Equals("size"))
				{
					width = int.Parse(line[1]);
					height = int.Parse(line[2]);
				}
				else if(line[0].Equals("description"))
				{
					description = "";
					for(int i=1;i<line.Length;i++)
						description += line[i];
				}
			}
			m = new tile_t[width,height];
			current = parser.getSection("CONTENT");
			for(int x=0;x<width;x++)
			{
				for(int y=0;y<height;y++)
				{
					m[x,y] = new tile_t(((string[])current[y])[x]);
				}
			}

			current = parser.getSection("RESOURCES");
			foreach(string[] line in current)
			{
				if(line[0].Equals("landfill"))
					addLandfill(int.Parse(line[2])-1,int.Parse(line[3])-1,int.Parse(line[1]),int.Parse(line[4]),int.Parse(line[5]));
				else if(line[0].Equals("oil"))
					addOilWell(int.Parse(line[1])-1,int.Parse(line[2])-1,0,int.Parse(line[3]),int.Parse(line[4]));
			}

			current = parser.getSection("PLAYERS");
			foreach(string[] line in current)
			{
				if(line.Length > 1)
					numPlayers++;
			}
		}
    
		public void draw(int x,int y, int x2, int y2, float scale, Graphics g, int xOffset, int yOffset)
		{
			System.Drawing.Pen p = new Pen(Color.LightGreen,5);
	    
			int size =(int)Math.Floor(tile.tileSize*scale);
		
			for(int ii=x; ii<=x2; ii++)
				for(int jj=y; jj<=y2; jj++)
				{
					if(!isInBounds(ii, jj))
						continue;
			
					g.DrawImage(m[ii,jj].getImage(), ii*size-xOffset, jj*size-yOffset, size, size);
					if(m[ii,jj].blanked)
						g.DrawRectangle(p, ii*size-xOffset, jj*size-yOffset, size-3,size-3);
					if(ii==selected.X && jj==selected.Y)
						g.DrawRectangle(p, (ii-1)*size-xOffset, (jj-1)*size-yOffset, 2*size-3,2*size-3);
					//if(m[i,j].selected)
					//    g.DrawRectangle(p,(i-x)*size,(j-y)*size,size-3,size-3);
				}
		}
	
		public void resize(int sizeX, int sizeY)
		{
			tile_t[,] newmap = new tile_t[sizeX, sizeY];
			int minSizeX = Math.Min(sizeX, width),
				minSizeY = Math.Min(sizeY, height);
		
			for(int yi=0; yi<height; yi++)
				for(int xi=0; xi<width;  xi++)
				{
					if(yi<minSizeY && xi<minSizeX)
						newmap[xi,yi] = m[xi,yi];
					else
						newmap[xi,yi] = new tile_t((tile)tiles.getAbsTiles()[0]);
				}
			width = sizeX;
			height = sizeY;
			m = newmap;
		}
		public void makeHorizSymmetric()
		{
			int oldWidth = width;
			resize(width*2, height);
		
			for(int yi=0; yi<height; yi++)
				for(int xi=0; xi<oldWidth; xi++)
				{
					m[width-1-xi, yi] = new tile_t(tiles.mirrorHoriz(m[xi, yi].myTile));
				}
			ArrayList land = (ArrayList)landfills.Clone();
			foreach(landfill l in land)
			{
				if(l.GetType()==typeof(oilwell))
					addOilWell(width-1-l.center.X,l.center.Y-1,0);
				else
					addLandfill(width-1-l.center.X,l.center.Y-1,0);
			}
		}
		public void makeVertSymmetric()
		{
			int oldHeight = height;
			resize(width, height*2);
		
			for(int yi=0; yi<oldHeight; yi++)
				for(int xi=0; xi<width; xi++)
				{
					m[xi, height-1-yi] = new tile_t(tiles.mirrorVert(m[xi, yi].myTile));
				}
			ArrayList land = (ArrayList)landfills.Clone();
			foreach(landfill l in land)
			{
				if(l.GetType()==typeof(oilwell))
					addOilWell(l.center.X-1,height-1-l.center.Y,0);
				else
					addLandfill(l.center.X-1,height-1-l.center.Y,0);
			}
		}
	

		public void write()
		{
			TextWriter fw = new StringWriter();
			write(fw);
			System.Windows.Forms.MessageBox.Show("System Security Exception - Writing to string");
			textView t = new textView();
			t.setText(((StringWriter)fw).ToString());
			t.ShowDialog();
			fw.Close();
		}

		public void write(string filename)
		{
			TextWriter fw = new StreamWriter(filename);
			write(fw);
			fw.Close();
		}

		void write(TextWriter fw)
		{
			fw.WriteLine("[ATTRIBUTES]");
			fw.WriteLine("tileset "+tilesetName);
			fw.WriteLine("size "+width + " " + height);
			fw.WriteLine("description \""+description+"\"");

			fw.WriteLine("[PLAYERS]");
		
			for(int i=0;i<numPlayers;i++)
				fw.WriteLine("\"Player "+(i+1)+"\" 0 0");
			
		
			fw.WriteLine("[RESOURCES]");
			foreach(landfill l in landfills)
			{
				fw.WriteLine(l.write());
			}

			fw.WriteLine("[CONTENT]");
			for(int y=0;y<height;y++)
			{
				for(int x=0;x<width;x++)
					fw.Write(m[x,y].getType()+" ");
				fw.WriteLine("");
			}
	                        
			//fw.Close();
		
		}

		tile_t[,] currentState()
		{
			tile_t[,] newM = new tile_t[width,height];
			for(int x = 0;x<width;x++)
				for(int y = 0;y<height;y++)
					newM[x,y] = new tile_t(m[x,y]);
			return newM;
		}
	
		public void pushState()
		{
			/*if(undoStack.Count>0)
			{
				tile_t[,] a =(tile_t[,]) undoStack.Pop();
				if(equals(a,currentState()))
				{
					System.Console.WriteLine("NOP");
					undoStack.Push(a);
					return;
				}
			}*/
			undoStack.Push(currentState());
			System.Console.WriteLine("Pushing State"+m[0,0].myTile.name);
			redoStack.Clear();
		}
	
		public void undoState()
		{
			try
			{
				if(undoStack.Count==0) return;
				redoStack.Push(currentState());
				m =(tile_t[,]) undoStack.Pop();
				System.Console.WriteLine("Popping State "+m[0,0].myTile.name);
			}
			catch(InvalidOperationException e)
			{
				System.Console.WriteLine(e.ToString());
			}
		}

		public void redoState()
		{
			try
			{
				if(redoStack.Count==0)return;
				undoStack.Push(currentState());
				m =(tile_t[,]) redoStack.Pop();
				System.Console.WriteLine("Popping redo State "+m[0,0].myTile.name);
			}
			catch(InvalidOperationException e)
			{
				System.Console.WriteLine(e.ToString());
			}
		}

		bool equals(tile_t[,] t1, tile_t[,] t2)
		{
			try
			{
				for(int x=0;x<width;x++)
					for(int y=0;y<height;y++)
						if(!t1[x,y].myTile.Equals(t2[x,y].myTile))
							return false;
			}
			catch(Exception)
			{
				return false;
			}
			return true;
		}

		public void addLandfill(int x, int y, int owner, int width, int height)
		{
			if(!isInBounds(x+1,y+1)) return;
			landfill l = new landfill(owner, x+1, y+1, width, height);
		
			landfills.Add(l);

			m[x,y].myTile = ((tile)tiles.getTile("lfNW"));
			m[x+1,y].myTile = ((tile)tiles.getTile("lfNE"));
			m[x,y+1].myTile = ((tile)tiles.getTile("lfSW"));
			m[x+1,y+1].myTile = ((tile)tiles.getTile("lfSE"));

			m[x,y].landf = l;
			m[x+1,y].landf = l;
			m[x,y+1].landf = l;
			m[x+1,y+1].landf = l;

		
			selected.X = x+1;
			selected.Y = y+1;
		}

		public int getTileOwner(int x, int y)
		{
			
			if(!isInBounds(x,y) || m[x,y].landf==null) return -1;
			selected.X =m[x,y].landf.center.X;
			selected.Y = m[x,y].landf.center.Y;
			return m[x,y].landf.owner;
		}

		public void addLandfill(int x, int y, int owner)
		{
			addLandfill(x,y,owner,4,4);
		}

		static void removeLandfill(landfill l)
		{
			landfills.Remove(l);
			Rectangle r = l.area();
			for(int x=r.Left;x<r.Right;x++)
				for(int y=r.Top;y<r.Bottom;y++)
				{
				
					m[x,y].landf=null;
					m[x,y].myTile = tiles.getTile("dir");
				}
		}
		public void addOilWell(int x, int y, int owner)
		{
			addOilWell(x,y,owner,4,4);
		}
		public void addOilWell(int x, int y, int owner,int width, int height)
		{
			if(!isInBounds(x+1,y+1)) return;
			oilwell l = new oilwell(x+1, y+1, width, height);
		
			landfills.Add(l);

			m[x,y].myTile = ((tile)tiles.getTile("owNW"));
			m[x+1,y].myTile = ((tile)tiles.getTile("owNE"));
			m[x,y+1].myTile = ((tile)tiles.getTile("owSW"));
			m[x+1,y+1].myTile = ((tile)tiles.getTile("owSE"));

			m[x,y].landf = l;
			m[x+1,y].landf = l;
			m[x,y+1].landf = l;
			m[x+1,y+1].landf = l;

			selected.X = x+1;
			selected.Y = y+1;
		}
		public void setSelectedOwner(int newOwn)
		{
			if(selected.X>=0 && selected.Y>=0)
			m[selected.X,selected.Y].landf.owner = newOwn;
		}

	}
}