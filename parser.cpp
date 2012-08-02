#include <string>
#include <map>
#include <vector>
#include <stdexcept>
#include "main.hpp"



using Parser::Line;
using Parser::Section;

const Section Parser::emptySection;

const Section *Parser::getSection(const char *sectionName) const
{
	std::string sectionNameStr = sectionName;
	for(unsigned jj=0; jj<sectionNameStr.size(); jj++)
		sectionNameStr[jj] = tolower(sectionNameStr[jj]);
	Contents::const_iterator ii = contents.find(sectionNameStr);
	
	if(ii==contents.end())
		return &emptySection;
	else
		return ii->second;
}


Parser::Parser(const char *filename)
{
	FILE *fin = fopen(filename, "r");
	if(!fin)
		throw std::runtime_error(retprintf("File not found: %s", filename));
	
	std::string input;
	preprocessFile(fin, input);
	fclose(fin);
	
	parseString(input);
}

// Postconditions of str:
//   contains the whole of what was in file fin
//   no comments and no blank lines
//   no line ends in '\'
//   ends in a newline
void Parser::preprocessFile(FILE *fin, std::string &str)
{
	char in, next;
	bool firstLine = true;
	
	while(!feof(fin))
	{
		in = getc(fin);
		
		switch(in)
		{
		case '#':
			// Skip the rest of this line
			do {
				in = getc(fin);
			} while(in != EOF && in!='\n');
			break;
			
		case '\"':
			// Treat the contents of this quoted string literally
			// (including #, which would otherwise be comment - but still
			// handle escapes, specifically \" and \\n)
			str += in;
			do {
				in = getc(fin);
				if(in=='\\') {
					next = getc(fin);
					if(next=='\"') {
						str += '\\';
						str += '\"';
					}
					else if(next=='\n')
						str += ' ';
					else {
						str += '\\';
						str += next;
					}
				}
				else
					str += in;
			} while(in != '\"');
			
			break;
			
		case '\\':
			// Convert (\,\n) to ' '
			next = getc(fin);
			if(next=='\n')
				str += ' ';
			else {
				str += in;
				str += next;
			}
			break;
			
		case '\n':
			// Put a newline here, unless this is a leading blank line
			if(!firstLine || str!="")
				str += '\n';
			if(firstLine)
				firstLine = false;
			
			// Eliminate all trailing whitespace (particularly, consecutive
			// newlines aka blank lines).
			do {
				next = getc(fin);
			} while(next != EOF && isspace(next));
			if(next != EOF)
				ungetc(next, fin);
			break;
			
		default:
			str += in;
			break;
		}
	}
	if(str[str.size()-1] != '\n')
		str += '\n';
}


void Parser::parseString(std::string &str)
{
	Section *currentSection = NULL;
	std::string sectionName("");
	
	std::string line;
	
	for(unsigned ii=0; ii<str.size(); ii++)
	{
		if(str[ii] == '\n') {
			if(line.size()>2 && line[0]=='[') {
				if(currentSection)
					contents[sectionName] = currentSection;
				
				currentSection = NEW Section();
				sectionName = line.substr(1, line.size()-2);
				for(unsigned jj=0; jj<sectionName.size(); jj++)
					sectionName[jj] = tolower(sectionName[jj]);
			} else {
				currentSection->push_back( parseLine(line) );
			}
			line = "";
		} else {
			line += str[ii];
		}
	}
	if(currentSection && currentSection->size())
		contents[sectionName] = currentSection;
}


Line Parser::parseLine(const std::string &str)
{
	Line ret;
	std::string field;
	bool quoted = false;
	
	for(unsigned ii=0; ii<str.size(); ii++)
	{
		switch(str[ii])
		{
			case ' ': case '\t':
				if(quoted)
					field += str[ii];
				else {
					if(field.size() > 0) {
						ret.push_back(field);
						field = "";
					}
				}
				break;
				
			case '\\':
				ii++;
				field += parseEscape(str, ii);
				break;
				
			case '\"':
				quoted = !quoted;
				break;
				
			default:
				field += str[ii];
				break;
		}
	}
	if(field.size() > 0)
		ret.push_back(field);
	return ret;
}

char Parser::parseEscape(const std::string &str, unsigned &ref_pos)
{
	const char *c_str = str.c_str();
	const char *p = c_str + ref_pos;
	
	char ret = '?';
	
	switch(*p) {
		// Treat backslash on unrecognized characters as a no-op.
		default:   p++; ret = *p;   break;
		
		// These match up with C escape sequences
		case 'a':  p++; ret = '\a'; break;
		case 'b':  p++; ret = '\b'; break;
		case 'f':  p++; ret = '\f'; break;
		case 'n':  p++; ret = '\n'; break;
		case 'r':  p++; ret = '\r'; break;
		case 't':  p++; ret = '\t'; break;
		case 'v':  p++; ret = '\v'; break;
		case '\\': p++; ret = '\\'; break;
		case '?':  p++; ret = '\?'; break;
		case '\'': p++; ret = '\''; break;
		case '"':  p++; ret = '\"'; break;
		case '\n': p++; ret = ' ';  break;
		case '{':  p++; ret = '{';  break;
		case '}':  p++; ret = '}';  break;
		case '$':  p++; ret = '$';  break;
		
		// Hex numbers of the form \xhh.
		case 'x':
			p++; // Skip over the x
			
			// First digit
			if(*p >= '0' && *p <= '9')      // 0-9
				ret = *p - '0';
			else if(*p >= 'a' && *p <= 'f') // a-f
				ret += *p - 'a';
			else if(*p >= 'A' && *p <= 'F') // A-F
				ret += *p - 'A';
			else
				break; // If it wasn't [0-9a-fA-F], stop here (don't advance).
			p++;
			
			// Second digit
			if(*p >= '0' && *p <= '9')
				ret = (ret<<4) + (*p-'0');
			else if(*p >= 'a' && *p <= 'f')
				ret += (ret<<4) + (*p-'a');
			else if(*p >= 'A' && *p <= 'F')
				ret += (ret<<4) + (*p-'A');
			else
				break;
			p++;
			break;
		
		// Octal numbers of the form \x0, \x00, or \x000.
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
			// First digit
			ret = *p - '0';
			p++;
			
			// Second digit
			if(*p < '0' || *p > '7')
				break;
			ret = (ret<<3) + *p - '0';
			p++;
			
			// Third digit
			if(*p < '0' || *p > '7')
				break;
			ret = (ret<<3) + *p - '0';
			p++;
			
			break;
	}
	
	ref_pos = p - c_str - 1;
	return ret;
}


