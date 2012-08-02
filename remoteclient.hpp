#ifndef REMOTECLIENT_HPP
#define REMOTECLIENT_HPP

#include "networknode.hpp"

class RemoteClient : public NetworkNode
{
public:
	explicit RemoteClient(
		SOCKET sockHandle = INVALID_SOCKET, 
		short port = NetworkNode::DEFAULT_PORT);

	~RemoteClient();

	NetNodeType getType() const { return NET_REMOTE_CLIENT; }
};

#endif