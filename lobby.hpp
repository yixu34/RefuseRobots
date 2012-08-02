#ifndef LOBBY_HPP
#define LOBBY_HPP

#include <map>
#include <string>
#include "screenview.hpp"
#include "main.hpp"
#include "frame.hpp"
#include "tracker.hpp"

// Keeps track of all of the players in the chat lobby before a game.

class Lobby:public Frame
{
public:
	Lobby();
	virtual ~Lobby();
	
	virtual bool isFull() const = 0;
	virtual void addPlayer(const std::string &name) = 0;
	virtual void removePlayer(const std::string &name);
	virtual void timepass();

	const char *getMapName();

	void setupWidgets(bool echoChat);

protected:
	std::string modifiedPlayerName(const std::string &name);
	void processChat();
	virtual void processPackets() = 0;

	// If a player joins and someone else already has his name, 
	// then the new player gets an number appended to his name.
	typedef std::map<std::string, int> PlayerPool;
	PlayerPool players;

	unsigned maxPlayers;
	std::string mapName;
	TextDisplay messages;
};

class HostLobby:public Lobby
{
public:
	HostLobby(const std::string &map, unsigned maxNumPlayers);
	~HostLobby();
	
	void onEnable();
	void onDisable();
	
	void set(const std::string &map, unsigned maxNumPlayers, std::string name);

	bool isFull() const;
	void addPlayer(const std::string &name);
	void removePlayer(const std::string &name);
	void disconnected(NetworkNode *node);

	void setupWidgets();

private:
	void processPackets();
	void sendMessage(std::string message);

	void tellOthersAboutNewPlayer(
		const std::string &nameToAdd, 
		int newestPlayerNode);

	void tellNewPlayerAboutOthers(int newestPlayerNode);
	
	std::string gameName;
	int numPlayers;
};

class ClientLobby:public Lobby
{
public:
	ClientLobby();
	~ClientLobby();
	
	bool isFull() const;
	void addPlayer(const std::string &name);
	void disconnected(NetworkNode *node);

	void setupWidgets();

private:
	void processPackets();
};

template<class T> class ScrollPane;

class JoinFrame:public Frame
{
public:
	JoinFrame();
	~JoinFrame();
	
	void onEnable();
	void setupWidgets();
	void timepass();
	
private:
	ScrollPane<ServerDesc> *serverview;
};

extern HostLobby *hostLobby;
extern ClientLobby *clientLobby;
extern JoinFrame *joinFrame;

#endif
