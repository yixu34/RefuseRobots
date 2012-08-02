#include "main.hpp"

Image cursorImage("cursor.png");


ScreenCursor::ScreenCursor()
	: EventListener(100)
{
	pos = ICoord(screenWidth/2, screenHeight/2);
	scroll = ICoord(0,0);
}

void ScreenCursor::moveCursor(int x, int y, int relx, int rely)
{
#ifdef FULLSCREEN
	if(x == screenWidth/2 && y == screenHeight/2)
		return;
	if(relx==0 && rely==0)
		return;
	
	pos.x += relx;
	pos.y += rely;
	
	SDL_WarpMouse(screenWidth/2, screenHeight/2);
	
	if(pos.x < 0) {
		scroll.x += pos.x;
		pos.x = 0;
	}
	if(pos.y < 0) {
		scroll.y += pos.y;
		pos.y = 0;
	}
	if(pos.x >= screenWidth) {
		scroll.x += (pos.x-screenWidth+1);
		pos.x = screenWidth-1;
	}
	if(pos.y >= screenHeight) {
		scroll.y += (pos.y-screenHeight+1);
		pos.y = screenHeight-1;
	}
#else
	pos = ICoord(x, y);
#endif
}

void ScreenCursor::redraw()
{
	// Draw cursor
	glColor3f(1.0, 1.0, 1.0);
	drawImage(pos.x, pos.y, 64, 64, cursorImage);
}

bool ScreenCursor::isOpaque()
{
	return false;
}

ICoord ScreenCursor::getLocation()
{
	return pos;
}

ICoord ScreenCursor::getScroll()
{
	return scroll;
}

void ScreenCursor::clearScroll()
{
	scroll = ICoord(0,0);
}

ScreenCursor *cursor = NULL;

void initCursor()
{
	if(!cursor)
		cursor = new ScreenCursor();
}
