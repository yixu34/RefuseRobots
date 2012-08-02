#include "util.hpp"
#include "networknode.hpp"
#include "packet.hpp"
#include "logger.hpp"
#include "command.hpp"
#include "main.hpp"
#include <cassert>

NetworkNode::NetworkNode(short port, SOCKET sockHandle)
{
	init(port, sockHandle);
}

NetworkNode::~NetworkNode()
{
}

void NetworkNode::init(short port, SOCKET sockHandle)
{
	memset(recvBuffer, 0, MAX_BUFFER_SIZE);

	socketHandle = sockHandle;
	sendOffset   = 0;
	recvBegin    = 0;
	recvEnd      = 0;
	ipAddress    = inet_addr("127.0.0.1");
	portNumber   = port;
	nodeId       = 0;
	isDisconnecting = false;
	username = "";
}

void NetworkNode::setNodeId(int id)
{
	nodeId = id;
}

int NetworkNode::getNodeId() const
{
	return nodeId;
}

void NetworkNode::disconnect()
{
	isDisconnecting = true;
	if(engine && engine->controller)
		engine->controller->disconnected(this);
	if(activeFrame)
		activeFrame->disconnected(this);
}

bool NetworkNode::hasOutgoing() const
{
	return !outgoingPackets.empty();
}

bool NetworkNode::hasIncoming() const
{
	return !incomingPackets.empty();
}

void NetworkNode::processOutgoing()
{
	while (!outgoingPackets.empty())
	{
		Packet *currentPacket = outgoingPackets.front();

		int packetSize   = currentPacket->getSize();
		int numBytesSent = send(
								socketHandle, 
								currentPacket->getData() + sendOffset, 
								packetSize - sendOffset, 
								0);

		logger.log("Sent %d bytes.\n", numBytesSent);
	    
		if (numBytesSent == packetSize)
		{
			// FIXME:  Multiple connected clients will try to delete
			// the outgoing packet!
			// Copying packets for now.
			sendOffset = 0;
			delete currentPacket;
			outgoingPackets.pop_front();
		}
		else if (numBytesSent < packetSize && numBytesSent >= 0)
			sendOffset += numBytesSent;
		else
		{
			//logger.log("KLAXON:  packet send error.  %d\n", WSAGetLastError());        
			outgoingPackets.pop_front();
		}
	} // while there are packets remaining
}

void NetworkNode::processIncoming()
{
	int numBytesRecv = recv(
							socketHandle, 
							recvBuffer + recvBegin + recvEnd, 
							MAX_BUFFER_SIZE - recvEnd, 
							0);
	
	if (numBytesRecv > 0)
		logger.log("Received %d bytes\n", numBytesRecv);

	// Sanity check on the number of bytes received
	if (numBytesRecv == 0)
		return;
	else if (numBytesRecv < 0)
	{
		// Try to handle disconnects gracefully
		int errorCode = WSAGetLastError();
		logger.log("recv err:  %d\n", errorCode);
		if (errorCode == WSAECONNRESET)
		{
			disconnect();

			// TODO:  Do server handoff.
			// FIXME: This should go back to the main menu, not exit
			if (getType() == NET_CLIENT)
			{
				shouldDestroyEngine = true;
				if(activeFrame != postMortem) {
					postMortem->set("You were disconnected.", true);

				Frame::changeFrame(postMortem);
				}
			}
		}
		
		//logger.stop();
		//disconnect();
		//assert(0);
		return;
	}

	int numUnprocessedBytes = recvEnd + numBytesRecv;
	int numProcessedBytes   = 0;
	bool wasPacketReceived  = false;

	while (numUnprocessedBytes > sizeof(int))
	{
		int packetSize = *(reinterpret_cast<int *>(recvBuffer + recvBegin));
		
		if (numUnprocessedBytes < packetSize)
			break;
		
		if (packetSize > MAX_PACKET_SIZE)
		{
			logger.log("Packet is too big!\n");
			EXIT_ASSERT;
			return;
		}
		else if (packetSize <= 0)
		{
			logger.log("Packet size %i is too small!\n  recvBegin=%i\n  numProcessedBytes=%i\n  numUnprocessedBytes=%i.\n",
					(int)packetSize, (int)recvBegin, (int)numProcessedBytes, (int)numUnprocessedBytes);
			return;
		}
		
		if(numUnprocessedBytes < packetSize)
			break;
		
		Packet *packet(NEW Packet(recvBuffer+recvBegin, packetSize));
		
		// Have remote clients push the packet onto the server's
		// incoming packet queue.
		//if (!isServer() && nodeId != 0)
		if (getType() == NET_REMOTE_CLIENT)
		{
			NetworkNode *theServer = network->getNode(0);
			theServer->incomingPackets.push_back(packet);
		}
		else
			incomingPackets.push_back(packet);

		wasPacketReceived = true;
		recvBegin += packetSize;
		numProcessedBytes += packetSize;
		numUnprocessedBytes -= packetSize;
	}// while enough bytes to read

	recvEnd = numUnprocessedBytes;

	if (wasPacketReceived)
	{
		if (recvEnd == 0)
			recvBegin = 0;
		else if (recvBegin + recvEnd + MAX_PACKET_SIZE > MAX_BUFFER_SIZE)
		{
			// Don't overrun the buffer!  Copy the left over bytes
			// to the beginning of the buffer
			memcpy(recvBuffer, recvBuffer+recvBegin, recvEnd);
			recvBegin = 0;
		}   
	}
}

void NetworkNode::sendPacket(Packet *packet)
{
	logTrafficOutgoing(packet);
	outgoingPackets.push_back(packet);
}
