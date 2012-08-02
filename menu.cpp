#include "menu.hpp"
#include "main.hpp"
#include "lobby.hpp"
#include "widget.hpp"
#include "widgetaction.hpp"

#include <fstream>

Image mainMenuBackground("mainMenu.png");

MainMenu *mainMenu                       = 0;
SinglePlayerMapMenu *singlePlayerMapMenu = 0;
MultiPlayerMapMenu *multiPlayerMapMenu   = 0;
CreditsScreen *creditsScreen             = 0;

std::string username;
std::string oldUsername;

Menu::Menu()
{
}

Menu::~Menu()
{
}

///////////////////////////////////////////////////////////////////////////

MainMenu::MainMenu()
{
}

MainMenu::~MainMenu()
{
}

void MainMenu::setupWidgets()
{
	TextLabelGroup *labels = new TextLabelGroup(screenCenterX / 4, 250, 75, false);

	// Single player
	TextLabel *hostSP = new TextLabel("Single Player");
		hostSP->setAction(new ChangeFrameAction(singlePlayerMapMenu));
	labels->addTextLabel(hostSP);

	// Host Multiplayer
	TextLabel *hostMP = new TextLabel("Host Multiplayer");
		hostMP->setAction(new ChangeFrameAction(multiPlayerMapMenu));
	labels->addTextLabel(hostMP);

	// Join Multiplayer
	TextLabel *joinMP = new TextLabel("Join Multiplayer");
		joinMP->setAction(new ChangeFrameAction(joinFrame));
	labels->addTextLabel(joinMP);

	// Credits
	TextLabel *credits = new TextLabel("Credits");
		credits->setAction(new ChangeFrameAction(creditsScreen));
	labels->addTextLabel(credits);

	// Quit
	TextLabel *quit = new TextLabel("Quit");
		quit->setAction(new ExitGameAction());
	labels->addTextLabel(quit);

	rootWidget->addChild(labels);
}

Image *MainMenu::getBackground() const
{
	return &mainMenuBackground;
}

///////////////////////////////////////////////////////////////////////////

MapSelectionMenu::MapSelectionMenu()
{
	parseMaps();
}

MapSelectionMenu::~MapSelectionMenu()
{
}

void MapSelectionMenu::parseMaps()
{
	WIN32_FIND_DATA entry;
	HANDLE handle = FindFirstFile("maps//*.map", &entry);
	if (handle == INVALID_HANDLE_VALUE) return;
	mapFileNames.push_back(strdup(entry.cFileName));
	while(FindNextFile(handle,&entry))
		mapFileNames.push_back(strdup(entry.cFileName));
	
	FindClose(handle);

	for(int ii = 0; ii < mapFileNames.size(); ii++)
	{
		Parser* temp = new Parser((std::string("maps/")+mapFileNames[ii]).c_str());  
		int maxPlayers = temp->getSection("Players")->size();
		
		MapDesc desc;
		desc.filename = mapFileNames[ii];
		desc.maxPlayers = maxPlayers;
		mapDescs.push_back(desc);
	}
}

SinglePlayerMapMenu::SinglePlayerMapMenu()
{
}

SinglePlayerMapMenu::~SinglePlayerMapMenu()
{
}

void SinglePlayerMapMenu::setupWidgets()
{
	rootWidget->addChild(new TextLabel("Select Map", 150, 130, false, fontMenuHeading));
	
	ScrollPane<MapDesc> *maps = new ScrollPane<MapDesc>(100, 200, screenWidth-200, 350, mapDescs);
	rootWidget->addChild(maps);
	
	StartSinglePlayerAction *startGame = new StartSinglePlayerAction(maps);
	maps->setAction(startGame);
	
	TextLabel *okLabel = new TextLabel("OK", 300, 600, true);
	okLabel->setAction(startGame);
	
	TextLabel *backLabel = new TextLabel("Back", screenCenterX, 600, true);
	backLabel->setAction(new ChangeFrameAction(mainMenu));
	
	rootWidget->addChild(okLabel);
	rootWidget->addChild(backLabel);
}

MultiPlayerMapMenu::MultiPlayerMapMenu()
{
}

MultiPlayerMapMenu::~MultiPlayerMapMenu()
{
}

void MultiPlayerMapMenu::setupWidgets()
{
	rootWidget->addChild(new TextLabel("Host Game", 100, 100, false, fontMenuHeading));

	ScrollPane<MapDesc> *maps = new ScrollPane<MapDesc>(100, 250, 820, 300, mapDescs);
	rootWidget->addChild(maps);
	
	TextLabel *gameNameLabel = new TextLabel("Game Name", 100, 200, false, fontDefault);
	TextBox *gamename = new TextBox(250, 200, 200, "New Game");
	TextLabel *playerNameLabel = new TextLabel("Player Name", 500, 200, false, fontDefault);
	TextBox *playername = new TextBox(650, 200, 200, username);
	rootWidget->addChild(gamename);
	rootWidget->addChild(playername);
	rootWidget->addChild(gameNameLabel);
	rootWidget->addChild(playerNameLabel);
	
	ShowHostLobbyAction *next = new ShowHostLobbyAction(maps, gamename, playername);
	maps->setAction(next);
	
	TextLabel *okLabel = new TextLabel("Start", 300, 700, true);
	okLabel->setAction(next);
	
	TextLabel *backLabel = new TextLabel("Back", 450, 700, true);
	backLabel->setAction(new ChangeFrameAction(mainMenu));
	
	rootWidget->addChild(okLabel);
	rootWidget->addChild(backLabel);
}

///////////////////////////////////////////////////////////////////////////

CreditsScreen::CreditsScreen()
{
}

CreditsScreen::~CreditsScreen()
{
}

void CreditsScreen::setupWidgets()
{
	const float titleOffsetX = 370;
	const float roleOffsetX  = 125;
	const float nameOffsetX  = 220;

	rootWidget->addChild(new TextLabel("Programmers", roleOffsetX, 150.0f, false));
	TextLabelGroup *programmers = new TextLabelGroup(nameOffsetX, 200.0f, 50.0f, false);
		programmers->addTextLabel(new TextLabel("Ben Kalb - Project Leader"));
		programmers->addTextLabel(new TextLabel("Jim Babcock - Lead Programmer"));
		programmers->addTextLabel(new TextLabel("Chris Fellows - The Forgotten One"));
		programmers->addTextLabel(new TextLabel("Diego Asturias - Programmer"));
		programmers->addTextLabel(new TextLabel("Walter King - Map Editor Programmer"));
		programmers->addTextLabel(new TextLabel("Yi Xu - Network Programmer"));
	rootWidget->addChild(programmers);

	rootWidget->addChild(new TextLabel("Artists", roleOffsetX, 500.0f, false));
	TextLabelGroup *artists = new TextLabelGroup(nameOffsetX, 550.0f, 50.0f, false);
		artists->addTextLabel(new TextLabel("Jason Liu"));
		artists->addTextLabel(new TextLabel("Yuri Malitsky"));
	rootWidget->addChild(artists);
}

bool CreditsScreen::mouseDown(int x, int y, int button, int mod)
{
	Frame::changeFrame(mainMenu);
	return true;
}

bool CreditsScreen::keyDown(SDL_keysym sym)
{
	Frame::changeFrame(mainMenu);
	return true;
}

