#include "main.hpp"

class UnitStore
{
public:
	UnitStore(Model *model);
	void addUnit(Model::Unit *u);
	void unitsNear(int x, int y, int radius, std::vector<Model::Unit*> &outUnits);
	
protected:
	int width, height;
	std::vector<Model::Unit*> ***areas;
};
const int granularity = 16;

UnitStore::UnitStore(Model *model)
{
	width  = (model->getSizeX()+(granularity-1))/granularity,
	height = (model->getSizeY()+(granularity-1))/granularity;
	
	areas = new std::vector<Model::Unit*>**[height];
	for(int ii=0; ii<height; ii++)
	{
		areas[ii] = new std::vector<Model::Unit*>*[width];
		for(int jj=0; jj<width; jj++)
			areas[ii][jj] = NULL;
	}
}

void UnitStore::addUnit(Model::Unit *u)
{
	int x = u->nextX / granularity,
	    y = u->nextY / granularity;
	if(!areas[y][x])
		areas[y][x] = new std::vector<Model::Unit*>;
	areas[y][x]->push_back(u);
}

static int visits=0, lookups=0;

void UnitStore::unitsNear(int x, int y, int radius, std::vector<Model::Unit*> &outUnits)
{
	int minX = (x-radius)/granularity,
	    minY = (y-radius)/granularity,
	    maxX = (x+radius+(granularity-1))/granularity,
	    maxY = (y+radius+(granularity-1))/granularity;
	if(minX<0) minX=0;
	if(minY<0) minY=0;
	if(maxX>=width)  maxX=width-1;
	if(maxY>=height) maxY=height-1;
	
	visits += (maxX-minX+1)*(maxY-minY+1);
	lookups++;
	
	for(int yi=minY; yi<=maxY; yi++)
	for(int xi=minX; xi<=maxX; xi++)
	{
		if(areas[yi][xi])
			outUnits.insert(outUnits.begin(), areas[yi][xi]->begin(), areas[yi][xi]->end());
	}
}

void ServerController::acquireTargets()
{
	UnitStore **units = new UnitStore*[model->maxPlayers+1];
	for(unsigned ii=1; ii<=model->maxPlayers; ii++)
		units[ii] = new UnitStore(model);
	
	std::vector<unitID> allUnitIDs;
	std::vector<Model::Unit*> allUnits;
	model->getUnitList(allUnitIDs);
	
	for(std::vector<unitID>::iterator ii=allUnitIDs.begin(); ii!=allUnitIDs.end(); ii++)
	{
		Model::Unit *u = model->getUnit(*ii);
		allUnits.push_back(u);
		units[u->owner]->addUnit(u);
	}
	
	for(std::vector<Model::Unit*>::iterator ii=allUnits.begin(); ii!=allUnits.end(); ii++)
	{
		Model::Unit *u = *ii;
		if(u->explicitTarget)
			continue;
		Model::Unit *target = model->getUnit(u->target);
		for(unsigned ii=1; ii<=model->maxPlayers; ii++)
		{
			if(ii==u->owner) // Don't attempt to target your own units
				continue;
			std::vector<Model::Unit*> candidates;
			units[ii]->unitsNear(u->nextX, u->nextY, u->type->range+1, candidates);
			
			for(std::vector<Model::Unit*>::iterator ii=candidates.begin(); ii!=candidates.end(); ii++)
			{
				Model::Unit *newtarget = *ii;
				if(canAutoTarget(u, newtarget) && isBetterTarget(u, newtarget, target))
				{
					u->target = newtarget->getId();
					target = newtarget;
					if(u->state == Model::Unit::attack)
						u->state = Model::Unit::fighting;
				}
			}
			if(target && u->nextShotTime<=getTime() && canShoot(u, target)) {
				if(u->type->facesTarget &&
					angleDifference(u->angle, angleHeading(u->x, u->y, target->nextX, target->nextY)) > 10)
					turnAndShoot(u, target);
				else
					fireBurst(u, target);
			}
		}
		if(u->state==Model::Unit::fighting && u->target==0)
			u->state = Model::Unit::attack;
	}
}

bool ServerController::canAutoTarget(Model::Unit *u, Model::Unit *target)
{
	// If it can't be targetted *at all*, it can't be targetted *automatically*
	if(!canTarget(u,target))
		return false;
	
	// Don't auto-target your own units
	if(u->owner == target->owner)
		return false;
	
	// Don't auto-target unseen units
	if(!model->playerCanSee(u->owner, target->nextX, target->nextY))
		return false;
	
	// Don't auto-target units not in range
	float distSq = (u->x-target->x)*(u->x-target->x) + (u->y-target->y)*(u->y-target->y);
	
	if(distSq > u->type->range*u->type->range ||
	   distSq < u->type->minimumRange*u->type->minimumRange)
		return false;
	
	// Otherwise, go for it.
	return true;
}

bool ServerController::canTarget(Model::Unit *u, Model::Unit *target)
{
	// Can't target null nothing
	if(!target) return false;
	
	// Can't target units you can't shoot at (air/ground)
	if(target->type->flying) {
		if(!u->type->targetsAir)
			return false;
	} else {
		if(!u->type->targetsGround)
			return false;
	}
	
	return true;
}

bool ServerController::canShoot(Model::Unit *u, Model::Unit *target)
{
	float targetX = target->nextX,
	      targetY = target->nextY;
	float distSq = (u->x-targetX)*(u->x-targetX) + (u->y-targetY)*(u->y-targetY);
	bool airTarget = target->type->flying;
	
	if(distSq > u->type->range*u->type->range ||
	   distSq < u->type->minimumRange*u->type->minimumRange)
		return false;
	
	if(target->type->flying)
		return u->type->targetsAir;
	else
		return u->type->targetsGround;
}

bool ServerController::isBetterTarget(Model::Unit *u, Model::Unit *newTarget, Model::Unit *oldTarget)
{
	// Something is better than nothing
	if(newTarget && !oldTarget)
		return true;
	
	// In range is better than out of range
	float oldDistSq = (u->x-oldTarget->x)*(u->x-oldTarget->x) + (u->y-oldTarget->y)*(u->y-oldTarget->y);
	float newDistSq = (u->x-newTarget->x)*(u->x-newTarget->x) + (u->y-newTarget->y)*(u->y-newTarget->y);
	float rangeSq = u->type->range*u->type->range;
	float minRangeSq = u->type->minimumRange*u->type->minimumRange;
	if( (oldDistSq>rangeSq||oldDistSq<minRangeSq) && (newDistSq<=rangeSq&&newDistSq>=minRangeSq) )
		return true;
	
	// Otherwise, keeping the same is better than switching target
	return false;
}
