#include "widget.hpp"
#include "widgetaction.hpp"
#include "main.hpp"

#include <cassert>

Widget::Widget(bool focused)
{
	keyFocus = focused;
	visible  = true;
	action   = 0;
	focus = NULL;
}

Widget::~Widget()
{
	for (WidgetPool::iterator it = childWidgets.begin(); it != childWidgets.end(); it++)
	{
		if (*it != this)
			delete (*it);
	}
}

void Widget::activate()
{
	if (action != 0)
		action->activate();
}

void Widget::setAction(WidgetAction *newAction)
{
	delete action;
	action = newAction;
}

void Widget::addChild(Widget *child)
{
	childWidgets.push_back(child);
}

bool Widget::mouseDown(float x, float y, int button, int mod)
{
	if(containsPoint(x, y)) {
		mouseDownSelf(x, y, button, mod);
		setKeyFocus(true);
	} else {
		setKeyFocus(false);
	}
	return true;
}

bool Widget::mouseDownSelf(float clickX, float clickY, int button, int mod)
{
	for (WidgetPool::iterator it = childWidgets.begin(); it != childWidgets.end(); it++)
	{
		Widget *currWidget = *it;
		if(currWidget->containsPoint(clickX, clickY))
		{
			if(currWidget->mouseDown(clickX, clickY, button, mod)) {
				if(focus && focus != currWidget) {
					focus->setKeyFocus(false);
				}
				currWidget->setKeyFocus(true);
				focus = currWidget;
			}
		}
	}
	activate();
	return true;
}

bool Widget::keyDown(SDL_keysym sym)
{
	keyDownSelf(sym);
	if(focus) focus->keyDown(sym);

/*	bool childHasFocus = false;
	for (WidgetPool::iterator it = childWidgets.begin(); it != childWidgets.end(); it++)
	{
		Widget *currWidget = *it;
		
		if (currWidget->hasKeyFocus())
		{
			currWidget->keyDown(sym);
			//currWidget->activate();
			childHasFocus = true;
		}
	}

	return keyFocus || childHasFocus;*/
	return true;

}

bool Widget::keyDownSelf(SDL_keysym sym)
{
	return false;
}

void Widget::redraw()
{
	if (visible)
	{
		redrawSelf();
		redrawChildren();
	}
}

void Widget::redrawChildren()
{
	for (WidgetPool::iterator it = childWidgets.begin(); it != childWidgets.end(); it++)
	{
		Widget *currWidget = *it;
		if (currWidget != this)
			currWidget->redraw();
	}
}

void Widget::setVisible(bool visible)
{
	this->visible = visible;
}

bool Widget::isVisible() const
{
	return visible;
}

void Widget::setKeyFocus(bool focused)
{
	keyFocus = focused;
}

bool Widget::hasKeyFocus() const
{
	return keyFocus;
}

void Widget::onLeavingFrame()
{
	// EMPTY
}

const Widget::WidgetPool &Widget::getChildWidgets() const
{
	return childWidgets;
}

bool Widget::rectContainsPoint(
	float rectX, float rectY, 
	float width, float height, 
	float clickX, float clickY)
{
	return (clickX >= rectX && clickX <= rectX + width &&
		    clickY >= rectY && clickY <= rectY + height);
}

//////////////////////////////////////////////////////////////////////////

RootWidget::RootWidget()
{
}

RootWidget::~RootWidget()
{
}

void RootWidget::redrawSelf()
{
	// EMPTY
}

bool RootWidget::containsPoint(float x, float y)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////

TextLabel::TextLabel(
	const std::string &text, 
	float newX, 
	float newY, 
	bool centered,
	const TextParams &params) : 
	text(text), x(newX), y(newY), isCentered(centered)
{
	textParams = &params;
	init();
}

TextLabel::TextLabel(
	const std::string &text, 
	bool centered, 
	const TextParams &params) : 
	text(text), x(0), y(0), isCentered(centered), textParams(&params)
{
	textParams = &params;
	init();
}

TextLabel::~TextLabel()
{
}

void TextLabel::init()
{
	TTF_SizeText(textParams->font, text.c_str(), &width, &height);

	if(isCentered)
		x -= width / 2;
}

void TextLabel::redrawSelf()
{
	drawText(x, y, *textParams, text.c_str());
}

bool TextLabel::containsPoint(float clickX, float clickY)
{
	return Widget::rectContainsPoint(
							x, y, 
							width, height, 
							clickX, clickY);
}

//////////////////////////////////////////////////////////////////////////

TextLabelGroup::TextLabelGroup(
	float newX, 
	float newY, 
	float space,
	bool centered, 
	const TextParams &params)
{
	cursorY    = 0;
	spacing    = space;
	isCentered = centered;
	textParams = &params;

	x = newX;
	y = newY;
}

