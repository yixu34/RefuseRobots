#include "main.hpp"
#include <vector>
#include <assert.h>

AIPlayer::AIPlayer(Model *model, CommandQueue *commandQueue, playerID playerId)
	: View(model, commandQueue, playerId)
{
	std::vector<Model::Scrapyard*> tempSc;
	model->getScrapyardList(tempSc);
	for(unsigned ii = 0; ii < tempSc.size(); ii++)
	{
		if(tempSc[ii]->owner == playerId)
		{
			myBases.insert(ii);
			mySquads.push_back(Squad::newSquad(Squad::defend, tempSc[ii]->center, this));
		}
		else if(tempSc[ii]->owner == 0)
		{
			neutralBases.insert(ii);
		}
		else
		{
			enemyBases.insert(ii);
		}
	}
	if(neutralBases.empty())
	{
		targetScrapyard = findNearestScrapyard(*myBases.begin(), enemyBases);
		mySquads.push_back(Squad::newSquad(Squad::attack, model->getScrapyard(targetScrapyard)->center, this));
	}
	else
	{
		targetScrapyard = findNearestScrapyard(*myBases.begin(), neutralBases);
		mySquads.push_back(Squad::newSquad(Squad::colonize, model->getScrapyard(targetScrapyard)->center, this));
	}
	OilSquad = Squad::newSquad(Squad::refuel, model->getScrapyard(*myBases.begin())->center, this);
	mySquads.push_back(OilSquad);
	mySquads.push_back(Squad::newSquad(Squad::interdict, model->getScrapyard(*enemyBases.begin())->center, this));
	int iii = mySquads.size();
	std::vector<unitID> tempunits;
	model->getUnitList(tempunits);
	Model::Unit * currUnit;
	for(unsigned ii = 0; ii < tempunits.size(); ii++)
	{
		currUnit = model->getUnit(tempunits[ii]);
		if(currUnit->owner == playerId)
		{
			newUnits.insert(tempunits[ii]);
		}
	}
}

std::string AIPlayer::scout = "tank1";
std::string AIPlayer::assault = "guntank";
std::string AIPlayer::transport = "transport";
std::string AIPlayer::artillery = "artillery";
std::string AIPlayer::helo = "helicopter";
std::string AIPlayer::nanites = "nanites";
std::string AIPlayer::oil = "oiltanker";
std::string AIPlayer::infantry = "infantry";
std::string AIPlayer::armor = "heavytank";

void AIPlayer::timepass(float dt)
{
	for(unsigned ii = 0; ii < mySquads.size(); ii++)
	{
		Squad::getSquad(mySquads[ii])->timepass(dt);
	}
	processConstruction();
}

void AIPlayer::notifyMoveUnit(unitID id, ICoord old, ICoord current)
{
}

void AIPlayer::notifyNewUnit(unitID id)
{
	if(model->getUnit(id)->owner == playerId)
	{
		newUnits.insert(id);
		assignUnits();
		processConstruction();
	}
}

void AIPlayer::notifyDeath(unitID id)
{
	newUnits.erase(id);
	assUnits.erase(id);
	for(unsigned ii = 0; ii < mySquads.size(); ii++)
	{
		Squad::getSquad(mySquads[ii])->notifyDeath(id);
	}
}

void AIPlayer::notifyConversion(int scrapyard, playerID loser, playerID winner)
{
	if(loser == 0)
	{
		if(winner == playerId)
		{
			neutralBases.erase(scrapyard);
			myBases.insert(scrapyard);
			mySquads.push_back(Squad::newSquad(Squad::SType::defend, model->getScrapyard(scrapyard)->center, this));
		}
		else
		{
			neutralBases.erase(scrapyard);
			enemyBases.insert(scrapyard);
		}
	}
	else if(loser == playerId)
	{
		myBases.erase(scrapyard);
		enemyBases.insert(scrapyard);
		for(unsigned ii = 0; ii < mySquads.size(); ii++)
		{
			if(Squad::getSquad(mySquads[ii])->getTarget() == model->getScrapyard(scrapyard)->center)
			{
				killSquad(mySquads[ii]);
			}
		}
	}
	else
	{
		if(winner == playerId)
		{
			enemyBases.erase(scrapyard);
			myBases.insert(scrapyard);
		}
	}
	if(scrapyard == targetScrapyard)
	{
		squadID attackSquad = -1;
		Squad * tS;
		for(unsigned ii = 0; ii < mySquads.size(); ii++)
		{
			tS = Squad::getSquad(mySquads[ii]);
			if(tS->getTarget() == model->getScrapyard(scrapyard)->center && (tS->getType() == Squad::attack || tS->getType() == Squad::colonize))
			{
				attackSquad = mySquads[ii];
			}
		}
		if(neutralBases.empty())
		{
			targetScrapyard = findNearestScrapyard(*myBases.begin(), enemyBases);
			Squad::getSquad(attackSquad)->setType(Squad::SType::attack);
			Squad::getSquad(attackSquad)->setTarget(model->getScrapyard(targetScrapyard)->center);
		}
		else
		{
			targetScrapyard = findNearestScrapyard(*myBases.begin(), neutralBases);
			Squad::getSquad(attackSquad)->setTarget(model->getScrapyard(targetScrapyard)->center);
		}
	}
}

