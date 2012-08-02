using System;
using System.Collections;
using System.IO;
namespace MapEditor
{
	public class Parser
	{
		Hashtable contents = new Hashtable();
		static string line;
		
		public class Section
		{
			ArrayList Lines = new ArrayList();

			public ArrayList getLines()
			{
				return Lines;
			}
			
			public Section(StreamReader sr)
			{
				string[] lineSplit;
				line = sr.ReadLine();
				if(line==null)return;
				while(!line.StartsWith("[")) 
				{
					if(!line.StartsWith("#"))
					{
						lineSplit = tileset.regex.Split(line);//(line.Split(" ".ToCharArray()));
						Lines.Add(lineSplit);
					}
					line = sr.ReadLine();
					if(line==null)return;
				}
			}
		}
		
		public ArrayList getSection(string key)
		{
			Section s = (Section)contents[key];
			if(s!=null)
				return s.getLines();
			else return new ArrayList();
		}

		public Parser()
		{
		}

		public Parser(string path)
		{
			StreamReader sr = new StreamReader(path);
			line = sr.ReadLine();
	        
			while(line!=null)
			{
				while(!line.StartsWith("[")) 
				{
					line = sr.ReadLine();
					if(line==null) {sr.Close();return;}
				}
				line = line.Substring(1,line.Length-2);
				contents.Add(line,new Section(sr));
				System.Console.WriteLine("Created: "+line);
			}
			sr.Close();
		}
	}
}