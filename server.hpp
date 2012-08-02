#ifndef SERVER_HPP
#define SERVER_HPP

#include "networknode.hpp"
#include "controller.hpp"

class Server : public NetworkNode
{
public:
	explicit Server(
		short port = NetworkNode::DEFAULT_PORT, 
		SOCKET sockHandle = INVALID_SOCKET);
	~Server();

	void processIncoming();
	int getNumClients() const;

	NetNodeType getType() const { return NET_SERVER; }

private:
	void addClient(unsigned int ipAddr, SOCKET clientSocket);
	bool listenOn();
	SOCKET acceptConnection(unsigned int *ipData);
};

#endif
