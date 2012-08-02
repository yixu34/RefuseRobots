#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <map>
#include <set>
#include <WinSock2.h>
#include "msvcfix.hpp"

class Packet;
class HostLobby;
class ClientLobby;
class NetworkNode;

// Just used temporarily for debugging.
class ScreenView;

// Processes network traffic and keeps track of all connections
// to other computers.

class Network
{
public:
	Network(NetworkNode *self);
	~Network();

	void timepass();
	int addConnection(NetworkNode *networkNode);
	void closeConnection(int nodeId);
	void disconnectAll();

	void addBackupConnection(int nodeId, SOCKET socketHandle);

	// Called in a ClientController context
	void sendToServer(Packet *packet);

	// Called in a ServerController context
	void sendToAll(Packet *packet);
	
	void sendToAllBut(Packet *packet, int nodeId);

	// Called in a ServerController context
	void sendTo(Packet *packet, int nodeId);

	Packet *getNextPacket();
	NetworkNode *getNode(int nodeId);
	NetworkNode *getSelf() const;
	int getNumNodes() const;
	int getLastNodeId() const;

	void startGame();

private:
	friend HostLobby;
	friend ClientLobby;

	// TEST:  Let the ScreenView print out debug data.  Can't have TOO many friends...
	friend ScreenView;

	void sendToClients(Packet *packet);

	void sendPacket(Packet *Packet, NetworkNode *networkNode);
	void setSockets();
	void pollSockets();
	void deleteSockets();

	typedef std::map<int, NetworkNode *> NetworkNodePool;
	NetworkNodePool networkNodes;
	NetworkNodePool::iterator nextPacketNodeIt;
	NetworkNode *myself;

	class BackupConnectionData
	{
	public:
		BackupConnectionData(int backupId, SOCKET backupHandle);
		~BackupConnectionData();

		bool operator<(const BackupConnectionData &rhs) const;

		int nodeId;
		SOCKET socketHandle;
	};

	typedef std::set<BackupConnectionData> BackupConnectionPool;
	BackupConnectionPool backupConnections;

	int nextNodeId;

	// Timepass selection data
	fd_set inputSet;
	fd_set outputSet;
	fd_set exceptionSet;
	timeval tv;
};

void initNetwork();
void shutDownNetwork();

#endif	//NETWORK_HPP