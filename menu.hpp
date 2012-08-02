#ifndef MENU_HPP
#define MENU_HPP

#include "frame.hpp"
#include "util.hpp"
#include <string>
#include <vector>

class Menu : public Frame
{
public:
	Menu();
	virtual ~Menu();
};

///////////////////////////////////////////////////////////////////////////

class MainMenu : public Menu
{
public:
	MainMenu();
	~MainMenu();
	void setupWidgets();
	Image *getBackground() const;
};

extern MainMenu *mainMenu;

///////////////////////////////////////////////////////////////////////////

class MapSelectionMenu : public Menu
{
public:
	MapSelectionMenu();
	~MapSelectionMenu();

protected:
	void parseMaps();

	std::vector<std::string> mapFileNames;
	std::vector<MapDesc> mapDescs;
};

class SinglePlayerMapMenu:public MapSelectionMenu
{
public:
	SinglePlayerMapMenu();
	~SinglePlayerMapMenu();

	void setupWidgets();
};

extern SinglePlayerMapMenu *singlePlayerMapMenu;

class MultiPlayerMapMenu:public MapSelectionMenu
{
public:
	MultiPlayerMapMenu();
	~MultiPlayerMapMenu();

	void setupWidgets();
};

extern MultiPlayerMapMenu *multiPlayerMapMenu;

///////////////////////////////////////////////////////////////////////////

class CreditsScreen : public Menu
{
public:
	CreditsScreen();
	~CreditsScreen();

	void setupWidgets();

	bool keyDown(SDL_keysym sym);
	bool mouseDown(int x, int y, int button, int mod);
};

extern CreditsScreen *creditsScreen;

extern std::string username;
extern std::string oldUsername;

#endif

