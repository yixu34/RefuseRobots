#ifndef TRACKER_HPP
#define TRACKER_HPP

#include <string>
#include <WinSock2.h>
#include "msvcfix.hpp"

struct ServerDesc
{
	std::string host;
	std::string name;
	
	std::string toString() { return name; }
};

class Tracker
{
public:
	Tracker();
	
	void registerServer(std::string name);
	void unregisterServer();
	void requestServerList();
	
	void timepass();
	bool failed();
	
	bool finishedServerList();
	std::vector<ServerDesc> getServers();
	
protected:
	bool makeConnection();
	void closeConnection();
	
	bool connected;
	SOCKET sock;
	SOCKADDR_IN connection;
	bool fail;
	
	std::string serverList;
	std::string pendingWrite, pendingRead;
	bool strippedHeaders;
	
	enum {
		actNone,
		actRegister,
		actUnregister,
		actGetList,
	} pendingAction;
	std::string serverName;
};

extern Tracker tracker;

#endif
