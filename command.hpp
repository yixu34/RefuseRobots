#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "types.hpp"
#include "util.hpp"
#include <vector>
#include <deque>

class Packet;

class Command
{
public:
	typedef enum CommandFormat {
		targetsUnit,
		targetsTile,
		targetsUnitType,
		targetsQueue,
		targetsRally,
	} CommandFormat;
	
	typedef enum CommandType {
		none = 0,
		
		// Commands for units with no targetting
		stop,
		unload,
		retreat,
		// Commands for units targetting ground
		move,
		attackMove,
		// Commands for units targetting units
		attack,
		follow,
		unloadSingle,
		// Commands for scrapyards
		buildUnit,
		cancelBuild,
		setRally,
		
		// Unimplemented commands
		patrol,
		holdPosition,
	} CommandType;
	
	Command() {}
	Command(CommandType type, const std::vector<unitID> &units, int x, int y);
	Command(CommandType type, const std::vector<unitID> &units, unitID unit);
	Command(CommandType type, int scrapyardId, std::string unitType);
	Command(CommandType type, int scrapyardId, int index);
	Command(CommandType type, int scrapyardId, int x, int y);
	
	void writeToPacket(Packet *packet) const;
	void readFromPacket(Packet *packet);
	
	CommandFormat format;      // What this command targets
	CommandType type;          // The type of command this is
	std::vector<unitID> units; // Which units the command was given to
	
	unitID targetUnit() const { return x; }
	ICoord targetTile() const { return ICoord(x,y); }
	int targetScrapyard() const { return scrapyard; }
	std::string targetUnitType() const { return unitType; }
	int targetIndex() const { return x; }
	
protected:
	// The command's target. For commands which target neither a unit nor a
	// tile, x=y=0. For commands which target a unit,  x=target unitID, y=0.
	int x, y, scrapyard;
	// For build commands
	std::string unitType;
};
typedef std::deque<Command> CommandQueue;


#endif
