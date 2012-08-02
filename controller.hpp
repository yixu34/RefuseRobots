#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP

#include "command.hpp"
#include "packet.hpp"
#include "model.hpp"
#include "networknode.hpp"

class Controller
{
public:
	Controller(
		Model *model, 
		CommandQueue *commandQueue, 
		MessageQueue *messageQueue);
	virtual ~Controller();
	virtual void timepass(float dt) = 0;
	
	bool isWaiting();
	virtual std::string waitingFor() = 0;
	virtual void disconnected(NetworkNode *net) = 0;

protected:
	virtual void drainUnitFuel();
	void moveUnits(float dt);
	virtual void onTileReached(unitID id) = 0;
	virtual void onAngleReached(unitID id) = 0;
	void updateUnitFacing(unitID id);
	void resolveProjectiles();
	virtual void resolveProjectile(unsigned index);
	virtual void killUnit(unitID id, int explosion);
	
	virtual void processCommands() = 0;
	virtual void processMessages() = 0;

	// Convert incoming packets into local commands or messages.
	virtual void convertPackets() = 0;
	
	bool waiting;
	
	Model *model;
	CommandQueue *incomingCommands;
	MessageQueue *incomingMessages;
};

class ServerController : public Controller
{
public:
	ServerController(
		Model *model, 
		CommandQueue *commandQueue, 
		MessageQueue *messageQueue);
	void timepass(float dt);
	
	playerID addPlayer(int nodeId, const char *name=NULL);
	
private:
	unitID createUnit(playerID owner, std::string unit, unsigned x, unsigned y);
	void checkVictoryConditions();
	void pathfindUnit(unitID id);
	void stopUnit(unitID id);
	void produceUnits(float dt);
	void onTileReached(unitID id);
	void onAngleReached(unitID id);
	void convertPackets();
	void processMessages();
	void processCommands();
	void sendChatMessage(std::string text, int player);
	void resolveProjectile(unsigned index);
	void killUnit(unitID id, int explosion);
	void updateScrapyard(int id);
	void damageUnit(int damage, unitID id);
	void thinkUnits();
	void acquireTargets();
	bool fireBurst(Model::Unit *shooter, Model::Unit *target);
	void turnAndShoot(Model::Unit *shooter, Model::Unit *target);
	bool canAutoTarget(Model::Unit *u, Model::Unit *target);
	bool canTarget(Model::Unit *u, Model::Unit *target);
	bool canShoot(Model::Unit *u, Model::Unit *target);
	bool isBetterTarget(Model::Unit *u, Model::Unit *newTarget, Model::Unit *oldTarget);
	void loadUnit(unitID unitId, unitID transportId);
	void unloadUnit(unitID transport, unitID unit);
	void drainUnitFuel();
	void updateUnitFuel(unitID id);
	void regenerateUnits();
	void convertScrapyard(int sid, playerID player);
	void updateUnitState(unitID unit);
	std::string waitingFor();
	void disconnected(NetworkNode *net);
	void playerLost(playerID player);
	void playerWon(playerID player);
	
	double nextIdleFuelTime, nextRegenTime;
	
	struct PlayerStats {
		std::string name;
		int lastHeard;
		int node;
		bool alive;
	};
	std::map<playerID, PlayerStats> players;
	
	int frame;
	int lastSent;
	float window;
	bool gameOver;
	void sendHeartbeat();
	void updateHeartbeat(int player, int time);
};

class ClientController : public Controller
{
public:
	ClientController(
		Model *model, 
		CommandQueue *commandQueue, 
		MessageQueue *messageQueue);
	void timepass(float dt);
	std::string waitingFor() { return "server"; }
	void disconnected(NetworkNode *net);

private:
	void onTileReached(unitID id);
	void onAngleReached(unitID id);
	void convertPackets();
	void processMessages();
	void processCommands();
	
	playerID playerId;
};

void cleanPathCache();

#endif
