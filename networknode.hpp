#ifndef NETWORKNODE_HPP
#define NETWORKNODE_HPP

#include <WinSock2.h>
#include <list>
#include "msvcfix.hpp"

class Command;
class Network;
class Packet;

// Represents a connection (socket) to another computer over the network.
// When a network node sends a packet, it means that it sends the packet
// to the destination end of this connection, not to the network node itself.

class NetworkNode
{
public:
	NetworkNode(short port, SOCKET sockHandle);
	virtual ~NetworkNode();

	void disconnect();
	bool hasOutgoing() const;
	bool hasIncoming() const;

	void setNodeId(int id);
	int getNodeId() const;

	enum NetNodeType
	{
		NET_SERVER, 
		NET_CLIENT, 
		NET_REMOTE_CLIENT
	};

	virtual NetNodeType getType() const = 0;
	
	std::string username;

protected:
	friend class Network;
	friend class HostLobby;

	void init(short port, SOCKET sockHandle);
	
	void sendPacket(Packet *packet);
	virtual void processOutgoing();
	virtual void processIncoming();
	
	typedef std::list<Packet *> PacketPool;
	PacketPool incomingPackets;
	PacketPool outgoingPackets;

	unsigned short portNumber;
	unsigned long ipAddress;
	SOCKET socketHandle;
	int nodeId;

	static const int MAX_PACKET_SIZE = 512;
	static const int MAX_BUFFER_SIZE = 512 * MAX_PACKET_SIZE;
	static const short DEFAULT_PORT  = 27000;

	char recvBuffer[MAX_BUFFER_SIZE];
	
	int recvBegin;
	int recvEnd;    
	int sendOffset;
	bool isDisconnecting;
};

#endif