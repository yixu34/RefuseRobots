#ifndef SQUAD_HPP
#define SQUAD_HPP

#include "main.hpp"

class Squad
{
public:
	enum SType {attack, defend, interdict, colonize, air, refuel};

	//Static SquadID manipulation stuff
	static squadID newSquad(SType type, ICoord target, AIPlayer * myPlayer);
	static Squad * getSquad(squadID id);
	static void removeSquad(squadID id);

	//Actual Squad Stuff
	Squad(SType type, ICoord targ, AIPlayer * myPlayer, squadID myID);//should maybe be protected
	bool containsUnit(unitID theUnit);
	void setTarget(ICoord newTarget);
	ICoord getTarget();
	void timepass(float dt);
	void assignUnit(unitID theUnit);
	void notifyDeath(unitID id);
	void setType(SType newType);
	SType getType();

protected:
	//Static SquadID manipulation stuff
	typedef std::map<squadID, Squad *> SquadMap;
	static SquadMap squads;
	static unsigned maxSquadID;

	//Actual Squad Stuff
	SType myType;
	squadID myID;
	AIPlayer * myPlayer;
	std::set<unitID> myUnits;
	ICoord target;
	int pendingCons;
	bool issuedCommand;
	bool capturing;
	unitID myNanites;
	unitID myTransport;
};

#endif
