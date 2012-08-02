#include "lobby.hpp"
#include "text.hpp"
#include "widget.hpp"
#include "widgetaction.hpp"
#include "menu.hpp"
#include "tracker.hpp"
#include <cassert>
#include <fstream>

HostLobby *hostLobby     = 0;
ClientLobby *clientLobby = 0;
JoinFrame *joinFrame     = 0;

Lobby::Lobby(): messages(7, 0, 0, 715, chatFont, true, false)
{
	messages.setTimeout(20.0);
}

Lobby::~Lobby()
{
	messages.clear();
}

void Lobby::timepass()
{
	processPackets();
}

const char *Lobby::getMapName()
{
	return mapName.c_str();
}

void Lobby::removePlayer(const std::string &name)
{
	if (players.find(name) != players.end())
		players.erase(name);
}

std::string Lobby::modifiedPlayerName(const std::string &name)
{
	// Modify the name if one already exists, and tell the client
	// to change his name locally.
	std::string nameToAdd = name;
	/*if (players.find(name) != players.end())
	{
		char buf[255];
		int newNameSuffix = ++players[name];
		nameToAdd += itoa(newNameSuffix, buf, 10);
	}*/

	return nameToAdd;
}

void Lobby::setupWidgets(bool echoChat)
{
	TextBox *textInput = new TextBox(7, 730, 600, "");
		textInput->setAction(new SendChatTextAction(textInput, echoChat?(&messages):NULL, &username));
	rootWidget->addChild(textInput);

	TextDisplayWidget *textDisplay = new TextDisplayWidget(7, 0, 0, 715, fontMiddle, true, &messages);
	rootWidget->addChild(textDisplay);
}

//////////////////////////////////////////////////////////////////////////

HostLobby::HostLobby(const std::string &map, unsigned maxNumPlayers)
{
	set(map, maxNumPlayers, "DefaultName");
	addPlayer(username);
}

HostLobby::~HostLobby()
{
}

void HostLobby::set(const std::string &map, unsigned maxNumPlayers, std::string name)
{
	maxPlayers = maxNumPlayers;
	mapName    = map;
	gameName   = name;
	numPlayers = 1;
}

void HostLobby::onEnable()
{
	tracker.registerServer(gameName);
}

void HostLobby::onDisable()
{
	tracker.unregisterServer();
}

bool HostLobby::isFull() const
{
	return numPlayers >= maxPlayers;
}

void HostLobby::tellOthersAboutNewPlayer(
	const std::string &nameToAdd, 
	int newestPlayerNode)
{
	Packet *newPlayerPacket = new Packet(msg_join_lobby);
		newPlayerPacket->putString(nameToAdd);

	for (Network::NetworkNodePool::iterator it = network->networkNodes.begin();
		it != network->networkNodes.end();
		it++)
	{
		if (it->first == newestPlayerNode)
			continue;

		network->sendTo(newPlayerPacket->clone(), it->first);
	}
	delete newPlayerPacket;
}

void HostLobby::tellNewPlayerAboutOthers(int newestPlayerNode)
{
	Packet *otherPlayerData = new Packet(msg_other_player_data);
	otherPlayerData->putInt(players.size());
	PlayerPool::iterator playerIt = players.begin();
	Network::NetworkNodePool::iterator nodeIt = network->networkNodes.begin();

	for ( ; playerIt != players.end() && nodeIt != network->networkNodes.end();
		playerIt++, nodeIt++)
	{
		// Don't tell the new player his own info!  It's for his own good!
		if (nodeIt->first == newestPlayerNode)
			continue;

		NetworkNode *currNode = nodeIt->second;
		int currNodeId        = currNode->nodeId;
		int currSockHandle    = currNode->socketHandle;
		std::string currName  = playerIt->first;

		otherPlayerData->putString(currName);
		otherPlayerData->putInt(currNodeId);
		otherPlayerData->putInt(currSockHandle);
	}
	network->sendTo(otherPlayerData, newestPlayerNode);
}

void HostLobby::addPlayer(const std::string &name)
{
	numPlayers++;
	
	// Modify the name if one already exists, and tell the client
	// to change his name locally.
	std::string nameToAdd = modifiedPlayerName(name);

	// We have some more work to do if we're the host...
	// Reject the player if the game is full
	if (players.size() >= maxPlayers)
		return;
	    
	// 1) Tell everyone BUT the new player about the new player.
	const int newestPlayerNode = network->getLastNodeId();
	tellOthersAboutNewPlayer(nameToAdd, newestPlayerNode);
	
	// 2) Tell the new player about everyone else.
	tellNewPlayerAboutOthers(newestPlayerNode);
	
	// 3) Tell the new player to change his name.
	Packet *changeName = new Packet(msg_change_name);
		changeName->putString(nameToAdd);
	network->sendTo(changeName, newestPlayerNode);
	
	NetworkNode *node = network->getNode(newestPlayerNode);
	node->username = nameToAdd;
	
	// 4) Tell the new player his nodeId
	Packet *assignNodeId = new Packet(msg_assign_nodeid);
		assignNodeId->putInt(newestPlayerNode);
	network->sendTo(assignNodeId, newestPlayerNode);

	// 5) Tell the new player the map name
	Packet *mapData = new Packet(msg_map_name);
		mapData->putString(mapName);
	network->sendTo(mapData, newestPlayerNode);

	// Now actually put it into your own list
	players[nameToAdd] = 0;
	
	messages.println(retprintf("%s has joined the game.", nameToAdd.c_str()));
}