TextLabelGroup::~TextLabelGroup()
{
}

void TextLabelGroup::addTextLabel(TextLabel *label)
{
	// Modify the incoming label to match the group's attributes.
	label->isCentered = isCentered;
	label->y          = y + (cursorY * spacing);
	label->x          = x;
	//label->textParams = textParams;

	//label->init();
	if(isCentered)
		label->x -= label->width / 2;
    
	addChild(label);
	cursorY++;
}

bool TextLabelGroup::containsPoint(float clickX, float clickY)
{
	for (WidgetPool::iterator it = childWidgets.begin(); it != childWidgets.end(); it++)
	{
		if ((*it)->containsPoint(clickX, clickY))
			return true;
	}

	return false;
}

void TextLabelGroup::redrawSelf()
{
	// This won't be needed unless we use some sort of fancy background image.
}

//////////////////////////////////////////////////////////////////////////

TextInputWidget::TextInputWidget(
	float newX, 
	float newY, 
	const TextParams &params)
{
	x = newX;
	y = newY;

	lastEnteredText = "";
	textInput = new TextInput(newX, newY, params, false);
}

TextInputWidget::~TextInputWidget()
{
	delete textInput;
}

bool TextInputWidget::containsPoint(float x, float y)
{
	// TODO:  Determine the exact dimensions of the text input.
	return false;
}

void TextInputWidget::redrawSelf()
{
	if (textInput != 0)
		textInput->redraw();
}

std::string TextInputWidget::getEnteredText() const
{
	return lastEnteredText;
}

bool TextInputWidget::keyDownSelf(SDL_keysym sym)
{
	if (textInput == 0)
		textInput = new TextInput(x, y, fontDefault, false);

	textInput->keyDown(sym);
	int inputResult = textInput->getResult();
	if (inputResult == 1)	// OK
	{
		lastEnteredText = textInput->getString();
		textInput->reset();
		delete textInput;
		textInput = 0;
		activate();
		return true;
	}
	else if (inputResult == -1) // cancelled
	{
		textInput->reset();
		delete textInput;
		textInput = 0;
		return true;
	}

	return false;
}

bool TextInputWidget::mouseDown(float x, float y, int button, int mod)
{
	return false;
}

//////////////////////////////////////////////////////////////////////////

TextDisplayWidget::TextDisplayWidget(
	float newX, 
	float newY, 
	float w, 
	float h, 
	const TextParams &params, 
	bool alignToBottom)
{
	init(newX, newY, w, h, params, alignToBottom);
	textDisplay = NEW TextDisplay(x, y, width, height, *textParams, alignedToBottom, false);
}

TextDisplayWidget::TextDisplayWidget(
	float newX, 
	float newY, 
	float w, 
	float h, 
	const TextParams &params, 
	bool alignToBottom, 
	TextDisplay *display)
{
	init(newX, newY, w, h, params, alignToBottom);
	textDisplay = display;
}

TextDisplayWidget::~TextDisplayWidget()
{
	delete textDisplay;
}

void TextDisplayWidget::init(
	float newX, 
	float newY, 
	float w, 
	float h, 
	const TextParams &params, 
	bool alignToBottom)
{
	x               = newX;
	y               = newY;
	width           = w;
	height          = h;
	textParams      = &params;
	alignedToBottom = alignToBottom;
}

bool TextDisplayWidget::containsPoint(float px, float py)
{
	return px>=x && py>=y && px<x+width && py<y+height;
}

void TextDisplayWidget::redrawSelf()
{
	textDisplay->redraw();
}

void TextDisplayWidget::onLeavingFrame()
{
	textDisplay->clear();
}

//////////////////////////////////////////////////////////////////////////

MessagePane::MessagePane(
	float newX, 
	float newY, 
	float w, 
	float h, 
	Image *img):x(newX), y(newY), width(w), height(h), image(img)
{
	memset(&color, 0, sizeof(color));
}

MessagePane::MessagePane(
	float newX, 
	float newY, 
	float w, 
	float h, 
	const Color &col):x(newX), y(newY), width(w), height(h), color(col)
{
	image = 0;
}

MessagePane::~MessagePane()
{
}

bool MessagePane::containsPoint(float clickX, float clickY)
{
	return Widget::rectContainsPoint(
								x, y, 
								width, height, 
								clickX, clickY);
}

void MessagePane::redrawSelf()
{
	if (image != 0) {
		drawImage(x, y, width, height, *image);
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor4ub(color.r, color.g, color.b, color.a);
		glBegin(GL_QUADS);
			glVertex2f(x,       y       );
			glVertex2f(x,       y+height);
			glVertex2f(x+width, y+height);
			glVertex2f(x+width, y       );
		glEnd();
	}
}