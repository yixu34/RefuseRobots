#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>
#include <SDL.h>
#include <stdexcept>
//#include "resourcepool.hpp"
#include "util.hpp"

/// Image: A texture
/// Treat an Image as you would an integer texture ID - that is, pass it by
/// value, not by pointer. You may create Images in global variables like this:
///   Image menuImage("menu.png");
/// and the image will be loaded later (when initImagePool is called, during
/// initialization).
///
/// It is always safe to copy Images or to make new ones; they will share data
/// automatically.
///
/// To draw an image, either use the drawImage() function, or call Image::bind
/// followed by OpenGL drawing functions.
class Image
{
public:
	Image();
	Image(std::string filename);
	Image(const Image &copy);
	~Image();
	
	void bind() const;
	
	// Get the dimensions of the image. Note that, if the image hasn't actually
	// been loaded yet (eg in a constructor for a global variable), these will
	// return 0 instead.
	unsigned getWidth() const;
	unsigned getHeight() const;
	
	friend bool operator==(Image a, Image b) { return a.textureID==b.textureID; }
	friend bool operator!=(Image a, Image b) { return a.textureID!=b.textureID; }
	
	// Get the average color over the entire image. (Used for minimap)
	Color averageColor() const;
	
protected:
	unsigned textureID;
};



class TextureLoadError : std::runtime_error
{
public:
	TextureLoadError(const std::string &message) : std::runtime_error(message) {}
};

void initImagePool();
void drawImage(float x, float y, float size_x, float size_y, Image image);

extern SDL_PixelFormat screenFormat, textureFormat;
SDL_Surface *convertImage(SDL_Surface *image);

#endif
