#include "main.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SquadID stuff

unsigned Squad::maxSquadID = 0;

Squad::SquadMap Squad::squads;

squadID Squad::newSquad(SType type, ICoord targ, AIPlayer * myPlayer)
{
	maxSquadID++;
	squads[maxSquadID] = new Squad(type, targ, myPlayer, maxSquadID);
	return maxSquadID;
}

Squad * Squad::getSquad(squadID id)
{
	if(squads.find(id) == squads.end())
		return NULL;
	return squads[id];
}

void Squad::removeSquad(squadID id)
{
	if(squads.find(id) != squads.end())
	{
		delete squads[id];
		squads.erase(id);
	}
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Squad::Squad(Squad::SType type, ICoord target, AIPlayer * myPlayer, squadID myID)
:target(target), myType(type), myPlayer(myPlayer), myID(myID)
{
	pendingCons = 0;
	issuedCommand = false;
	capturing = false;
	myTransport = 0;
	myNanites = 0;
}

bool Squad::containsUnit(unitID theUnit)
{
	return myUnits.find(theUnit) != myUnits.end() || theUnit == myNanites || theUnit == myTransport;
}

void Squad::setTarget(ICoord newTarget)
{
	if(myType != refuel)
	{
		target = newTarget;
		issuedCommand = false;
		return;
	}
	int bestDist = -1;
	unitID assignee = 0;
	int dist;
	ICoord unitPos;
	for(std::set<unitID>::iterator ii = myUnits.begin(); ii != myUnits.end(); ii++)
	{
		unitPos = ICoord(myPlayer->model->getUnit(*ii)->getX(), myPlayer->model->getUnit(*ii)->getY());
		dist = distanceFrom(myPlayer->model, unitPos, newTarget);
		if(bestDist == -1 ||  dist < bestDist && myPlayer->model->getUnit(*ii)->fuel != 0)
		{
			assignee = *ii;
			bestDist = dist;
		}
	}
	myPlayer->issueCommand(assignee, Command::move, newTarget.x, newTarget.y);
}

ICoord Squad::getTarget()
{
	return target;
}

Squad::SType Squad::getType()
{
	return myType;
}

void Squad::timepass(float dt)
{
	switch(myType)
	{
	case attack:
		if(myNanites && myTransport && !myPlayer->model->getUnit(myNanites)->inTransport && !issuedCommand)
		{
			myPlayer->issueCommand(myTransport, Command::CommandType::follow, myNanites, 0);
		}
		if(myNanites && myTransport && myPlayer->model->getUnit(myNanites)->inTransport && !issuedCommand && !myUnits.empty())
		{
			myPlayer->issueCommand(myUnits, Command::CommandType::attackMove, target.x, target.y);
			myPlayer->issueCommand(myTransport, Command::CommandType::move, target.x, target.y);
			issuedCommand = true;
		}
		else if(issuedCommand && myNanites && myTransport && myPlayer->model->getUnit(myNanites)->inTransport)
		{
			ICoord dumpSite = myPlayer->model->spaceNearestTo(myPlayer->model->getUnit(myTransport)->getX(), myPlayer->model->getUnit(myTransport)->getY(), false);
			if(myPlayer->model->getTile(dumpSite.x, dumpSite.y)->scrapyard == myPlayer->targetScrapyard)
			{
				myPlayer->issueCommand(myTransport, Command::CommandType::unload, 0, 0);
			}
		}
		else
		{
			if(pendingCons < 3)
			{
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::helo, 2));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::helo, 3));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::assault, 3));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::assault, 2));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::scout, 4));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::nanites, 2));
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::transport, 3));

				pendingCons += 7;
			}
		}
		break;
	case defend:
		if(myUnits.size() > 2 && !issuedCommand)
		{
			myPlayer->issueCommand(myUnits, Command::CommandType::attackMove, target.x, target.y);
			issuedCommand = true;
		}
		else if(myUnits.size() > 5 && issuedCommand)
		{
			//other stuff here
		}
		else
		{
			if(pendingCons < 3)
			{
				int priority = myUnits.empty() ? 6 : 1;
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::infantry, priority));
				pendingCons++;
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::artillery, priority));
				pendingCons++;
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::armor, priority));
				pendingCons++;
			}
		}
		break;
	case interdict:
		if(!issuedCommand && !myUnits.empty() && myUnits.size() < 10)
		{
			Model::Unit * tU = myPlayer->model->getUnit(*myUnits.begin());
			myPlayer->issueCommand(myUnits, Command::move, tU->getX(), tU->getY());
			issuedCommand = true;
		}
		else if(myUnits.size() >= 10 && !issuedCommand)
		{
			myPlayer->issueCommand(myUnits, Command::attackMove, target.x, target.y);
			issuedCommand = true;
		}
		if(myUnits.size() < 10 && pendingCons < 1)
		{
			myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::artillery, 1));
			pendingCons++;
		}
		break;
	case colonize:
		if(myNanites && myTransport && !myPlayer->model->getUnit(myNanites)->inTransport && !issuedCommand)
		{
			myPlayer->issueCommand(myTransport, Command::CommandType::follow, myNanites, 0);
		}
		else if(myNanites && myTransport && myPlayer->model->getUnit(myNanites)->inTransport && !issuedCommand)
		{
			myPlayer->issueCommand(myTransport, Command::CommandType::move, target.x, target.y);
			issuedCommand = true;
		}
		if(myNanites && myTransport && myPlayer->model->getUnit(myNanites)->inTransport)
		{
			ICoord dumpSite = myPlayer->model->spaceNearestTo(myPlayer->model->getUnit(myTransport)->getX(), myPlayer->model->getUnit(myTransport)->getY(), false);
			if(myPlayer->model->getTile(dumpSite.x, dumpSite.y)->scrapyard == myPlayer->targetScrapyard)
			{
				myPlayer->issueCommand(myTransport, Command::CommandType::unload, 0, 0);
				myPlayer->issueCommand(myTransport, Command::stop, 0, 0);
			}
		}
		else
		{
			if(pendingCons < 1)
			{
				if(!myNanites)
				{
					myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::nanites, 8));
					pendingCons++;
				}
				if(!myTransport)
				{
					myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::transport, 8));
					pendingCons ++;
				}
			}
		}
		break;
	case air:
		if(myUnits.size() > 5 && !issuedCommand)
		{
			myPlayer->issueCommand(myUnits, Command::CommandType::attackMove, target.x, target.y);
			issuedCommand = true;
		}
		else if(myUnits.size() > 5 && issuedCommand)
		{
			//other stuff here
		}
		else
		{
			if(pendingCons < 3)
			{
				int priority = myUnits.empty() ? 3 : 1;
				
				for(int ii=0; ii<5; ii++) {
					myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::helo, priority));
					pendingCons++;
				}
			}
		}
		break;
	case refuel:
		if(myUnits.empty() && pendingCons == 0)
		{
			for(int ii = 0; ii < 3; ii++)
			{
				myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::oil, 5));
				pendingCons++;
			}
		}
		else if(pendingCons < 1 && myUnits.size() < 5)
		{
			myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::oil, 1));
			pendingCons++;
		}
		break;
	default:
		break;
	}
}

