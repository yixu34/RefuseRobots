#include "Packet.hpp"
#include "memory.hpp"
#include <cstdlib>
#include <cstring>
#include <WinSock2.h>
#include "msvcfix.hpp"

Packet::Packet()
{
	init();
}

Packet::Packet(int type)
{
	init();
	putInt(type);
}

Packet::Packet(
	char *dataWithHeader, 
	int sizeWithoutHeader)
{
	pos   = 4;
	size = alloc = sizeWithoutHeader + sizeof(int);

	data = static_cast<unsigned char *>(malloc(alloc));
	memcpy(data, dataWithHeader, size);
	memcpy(data, &size, sizeof(int));
}

void Packet::init()
{
	pos   = 4;
	size  = 4;
	alloc = 32 + sizeof(int);   

	data  = static_cast<unsigned char*>(malloc(alloc));
	memset(data, 0, alloc);
	memcpy(data, &size, sizeof(int));
}

Packet::~Packet()
{
	free(data);
}

//////////////////////////////////////////////////////////////////////////

void Packet::putChar(char value)
{
	expand(1);
	data[size++] = value;
}

void Packet::putShort(short value)
{
	expand(2);
	data[size++] = (value      & 0xFF);
	data[size++] = (value >> 8 & 0xFF);
}

void Packet::putInt(int value)
{
	expand(4);
	data[size++] = (value       & 0xFF);
	data[size++] = (value >> 8  & 0xFF);
	data[size++] = (value >> 16 & 0xFF);
	data[size++] = (value >> 24 & 0xFF);
}

void Packet::putFloat(float value)
{
	expand(sizeof(float));
	memcpy(data + size, &value, sizeof(float));
	size += sizeof(float);
}

void Packet::putString(const std::string &value)
{
	const char *toPut = value.c_str();

	// Make room for the null terminator too
	expand(static_cast<int>(strlen(toPut) + 1));

	while (*toPut != '\0')
		data[size++] = *(toPut++);

	data[size++] = '\0';
}

//////////////////////////////////////////////////////////////////////////

char Packet::getChar()
{
	return data[pos++];
}

short Packet::getShort()
{
	short ret = 
		(static_cast<unsigned>(data[pos    ])) |
		(static_cast<unsigned>(data[pos + 1])) << 8;

	pos += 2;
	return ret;
}

int Packet::getInt()
{
	int ret = 
		(static_cast<unsigned>(data[pos    ]))       |
		(static_cast<unsigned>(data[pos + 1])) << 8  |
		(static_cast<unsigned>(data[pos + 2])) << 16 |
		(static_cast<unsigned>(data[pos + 3])) << 24;

	pos += 4;
	return ret;
}

float Packet::getFloat()
{
	float ret;
	memcpy(&ret, data + pos, sizeof(float));
	pos += sizeof(float);
	return ret;
}

std::string Packet::getString()
{
	std::string ret("");
	while (data[pos] != '\0')
		ret += data[pos++];

	++pos;  // Skip over the null terminator

	return ret;
}

//////////////////////////////////////////////////////////////////////////

int Packet::getSize() const
{
	return size;
}

const char *Packet::getData()
{
	// About to pass this data over the network, so encode the size also
	memcpy(data, &size, sizeof(int));
	return reinterpret_cast<char *>(data);
}

void Packet::expand(int numBytes)
{
	while (size + numBytes >= alloc)
		alloc *= 2;
	data = static_cast<unsigned char *>(realloc(data, alloc));
}

Packet *Packet::clone()
{
	return NEW Packet(
		reinterpret_cast<char *>(this->data), 
		size - sizeof(int));
}