void HostLobby::removePlayer(const std::string &name)
{
	Lobby::removePlayer(name);
	numPlayers--;
	
	messages.println(retprintf("%s has joined the game.", name.c_str()));

	int unusedNodeId = -1;
	Packet *dropPlayerPacket = new Packet(msg_disconnect);
		dropPlayerPacket->putInt(unusedNodeId);
		dropPlayerPacket->putString(name);
	network->sendToAll(dropPlayerPacket);
}

void HostLobby::processPackets()
{
	Packet *packet = network->getNextPacket();
	while(packet != NULL)
	{
		int packetType = packet->getInt();
		switch(packetType)
		{
			// These are all received by the host.
			case msg_lobby_chat: {
				std::string chatText   = packet->getString();
				std::string playerName = packet->getString();
				messages.println(formatChatText(playerName, chatText));

				// Now forward to clients...
				Packet *chatPacket = new Packet(msg_lobby_chat);
					chatPacket->putString(chatText);
					chatPacket->putString(playerName);
				network->sendToAll(chatPacket);
				break;
			}
			case msg_join_lobby: {
				if (!isFull())
				{
					std::string name = packet->getString();
					addPlayer(name);
				}
				else
				{
					sendMessage("A player tried to join, but the game is full.");
					int destNodeId = network->getLastNodeId();
					Packet *joinFailed = new Packet(msg_join_failed);
						network->sendTo(joinFailed, destNodeId);
					network->getNode(destNodeId)->disconnect();
				}
				break;
			}
			
			case msg_disconnect: {
				int nodeId       = packet->getInt();
				std::string name = packet->getString();
				removePlayer(name);
				network->getNode(nodeId)->disconnect();
				
				sendMessage(name + " has left the game.");
				break;
			}

			default:
				logger.log("Lobby host unhandled msg:  %d\n", packetType);
				break;
		}
		delete packet;
		packet = network->getNextPacket();
	}
}

void HostLobby::sendMessage(std::string message)
{
	messages.println(message);
	Packet *chatPacket = new Packet(msg_lobby_chat);
		chatPacket->putString(message);
		chatPacket->putString("");
	network->sendToAll(chatPacket);
}

void HostLobby::setupWidgets()
{
	Lobby::setupWidgets(true);

	int startWidth;
	int startHeight;
	TTF_SizeText(fontMiddle.font, "Start", &startWidth, &startHeight);

	int cancelWidth;
	int cancelHeight;
	TTF_SizeText(fontMiddle.font, "Cancel", &cancelWidth, &cancelHeight);

	// Cancel
	TextLabel *cancel = new TextLabel("Cancel", screenWidth - cancelWidth, screenHeight - cancelHeight, false);
		cancel->setAction(new ChangeFrameAction(mainMenu));
	rootWidget->addChild(cancel);

	// Start
	const int buttonSpacing = 25;
	TextLabel *start = new TextLabel("Start", screenWidth - cancelWidth - startWidth - buttonSpacing, screenHeight - cancelHeight, false);
		start->setAction(new StartMultiPlayerAction(username));
	rootWidget->addChild(start);
}

void HostLobby::disconnected(NetworkNode *node)
{
	// TODO: Handle unclean disconnects
}

//////////////////////////////////////////////////////////////////////////

ClientLobby::ClientLobby()
{
	// Send a join lobby msg to the host.
	/*Packet *joinPacket = new Packet(msg_join_lobby);
		joinPacket->putString(username);
	network->sendToServer(joinPacket);*/
}
ClientLobby::~ClientLobby()
{
	// In the lobby, want to quit.
	/*Packet *leaveLobby = new Packet(msg_disconnect);
		leaveLobby->putInt(network->getSelf()->getNodeId());
		leaveLobby->putString(username);
	network->sendToServer(leaveLobby);*/
}

void ClientLobby::addPlayer(const std::string &name)
{
	std::string nameToAdd = modifiedPlayerName(name);
	players[nameToAdd] = 0;

	messages.println(nameToAdd + " has joined the game.");
}

