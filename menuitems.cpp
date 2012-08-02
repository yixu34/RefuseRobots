#include "menuitems.hpp"
#include "main.hpp"

MenuItems::MenuItems()
{
	currentY = 0;
}
void MenuItems::add(char* name, TextParams* fontName, bool center, float seperation)
{
	int leftX,  rightX;
	int upperY,  lowerY;
	int tempW, tempH;
	TTF_SizeText(fontName->font, name, &tempW, &tempH);

	if(center)
		leftX = (float)(screenWidth - tempW)/2.0f;
	else
		leftX = 0;

	upperY = currentY + seperation;
	lowerY = upperY + tempH;
	rightX = leftX + tempW;
	currentY = lowerY;
	menuEntries.push_back(new menuItem(name, leftX,  rightX, upperY,  lowerY, fontName));
}
void MenuItems::add(char* name, bool center, float seperation)
{
	add(name, &fontMiddle, center, seperation);
}
void MenuItems::add(char* name, bool center, int x, int y)
{
	int leftX,  rightX;
	int upperY,  lowerY;
	int tempW, tempH;
	TTF_SizeText(fontMiddle.font, name, &tempW, &tempH);

	if(center)
		leftX = (float)(screenWidth - tempW)/2.0f;
	else
		leftX = x;

	rightX = leftX + tempW;
	upperY = y;
	lowerY = upperY + tempH;
	menuEntries.push_back(new menuItem(name, leftX,  rightX, upperY,  lowerY, &fontMiddle));
}
void MenuItems::displayButtons()
{
	for(unsigned ii = 0; ii < menuEntries.size(); ii++)
	{
		drawText(menuEntries[ii]->leftX,menuEntries[ii]->upperY, *menuEntries[ii]->fontName, menuEntries[ii]->displayName);
	}
}
int MenuItems::processClick(int x,int y)
{
	for(unsigned ii = 0; ii < menuEntries.size(); ii++)
	{
		if(x >= menuEntries[ii]->leftX && x <= menuEntries[ii]->rightX
			&& y >= menuEntries[ii]->upperY && y <= menuEntries[ii]->lowerY)
				return ii;
	}
	return -1;
}
