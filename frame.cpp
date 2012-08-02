#include "menu.hpp"
#include "frame.hpp"
#include "widget.hpp"
#include "main.hpp"
#include "lobby.hpp"

#include <fstream>
#include <deque>

void parseUsername()
{
	char temp[256];
	std::ifstream settings("settings.txt");
	if(settings.good()) {
		settings.getline (temp,256);
		settings.close();
		username=temp;
	} else {
		username="Player";
	}
}

void initFrames()
{
	mainMenu            = new MainMenu();
	singlePlayerMapMenu = new SinglePlayerMapMenu();
	multiPlayerMapMenu  = new MultiPlayerMapMenu();
	creditsScreen       = new CreditsScreen();

	parseUsername();

	hostLobby   = new HostLobby("", 0);
	clientLobby = new ClientLobby();
	joinFrame   = new JoinFrame();
	postMortem  = new PostMortem();

	mainMenu->setupWidgets();
	singlePlayerMapMenu->setupWidgets();
	multiPlayerMapMenu->setupWidgets();
	creditsScreen->setupWidgets();
	hostLobby->setupWidgets();
	clientLobby->setupWidgets();
	joinFrame->setupWidgets();

	activeFrame = mainMenu;
	activeFrame->enable();
}

///////////////////////////////////////////////////////////////////////////

Frame *activeFrame     = 0;
PostMortem *postMortem = 0;

Frame::Frame() : EventListener(2)
{
	rootWidget = new RootWidget();
	disable();
}

Frame::~Frame()
{
	delete rootWidget;
}

void Frame::enable()
{
	enabled = true;
	enableEvents();
	onEnable();
}

void Frame::disable()
{
	enabled = false;
	disableEvents();
	onDisable();
	
	// Tell each widget that the frame changed
	typedef std::deque<Widget *> WidgetTraversal;
	WidgetTraversal widgets;
	widgets.push_back(rootWidget);

	while (!widgets.empty())
	{
		Widget *currWidget = widgets.front();
		widgets.pop_front();

		currWidget->onLeavingFrame();

		const Widget::WidgetPool &childWidgets = currWidget->getChildWidgets();
		for (Widget::WidgetPool::const_iterator it = childWidgets.begin();
			 it != childWidgets.end();
			 it++)
		{
			Widget *currChildWidget = *it;
			widgets.push_back(currChildWidget);
		}
	}
}

Image *Frame::getBackground() const
{
	return &mainMenuBackground;
}

void Frame::redraw()
{
	if (!enabled)
		return;

	Image *background = getBackground();
	if (background != 0)
		drawImage(0, 0, screenWidth, screenHeight, *background);

	if (rootWidget != 0 && rootWidget->isVisible())
		rootWidget->redraw();
}

void Frame::timepass()
{
	// EMPTY
}

bool Frame::keyDown(SDL_keysym sym)
{
	return rootWidget->keyDown(sym);
}

bool Frame::keyUp(SDL_keysym sym)
{
	return false;
}

bool Frame::mouseDown(int x, int y, int button, int mod)
{
	return rootWidget->mouseDown(x, y, button, mod);
}

bool Frame::mouseUp(int x, int y, int button)
{
	return false;
}

void Frame::changeFrame(Frame *newFrame)
{
	if(activeFrame) activeFrame->disable();
	activeFrame = newFrame;
	if(activeFrame) activeFrame->enable();
}

///////////////////////////////////////////////////////////////////////////

PostMortem::PostMortem()
{
	safeToQuit = false;
}

PostMortem::~PostMortem()
{
}

void PostMortem::setupWidgets()
{
	delete rootWidget;
	rootWidget = new RootWidget();
	rootWidget->addChild(new TextLabel(text, screenCenterX, 150));
	rootWidget->addChild(new TextLabel(safeToQuit ? "Press any key to continue" : "Please wait for the game to finish",screenCenterX,250));
}

void PostMortem::set(const std::string &str)
{
	text = str;
	setupWidgets();
}

void PostMortem::set(const std::string &str, bool safe)
{
	safeToQuit = safe;
	text = str;
	setupWidgets();
}

void PostMortem::set(bool safe)
{
	safeToQuit = safe;
	setupWidgets();
}

bool PostMortem::keyDown(SDL_keysym sym)
{
	
	if(safeToQuit)
		close();
	else if(sym.sym == SDLK_F4 && sym.mod & KMOD_LALT){
		Packet *playerLostMsg = NEW Packet(msg_player_lost);
		network->sendToAll(playerLostMsg);
		close();
	}
	return true;
}
bool PostMortem::mouseDown(int x, int y, int button, int mod)
{
	if(safeToQuit)
		close();
	return true;
}
void PostMortem::close()
{
	shouldDestroyEngine = true;
	Frame::changeFrame(mainMenu);
}

