#include "widget.hpp"
#include "widgetaction.hpp"
#include "main.hpp"

#include <cassert>


TextBox::TextBox(float newX, float newY, int w, 
	const std::string &initialText, 
	const TextParams &params, 
	int cursorW)
	: x(newX), y(newY), width(w), text(initialText), textParams(&params), cursorWidth(cursorW)
{
	cursorPos = text.size();

	// Infer the height from the text parameters.
	int dummyWidth, dummyHeight;
	TTF_SizeText(textParams->font, text.c_str(), &dummyWidth, &dummyHeight);
	height = dummyHeight;

	isHighlighted = false;
}

TextBox::~TextBox()
{
}

bool TextBox::containsPoint(float clickX, float clickY)
{
	float xRight  = x + width;
	float yBottom = y + height;

	return (clickX >= x && clickX <= xRight &&
		    clickY >= y && clickY <= yBottom);
}

void TextBox::redrawSelf()
{
	if (!keyFocus)
		isHighlighted = false;
	
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glColor4ub(30, 30, 30, 100);
	glBegin(GL_QUADS);
		glVertex2f(x,       y);
		glVertex2f(x+width, y);
		glVertex2f(x+width, y+height);
		glVertex2f(x,       y+height);
	glEnd();

	if (isHighlighted)
		drawHighlight();

	drawOutline();

	screenPrintf(x, y, *textParams, text.c_str());
	if (keyFocus)
		drawCursor();

}

void TextBox::drawOutline()
{
	glColor3ub(255, 255, 255);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x,       y       );
		glVertex2f(x,       y+height);
		glVertex2f(x+width, y+height);
		glVertex2f(x+width, y       );
	glEnd();
}

void TextBox::drawHighlight()
{
	int tempWidth;
	int tempHeight;
	TTF_SizeText(textParams->font, text.c_str(), &tempWidth, &tempHeight);
	float boxWidth  = tempWidth;
	float boxHeight = tempHeight;
	glColor3ub(80, 80, 80);
	glBegin(GL_QUADS);
		glVertex2f(x,          y          );
		glVertex2f(x,          y+boxHeight);
		glVertex2f(x+boxWidth, y+boxHeight);
		glVertex2f(x+boxWidth, y          );
	glEnd();
}

void TextBox::drawCursor()
{
	int w, h;
	TTF_SizeText(textParams->font, text.substr(0, cursorPos).c_str(), &w, &h);
	float cursorX = x+w-2;
	
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
}

bool TextBox::keyDownSelf(SDL_keysym sym)
{
	std::string beforeCursor = text.substr(0, cursorPos),
	            afterCursor  = text.substr(cursorPos, text.size()-cursorPos);

	switch(sym.sym)
	{
		case SDLK_ESCAPE:
			reset();
			break;
		case SDLK_RETURN:
			activate();
			break;
		case SDLK_BACKSPACE:
			if(cursorPos > 0) {
				if (isHighlighted)
					reset();
				else
				{
					text = beforeCursor.substr(0, beforeCursor.size()-1) + afterCursor;
					cursorPos--;
				}
			}
			break;
		case SDLK_LEFT:
		case SDLK_KP4:
			if (isHighlighted)
			{
				isHighlighted = false;
				cursorPos = 0;
			}
			else if (cursorPos > 0)
				cursorPos--;
			break;

		case SDLK_RIGHT:
		case SDLK_KP6:
			if (isHighlighted)
			{
				isHighlighted = false;
				cursorPos = text.length();
			}
			else if (cursorPos < text.length())
				cursorPos++;
			
			break;

		default:
			if(sym.unicode > 15 && sym.unicode < 128) {
				if (isHighlighted)
				{
					reset();
					text = sym.unicode;
					cursorPos = 1;
				}
				else
				{
					text = beforeCursor;
					text += sym.unicode;
					text += afterCursor;
					cursorPos++;
				}
			} else
				return false;
	}

	return true;
}

void TextBox::reset()
{
	text          = "";
	cursorPos     = 0;
	isHighlighted = false;
}

bool TextBox::mouseDownSelf(float clickX, float clickY, int button, int mod)
{
	isHighlighted = !isHighlighted;

	// Just unhighlighted the box; place the cursor near where you clicked.
	// FIXME:  The cursor doesn't move to exactly where you click...
	if (!isHighlighted)
	{
		float xDist        = clickX - x;
		int numCursorSlots = xDist / cursorWidth;

		// Sanity check.
		if (numCursorSlots < 0)
			cursorPos = 0;
		else if (numCursorSlots > text.length())
			cursorPos = text.length();
		else
			cursorPos = numCursorSlots;
	}

	return true;
}

