#include "widgetaction.hpp"
#include "widget.hpp"
#include "menu.hpp"
#include "main.hpp"
#include "lobby.hpp"

#include <string>

WidgetAction::WidgetAction()
{
}

WidgetAction::~WidgetAction()
{
}

void WidgetAction::enqueueAction(WidgetAction *action)
{
	nextActions.push_back(action);
}

void WidgetAction::activate()
{
	activateSelf();
	for (ActionSequence::iterator it = nextActions.begin(); it != nextActions.end(); it++)
		(*it)->activate();
}

//////////////////////////////////////////////////////////////////////////

TestAction::TestAction()
{
}

TestAction::~TestAction()
{
}

void TestAction::activateSelf()
{
	exit(0);
}

//////////////////////////////////////////////////////////////////////////

ChangeFrameAction::ChangeFrameAction(Frame *newFrame)
{
	nextFrame = newFrame;
}

ChangeFrameAction::~ChangeFrameAction()
{
}

void ChangeFrameAction::activateSelf()
{
	Frame::changeFrame(nextFrame);
}

//////////////////////////////////////////////////////////////////////////

ExitGameAction::ExitGameAction()
{
}

ExitGameAction::~ExitGameAction()
{
}

void ExitGameAction::activateSelf()
{
	exit(0);
}

//////////////////////////////////////////////////////////////////////////

StartSinglePlayerAction::StartSinglePlayerAction(ScrollPane<MapDesc> *mapselect)
	: mapselect(mapselect)
{
}

StartSinglePlayerAction::~StartSinglePlayerAction()
{
}

void StartSinglePlayerAction::activateSelf()
{
	if(!mapselect->haveSelection())
		return;
	std::string mapName = mapselect->getSelectedElement().filename;
	int mapNumPlayers = mapselect->getSelectedElement().maxPlayers;
	
	logger.start("server.log");
	engine = NEW Engine(mapName.c_str());
	ServerController *controller = new ServerController(engine->model, engine->commands, engine->messages);
	engine->controller = controller;
	controller->addPlayer(-1, username.c_str());
	engine->view->setPlayerName(username);
	
	for(int x = 0; x < mapNumPlayers-1; x++) {
		engine->aiPlayers.push_back(new AIPlayer(engine->model, engine->commands,
			controller->addPlayer(-2, "Computer")));
	}
	
	network = 0;
	Frame::changeFrame(NULL);
}

//////////////////////////////////////////////////////////////////////////

StartMultiPlayerAction::StartMultiPlayerAction(const std::string &name)
{
	playerName = name;
}

StartMultiPlayerAction::~StartMultiPlayerAction()
{
}

void StartMultiPlayerAction::activateSelf()
{
	engine = NEW Engine(hostLobby->getMapName());
	ServerController *controller = new ServerController(engine->model, engine->commands, engine->messages);
	engine->controller = controller;
	controller->addPlayer(-1, playerName.c_str());
	engine->view->setPlayerName(playerName);
	network->startGame();
	
	Frame::changeFrame(NULL);
}

//////////////////////////////////////////////////////////////////////////

ShowHostLobbyAction::ShowHostLobbyAction(ScrollPane<MapDesc> *pane, TextBox *gamename, TextBox *playername)
{
	mapselect = pane;
	name = gamename;
	user = playername;
}

ShowHostLobbyAction::~ShowHostLobbyAction()
{
}

void ShowHostLobbyAction::activateSelf()
{
	if (network == 0)
		network = new Network(new Server());
	if(!mapselect->haveSelection())
		return;
	
	std::string mapName = mapselect->getSelectedElement().filename;
	int mapNumPlayers = mapselect->getSelectedElement().maxPlayers;
	
	std::string gameName = name->getText();
	setUsername(user->getText());
	
	hostLobby->set(mapName, mapNumPlayers, gameName);
	
	Frame::changeFrame(hostLobby);
	
	logger.start("server.log");
}

//////////////////////////////////////////////////////////////////////////

SendChatTextAction::SendChatTextAction(
	TextBox *input, 
	TextDisplay *display, 
	std::string *name)
{
	textInput   = input;
	textDisplay = display;
	playerName  = name;
}

SendChatTextAction::~SendChatTextAction()
{
}

void SendChatTextAction::activateSelf()
{
	std::string text = textInput->getText();
	textInput->reset();
	
	if(textDisplay)
		textDisplay->println(formatChatText(*playerName, text));

	Packet *textPacket = new Packet(msg_lobby_chat);
		textPacket->putString(text);
		textPacket->putString(*playerName);
	network->sendToAll(textPacket);
}

//////////////////////////////////////////////////////////////////////////

ConnectToServerAction::ConnectToServerAction(ScrollPane<ServerDesc> *serverlist, TextBox *playername)
	: serverlist(serverlist), playername(playername) { }

ConnectToServerAction::~ConnectToServerAction() { }

void ConnectToServerAction::activateSelf()
{
	if(!serverlist->haveSelection())
		return;
	ServerDesc server = serverlist->getSelectedElement();
	
	setUsername(playername->getText());
	
	Client *client = new Client(server.host);
	
	delete network;
	network = new Network(client);
	if (!client->connectTo())
	{
		delete network;
		network = NULL;
		return;
	}
	
	logger.start("client.log");

	Frame::changeFrame(clientLobby);

	// Tell the server that you want to join.
	Packet *joinPacket = new Packet(msg_join_lobby);
		joinPacket->putString(username);
	network->sendToServer(joinPacket);
}

//////////////////////////////////////////////////////////////////////////

ConnectToManualServerAction::ConnectToManualServerAction(TextInputWidget *input)
{
	textInput = input;
}

ConnectToManualServerAction::~ConnectToManualServerAction()
{
}

void ConnectToManualServerAction::activateSelf()
{
	std::string ipString = textInput->getEnteredText();
	if (ipString == "")
		ipString = "127.0.0.1";
	
	Client *client = new Client(ipString);
	delete network;
	network = new Network(client);
	if (!client->connectTo())
	{
		delete network;
		network = NULL;
		return;
	}

	logger.start("client.log");

	Frame::changeFrame(clientLobby);

	// Tell the server that you want to join.
	Packet *joinPacket = new Packet(msg_join_lobby);
		joinPacket->putString(username);
	network->sendToServer(joinPacket);
}

//////////////////////////////////////////////////////////////////////////

void RefreshListAction::activateSelf()
{
	tracker.requestServerList();
}

//////////////////////////////////////////////////////////////////////////

void setUsername(std::string name)
{
	username = name;
}
