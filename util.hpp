#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include <map>
#include "event.hpp"

class Model;

#ifdef NDEBUG
#	define MYASSERT(exp) do {} while(0)
#	define EXIT_ASSERT do {} while(0)
#else
#	define MYASSERT(exp) do { if(!(exp)) { logger.stop(); assert(0); } } while(0)
#	define EXIT_ASSERT do { logger.stop(); assert(0); } while(0)
#endif

std::string retprintf(const char *fmt, ...);
float angleDifference(float a, float b);
float angleTowards(float angle, float dest, float dist);
float angleHeading(float sourceX, float sourceY, float destX, float destY);
float randFloat(float min, float max);

std::string formatChatText(
	const std::string &playerName, 
	const std::string &text);

void updateTime();
void resetTime();
double getTime();
double getDt();
double getFramerate();

template<class T>
struct Coord
{
	Coord() : x(0), y(0) {}
	Coord(T x, T y) : x(x), y(y) {}
	inline bool operator==(const Coord &o) const { return x==o.x && y==o.y; }
	inline bool operator<(const Coord &o) const { return y<o.y || (y==o.y && x<o.x); }
	T x, y;
};
typedef Coord<int> ICoord;
typedef Coord<float> FCoord;
typedef Coord<double> DCoord;

int distanceFrom(Model *model, ICoord source, ICoord dest);

struct Color
{
	Color() : r(0),g(0),b(0),a(0) {}
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) : r(r),g(g),b(b),a(a) {}
	unsigned char r,g,b,a;
};


class Parser
{
public:
	typedef std::vector<std::string> Line;
	typedef std::vector<Line> Section;
	typedef std::map<std::string, Section*> Contents;
	
	Parser(const char *filename);
	const Section *getSection(const char *sectionName) const;
	
private:
	Contents contents;
	static const Section emptySection;
	
	void preprocessFile(FILE *fin, std::string &str);
	void parseString(std::string &str);
	Line parseLine(const std::string &str);
	char parseEscape(const std::string &str, unsigned &pos);
};


class ScreenCursor :public EventListener
{
public:
	ScreenCursor();
	
	void moveCursor(int x, int y, int relx, int rely);
	void redraw();
	bool isOpaque();
	ICoord getLocation();
	ICoord getScroll();
	void clearScroll();
	
protected:
	ICoord pos;
	ICoord scroll;
};
extern ScreenCursor *cursor;
void initCursor();

void mapRect(Model *model, ICoord center, ICoord size, int &left, int &top, int &right, int &bottom);


struct MapDesc {
	std::string toString() { return retprintf("%s (%i players)", filename.c_str(), maxPlayers); }
	
	std::string filename;
	int maxPlayers;
};



#endif
