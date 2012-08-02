#include "main.hpp"
#include <SDL_image.h>
#include <SDL_mixer.h>

void initMisc(int argc, char **argv);
void initSdl(int argc, char **argv);
void initOpenGL(int argc, char **argv);
void cleanupSdl();

/* The little engine that could. */
Engine *engine = NULL;
/* Video Settings */
SDL_PixelFormat screenFormat, textureFormat;


int main(int argc, char **argv)
{
	initMisc(argc, argv);
	initSdl(argc, argv);
	initOpenGL(argc, argv);
	initImagePool();
	initSound();
	initFonts();
	initNetwork();
	initFrames();
	initCommandCard();
	initCursor();
	
	mainLoop();
	
	return 0;
}

void initMisc(int argc, char **argv)
{
	glutInit(&argc, argv);
	srand(time(NULL));
}

void initSdl(int argc, char **argv)
{
	int vidFlags;
	
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE) < 0) {
		fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(cleanupSdl);
	
	SDL_EnableUNICODE(1);
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
#ifdef FULLSCREEN
	vidFlags = SDL_OPENGL | SDL_FULLSCREEN;
#else
	vidFlags = SDL_OPENGL;
#endif
	
	SDL_SetVideoMode(
		screenWidth, screenHeight,
		32, vidFlags);
	
	SDL_Surface *screenSurface = SDL_GetVideoSurface();
	if(!screenSurface) {
		fprintf(stderr, "SDL_GetVideoSurface failed: %s\n", SDL_GetError());
		exit(1);
	}
	if(screenSurface->format->BitsPerPixel != 32) {
		fprintf(stderr, "Failed setting color depth to 32-bit.");
		exit(1);
	}
	screenFormat = *screenSurface->format;
	textureFormat = screenFormat;
	textureFormat.Rmask = 0x000000FF; textureFormat.Rshift = 0x00;
	textureFormat.Gmask = 0x0000FF00; textureFormat.Gshift = 0x08;
	textureFormat.Bmask = 0x00FF0000; textureFormat.Bshift = 0x10;
	textureFormat.Amask = 0xFF000000; textureFormat.Ashift = 0x18; textureFormat.Aloss = 0;
	
	SDL_ShowCursor(0);

	SDL_WM_SetCaption("Refuse Robots, GDIAC 2005","Refuse Robots");
}


void initOpenGL(int argc, char **argv)
{
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); 
	
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	
	glViewport(0, 0, screenWidth, screenHeight);

	glMatrixMode(GL_PROJECTION);
	glOrtho(0, screenWidth, screenHeight, 0, -1, 1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}

void cleanupSdl()
{
	SDL_Quit();
}



Engine::Engine(const char *mapName)
{
	model      = NEW Model(mapName);
	commands   = NEW CommandQueue();
	messages   = NEW MessageQueue();
	//FIXME do we REALLY want to just assign playerID 1 to the human?
	view       = NEW ScreenView(model, commands, 1);
	
	controller = 0;
}

Engine::~Engine()
{
	delete model;
	delete commands;
	delete controller;
	delete view;
}
