#include "main.hpp"
#include <SDL_ttf.h>
#include "logger.hpp"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

// Cached rendering of a string
struct TextCache
{
	TextCache(std::string text, const TextParams &params);
	~TextCache();
	void draw(float x, float y);
	bool match(std::string text, const TextParams &params);
	
	std::string text;
	TextParams params;
	int w, h;
	GLuint texID;
	bool preserve; // Whether this thing should stay in the cache another frame
};
typedef std::multimap<std::string, TextCache*> TextCacheStore;
static TextCacheStore textCache;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

typedef std::map<std::string, TTF_Font*> FontPool;
static FontPool *fontPool = NULL;

TextParams fontSmaller("fonts/Vera.ttf", 12, 0, 1,1,1, 0,0,0);
TextParams fontDefault("fonts/Vera.ttf", 18, 0, 1,1,1, 0,0,0);
TextParams chatFont("fonts/Vera.ttf", 25, 0, 1,1,1, 0,0,0);
TextParams fontBigger("fonts/Vera.ttf", 30, TTF_STYLE_BOLD, 1,1,1, 0,0,0);
TextParams fontMiddle("fonts/Vera.ttf", 35, 0, 1,1,1, 0,0,0);
TextParams fontBiggest("fonts/Vera.ttf", 80, 0, 1,1,1, 0,0,0);
TextParams fontMenuHeading("fonts/Vera.ttf", 68, 0, 1,1,1, 0,0,0);

static void cleanupFonts() {
	for(FontPool::iterator ii=fontPool->begin(); ii!=fontPool->end(); ii++)
		TTF_CloseFont(ii->second);
	fontPool->clear();
}
void initFonts() {
	if(!TTF_WasInit() && TTF_Init() < 0) {
		fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
		exit(1);
	}
	atexit(cleanupFonts);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

TextParams::TextParams(std::string font, float size, int style,
	                   float red, float green, float blue,
	                   float shadowRed, float shadowGreen, float shadowBlue)
	: r(red), g(green), b(blue), sr(shadowRed), sg(shadowGreen), sb(shadowBlue)
{
	if(!TTF_WasInit() && TTF_Init() < 0) {
		fprintf(stderr, "TTF_Init failed: %s\n", TTF_GetError());
		exit(1);
	}
	if(!fontPool)
		fontPool = NEW FontPool;
	
	std::string fontName = retprintf("%s,%.1f,%i", font.c_str(), size, style);
	if(fontPool->find(fontName) == fontPool->end()) {
		TTF_Font *f = TTF_OpenFont(font.c_str(), size);
		(*fontPool)[fontName] = f;
		TTF_SetFontStyle(f, style);
	}
	this->font = (*fontPool)[fontName];
}


bool TextParams::operator==(const TextParams &o) const
{
	return font==o.font
	    && r==o.r && g==o.g && b==o.b
	    && sr==o.sr && sg==o.sg && sb==o.sb;
}


static void textureSize(int oldX, int oldY, int &newX, int &newY)
{
	for(newX=8; newX<oldX; newX<<=1);
	for(newY=8; newY<oldY; newY<<=1);
}


TextCache::TextCache(std::string text, const TextParams &params)
	: params(params)
{
	// Render text
	SDL_Color white = {255,255,255,0};
	SDL_Surface *renderedText = convertImage(TTF_RenderText_Blended(params.font, text.c_str(), white));
	
	// Copy the text onto a new surface with a meaningful size
	int sizeX, sizeY;
	SDL_SetAlpha(renderedText, 0, 255);
	textureSize(renderedText->w, renderedText->h, sizeX, sizeY);
	SDL_Surface *image = SDL_CreateRGBSurface(SDL_SWSURFACE, sizeX, sizeY, 32,
		textureFormat.Rmask, textureFormat.Gmask, textureFormat.Bmask, textureFormat.Amask);
	SDL_FillRect(image, NULL, 0); 
	SDL_BlitSurface(renderedText, NULL, image, NULL);
	
	GLuint textureID = 0;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	glTexImage2D(GL_TEXTURE_2D,               //type of texture
		         0,                           //level of detail (mipmap)
		         4,                           //components per pixel
		         image->w, image->h,          //dimensions
		         0,                           //image border
		         GL_RGBA,                     //colors order
		         GL_UNSIGNED_BYTE,            //components data type
		         image->pixels);              //pixel data

/*	gluBuild2DMipmaps(GL_TEXTURE_2D,          //type of texture
	                  4,                    //
	                  image->w, image->h,   //dimensions
	                  GL_RGBA,              //format
	                  GL_UNSIGNED_BYTE,     //channel type
	                  image->pixels);       //data
*/
	
	this->text = text;
	this->w = image->w;
	this->h = image->h;
	this->texID = textureID;
	this->preserve = false;
	
	SDL_FreeSurface(renderedText);
	SDL_FreeSurface(image);
}

TextCache::~TextCache()
{
	glDeleteTextures(1, &texID);
}

void TextCache::draw(float x, float y)
{
	preserve = true; // It's being drawn, so keep this text cached
	
	glBindTexture(GL_TEXTURE_2D, texID);
	glBegin(GL_QUADS);
		glColor3f(params.sr, params.sg, params.sb);
		glTexCoord2f(0.0, 0.0); glVertex2f(x+1,   y+1  );
		glTexCoord2f(1.0, 0.0); glVertex2f(x+1+w, y+1  );
		glTexCoord2f(1.0, 1.0); glVertex2f(x+1+w, y+1+h);
		glTexCoord2f(0.0, 1.0); glVertex2f(x+1,   y+1+h);
		
		glColor3f(params.r, params.g, params.b);
		glTexCoord2f(0.0, 0.0); glVertex2f(x,   y  );
		glTexCoord2f(1.0, 0.0); glVertex2f(x+w, y  );
		glTexCoord2f(1.0, 1.0); glVertex2f(x+w, y+h);
		glTexCoord2f(0.0, 1.0); glVertex2f(x,   y+h);
	glEnd();
}

bool TextCache::match(std::string text, const TextParams &params)
{
	return text==this->text && params==this->params;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

void drawText(float x, float y, const TextParams &params, const char *text)
{
	if(!text || !*text)
		return;
	
	for(TextCacheStore::iterator ii = textCache.find(text); ii != textCache.end(); ii++)
	{
		if(ii->second->match(text, params)) {
			ii->second->draw(x, y);
			return;
		}
	}
	TextCache *c = NEW TextCache(text, params);
	c->draw(x, y);
	textCache.insert(std::pair<std::string, TextCache*>(text, c));
}

// Print text to the screen
void screenPrintf(float x, float y, const TextParams &params, const char *fmt, ...)
{
	va_list args;
	char buf[1024];
	va_start(args, fmt);
	_vsnprintf(buf, 1024, fmt, args);
	va_end(args);
	drawText(x, y, params, buf);
}

// Evict old entries from the text rendering cache
void updateTextCache()
{
	TextCacheStore::iterator ii = textCache.begin();
	
	while(ii != textCache.end()) {
		TextCache *pos = ii->second;
		if(pos->preserve == false) {
			delete pos;
			ii = textCache.erase(ii);
		} else
			ii++;
	}
}

