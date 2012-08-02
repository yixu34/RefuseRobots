#ifndef MENUITEMS_HPP
#define MENUITEMS_HPP

#include <SDL.h>
#include "text.hpp"
#include "util.hpp"
using namespace std;

struct menuItem
{
	char* displayName;
	int leftX,  rightX, upperY,  lowerY;
	TextParams* fontName;
	menuItem(char* name, int lX, int rX, int uY, int lY, TextParams* font)
	{
		displayName = name;
		leftX = lX;
		rightX = rX;
		upperY =  uY;
		lowerY = lY;
		fontName = font;
	}
};
class MenuItems
{
	public:
		MenuItems();
		void add(char* name, TextParams* font, bool center, float seperation);
		void add(char* name, bool center, float seperation);
		void add(char* name, bool center, int x, int y);
		void displayButtons();
		int processClick(int x, int y);
	protected:
		vector<menuItem *> menuEntries;
		int currentY;
};

#endif