int AIPlayer::findNearestScrapyard(int homeID, std::set<int> possibles)
{
	ICoord targ, current;
	ICoord home = model->getScrapyard(homeID)->center;
	int bestDist = -1;
	int currSC;
	for(std::set<int>::iterator ii = possibles.begin(); ii != possibles.end(); ii++)
	{
		current = model->getScrapyard(*ii)->center;
		int dist = distanceFrom(model, current, home);
		if(bestDist == -1 || bestDist > dist)
		{
			bestDist = dist;
			targ = current;
			currSC = *ii;
		}
	}
	return currSC;
}

int AIPlayer::findNearestOilWell(ICoord origin)
{
	int well = 0;
	int bestDist = -1;
	int dist;
	for(int ii = 0; ii < model->oilWells.size(); ii++)
	{
		dist = distanceFrom(model, model->oilWells[ii].center, origin);
		if(bestDist == -1 || dist < bestDist)
		{
			well = ii;
			bestDist = dist;
		}		
	}
	return well;
}

void AIPlayer::notifyLowFuel(unitID id, bool empty)
{
	if(model->getUnit(id)->owner != playerId)
		return;

	if(Squad::getSquad(OilSquad)->containsUnit(id))
	{
		ICoord tPos = ICoord(model->getUnit(id)->getX(), model->getUnit(id)->getY());
		int well = findNearestOilWell(tPos);
		issueCommand(id, Command::move, model->oilWells[well].center.x, model->oilWells[well].center.y);
		return;
	}
	ICoord target = ICoord(model->getUnit(id)->getX(), model->getUnit(id)->getY());
	Squad::getSquad(OilSquad)->setTarget(target);
}

void AIPlayer::killSquad(squadID id)
{
	//TODO: Reassign all the units and stop giving orders to squad.
}

void AIPlayer::processConstruction()
{
	if(pendingConstruction.empty())
	{
		return;
	}
	//TODO find better algorithm, if one exists.
	//find the first production order not already being filled somewhere.
	unsigned ii = 0;
	bool endloop = false;
	while(ii < pendingConstruction.size() && ii < myBases.size())
	{
		endloop = true;
		for(std::set<int>::iterator jj = myBases.begin(); jj != myBases.end(); jj++)
		{
			if(pendingConstruction[ii].unitType == model->getScrapyard(*jj)->currentProduction())
			{
				endloop = false;
				ii++;
				break;
			}
		}
		if(endloop)
			break;
	}
	if(pendingConstruction.size() <= ii+1) // Nothing left!
		return;
	
	//set the scrapyard::nextType on each scrapyard to next item in pendingConstruction
	Model::Scrapyard * tSC;
	std::string desiredProduction = pendingConstruction[ii].unitType;
	assert(desiredProduction.length()!=0);
	for(std::set<int>::iterator jj = myBases.begin(); jj != myBases.end(); jj++)
	{
		tSC = model->getScrapyard(*jj);
		if(tSC->buildQueue.size() == tSC->queuePos + 1)
			issueCommand(*jj, Command::CommandType::buildUnit, desiredProduction);
		else
			tSC->buildQueue[tSC->queuePos+1] = desiredProduction;
	}
}

void AIPlayer::orderConstruction(ConstructOrder CO)
{
	for(unsigned ii = 0; ii < pendingConstruction.size(); ii++)
	{
		if(pendingConstruction[ii].priority < CO.priority)
		{
			pendingConstruction.insert(pendingConstruction.begin()+ii,CO);
			return;
		}
	}
	pendingConstruction.push_back(CO);
}

void AIPlayer::assignUnits()
{
	if(newUnits.empty())
		return;
	//loop through the new units, assigning them as appropriate;
	bool assigned;
	
	for(std::set<unitID>::iterator ii = newUnits.begin(); ii != newUnits.end(); ii++)
	{
		assigned = false;
		
		//assign unit to the squad that asked for it.
		for(unsigned jj = 0; jj < pendingConstruction.size(); jj++)
		{
			if(UnitInfo::getUnitType(pendingConstruction[jj].unitType) == model->getUnit(*ii)->type)
			{
				Squad::getSquad(pendingConstruction[jj].dest)->assignUnit(*ii);
				assUnits.insert(*ii);
				ii = newUnits.erase(ii);
				ii--;
				pendingConstruction.erase(pendingConstruction.begin()+jj);
				assigned = true;
				break;
			}
		}
		if(assigned)
			continue;
		
		//if it is not needed, make a new squad out of it.
		Squad::SType type = Squad::SType::interdict;
		//set squad type based on unit (air, artillery, etc.)
		if(UnitInfo::getUnitType(assault) == model->getUnit(*ii)->type || UnitInfo::getUnitType(scout) == model->getUnit(*ii)->type)
		{
			type = Squad::attack;
		}
		if(UnitInfo::getUnitType(helo) == model->getUnit(*ii)->type)
		{
			type = Squad::air;
		}
		if(UnitInfo::getUnitType(nanites) == model->getUnit(*ii)->type)
		{
			type = Squad::attack;
		}
		ICoord target = ICoord(5,5);
		//set squad target location
		if(type == Squad::attack || type == Squad::air || type == Squad::interdict)
		{
			target = model->getScrapyard(*enemyBases.begin())->center;
		}
		//FIXME set target to a reasonable value
		mySquads.push_back(Squad::newSquad(type, target, this));
		//add the unit to the squad, and the squad to the list
		Squad::getSquad(mySquads[mySquads.size()-1])->assignUnit(*ii);
		assUnits.insert(*ii);
		ii = newUnits.erase(ii);
		ii--;
		assigned = true;
		if(newUnits.empty())
			return;
	}
}