void Squad::setType(SType newType)
{
	myType = newType;
	issuedCommand = false;
}

void Squad::notifyDeath(unitID id)
{
	if(myUnits.find(id) == myUnits.end() && myNanites != id && myTransport != id)
	{
		return;
	}
	myUnits.erase(id);
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::scout))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::scout, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::assault))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::assault, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::transport))
	{
		myTransport = 0;
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::transport, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::artillery))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::artillery, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::helo))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::helo, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::nanites))
	{
		myNanites = 0;
		capturing = false;
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::nanites, 8));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::oil))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::oil, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::infantry))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::infantry, 1));
		pendingCons++;
		return;
	}
	if(myPlayer->model->getUnit(id)->type == UnitInfo::getUnitType(AIPlayer::armor))
	{
		myPlayer->orderConstruction(AIPlayer::ConstructOrder(myID, AIPlayer::armor, 1));
		pendingCons++;
		return;
	}
}

void Squad::assignUnit(unitID theUnit)
{
	issuedCommand = false;
	if(pendingCons > 0) pendingCons--;
	if(myPlayer->model->getUnit(theUnit)->type == UnitInfo::getUnitType(AIPlayer::transport) && !myTransport)
	{
		myTransport = theUnit;
		return;
	}
	if(myPlayer->model->getUnit(theUnit)->type == UnitInfo::getUnitType(AIPlayer::nanites) && !myNanites)
	{
		myNanites = theUnit;
		return;
	}
	myUnits.insert(theUnit);
}