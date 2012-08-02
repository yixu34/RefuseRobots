#include "network.hpp"
#include "packet.hpp"
#include "logger.hpp"
#include "networknode.hpp"
#include "main.hpp"
#include <cassert>

Network *network = 0;

Network::Network(NetworkNode *self)
{
	// If you are a server, then your nodeId is 0, and all remote
	// clients that connect to you have increasing nodeId's.
	// If you are a client and you connect to a server, then you are
	// the only network node that you know about, and so your nodeId is 0.
	nextNodeId = 0; 
	myself     = self;

	addConnection(self);
	nextPacketNodeIt = networkNodes.begin();
}

Network::~Network()
{
	// Process all remaining packets before dying!
	if (myself->hasOutgoing())
		myself->processOutgoing();
	if (myself->hasIncoming())
		myself->processIncoming();

	for (NetworkNodePool::iterator it = networkNodes.begin();
		 it != networkNodes.end();
		 it++)
	{
		delete it->second;
	}
}

void Network::timepass()
{
	setSockets();
	pollSockets();
	deleteSockets();
}

void Network::setSockets()
{
	FD_ZERO(&inputSet);
	FD_ZERO(&outputSet);
	FD_ZERO(&exceptionSet);
	tv.tv_sec  = 0;
	tv.tv_usec = 0;

	// Set up all sockets for select
	for (NetworkNodePool::iterator it = networkNodes.begin();
		 it != networkNodes.end();
		 ++it)
	{
		NetworkNode *currentNode    = it->second;
		const SOCKET currentSocketHandle = currentNode->socketHandle;

		// Skip invalid sockets or sockets that should be deleted
		if (currentNode->isDisconnecting || 
			currentSocketHandle == INVALID_SOCKET)
		{
			continue;
		}

		FD_SET(currentSocketHandle, &inputSet);
		FD_SET(currentSocketHandle, &exceptionSet);

		if (currentNode->hasOutgoing())
			FD_SET(currentSocketHandle, &outputSet);
	}

	// Try to poll all socket handles
	int selectionResult = select(
	                        0,  // numflds is ignored in WinSock 
	                        &inputSet, 
	                        &outputSet, 
	                        &exceptionSet, 
	                        &tv);

	if (selectionResult == SOCKET_ERROR)
//	    || selectionResult == 0)
	{
		logger.log("Selection result error:  %d\n", WSAGetLastError());
		return;
	}
}

void Network::pollSockets()
{
	// Handle input, output, and exceptions
	for (NetworkNodePool::iterator it = networkNodes.begin();
	     it != networkNodes.end();
	     ++it)
	{
		NetworkNode *currentNode    = it->second;
		const SOCKET currentSocketHandle = currentNode->socketHandle;

		if(currentNode->isDisconnecting ||
		   currentSocketHandle == INVALID_SOCKET)
		{
			continue;
		}

		if(FD_ISSET(currentSocketHandle, &exceptionSet))
			currentNode->disconnect();

		if(!currentNode->isDisconnecting && 
		   FD_ISSET(currentSocketHandle, &outputSet))
		{
			currentNode->processOutgoing();
		}

		if (!currentNode->isDisconnecting &&
		    FD_ISSET(currentSocketHandle, &inputSet))
		{
			currentNode->processIncoming();
		}
	}
}

void Network::deleteSockets()
{
	// Handle deleting any sockets
	for (NetworkNodePool::iterator it = networkNodes.begin();
		it != networkNodes.end();
		++it)
	{
		NetworkNode *currentNode = it->second;
		int nodeId = currentNode->nodeId;
		SOCKET nodeSocket = currentNode->socketHandle;

		if (currentNode->isDisconnecting)
		{
			if (nodeSocket != INVALID_SOCKET)
			{
				// Process all remaining packets before disconnecting.
				if (currentNode->hasIncoming())
					currentNode->processIncoming();
				if (currentNode->hasOutgoing())
					currentNode->processOutgoing();

				closesocket(nodeSocket);
			}

			currentNode->socketHandle = INVALID_SOCKET;
			closeConnection(nodeId);

			// Must return now, otherwise you'll try to delete this node again
			// in the next iteration.
			return;
		}
	}
}

int Network::addConnection(NetworkNode *node)
{
	node->nodeId = nextNodeId;
	networkNodes[nextNodeId] = node;
	nextNodeId++;
	nextPacketNodeIt = networkNodes.begin();

	return node->nodeId;
}

void Network::closeConnection(int nodeId)
{
	if (networkNodes.find(nodeId) == networkNodes.end())
		return;

	NetworkNode *nodeToDelete = networkNodes[nodeId];
	if (!nodeToDelete->isDisconnecting)
		nodeToDelete->disconnect();

	delete networkNodes[nodeId];
	networkNodes.erase(nodeId);
	nextPacketNodeIt = networkNodes.begin();
}

