#include "main.hpp"

std::string defaultUnit("infantry");
const int maxQueueSize = 8;

Model::Scrapyard::Scrapyard(playerID owner, unsigned x, unsigned y, unsigned sizeX, unsigned sizeY, int scrapyardId)
	: owner(owner), center(x,y), size(sizeX,sizeY), id(scrapyardId), rally(x,y)
{
	clear();
}


void ServerController::produceUnits(float dt)
{
	std::vector<Model::Scrapyard*> scrapyards;
	model->getScrapyardList(scrapyards);
	
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		Model::Scrapyard *s = scrapyards[ii];
		const UnitInfo *unitType = UnitInfo::getUnitType(s->currentProduction());
		float cost = unitType->getCost();
		
		if(s->owner==0 || s->startTime+cost > getTime())
			continue;
		
		ICoord pos = model->spaceNearestTo(s->center.x, s->center.y, unitType->flying);
		unitID id = createUnit(s->owner, s->currentProduction(), pos.x, pos.y);
		if(!(s->rally==s->center)) {
			Model::Unit *unit = model->getUnit(id);
			model->setUnitDestination(id, s->rally.x, s->rally.y);
			unit->state = Model::Unit::move;
		}
		
		s->produceNext();
		
		updateScrapyard(ii);
	}
}

/// Start producing the next thing in the build queue (current one finished)
void Model::Scrapyard::produceNext()
{
	const UnitInfo *unitType = UnitInfo::getUnitType(currentProduction());
	startTime += unitType->getCost();
	
	if(repeating) {
		queuePos++;
		if(queuePos >= (int)buildQueue.size())
			queuePos = 0;
	}
	else if(buildQueue.size() > 1) {
		buildQueue.erase(buildQueue.begin()+queuePos);
	}
	if(buildQueue.size()==1)
		repeating = true;
}

/// Add #unit to the end of this scrapyard's build queue
void Model::Scrapyard::enqueue(std::string unit)
{
	if(unit=="repeat")
	{
		repeating = true;
		return;
	}
	
	repeating = false;
	while(queuePos > 0) {
		buildQueue.erase(buildQueue.begin());
		queuePos--;
	}
	if(buildQueue.size() < maxQueueSize)
		buildQueue.push_back(unit);
}

/// Cancel the nth unit in the build queue
void Model::Scrapyard::cancel(int index)
{
	if(index >= buildQueue.size())
		return;
	
	// Cancelling the current unit costs half your progress
	if(index == queuePos)
		startTime = (startTime+getTime())/2;
	
	buildQueue.erase(buildQueue.begin()+index);
	if(queuePos > index)
		queuePos--;
	if(queuePos >= buildQueue.size())
		queuePos = 0;
	
	// If nothing left in the queue, start making the default unit
	if(buildQueue.size() == 0)
		buildQueue.push_back(defaultUnit);
	if(buildQueue.size() == 1)
		repeating = true;
}

void Model::Scrapyard::clear()
{
	startTime = getTime();
	buildQueue.clear();
	buildQueue.push_back(defaultUnit);
	repeating = true;
	queuePos = 0;
}

/// Get the unit type which is currently being produced
std::string Model::Scrapyard::currentProduction() const
{
	return buildQueue[queuePos];
}

/// Send a network update with the status (build queue, etc) of a scrapyard
void ServerController::updateScrapyard(int id)
{
	if(network)
	{
		Model::Scrapyard *s = model->getScrapyard(id);
		
		Packet *msg = NEW Packet(msg_update_scrapyard);
			msg->putInt(id);
			msg->putInt(s->owner);
			msg->putFloat(getTime() - s->startTime);
			msg->putShort(s->queuePos);
			msg->putChar(s->repeating?1:0);
			msg->putShort(s->buildQueue.size());
			msg->putShort(s->rally.x);
			msg->putShort(s->rally.y);
			for(unsigned ii=0; ii<s->buildQueue.size(); ii++)
				msg->putString(s->buildQueue[ii]);
		network->sendToAll(msg);
	}
}

