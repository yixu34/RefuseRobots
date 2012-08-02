#include "image.hpp"
#include <cassert>
#include <SDL_image.h>
#include <glut.h>

static bool operator==(const SDL_PixelFormat &a, const SDL_PixelFormat &b);

class ImagePool
{
public:
	ImagePool();
	bool ready();
	void deferLoad(std::string filename, Image *img);
	void copyDeferredLoad(const Image *from, Image *to);
	void cancelDeferredLoad(Image *img);
	void loadImage(std::string filename, GLuint texID);
	GLuint getTexID(std::string filename);
	void loadImages();
	ICoord imageSize(GLuint texID) const;
	Color averageColor(GLuint texID);
	
protected:
	Color averageColor(void *pixels, unsigned w, unsigned h);
	
	typedef std::pair<std::string, Image*> StringImagePair;
	typedef std::vector< StringImagePair > UnloadedPool;
	typedef std::map< std::string, GLuint> TexturePool;
	typedef std::map<Image*, std::string> ReverseUnloadedPool;
	typedef std::map<GLuint, ICoord> ImageSizes;
	
	bool loaded;
	UnloadedPool unloadedImages;
	ReverseUnloadedPool pendingImages;
	ImageSizes imageSizes;
	TexturePool texIDs;
	std::map<GLuint, Color> averageColors;
};
ImagePool *imagePool = NULL;


void initImagePool()
{
	if(!imagePool)
		imagePool = new ImagePool();
	imagePool->loadImages();
}


Image::Image()
{
	textureID = 0;
}

Image::Image(std::string filename)
{
	if(!imagePool) {
		imagePool = new ImagePool();
	}
	if(imagePool->ready())
		textureID = imagePool->getTexID(filename);
	else
		imagePool->deferLoad(filename, this);
}

Image::Image(const Image &copy)
{
	if(imagePool->ready()) {
		textureID = copy.textureID;
	} else {
		imagePool->copyDeferredLoad(&copy, this);
	}
}
Image::~Image()
{
	if(!imagePool->ready()) {
		imagePool->cancelDeferredLoad(this);
	}
}

void Image::bind() const
{
	glBindTexture(GL_TEXTURE_2D, textureID);
}

unsigned Image::getWidth() const
{
	if(!imagePool || !imagePool->ready())
		return 0;
	else
		return imagePool->imageSize(textureID).x;
}

unsigned Image::getHeight() const
{
	if(!imagePool || !imagePool->ready())
		return 0;
	else
		return imagePool->imageSize(textureID).y;
}
Color Image::averageColor() const
{
	if(!imagePool || !imagePool->ready())
		return Color(0,0,0,0);
	else
		return imagePool->averageColor(textureID);
}




ImagePool::ImagePool()
{
	loaded = false;
}

bool ImagePool::ready()
{
	return loaded;
}

/// Convert an image to textureFormat.
/// Returns a new SDL_Surface containing the reformatted image. The original
/// surface is freed. Returns NULL if the conversion failed, or #image if no
/// conversion is necessary.
SDL_Surface *convertImage(SDL_Surface *image)
{
	if(!image)
		return NULL;
	else if(textureFormat == *(image->format))
		return image;
	else
	{
		SDL_Surface *newImage;
		newImage = SDL_ConvertSurface(image, &textureFormat, SDL_SWSURFACE);
		SDL_FreeSurface(image);
		return newImage;
	}
}

void ImagePool::loadImage(std::string filename, GLuint textureID)
{
	if(filename.length()==0) {
		textureID = 0;
		return;
	}
	filename = std::string("images/") + filename;
	int MinFilter=GL_LINEAR, MagFilter=GL_LINEAR;
	bool use_mipmap=false;
	
	SDL_Surface *image;
	SDL_PixelFormat *desired_format = &textureFormat;
	
	// Load image using SDL_image
	image = IMG_Load(filename.c_str());
	if (image==NULL) {
		throw TextureLoadError(retprintf("IMG_Load failed for %s: %s\n", filename.c_str(), SDL_GetError()));
	}
	
	// Convert to the appropriate pixel format if necessary
	image = convertImage(image);
	if(!image) {
		throw TextureLoadError(retprintf(
			"Failed converting %s to an appropriate format.\n", filename.c_str()));
	}
	
	
	// Make a texture
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter);
	
	imageSizes[textureID] = ICoord(image->w, image->h);
	
	if((image->w != 64 && image->w != 128 && image->w != 256) ||
	   (image->h != 64 && image->h != 128 && image->h != 256))
		use_mipmap = true;              //force mipmap for non-standard textures
	
	int numChannels;
	int channelOrder;
	int channelType;
	
	channelOrder = GL_RGBA;
	channelType = GL_UNSIGNED_BYTE;
	numChannels = 4;
	
	if (use_mipmap) {
		gluBuild2DMipmaps(GL_TEXTURE_2D,          //type of texture
		                  numChannels,            //
		                  image->w, image->h,     //dimensions
		                  channelOrder,           //format
		                  channelType,            //channel type
		                  image->pixels);         //data
	} else {
		glTexImage2D(GL_TEXTURE_2D,               //type of texture
		             0,                           //level of detail (mipmap)
		             numChannels,                 //components per pixel
		             image->w, image->h,          //dimensions
		             0,                           //image border
		             channelOrder,                //colors order
		             channelType,                 //components data type
		             image->pixels);              //pixel data
	}
	
	averageColors[textureID] = averageColor(image->pixels, image->w, image->h);
	
	// Clean up
	SDL_FreeSurface(image);
}

