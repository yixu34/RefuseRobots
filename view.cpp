#include "main.hpp"

View::View(Model *model, CommandQueue *commandQueue, playerID playerId)
{
	this->model = model;
	this->commands = commandQueue;
	this->playerId = playerId;
	//TODO: SOUND is this the correct place for this sound?
	playSoundtrack("sounds/MainLoop.ogg");
}

View::~View()
{
}

void View::setPlayerId(playerID id)
{
	playerId = id;
	onSetPlayerId(id);
}

void View::setPlayerName(const std::string &name)
{
	playerName = name;
}

void View::issueCommand(const std::set<unitID> &units, Command::CommandType type, int x, int y)
{
	if(units.empty())
		return;
	
	std::vector<unitID> unitCommands;
	
	unitCommands.insert(unitCommands.end(), units.begin(), units.end());
	commands->push_back( Command(type, unitCommands, x, y) );
}

void View::issueCommand(const unitID theUnit, Command::CommandType type, int x, int y)
{
	std::set<unitID> theSet;
	theSet.insert(theUnit);
	issueCommand(theSet, type, x, y);
}

void View::issueCommand(const int scrapyardId, Command::CommandType type, std::string unitType)
{
	if(scrapyardId < 0) return;
	commands->push_back( Command(type, scrapyardId, unitType) );
}

void View::issueCommand(const int scrapyardId, Command::CommandType type, int index)
{
	if(scrapyardId < 0) return;
	commands->push_back( Command(type, scrapyardId, index) );
}

void View::issueCommand(const int scrapyardId, Command::CommandType type, int x, int y)
{
	if(scrapyardId < 0) return;
	commands->push_back( Command(type, scrapyardId, x, y) );
}

void View::timepass(float dt)
{
}

void View::notifyNewUnit(unitID id)
{
}
void View::notifyMoveUnit(unitID id, ICoord old, ICoord current)
{
}
void View::notifyDeath(unitID id)
{
}
void View::notifyConversion(int scrapyard, playerID loser, playerID winner)
{
}
void View::notifyLowFuel(unitID id, bool empty)
{
}

// Find the nearest idle fuel truck to (x,y).
// If there is no such fuel truck, returns 0.
unitID View::fuelTruckNear(int x, int y)
{
	std::vector<unitID> units;
	model->getUnitList(units);
	unitID best = 0;
	int bestDist = 0;
	
	for(std::vector<unitID>::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = model->getUnit(*ii);
		if(!u->type->refueler)
			continue;
		int dist = (u->x-x)*(u->x-x) + (u->y-y)*(u->y-y);
		if(!u->escort && (!best || dist<bestDist))
		{
			bestDist = dist;
			best = *ii;
		}
	}
	return best;
}


bool cheatsEnabled=false, 
	 cheatFogRevealed=false, 
	 cheatFastBuild=false, 
	 cheatFastMove=false;

void View::cheat(std::string str)
{
#ifdef ENABLE_CHEATS
	if(str=="korea")
		cheatsEnabled = true;
	if(!cheatsEnabled)
		return;
	if(str=="his noodly appendage")
	{
		if(!cheatFogRevealed) {
			for(int ii=1; ii<=model->maxPlayerID; ii++)
				model->revealFog(ii, 1000, 500, 500);
		} else {
			for(int ii=1; ii<=model->maxPlayerID; ii++)
				model->concealFog(1, 1000, 500, 500);
		}
		
		cheatFogRevealed = !cheatFogRevealed;
	}
	else if(str=="nuclear launch detected")
	{
		cheatFastBuild = !cheatFastBuild;
	}
	else if(str=="rocket boots")
	{
		cheatFastMove = !cheatFastMove;
	}
	else if(str=="the pact is sealed")
	{
		std::vector<Model::Scrapyard *> scrapyards;
		model->getScrapyardList(scrapyards);
		for(std::vector<Model::Scrapyard *>::iterator it = scrapyards.begin();
			it != scrapyards.end();
			it++)
		{
			Model::Scrapyard *currScrapyard = *it;
			currScrapyard->owner = 0;
		}
	}
	else if(str=="clearnetlog")
	{
		logTrafficClear();
	}
	else if(str=="savetraffic")
	{
		logTrafficSave("traffic_log.txt");
	}
#endif
}


void viewNotifyNewUnit(unitID id)
{
	if(!engine)
		return;
	if(engine->view)
		engine->view->notifyNewUnit(id);
	for(unsigned ii=0; ii<engine->aiPlayers.size(); ii++)
		engine->aiPlayers[ii]->notifyNewUnit(id);
}

void viewNotifyMoveUnit(unitID id, ICoord old, ICoord current)
{
	if(!engine)
		return;
	if(engine->view)
		engine->view->notifyMoveUnit(id,old,current);
	for(unsigned ii=0; ii<engine->aiPlayers.size(); ii++)
		engine->aiPlayers[ii]->notifyMoveUnit(id, old, current);
}

void viewNotifyDeath(unitID id)
{
	if(!engine)
		return;
	if(engine->view)
		engine->view->notifyDeath(id);
	for(unsigned ii=0; ii<engine->aiPlayers.size(); ii++)
		engine->aiPlayers[ii]->notifyDeath(id);
}

void viewNotifyConversion(int scrapyard, playerID loser, playerID winner)
{
	if(!engine)
		return;
	if(engine->view)
		engine->view->notifyConversion(scrapyard, loser, winner);
	for(unsigned ii=0; ii<engine->aiPlayers.size(); ii++)
		engine->aiPlayers[ii]->notifyConversion(scrapyard, loser, winner);
}

void viewNotifyLowFuel(unitID id, bool empty)
{
	if(!engine)
		return;
	if(engine->view)
		engine->view->notifyLowFuel(id, empty);
	for(unsigned ii=0; ii<engine->aiPlayers.size(); ii++)
		engine->aiPlayers[ii]->notifyLowFuel(id, empty);
}



