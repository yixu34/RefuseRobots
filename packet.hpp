#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <deque>
#include "protocol.hpp"

// Encapsulates serialized raw data that can be passed
// over a network.

class Packet
{
public:
	explicit Packet();
	explicit Packet(int type);
	Packet(char *dataWithHeader, int sizeWithHeader);
	~Packet();
	
	void putChar(char value);
	void putShort(short value);
	void putInt(int value);
	void putFloat(float value);
	void putString(const std::string &value);
	
	char getChar();
	short getShort();
	int getInt();
	float getFloat();
	std::string getString();
	
	const char *getData();
	
	int getSize() const;

	Packet *clone();
	
private:
	// Pass by value?  I think not, sir!
	Packet(const Packet &rhs);  
	void init();

	void expand(int numBytes);

	// This should in fact be unsigned, unless someone knows of a more
	// clever bit-shifting trick...
	unsigned char *data;
	int pos;
	int size;
	int alloc;
};

typedef std::deque<Packet *> MessageQueue;

void logTrafficIncoming(Packet *packet);
void logTrafficOutgoing(Packet *packet);
void logTrafficClear();
void logTrafficSave(std::string filename);
void logTrafficStats(int &bytesSent, int &bytesRecv, int &packSent, int &packRecv);

#endif  //PACKET_HPP
