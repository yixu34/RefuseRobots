#include "server.hpp"
//#include "client.hpp"
#include "remoteclient.hpp"

#include "main.hpp"
#include "packet.hpp"
#include <string>
#include <cassert>

Server::Server(short port, SOCKET sockHandle)
	: NetworkNode(port, sockHandle)
{
	if (!listenOn())
		EXIT_ASSERT;
}

Server::~Server()
{
}

void Server::processIncoming()
{
	unsigned int ipAddr;
	SOCKET clientSocket = acceptConnection(&ipAddr);

	int dummyLength = 1;
	setsockopt(
		clientSocket, 
		SOL_SOCKET, 
		SO_DONTLINGER, 
		reinterpret_cast<char *>(&dummyLength), 
		sizeof(dummyLength));

	// If a client is trying to connect, add him to the game.
	if (clientSocket != INVALID_SOCKET)
		addClient(ipAddr, clientSocket);

	NetworkNode::processIncoming();
}

void Server::addClient(unsigned int ipAddr, SOCKET clientSocket)
{
	struct in_addr addr;
	addr.S_un.S_addr = ntohl(ipAddr);
	char ipStr[128];
	strcpy(ipStr, inet_ntoa(addr));

	RemoteClient *remoteClient = NEW RemoteClient(clientSocket);
	network->addConnection(remoteClient);
}

int Server::getNumClients() const
{
	// Don't count yourself as a client.
	return network->getNumNodes() - 1;
}

bool Server::listenOn()
{
	struct sockaddr_in sa;
	int dummyLength = 1;

	// Create socket handle
	socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketHandle == INVALID_SOCKET)
		return false;

	// Reuse server socket addresses even if busy
	if (setsockopt(
			socketHandle, 
			SOL_SOCKET, 
			SO_REUSEADDR, 
			reinterpret_cast<char *>(&dummyLength), 
			sizeof(dummyLength)) == SOCKET_ERROR)
	{
		closesocket(socketHandle);
		socketHandle = INVALID_SOCKET;
		return false;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	sa.sin_port        = htons(portNumber);

	// Bind the socket
	if (bind(socketHandle, reinterpret_cast<struct sockaddr *>(&sa), sizeof(sa)) == SOCKET_ERROR)
	{
		closesocket(socketHandle);
		socketHandle = INVALID_SOCKET;
		return false;
	}

	// Start listening
	if (listen(socketHandle, 256) == SOCKET_ERROR)
	{
		closesocket(socketHandle);
		socketHandle = INVALID_SOCKET;
		return false;
	}

	// Set socket to non-blocking
	unsigned long val = 1;
	if (ioctlsocket(socketHandle, FIONBIO, &val) == SOCKET_ERROR)
	{
		closesocket(socketHandle);
		socketHandle = INVALID_SOCKET;
		return false;
	}

	return true;
}

SOCKET Server::acceptConnection(unsigned int *ipData)
{
	SOCKET newSocketHandle;
	struct sockaddr_in sa;
	int socketSize = sizeof(sa);

	newSocketHandle = accept(
						socketHandle, 
						reinterpret_cast<struct sockaddr *>(&sa), 
						&socketSize);

	if (newSocketHandle == INVALID_SOCKET)
		return INVALID_SOCKET;

	if (getpeername(
			newSocketHandle, 
			reinterpret_cast<struct sockaddr *>(&sa), 
			&socketSize) == SOCKET_ERROR)
	{
		closesocket(newSocketHandle);
		return INVALID_SOCKET;
	}

	*ipData = ntohl(sa.sin_addr.s_addr);
	
	return newSocketHandle;
}