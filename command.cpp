#include "command.hpp"
#include "packet.hpp"
#include "protocol.hpp"

Command::Command(CommandType type, const std::vector<unitID> &units, int x, int y)
	: type(type), units(units), x(x), y(y), format(targetsTile)
{
}

Command::Command(CommandType type, const std::vector<unitID> &units, unitID unit)
	: type(type), units(units), x(unit), y(0), format(targetsUnit)
{
}

Command::Command(CommandType type, int scrapyardId, std::string unitType)
	: type(type), scrapyard(scrapyardId), unitType(unitType), format(targetsUnitType)
{
}

Command::Command(CommandType type, int scrapyardId, int index)
	: type(type), scrapyard(scrapyardId), x(index), format(targetsQueue)
{
}

Command::Command(CommandType type, int scrapyardId, int x, int y)
	: type(type), scrapyard(scrapyardId), x(x), y(y), format(targetsRally)
{
}

void Command::writeToPacket(Packet *packet) const
{
	packet->putInt(msg_command);
	packet->putChar(format);
	packet->putInt(type);
	
	if(format != targetsUnitType)
	{
		packet->putInt((int)units.size());
		for (std::vector<unitID>::const_iterator it = units.begin();
			it != units.end(); 
			it++)
		{
			packet->putInt(*it);
		}
	}
	switch(format)
	{
		case targetsTile:
			packet->putInt(x);
			packet->putInt(y);
			break;
		case targetsUnit:
			packet->putInt(x);
			break;
		case targetsUnitType:
			packet->putInt(scrapyard);
			packet->putString(unitType);
			break;
		case targetsQueue:
			packet->putInt(scrapyard);
			packet->putInt(x);
			break;
		case targetsRally:
			packet->putInt(scrapyard);
			packet->putInt(x);
			packet->putInt(y);
			break;
	}
}

void Command::readFromPacket(Packet *packet)
{
	// Don't read in the msg_command flag; that gets checked outside.
	
	format = static_cast<CommandFormat>(packet->getChar());
	type = static_cast<CommandType>(packet->getInt());
	
	if(format != targetsUnitType)
	{
		int numUnits = packet->getInt();
		units.clear();
		for (int i = 0; i < numUnits; i++)
			units.push_back(packet->getInt());
	}
	
	switch(format)
	{
		case targetsTile:
			scrapyard = 0;
			x = packet->getInt();
			y = packet->getInt();
			unitType = "";
			break;
		case targetsUnit:
			scrapyard = 0;
			x = packet->getInt();
			y = 0; unitType = "";
			break;
		case targetsUnitType:
			scrapyard = packet->getInt();
			x = y = 0;
			unitType = packet->getString();
			break;
		case targetsQueue:
			scrapyard = packet->getInt();
			x = packet->getInt();
			y = 0;
			unitType = "";
			break;
		case targetsRally:
			scrapyard = packet->getInt();
			x = packet->getInt();
			y = packet->getInt();
			unitType = "";
			break;
	}
}
