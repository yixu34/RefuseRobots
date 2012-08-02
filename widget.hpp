#ifndef WIDGET_HPP
#define WIDGET_HPP

#include <vector>
#include <string>

#include <SDL.h>

#include "text.hpp"
#include "screenview.hpp"

class WidgetAction;

// Represents menu items.  (Buttons, text labels, popup windows,etc.)
class Widget
{
public:
	Widget(bool focused = false);
	virtual ~Widget();

	bool mouseDown(float x, float y, int button, int mod);
	bool keyDown(SDL_keysym sym);

	virtual bool containsPoint(float clickX, float clickY) = 0;
	virtual void onLeavingFrame();

	void redraw();
	void addChild(Widget *child);
	void setAction(WidgetAction *newAction);

	void setVisible(bool visible);
	bool isVisible() const;

	void setKeyFocus(bool focused);
	bool hasKeyFocus() const;

	typedef std::vector<Widget *> WidgetPool;
	const WidgetPool &getChildWidgets() const;

	static bool rectContainsPoint(
		float rectX, float rectY, 
		float width, float height, 
		float clickX, float clickY);

protected:
	virtual bool mouseDownSelf(float clickX, float clickY, int button, int mod);
	virtual void redrawSelf() = 0;
	void redrawChildren();
	void activate();

	virtual bool keyDownSelf(SDL_keysym sym);

	WidgetAction *action;
	bool visible;
	bool keyFocus;

	WidgetPool childWidgets;
	Widget *focus;
};

//////////////////////////////////////////////////////////////////////////

class RootWidget:public Widget
{
public:
	RootWidget();
	~RootWidget();

	bool containsPoint(float clickX, float clickY);

private:
	void redrawSelf();
};

//////////////////////////////////////////////////////////////////////////

class TextLabelGroup;

class TextLabel:public Widget
{
public:
	TextLabel(
		const std::string &text, 
		float newX, 
		float newY, 
		bool centered = true,  
		const TextParams &params = fontMiddle);

	TextLabel(
		const std::string &text, 
		bool centered = true, 
		const TextParams &params = fontMiddle);

	~TextLabel();

	bool containsPoint(float clickX, float clickY);

private:
	friend class TextLabelGroup;

	void redrawSelf();
	void init();
	void addChild();

	std::string text;
	float x;
	float y;
	bool isCentered;
	int width;
	int height;
	const TextParams *textParams;
};

//////////////////////////////////////////////////////////////////////////

class TextLabelGroup:public Widget
{
public:
	TextLabelGroup(
		float newX, 
		float newY, 
		float space, 
		bool centered, 
		const TextParams &params = fontMiddle);

	~TextLabelGroup();

	void addTextLabel(TextLabel *label);
	bool containsPoint(float clickX, float clickY);

private:
	void redrawSelf();

	float spacing;
	float x;
	float y;
	int cursorY;
	bool isCentered;
	const TextParams *textParams;
};

//////////////////////////////////////////////////////////////////////////

// Wraps a TextInput object for the menus.
class TextInputWidget:public Widget
{
public:
	TextInputWidget(
		float newX, 
		float newY, 
		const TextParams &params = chatFont);

	~TextInputWidget();

	bool containsPoint(float clickX, float clickY);
	//bool keyDown(SDL_keysym sym);
	bool mouseDown(float x, float y, int button, int mod);

	std::string getEnteredText() const;

private:
	void redrawSelf();
	bool keyDownSelf(SDL_keysym sym);

	float x;
	float y;

	std::string lastEnteredText;

	TextInput *textInput;
};

//////////////////////////////////////////////////////////////////////////

class TextDisplay;

class TextDisplayWidget:public Widget
{
public:
	TextDisplayWidget(
		float newX, 
		float newY, 
		float w, 
		float h, 
		const TextParams &params, 
		bool alignToBottom);

	TextDisplayWidget(
		float newX, 
		float newY, 
		float w, 
		float h, 
		const TextParams &params, 
		bool alignToBottom, 
		TextDisplay *display);

	~TextDisplayWidget();

	bool containsPoint(float clickX, float clickY);

	void onLeavingFrame();

private:
	void redrawSelf();
	void init(
		float newX, 
		float newY, 
		float w, 
		float h, 
		const TextParams &params, 
		bool alignToBottom);

	float x;
	float y;
	float width;
	float height;
	bool alignedToBottom;

	const TextParams *textParams;

	TextDisplay *textDisplay;
};

//////////////////////////////////////////////////////////////////////////

// Used for entering text that is not in-game or in the lobby, e.g. for
// entering your username.  Needed because TextInput's and their wrapping
// TextInputWidget's are dependent on the events system.
class TextBox:public Widget
{
public:
	TextBox(float newX, float newY, int w,
		const std::string &initialText = "", 
		const TextParams &params = fontDefault, 
		int cursorW = TextBox::defaultCursorWidth);

	~TextBox();

	bool containsPoint(float clickX, float clickY);

	static const int defaultCursorWidth = 3;
	std::string getText() { return text; }

	void reset();

private:
	void redrawSelf();
	void drawCursor();
	void drawHighlight();
	void drawOutline();

	bool keyDownSelf(SDL_keysym sym);
	bool mouseDownSelf(float clickX, float clickY, int button, int mod);

	int cursorPos;
	int cursorWidth;

	float x;
	float y;
	float width;
	float height;

	std::string text;

	bool isHighlighted;

	const TextParams *textParams;
};

//////////////////////////////////////////////////////////////////////////

class MessagePane:public Widget
{
public:
	MessagePane(
		float newX, 
		float newY, 
		float w, 
		float h, 
		const Color &col);

	MessagePane(
		float newX, 
		float newY, 
		float w, 
		float h, 
		Image *img);

	~MessagePane();

	bool containsPoint(float clickX, float clickY);

private:
	void redrawSelf();

	float x;
	float y;
	float width;
	float height;
	Color color;
	Image *image;
};

#include "scrollpane.hpp"

#endif
