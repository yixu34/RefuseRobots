#include "main.hpp"

TextInput::TextInput(float x, float y,TextParams fontToUse, bool startEnabled)
	: EventListener(3),
	  x(x), y(y), cursorPos(0), str(""), result(0),fontName(fontToUse)
{
	if (!startEnabled)
		disableEvents();
}

bool TextInput::keyDown(SDL_keysym sym)
{
	std::string beforeCursor = str.substr(0, cursorPos),
	            afterCursor  = str.substr(cursorPos, str.size()-cursorPos);
	switch(sym.sym)
	{
		case SDLK_ESCAPE:
			disableEvents();
			result = -1;
			break;
		case SDLK_RETURN:
			disableEvents();
			result = 1;
			break;
		case SDLK_BACKSPACE:
			if(cursorPos > 0) {
				str = beforeCursor.substr(0, beforeCursor.size()-1) + afterCursor;
				cursorPos--;
			}
			break;
		default:
			if(sym.unicode > 15 && sym.unicode < 128) {
				str = beforeCursor;
				str += sym.unicode;
				str += afterCursor;
				cursorPos++;
			} else
				return false;
	}
	return true;
}

bool TextInput::keyUp(SDL_keysym sym)
{
	return false;
}

void TextInput::redraw()
{
	int w, h;
	TTF_SizeText(fontName.font, str.c_str(), &w, &h);
	float cursorX = x+w;
	float cursorWidth = 3;
	
	glColor3ub(255, 255, 255);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_QUADS);
		glVertex2f(cursorX,             y);
		glVertex2f(cursorX+cursorWidth, y);
		glVertex2f(cursorX+cursorWidth, y+h);
		glVertex2f(cursorX,             y+h);
	glEnd();
	glColor3ub(0, 0, 0);
	glLineWidth(1.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(cursorX,             y);
		glVertex2f(cursorX+cursorWidth, y);
		glVertex2f(cursorX+cursorWidth, y+h);
		glVertex2f(cursorX,             y+h);
	glEnd();
	
	screenPrintf(x, y, fontName, "%s", str.c_str());
}

bool TextInput::isOpaque()
{
	return false;
}

std::string TextInput::getString()
{
	return str;
}

int TextInput::getResult()
{
	return result;
}

void TextInput::reset()
{
	str       = "";
	result    = 0;
	cursorPos = 0;
	enableEvents();
}
