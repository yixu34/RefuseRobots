#include "main.hpp"

// Directly update the model based on the command type
void ServerController::processCommands()
{
	while (!incomingCommands->empty())
	{
		Command &command = incomingCommands->front();
		
		for(std::vector<unitID>::iterator ii=command.units.begin();
		    ii!=command.units.end(); )
		{
			if(!model->getUnit(*ii))
				ii = command.units.erase(ii);
			else
				ii++;
		}
		
		typedef std::vector<unitID> UnitList;
		switch (command.type)
		{
		case Command::buildUnit: {
			Model::Scrapyard *s = model->getScrapyard(command.targetScrapyard());
			s->enqueue(command.targetUnitType());
			updateScrapyard(command.targetScrapyard());
			break;
		}
		
		case Command::cancelBuild: {
			Model::Scrapyard *s = model->getScrapyard(command.targetScrapyard());
			s->cancel(command.targetIndex());
			updateScrapyard(command.targetScrapyard());
			break;
		}
		
		case Command::setRally: {
			Model::Scrapyard *s = model->getScrapyard(command.targetScrapyard());
			s->rally = command.targetTile();
			break;
		}
		
		case Command::move:
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				ICoord dest = command.targetTile();
				model->setUnitDestination(*it, dest.x, dest.y);
			}
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				Model::Unit *u = model->getUnit(*it);
				u->state = Model::Unit::move;
				u->escort = 0;
				u->explicitTarget = false;
				u->target = 0;
				if(!u->moving)
					pathfindUnit(*it);
			}
			break;
			
		case Command::retreat:
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				Model::Unit *u = model->getUnit(*it);
				ICoord dest = model->nearestScrapyardTo(u->nextX, u->nextY);
				model->setUnitDestination(*it, dest.x, dest.y);
			}
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				Model::Unit *u = model->getUnit(*it);
				u->state = Model::Unit::move;
				u->escort = 0;
				u->explicitTarget = false;
				u->target = 0;
				if(!u->moving)
					pathfindUnit(*it);
			}
			break;
		
		case Command::follow: {
			Model::Unit *target = model->getUnit(command.targetUnit());
			if(!target) break;
			
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++)
			{
				Model::Unit *u = model->getUnit(*it);
				if(u == target)
					continue;
				u->target = 0;
				u->escort = target->getId();
				u->state = Model::Unit::follow;
				u->explicitTarget = true;
				
				// If able to pick the target up, do so
				if(u->loadedUnits.size() < u->type->transports && target->type->transportable)
					target->escort = *it;
			}
			break;
			}
			
		case Command::stop:
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++)
				stopUnit(*it);
			break;
		
		case Command::attack: {
			Model::Unit *target = model->getUnit(command.targetUnit());
			if(!target) break;
			
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++)
			{
				Model::Unit *u = model->getUnit(*it);
				if(u == target)
					continue;
				if(canTarget(u, target)) {
					u->target = target->getId();
					u->state = Model::Unit::attack;
					u->escort = 0;
					u->explicitTarget = true;
					if(canShoot(u, target)) {
						if(u->type->facesTarget &&
						   angleDifference(u->angle, angleHeading(u->x, u->y, target->nextX, target->nextY)) > 10)
							turnAndShoot(u, target);
						else
							fireBurst(u, target);
					}
				}
			}
			break;
		}
		
		case Command::attackMove:
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				ICoord dest = command.targetTile();
				model->setUnitDestination(*it, dest.x, dest.y);
			}
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++) {
				Model::Unit *u = model->getUnit(*it);
				u->state = Model::Unit::attack;
				u->escort = 0;
				u->explicitTarget = false;
				u->target = 0;
				if(!u->moving)
					pathfindUnit(*it);
			}
			break;
			
		case Command::unload:
			for (UnitList::const_iterator it = command.units.begin(); it != command.units.end(); it++)
			{
				unitID id = *it;
				Model::Unit *transport = model->getUnit(id);
				
				for(std::vector<unitID>::iterator jj=transport->loadedUnits.begin(); jj!=transport->loadedUnits.end(); jj++)
					unloadUnit(id, *jj);
				transport->loadedUnits.clear();
			}
			break;
		
		case Command::unloadSingle: {
			int who = command.targetUnit();
			for (UnitList::const_iterator ii = command.units.begin(); ii != command.units.end(); ii++) {
				Model::Unit *transport = model->getUnit(*ii);
				
				for(std::vector<unitID>::iterator jj=transport->loadedUnits.begin(); jj!=transport->loadedUnits.end(); jj++)
				{
					if(*jj == who) {
						unloadUnit(transport->getId(), *jj);
						transport->loadedUnits.erase(jj);
						break;
					}
				}
			}
			break;
		}
			
		case Command::patrol:
			break;
		
		case Command::holdPosition:
			break;
		
		default:
			break;
		}

		incomingCommands->pop_front();
	}
}

void ServerController::stopUnit(unitID id)
{
	Model::Unit *u = model->getUnit(id);
	u->destX = u->nextX;
	u->destY = u->nextY;
	u->state = Model::Unit::move;
	u->explicitTarget = false;
	u->target = 0;
}

