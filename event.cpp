#include "main.hpp"
#include <algorithm>
#include "tracker.hpp"

typedef std::vector<EventListener*> EventListenerPool;
static EventListenerPool eventListeners, eventListenersCopy;
static bool eventListenersChanged = false;

bool EventListener::keyDown(SDL_keysym sym)                       { return false; }
bool EventListener::keyUp(SDL_keysym sym)                         { return false; }
bool EventListener::mouseDown(int x, int y, int button, int mod)  { return false; }
bool EventListener::mouseUp(int x, int y, int button)             { return false; }
bool EventListener::mouseMotion(int x, int y, int xrel, int yrel) { return false; }
void EventListener::redraw()                                      {               }
bool EventListener::isOpaque()                                    { return true;  }

bool shouldDestroyEngine = false;


static bool comparePriorities(const EventListener *a, const EventListener *b) {
	return a->getEventPriority() > b->getEventPriority();
}

EventListener::EventListener(int priority)
{
	areEventsEnabled = false;
	eventPriority = priority;
	enableEvents();
}

EventListener::~EventListener()
{
	disableEvents();
}

void EventListener::enableEvents()
{
	if(areEventsEnabled) return;
	areEventsEnabled = true;
	eventListeners.push_back(this);
	eventListenersChanged = true;
}

void EventListener::disableEvents()
{
	if(!areEventsEnabled) return;
	areEventsEnabled = false;
	for(EventListenerPool::iterator ii=eventListeners.begin(); ii!=eventListeners.end(); ii++) {
		if(*ii == this) {
			eventListeners.erase(ii);
			break;
		}
	}
	for(EventListenerPool::iterator ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++) {
		if(*ii == this) {
			eventListenersCopy.erase(ii);
			break;
		}
	}
}

bool EventListener::eventsEnabled() const
	{ return areEventsEnabled; }
int EventListener::getEventPriority() const
	{ return eventPriority; }
void EventListener::setEventPriority(int priority) {
	disableEvents();
	eventPriority = priority;
	enableEvents();
}


void handleEvent(SDL_Event &event);
void handleEvents();
void drawScreen();

void mainLoop()
{
	while(1) 
	{
		updateTime();
		handleEvents();
		if(network != NULL)
			network->timepass();
		tracker.timepass();
		if (activeFrame)
			activeFrame->timepass();
		if(engine != NULL)
		{
			if(engine->controller)
				engine->controller->timepass(getDt());
			if(engine->view)
				engine->view->timepass(getDt());
			for(unsigned ii=0; ii < engine->aiPlayers.size(); ii++)
				engine->aiPlayers[ii]->timepass(getDt());
		}
		drawScreen();
		
		if(shouldDestroyEngine) {
			if(engine) {
				delete engine;
				engine = NULL;
			}
			if(network) {
				delete network;
				network = NULL;
			}
			shouldDestroyEngine = false;
		}
	}
}

void handleEvents()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
		handleEvent(event);
}

void updateEventListeners()
{
	if(eventListenersChanged) {
		sort(eventListeners.begin(), eventListeners.end(), comparePriorities);
		eventListenersCopy = eventListeners;
		eventListenersChanged = false;
	}
}

static int kmod = 0;

void handleEvent(SDL_Event &event)
{
	updateEventListeners();
	
	EventListenerPool::iterator ii;
	
	switch(event.type)
	{
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
				case SDLK_LSHIFT: kmod |= KMOD_LSHIFT; break;
				case SDLK_RSHIFT: kmod |= KMOD_RSHIFT; break;
				case SDLK_LCTRL:  kmod |= KMOD_LCTRL;  break;
				case SDLK_RCTRL:  kmod |= KMOD_RCTRL;  break;
				case SDLK_LALT:   kmod |= KMOD_LALT;   break;
				case SDLK_RALT:   kmod |= KMOD_RALT;   break;
			}
			for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++)
				if((*ii)->keyDown(event.key.keysym)) break;
			break;
		case SDL_KEYUP:
			switch(event.key.keysym.sym) {
				case SDLK_LSHIFT: kmod &= ~KMOD_LSHIFT; break;
				case SDLK_RSHIFT: kmod &= ~KMOD_RSHIFT; break;
				case SDLK_LCTRL:  kmod &= ~KMOD_LCTRL;  break;
				case SDLK_RCTRL:  kmod &= ~KMOD_RCTRL;  break;
				case SDLK_LALT:   kmod &= ~KMOD_LALT;   break;
				case SDLK_RALT:   kmod &= ~KMOD_RALT;   break;
			}
			for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++)
				if((*ii)->keyUp(event.key.keysym)) break;
			break;
		case SDL_MOUSEMOTION:
			cursor->moveCursor(event.motion.x, event.motion.y, event.motion.xrel, event.motion.yrel);
			for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++) {
				if((*ii)->mouseMotion(cursor->getLocation().x, cursor->getLocation().y,
				                      event.motion.xrel, event.motion.yrel))
					break;
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++) {
				if((*ii)->mouseDown(cursor->getLocation().x, cursor->getLocation().y,
				                    event.button.button, kmod))
					break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++) {
				if((*ii)->mouseUp(cursor->getLocation().x, cursor->getLocation().y,
				                  event.button.button))
					break;
			}
			break;
		case SDL_QUIT:
			dumpAllocs();
			exit(0);
			break;
	}
}


void drawScreen()
{
	EventListenerPool::iterator ii, start, end;
	updateEventListeners();
	
	start = eventListenersCopy.begin();
	end = eventListenersCopy.end();
	
	glClear(GL_COLOR_BUFFER_BIT);
	
	if(eventListenersCopy.size()>0)
	{
		for(ii=eventListenersCopy.begin(); ii!=eventListenersCopy.end(); ii++) {
			if((*ii)->isOpaque()) {
				end = ii;
				end++;
				break;
			}
		}
		for(ii=end; ii!=start; ) {
			ii--;
			(*ii)->redraw();
		}
	}
	
	updateTextCache();
	SDL_GL_SwapBuffers();
}