void ClientLobby::setupWidgets()
{
	Lobby::setupWidgets(false);

	int cancelWidth;
	int cancelHeight;
	TTF_SizeText(fontMiddle.font, "Cancel", &cancelWidth, &cancelHeight);

	// Cancel
	// TODO:  Send disconnect packet.
	TextLabel *cancel = new TextLabel("Cancel", screenWidth - cancelWidth, screenHeight - cancelHeight, false);
		//cancel->setAction(new ChangeFrameAction(mainMenu));
		cancel->setAction(new LeaveLobbyAction());
	rootWidget->addChild(cancel);
}

void LeaveLobbyAction::activateSelf()
{
	Packet *discMessage = new Packet(msg_disconnect);
		discMessage->putInt(network->getSelf()->getNodeId());
		discMessage->putString(username);
	network->sendToServer(discMessage);
	
	activeFrame->disable();
	activeFrame = NULL;
	network->disconnectAll();
	activeFrame = mainMenu;
	activeFrame->enable();
}

bool ClientLobby::isFull() const
{
	return false;
}

void ClientLobby::processPackets()
{
	Packet *packet = network->getNextPacket();
	while(packet != NULL)
	{
		int packetType = packet->getInt();
		switch(packetType)
		{
			//
			// These are all from the client's perspective.
			//
			case msg_start_game:
				engine = NEW Engine(mapName.c_str());
				engine->controller = new ClientController(engine->model, engine->commands, engine->messages);
				engine->view->setPlayerName(username);
				messages.clear();
				Frame::changeFrame(0);
				return;
				
			case msg_lobby_chat: {
				std::string chatText   = packet->getString();
				std::string playerName = packet->getString();
				
				messages.println(formatChatText(playerName, chatText));
				break;
			}

			case msg_other_player_data: {
				int numOtherPlayers = packet->getInt();
				for (int i = 0; i < numOtherPlayers; i++)
				{
					std::string currPlayer = packet->getString();
					int currNodeId         = packet->getInt();
					int currSocketHandle   = packet->getInt();

					addPlayer(currPlayer);
					network->addBackupConnection(currNodeId, currSocketHandle);
				}
				break;
			}

			case msg_join_lobby: {
				std::string name = packet->getString();
				addPlayer(name);
				break;
			}

			case msg_disconnect: {
				int unusedNodeId = packet->getInt();
				std::string name = packet->getString();
				removePlayer(name);
				messages.println(name + " has left the game.");
				break;
			}

			case msg_join_failed:
				// Fall back to the main menu
				//disableEvents = true;
				//showMainMenu();
				/*delete chatMessage;
				chatMessage = 0;*/
				//connected = false;
				//return disableEvents;
				Frame::changeFrame(joinFrame);
				return;

			case msg_change_name: {
				std::string newName = packet->getString();
				oldUsername = username;
				username = newName;
				addPlayer(username);
				break;
			}

			case msg_assign_nodeid: {
				int nodeId = packet->getInt();
				network->getSelf()->setNodeId(nodeId);
				break;
			}

			case msg_map_name: {
				mapName = packet->getString();
				break;
			}

			default:
				logger.log("Lobby join unhandled msg:  %d\n", packetType);
				break;
		}
		delete packet;
		packet = network->getNextPacket();
	}
	
	glColor3f(1.0,1.0,1.0);
}

void ClientLobby::disconnected(NetworkNode *node)
{
	Frame::changeFrame(joinFrame);
}

//////////////////////////////////////////////////////////////////////////

JoinFrame::JoinFrame()
{
}

JoinFrame::~JoinFrame()
{
}

void JoinFrame::onEnable()
{
	tracker.requestServerList();
}

void JoinFrame::setupWidgets()
{
	rootWidget->addChild(new TextLabel("Join Game", 100, 100, false, fontMenuHeading));
	
	std::vector<ServerDesc> emptylist;
	serverview = new ScrollPane<ServerDesc>(100, 250, 820, 300, emptylist);
	rootWidget->addChild(serverview);
	
	TextLabel *playerNameLabel = new TextLabel("Player Name", 100, 200, false, fontDefault);
	TextBox *playername = new TextBox(250, 200, 200, username);
	rootWidget->addChild(playerNameLabel);
	rootWidget->addChild(playername);
	
	TextLabel *refresh = new TextLabel("Refresh", 100, 700, false);
	refresh->setAction(new RefreshListAction());
	rootWidget->addChild(refresh);
	
	TextLabel *cancel = new TextLabel("Cancel", 300, 700, false);
	cancel->setAction(new ChangeFrameAction(mainMenu));
	rootWidget->addChild(cancel);
	
	ConnectToServerAction *action = new ConnectToServerAction(serverview, playername);
	serverview->setAction(action);
}

void JoinFrame::timepass()
{
	std::vector<ServerDesc> serverdescs = tracker.getServers();
	serverview->setElements(serverdescs);
}

//////////////////////////////////////////////////////////////////////////