void Network::disconnectAll()
{
	for (NetworkNodePool::iterator it = networkNodes.begin();
		it != networkNodes.end();
		++it)
	{
		it->second->disconnect();
	}
	if(myself) {
		myself->disconnect(); // Weird as this looks...
	}
}

void Network::addBackupConnection(int nodeId, SOCKET socketHandle)
{
	BackupConnectionData backupConnection(nodeId, socketHandle);
	backupConnections.insert(backupConnection);
}

void Network::sendToServer(Packet *packet)
{
	sendPacket(packet, myself); // FIXME: 'myself' is an incorrect variable name
}

void Network::sendPacket(Packet *packet, NetworkNode *networkNode)
{
	if (networkNode == 0)
	{
		logger.log("KLAXON:  Sending packet when node is null.\n");
		return;
	}

	networkNode->sendPacket(packet);
}

void Network::sendToClients(Packet *packet)
{
	// FIXME:  Use shared pointers to prevent this packet copying hee-haw
	for (NetworkNodePool::iterator it = networkNodes.begin();
		 it != networkNodes.end();
		 it++)
	{
		NetworkNode *currentNode = it->second;
		if (currentNode == myself)	// Don't send to self
			continue;

		currentNode->sendPacket(packet->clone());
	}
}

void Network::sendToAll(Packet *packet)
{
	if (myself->getType() == NetworkNode::NET_SERVER)
		sendToClients(packet);
	else
		sendToServer(packet);
}

void Network::sendToAllBut(Packet *packet, int nodeId)
{
	// FIXME:  Use shared pointers to prevent this packet copying hee-haw
	for (NetworkNodePool::iterator it = networkNodes.begin();
		 it != networkNodes.end();
		 it++)
	{
		NetworkNode *currentNode = it->second;
		if (currentNode == myself)	// Don't send to self
			continue;
		if(currentNode->getNodeId() == nodeId) // Don't send to excluded
			continue;
		
		currentNode->sendPacket(packet->clone());
	}
}

void Network::sendTo(Packet *packet, int nodeId)
{
	if (networkNodes.find(nodeId) == networkNodes.end())
		return;

	NetworkNode *node = networkNodes[nodeId];
	node->sendPacket(packet);
}

Packet *Network::getNextPacket()
{
	// Retrieve incoming packets from all network nodes on the network.
	// If you are client, then you only need to read your own packets.
	// If you are a server, then you need to read packets from EACH client.

	assert(!networkNodes.empty());
	if (nextPacketNodeIt == networkNodes.end())
		nextPacketNodeIt = networkNodes.begin();

	// Keep looking for network nodes until you find one that:
	// 1) is not NULL
	// 2) is not set for deletion
	// 3) is not past the last node
	// 4) has incoming packets
	NetworkNode *currentNode = nextPacketNodeIt->second;
	bool shouldSkip = currentNode == 0 || 
		currentNode->isDisconnecting || 
		!currentNode->hasIncoming();

	while (shouldSkip && nextPacketNodeIt != networkNodes.end())
	{
		nextPacketNodeIt++;
	}

	// Went to the end and couldn't find a valid node; return null.
	if (nextPacketNodeIt == networkNodes.end())
		return 0;

	if (currentNode->hasIncoming())
	{
		Packet *frontPacket = currentNode->incomingPackets.front();
		currentNode->incomingPackets.pop_front();
		logTrafficIncoming(frontPacket);
		return frontPacket;
	}
	else
		return 0;
}

NetworkNode *Network::getNode(int nodeId)
{
	if (networkNodes.find(nodeId) == networkNodes.end())
		return 0;
	else
		return networkNodes[nodeId];
}

NetworkNode *Network::getSelf() const
{
	return myself;
}

int Network::getNumNodes() const
{
	return networkNodes.size();
}

int Network::getLastNodeId() const
{
	return nextNodeId - 1;
}

void Network::startGame()
{
	Packet *startSignal= new Packet(msg_start_game);
	sendToAll(startSignal);

	assert(engine && engine->controller != 0);
	for (NetworkNodePool::iterator it = networkNodes.begin();
		 it != networkNodes.end();
		 it++)
	{
		if (it->second != myself)
			(static_cast<ServerController *>(engine->controller))->addPlayer(it->first);
	}
}

Network::BackupConnectionData::BackupConnectionData(int backupId, SOCKET backupHandle)
{
	nodeId       = backupId;
	socketHandle = backupHandle;
}

Network::BackupConnectionData::~BackupConnectionData()
{
}

bool Network::BackupConnectionData::operator<(const Network::BackupConnectionData &rhs) const
{
	return nodeId < rhs.nodeId;
}

void initNetwork()
{
	WSAData sockData;
	if (WSAStartup(MAKEWORD(2, 2), &sockData) == SOCKET_ERROR)
	{
		EXIT_ASSERT;
		return;
	}
	atexit(shutDownNetwork);
}

void shutDownNetwork()
{
	WSACleanup();
}