void ServerController::updateUnitFuel(unitID id)
{
	if(network) {
		// Send a pathfind message so the fuel display will update
		Model::Unit *u = model->getUnit(id);
		if(!u) return;
		Packet *fuelMsg = NEW Packet(msg_update_fuel);
			fuelMsg->putInt(id);
			fuelMsg->putInt(u->fuel);
		network->sendToAll(fuelMsg);
	}
}

void ServerController::thinkUnits()
{
	std::vector<unitID> units;
	model->getUnitList(units);
	bool stoppedFighting = false;
	
	for(std::vector<unitID>::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = model->getUnit(*ii);
		Model::Unit *target= model->getUnit(u->target);
		Model::Tile *tile = model->getTile(u->x, u->y);
		
		if(target && !model->playerCanSee(u->owner, target->nextX, target->nextY)) {
			u->target = 0;
			target = NULL;
		}
		
		if(u->inTransport)
			continue;
		
		if(u->type->usesFuel || u->type->refueler)
		{
			if(u->fuel == 0)
			{
				viewNotifyLowFuel(u->getId(), true);
			}
			else if(u->fuel < 0.2*u->fuelMax)
			{
				viewNotifyLowFuel(u->getId(), false);
			}
		}

		if(tile->providesFuel && (u->type->usesFuel || u->type->refueler)) {
			if(u->fuel != u->fuelMax) {
				u->fuel = u->fuelMax;
				updateUnitFuel(u->getId());
			}
			if(!u->moving)
				pathfindUnit(u->getId());
		}
		
		if(u->type->refueler) {
			// Check adjacent tiles for units in need of fuel
			int minX, minY, maxX, maxY;
			mapRect(model, ICoord(u->x, u->y), ICoord(1,1), minX, minY, maxX, maxY);
			
			for(unsigned yi=minY; yi<=(unsigned)maxY; yi++)
			for(unsigned xi=minX; xi<=(unsigned)maxX; xi++) {
				Model::Unit *refuelee = model->getUnitAt(xi, yi, true);
				if(!refuelee) refuelee = model->getUnitAt(xi, yi, false);
				if(!refuelee) continue;
				if(!refuelee->type->usesFuel) continue;
				if(refuelee->owner != u->owner) continue;
				if(u->fuel==0 || refuelee->fuel==refuelee->fuelMax) continue;
				int fuelNeeded = (refuelee->fuelMax - refuelee->fuel) / refuelee->type->mileage;
				if(u->fuel > fuelNeeded) {
					u->fuel -= fuelNeeded;
					refuelee->fuel = refuelee->fuelMax;
				} else {
					refuelee->fuel += u->fuel * refuelee->type->mileage;
					u->fuel = 0;
				}
				updateUnitFuel(refuelee->getId());
				updateUnitFuel(u->getId());
			}
		}
		
		if(u->type->convertsScrapyard) {
			if(tile->scrapyard >= 0) {
				Model::Scrapyard *s = model->getScrapyard(tile->scrapyard);
				if(s->owner != u->owner && u->state != Model::Unit::sinking) {
					u->state = Model::Unit::sinking;
					updateUnitState(u->getId());
				}
				else if(u->state == Model::Unit::sinking && s->owner == u->owner){
					u->z = 0;
					u->state = Model::Unit::move;
					updateUnitState(u->getId());
				}
				else if(u->state == Model::Unit::sinking && u->z <= -1.0)
				{
					convertScrapyard(tile->scrapyard, u->owner);
					killUnit(u->getId(), Explosion::none);
					continue;
				}
			}
		}
		
		if(u->target && !target) {
			u->target = 0;
			u->explicitTarget = false;
			if(u->state == Model::Unit::fighting) {
				u->state = Model::Unit::attack;
				stoppedFighting = true;
			}
		}
		
		if(stoppedFighting && !u->moving)
			pathfindUnit(*ii);
		
		if(target && getTime() > u->nextShotTime) {
			if(u->turningToShoot)
			{ }
			else if(
			   u->type->facesTarget && // If this should face its target but needs to turn first
			   angleDifference(u->angle, angleHeading(u->x, u->y, target->nextX, target->nextY)) > 10 &&
			   canShoot(u, target))
			{
				turnAndShoot(u, target);
			} else {
				fireBurst(u, target);
			}
		}
		
		if(u->escort) {
			Model::Unit *escort = model->getUnit(u->escort);
			if(escort && !escort->inTransport) {
				model->setUnitDestination(u->getId(), escort->x, escort->y);
				if( abs(u->x-escort->x)<=1 && abs(u->y-escort->y)<=1 &&
				    escort->type->transports > escort->loadedUnits.size() &&
				    u->type->transportable)
				{
					u->escort = 0;
					u->state = Model::Unit::move;
					loadUnit(u->getId(), escort->getId());
				}
			} else
				u->escort = 0;
		}
	}
	
	acquireTargets();
}

