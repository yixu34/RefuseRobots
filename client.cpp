#include "client.hpp"
#include "logger.hpp"
#include "util.hpp"
#include <cassert>

Client::Client(
	const std::string &ip, 
	SOCKET sockHandle, 
	short port)
	: NetworkNode(port, sockHandle)
{
	ipToConnect = ip;
}

Client::Client(short port)
	: NetworkNode(port, INVALID_SOCKET)
{
}

Client::~Client()
{
}

bool Client::connectTo()
{
	const char *ipStr = ipToConnect.c_str();
	if (ipStr == 0)
	{
		logger.log("Attempting to connect with null ip string.\n");
		return false;
	}

	// Create the socket handle
	socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socketHandle == INVALID_SOCKET)
	{
		return false;
	}

	ipAddress = inet_addr(ipStr);
	struct sockaddr_in sa;
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = ipAddress;
	sa.sin_port        = htons(portNumber);

	// Attempt to connect
	if (connect(socketHandle, reinterpret_cast<struct sockaddr *>(&sa), sizeof(sa)) == SOCKET_ERROR)
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