#ifndef TEXT_HPP
#define TEXT_HPP

#include <string>
#include <SDL_ttf.h>
#include <deque>

struct TextParams
{
	TextParams(std::string font, float size, int style,
	           float red, float green, float blue,
	           float shadowRed, float shadowGreen, float shadowBlue);
	bool operator==(const TextParams &o) const;
	
	TTF_Font *font;
	float r,g,b;
	float sr,sg,sb;
};
extern TextParams fontSmaller, fontDefault, fontBigger, fontBiggest,fontMiddle,chatFont, fontMenuHeading;

void initFonts();
void updateTextCache();

void drawText(float x, float y, const TextParams &params, const char *text);
void screenPrintf(float x, float y, const TextParams &params, const char *fmt, ...);



#endif
