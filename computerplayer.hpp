#ifndef COMPUTERPLAYER_HPP
#define COMPUTERPLAYER_HPP

#include "view.hpp"
#include "main.hpp"
#include <vector>
#include <deque>

typedef unsigned int squadID;

class AIPlayer :public View
{
public:
	friend class Squad;
	AIPlayer(Model *model, CommandQueue *commandQueue, playerID playerId);

	static std::string scout;
	static std::string assault;
	static std::string transport;
	static std::string artillery;
	static std::string helo;
	static std::string nanites;
	static std::string oil;
	static std::string infantry;
	static std::string armor;

	void timepass(float dt);

	void killSquad(squadID id);
	
	void notifyNewUnit(unitID id);
	void notifyDeath(unitID id);
	void notifyMoveUnit(unitID id, ICoord old, ICoord current);
	void notifyConversion(int scrapyard, playerID loser, playerID winner);
	void notifyLowFuel(unitID id, bool empty);
	
	struct ConstructOrder {
		ConstructOrder(squadID dest, std::string type, int priority) : dest(dest), unitType(type), priority(priority) {}
		squadID dest;
		std::string unitType;
		int priority;
	};

	void orderConstruction(ConstructOrder CO);

	int findNearestOilWell(ICoord origin);

protected:	
    
	void assignUnits();

	void processConstruction();

	std::deque<ConstructOrder> pendingConstruction;
	std::vector<squadID> mySquads;
	//units without squads
	std::set<unitID> newUnits;
	//units in squads
	std::set<unitID> assUnits;
	//set of scrapyardIDs
	std::set<int> myBases;
	std::set<int> neutralBases;
	std::set<int> enemyBases;
	int targetScrapyard;
	int findNearestScrapyard(int homeID, std::set<int> possibles);
	int OilSquad;
};

#endif
