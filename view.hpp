#ifndef VIEW_HPP
#define VIEW_HPP

#include "types.hpp"
#include "model.hpp"
#include "command.hpp"
#include <SDL.h>
#include <set>
#include <string>

class View
{
public:
	View(Model *model, CommandQueue *commandQueue, playerID playerId);
	virtual ~View();
	
	void setPlayerId(playerID id);
	void setPlayerName(const std::string &name);
	void issueCommand(const std::set<unitID> &units, Command::CommandType type, int x, int y);
	void issueCommand(unitID theUnit, Command::CommandType type, int x, int y);
	void issueCommand(int scrapyardId, Command::CommandType type, std::string unitType);
	void issueCommand(int scrapyardId, Command::CommandType type, int index);
	void issueCommand(int scrapyardId, Command::CommandType type, int x, int y);
	virtual void timepass(float dt);
	
	virtual void onSetPlayerId(playerID id) {}
	
	virtual void notifyNewUnit(unitID id);
	virtual void notifyMoveUnit(unitID id, ICoord old, ICoord current);
	virtual void notifyDeath(unitID id);
	virtual void notifyConversion(int scrapyard, playerID loser, playerID winner);
	virtual void notifyLowFuel(unitID id, bool empty); // Only called if server
	
	void cheat(std::string str);
	
protected:
	unitID fuelTruckNear(int x, int y);
	
	Model *model;
	CommandQueue *commands;
	
	// The player id of this view.  Only allow selection of units
	// whose player id is equal to this view's.
	playerID playerId;

	std::string playerName;
};

void viewNotifyNewUnit(unitID id);
void viewNotifyMoveUnit(unitID id, ICoord old, ICoord current);
void viewNotifyDeath(unitID id);
void viewNotifyConversion(int scrapyard, playerID loser, playerID winner);
void viewNotifyLowFuel(unitID id, bool empty); // Only called ifserver

extern bool cheatsEnabled, cheatFogRevealed, cheatFastBuild, cheatFastMove;


#endif
