#include "main.hpp"

TextDisplay::TextDisplay(float x, float y, float width, float height,
                         TextParams textParams, bool alignToBottom, bool startEnabled)
	: x(x), y(y), width(width), height(height), alignToBottom(alignToBottom),
	EventListener(3), timeout(0), textParams(textParams)
{
	if (!startEnabled)
		disableEvents();
}

void TextDisplay::redraw()
{
	int ypos;
	int fontHeight = TTF_FontHeight(textParams.font);
	
	if(alignToBottom) {
		ypos = y + height - fontHeight*lines.size();
	}
	else
		ypos = y;
	
	for(LineList::iterator ii=lines.begin(); ii!=lines.end(); ii++)
	{
		screenPrintf(x, ypos, textParams, "%s", ii->second.c_str());
		ypos += fontHeight;
	}
}

bool TextDisplay::isOpaque()
{
	return false;
}

void TextDisplay::timepass(float dt)
{
	if(!timeout)
		return;
	while(lines.size()>0 && lines[0].first+timeout < getTime())
		lines.pop_front();
}
	
void TextDisplay::setTimeout(float timeout)
{
	this->timeout = timeout;
}

void TextDisplay::println(std::string line)
{
	lines.push_back( std::pair<float,std::string>(getTime(), line) );
}

void TextDisplay::clear()
{
	lines.clear();
}

