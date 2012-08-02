#ifndef EVENT_HPP
#define EVENT_HPP

#include <SDL.h>

// Base class for things which handle events such as keystrokes, mouse clicks,
// and redraw requests.
//
// Each EventListener has a priority, which determines which one has priority
// in receiving events. The highest-priority EventListener will receive
// keystrokes first and be drawn on top. If an event function returns false,
// then the event will be passed to the next lowest priority.
class EventListener
{
public:
	EventListener(int priority);
	virtual ~EventListener();
	virtual bool keyDown(SDL_keysym sym);
	virtual bool keyUp(SDL_keysym sym);
	virtual bool mouseDown(int x, int y, int button, int mod);
	virtual bool mouseUp(int x, int y, int button);
	virtual bool mouseMotion(int x, int y, int xrel, int yrel);
	virtual void redraw();
	
	virtual bool isOpaque();
	int getEventPriority() const;
	
protected:
	void enableEvents();
	void disableEvents();
	bool eventsEnabled() const;
	void setEventPriority(int priority);
	
private:
	bool areEventsEnabled;
	int eventPriority;
};

void mainLoop();
extern bool shouldDestroyEngine;

#endif