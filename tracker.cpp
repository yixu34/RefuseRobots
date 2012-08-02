#include "main.hpp"
#include "tracker.hpp"

Tracker tracker;
static std::string httpEscapeString(std::string str);
static char hexDigit(int n);

Tracker::Tracker()
{
	connected = false;
	fail = false;
	pendingWrite = pendingRead = serverList = "";
	pendingAction = actNone;
}

void Tracker::registerServer(std::string name)
{
	serverName = name;
	
	if(connected)
		return;
	if(!makeConnection())
		return;
	
	std::string httpHeader = std::string("GET /addserver.php?name=")+httpEscapeString(name)+" HTTP/1.0\n"
	                                     "HOST: refrobots.jimrandomh.org\n"
	                                     "Accept: */*\n"
	                                     "User-Agent: RR\n\n";
	int bytesSent = send(sock, httpHeader.c_str(), httpHeader.size(), 0);
	
	if(bytesSent < (int)httpHeader.size()) {
		pendingWrite = httpHeader.substr(bytesSent, httpHeader.size()-bytesSent);
		pendingAction = actRegister;
	} else {
		closeConnection();
	}
}

static std::string httpEscapeString(std::string str)
{
	std::string ret="";
	for(int ii=0; ii<str.size(); ii++)
	{
		char ch = str[ii];
		if((ch>='a' && ch<='z') || (ch>='A' && ch<='Z') || (ch>='0' && ch<='9'))
			ret += ch;
		else {
			ret += '%';
			ret += hexDigit(ch>>4);
			ret += hexDigit(ch&0x0F);
		}
	}
	return ret;
}
static char hexDigit(int n)
{
	if(n<=9)
		return '0'+n;
	else
		return 'a'+n;
}

void Tracker::unregisterServer()
{
	if(connected)
		return;
	if(!makeConnection())
		return;
	
	std::string httpHeader = std::string("GET /removeserver.php HTTP/1.0\n"
	                                     "HOST: refrobots.jimrandomh.org\n"
	                                     "Accept: */*\n"
	                                     "User-Agent: RR\n\n");
	int bytesSent = send(sock, httpHeader.c_str(), httpHeader.size(), 0);
	
	if(bytesSent < (int)httpHeader.size()) {
		pendingWrite = httpHeader.substr(bytesSent, httpHeader.size()-bytesSent);
		pendingAction = actUnregister;
	} else {
		closeConnection();
	}
}

void Tracker::requestServerList()
{
	if(connected)
		return;
	if(!makeConnection())
		return;
	
	std::string httpHeader = std::string("GET /getservers.php HTTP/1.0\n"
	                                     "HOST: refrobots.jimrandomh.org\n"
	                                     "Accept: */*\n"
	                                     "User-Agent: RR\n\n");
	int bytesSent = send(sock, httpHeader.c_str(), httpHeader.size(), 0);
	
	if(bytesSent < (int)httpHeader.size()) {
		pendingWrite = httpHeader.substr(bytesSent, httpHeader.size()-bytesSent);
	}
	pendingAction = actGetList;
	strippedHeaders = false;
}


void Tracker::closeConnection()
{
	pendingAction = actNone;
	closesocket(sock);
	pendingWrite = pendingRead = "";
	sock = INVALID_SOCKET;
	connected = false;
}

bool Tracker::makeConnection()
{
	hostent *host = gethostbyname("refrobots.jimrandomh.org");
	if(!host) {
		fail = true;
		return false;
	}
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	connection.sin_family = AF_INET;
	connection.sin_addr = *((in_addr*)*host->h_addr_list);
	connection.sin_port = htons(80);
	
	if(connect(sock, (sockaddr*)&connection, sizeof(connection)) == SOCKET_ERROR)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
		fail = true;
		return false;
	}

	// Set socket to non-blocking
	unsigned long val = 1;
	if (ioctlsocket(sock, FIONBIO, &val) == SOCKET_ERROR)
	{
		closesocket(sock);
		sock = INVALID_SOCKET;
		fail = true;
		return false;
	}
	
	connected = true;
	pendingWrite = pendingRead = "";
	return true;
}

bool Tracker::failed()
{
	return fail;
}

bool Tracker::finishedServerList()
{
	return !connected;
}

void Tracker::timepass()
{
	if(pendingWrite.size() > 0)
	{
		int bytesSent = send(sock, pendingWrite.c_str(), pendingWrite.size(), 0);
		pendingWrite = pendingWrite.substr(bytesSent, pendingWrite.size()-bytesSent);
	}
	if(pendingAction == actGetList)
	{
		char buf[4100];
		int bytesReceived = recv(sock, buf, 4096, 0);
		if(bytesReceived > 0) {
			buf[bytesReceived] = 0;
			pendingRead += buf;
			serverList = pendingRead;
		} else if(bytesReceived == 0) {
			serverList = pendingRead;
			strippedHeaders = false;
			closeConnection();
		} else if(bytesReceived < 0) {
			int err = WSAGetLastError();
			if(err != WSAEWOULDBLOCK) {
				serverList = pendingRead;
				strippedHeaders = false;
				closeConnection();
			}
		}
	}
}

static ServerDesc parseServer(std::string s);

std::vector<ServerDesc> Tracker::getServers()
{
	std::vector<ServerDesc> ret;
	std::string s;
	
	if(!strippedHeaders)
	{
		// First convert CRLF to LF
		s = "";
		for(int ii=0; ii<(int)serverList.size(); ii++) {
			if(serverList[ii] != '\r')
				s += serverList[ii];
		}
		serverList = s;
		
		// Skip till there's two blank lines in a row (meaning start of data)
		for(int ii=0; ii<(int)(serverList.size()-1); ii++)
		{
			if(serverList[ii]=='\n' && serverList[ii+1]=='\n') {
				serverList = serverList.substr(ii+2, serverList.size()-ii-2);
				strippedHeaders = true;
				break;
			}
		}
	}
	
	for(int ii=0; ii<(int)serverList.size(); ii++)
	{
		if(serverList[ii]=='\n') {
			ret.push_back(parseServer(s));
			s = "";
		} else {
			s += serverList[ii];
		}
	}
	
	return ret;
}

static ServerDesc parseServer(std::string s)
{
	ServerDesc ret;
	int splitpoint = s.find(' ');
	
	ret.host = s.substr(0, splitpoint);
	ret.name = s.substr(splitpoint+1, s.size()-splitpoint-1);
	
	return ret;
}
