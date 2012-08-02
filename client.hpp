#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "networknode.hpp"
#include <string>

class Client : public NetworkNode
{
public:
	explicit Client(
		const std::string &ip, 
		SOCKET sockHandle = INVALID_SOCKET, 
		short port = NetworkNode::DEFAULT_PORT);

	explicit Client(short port = NetworkNode::DEFAULT_PORT);
	~Client();

	bool connectTo();

	NetNodeType getType() const { return NET_CLIENT; }

private:
	std::string ipToConnect;
};

#endif