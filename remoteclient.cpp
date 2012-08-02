#include "remoteclient.hpp"

RemoteClient::RemoteClient(SOCKET sockHandle, short port)
: NetworkNode(port, sockHandle)
{
}

RemoteClient::~RemoteClient()
{
}