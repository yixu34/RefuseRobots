#ifndef FRAME_HPP
#define FRAME_HPP

#include "event.hpp"
#include "image.hpp"
#include "networknode.hpp"

#include <string>

void initFrames();

class RootWidget;

class Frame : public EventListener
{
public:
	Frame();
	virtual ~Frame();

	virtual void redraw();
	virtual void timepass();

	virtual Image *getBackground() const;

	void enable();
	void disable();

	virtual void setupWidgets() = 0;
	
	virtual void onEnable() {}
	virtual void onDisable() {}

	bool keyDown(SDL_keysym sym);
	bool keyUp(SDL_keysym sym);
	bool mouseDown(int x, int y, int button, int mod);
	bool mouseUp(int x, int y, int button);
	//bool mouseMotion(int x, int y, int xrel, int yrel);
	
	virtual void disconnected(NetworkNode *node) {}

	static void changeFrame(Frame *newFrame);

protected:
	RootWidget *rootWidget;
	bool enabled;
};

///////////////////////////////////////////////////////////////////////////

class PostMortem:public Frame
{
public:
	PostMortem();
	~PostMortem();

	void setupWidgets();
	void set(const std::string &str, bool safe);
	void set(bool safe);
	void set(const std::string &str);

	bool keyDown(SDL_keysym sym);
	bool mouseDown(int x, int y, int button, int mod);
	
private:
	void close();
	
	std::string text;
	bool safeToQuit;
};

extern PostMortem *postMortem;

///////////////////////////////////////////////////////////////////////////

extern Frame *activeFrame;

extern Image mainMenuBackground;

#endif