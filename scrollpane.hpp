#ifndef SCROLLPANE_HPP
#define SCROLLPANE_HPP

#include "main.hpp"

const int scrollPaneTopMargin = 0,
          scrollPaneLeftMargin = 5;
const int scrollPaneScrollbarWidth = 20;
const int scrollButtonHeight = 20;

template<class T>
class ScrollPane : public Widget
{
public:
	ScrollPane(int x, int y, int width, int height, const std::vector<T> &elements, const TextParams *font=&fontDefault);
	bool mouseDownSelf(float x, float y, int button, int mod);
	bool containsPoint(float x, float y);
	
	void setElements(const std::vector<T> &e);
	void clearSelection();
	
	bool haveSelection();
	int getSelectedIndex();
	T getSelectedElement();
	
protected:
	void redrawSelf();
	bool keyDownSelf(SDL_keysym sym);
	
	bool haveScrollbar();
	
private:
	void updateList();
	
	std::vector<T> elements;
	int x, y, width, height;
	int scrollPos, maxScroll;
	const TextParams *font;
	int lineHeight;
	int lines;
	int selection;
	
	double lastClickTime;
	int lastClickIndex;
};


template<class T>
ScrollPane<T>::ScrollPane(int x, int y, int width, int height, const std::vector<T> &e, const TextParams *font)
	: x(x), y(y), width(width), height(height), scrollPos(0), font(font)
{
	selection = -1;
	TTF_SizeText(font->font, "Test", NULL, &lineHeight);
	lines = height/lineHeight;
	setElements(e);
	lastClickTime = 0;
	lastClickIndex = -1;
}

template<class T>
void ScrollPane<T>::updateList()
{
	maxScroll = elements.size() - height/lineHeight;
}

template<class T>
void ScrollPane<T>::setElements(const std::vector<T> &e)
{
	elements.clear();
	if(e.size())
		elements.insert(elements.begin(), e.begin(), e.end());
	updateList();
}

template<class T>
void ScrollPane<T>::clearSelection()
{
	selection = -1;
}

template<class T>
void ScrollPane<T>::redrawSelf()
{
	int scrollbarX = haveScrollbar() ? (x+width-scrollPaneScrollbarWidth) : (x+width);
	glLineWidth(1.0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glColor4ub(30, 30, 30, 100);
	glBegin(GL_QUADS);
		glVertex2f(x,       y);
		glVertex2f(x+width, y);
		glVertex2f(x+width, y+height);
		glVertex2f(x,       y+height);
	glEnd();
	
	int scrollbarWidth = haveScrollbar()?scrollPaneScrollbarWidth:0;
	
	if(selection != -1 && selection>=scrollPos && selection<scrollPos+lines)
	{
		glColor3ub(80, 80, 80);
		glBegin(GL_QUADS);
			glVertex2f(x,          y + lineHeight* selection    -scrollPos*lineHeight+scrollPaneTopMargin);
			glVertex2f(scrollbarX, y + lineHeight* selection    -scrollPos*lineHeight+scrollPaneTopMargin);
			glVertex2f(scrollbarX, y + lineHeight*(selection+1) -scrollPos*lineHeight+scrollPaneTopMargin);
			glVertex2f(x,          y + lineHeight*(selection+1) -scrollPos*lineHeight+scrollPaneTopMargin);
		glEnd();
	}
	for(unsigned ii=scrollPos; ii<elements.size() && ii-scrollPos<lines; ii++)
	{
		screenPrintf(x+scrollPaneLeftMargin,
		             y+lineHeight*ii-scrollPos*lineHeight+scrollPaneTopMargin,
		             *font, "%s", elements[ii].toString().c_str());
	}
	
	// Draw border
	glColor3ub(80,80,80);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(x,       y);
		glVertex2f(x+width, y);
		glVertex2f(x+width, y+height);
		glVertex2f(x,       y+height);
	glEnd();
	
	if(haveScrollbar()) {
		glBegin(GL_LINES);
			glVertex2f(x+width-scrollbarWidth, y);
			glVertex2f(x+width-scrollbarWidth, y+height);
			glVertex2f(scrollbarX, y+scrollButtonHeight);
			glVertex2f(x+width,    y+scrollButtonHeight);
			glVertex2f(scrollbarX, y+height-scrollButtonHeight);
			glVertex2f(x+width,    y+height-scrollButtonHeight);
		glEnd();
	}
}

template<class T>
bool ScrollPane<T>::mouseDownSelf(float mx, float my, int button, int mod)
{
	if(!containsPoint(mx,my))
		return false;
	
	// Click on scrollbar
	if(haveScrollbar() && mx>x+width-scrollPaneScrollbarWidth && mx<x+width)
	{
		// Top scroll button?
		if(my < y+scrollButtonHeight) {
			if(scrollPos>0)
				scrollPos--;
		}
		// Bottom scroll button?
		else if(my > y+height-scrollButtonHeight) {
			if(scrollPos<maxScroll)
				scrollPos++;
		}
		// Elsewhere on scrollbar?
		else {
			// TODO
		}
	}
	else
	{
		int index = (my-y)/lineHeight + scrollPos;
		if(index == lastClickIndex && lastClickTime+doubleClickDelay>getTime())
			activate();
		lastClickTime = getTime();
		lastClickIndex = index;
		if(index>=0 && index<elements.size())
			selection = index;
	}
	return true;
}

template<class T>
bool ScrollPane<T>::containsPoint(float px, float py)
{
	return px>=x && py>=y && px<x+width && py<y+height;
}

template<class T>
bool ScrollPane<T>::keyDownSelf(SDL_keysym sym)
{
	if(sym.sym==SDLK_RETURN) {
		activate();
		return true;
	}
	return false;
}


template<class T>
bool ScrollPane<T>::haveScrollbar()
{
	return elements.size()*lineHeight >= height;
}

template<class T>
int ScrollPane<T>::getSelectedIndex()
{
	return selection;
}
template<class T>
T ScrollPane<T>::getSelectedElement()
{
	return elements[selection];
}
template<class T>
bool ScrollPane<T>::haveSelection()
{
	return selection>=0 && selection<elements.size();
}

#endif