Color ImagePool::averageColor(void *pixels, unsigned w, unsigned h)
{
	unsigned long r=0,g=0,b=0;
	unsigned char *bytes = (unsigned char*)pixels;
	unsigned numBytes = w*h*4;
	unsigned ii;
	
	for(ii=0; ii<numBytes; ii+=4)
	{
		r += bytes[ii];
		g += bytes[ii+1];
		b += bytes[ii+2];
	}
	
	return Color( (double)r/(w*h), (double)g/(w*h), (double)b/(w*h), 255 );
}

Color ImagePool::averageColor(GLuint textureID)
{
	return averageColors[textureID];
}

GLuint ImagePool::getTexID(std::string filename)
{
	TexturePool::iterator ii = texIDs.find(filename);
	if(ii==texIDs.end()) {
		GLuint newID;
		glGenTextures(1, &newID);
		loadImage(filename, newID);
		texIDs[filename] = newID;
		return newID;
	} else {
		return ii->second;
	}
}

ICoord ImagePool::imageSize(GLuint texID) const
{
	ImageSizes::const_iterator ii=imageSizes.find(texID);
	if(ii==imageSizes.end())
		return ICoord(0,0);
	else
		return ii->second;
}

void ImagePool::deferLoad(std::string filename, Image *img)
{
	unloadedImages.push_back( StringImagePair(filename, img) );
	pendingImages[img] = filename;
}
void ImagePool::copyDeferredLoad(const Image *from, Image *to)
{
	pendingImages[to] = pendingImages[(Image*)from];
	unloadedImages.push_back( StringImagePair(pendingImages[to], to) );
}
void ImagePool::cancelDeferredLoad(Image *img)
{
	pendingImages.erase(img);
	
	for(UnloadedPool::iterator ii=unloadedImages.begin(); ii!=unloadedImages.end(); ii++) {
		if(ii->second == img) {
			unloadedImages.erase(ii);
			break;
		}
	}
}

void ImagePool::loadImages()
{
	loaded = true;
	for(UnloadedPool::iterator ii = unloadedImages.begin(); ii!=unloadedImages.end(); ii++)
		*(ii->second) = Image(ii->first.c_str());
	unloadedImages.clear();
	pendingImages.clear();
}


void drawImage(float x, float y, float size_x, float size_y, Image image)
{
	image.bind();
	
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(x,        y       );
		glTexCoord2f(1.0, 0.0); glVertex2f(x+size_x, y       );
		glTexCoord2f(1.0, 1.0); glVertex2f(x+size_x, y+size_y);
		glTexCoord2f(0.0, 1.0); glVertex2f(x,        y+size_y);
	glEnd();
}



static bool operator==(const SDL_PixelFormat &a, const SDL_PixelFormat &b)
{
	return a.palette == b.palette &&
		a.BitsPerPixel == b.BitsPerPixel &&
		a.BytesPerPixel == b.BytesPerPixel &&
		a.Rloss == b.Rloss && a.Gloss == b.Gloss &&
		a.Bloss == b.Bloss && a.Aloss == b.Aloss &&
		a.Ashift == b.Ashift && a.Rshift == b.Rshift &&
		a.Gshift == b.Gshift && a.Bshift == b.Bshift &&
		a.Amask == b.Amask && a.Rmask == b.Rmask &&
		a.Gmask == b.Gmask && a.Bmask == b.Bmask &&
		a.colorkey == b.colorkey && a.alpha == b.alpha;
}
