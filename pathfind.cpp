#include "main.hpp"

class PathMap
{
public:
	friend PathMap *lookupPath(ICoord dest, Model *model);
	
	PathMap(const PathMap &copy);
	~PathMap();
	
	double lastReference;
	short **distances;
	
protected:
	PathMap(ICoord dest, Model *model);
	void fillPaths(ICoord dest, Model *model);
	
	int sizeX, sizeY;
	int *refcount;
};
typedef std::map<ICoord, PathMap*> PathCache;
static PathCache cachedPaths;

PathMap *lookupPath(ICoord dest, Model *model)
{
	PathCache::iterator it = cachedPaths.find(dest);
	if(it != cachedPaths.end()) {
		PathMap *ret = it->second;
		ret->lastReference = getTime();
		return ret;
	}
	else {
		PathMap *path = new PathMap(dest, model);
		cachedPaths[dest] = path;
		return path;
	}
}

int distanceFrom(Model *model, ICoord source, ICoord dest)
{
	PathMap *pathfinding = lookupPath(dest, model);
	return pathfinding->distances[source.y][source.x];
}

// Un-cache any path which hasn't been used for 5 seconds
void cleanPathCache()
{
	for(PathCache::iterator ii=cachedPaths.begin(); ii!=cachedPaths.end(); )
	{
		if(ii->second->lastReference+5 < getTime()) {
			delete ii->second;
			ii = cachedPaths.erase(ii);
		}
		else
			ii++;
	}
}

PathMap::PathMap(ICoord dest, Model *model)
{
	sizeX = model->getSizeX(),
	sizeY = model->getSizeY();
	lastReference = getTime();
	
	refcount = new int(1);
	distances = new short*[sizeY];
	for(int ii=0; ii<sizeY; ii++)
		distances[ii] = new short[sizeX];
	
	fillPaths(dest, model);
}

void PathMap::fillPaths(ICoord dest, Model *model)
{
	std::deque<ICoord> frontier;
	
	for(int ii=0; ii<sizeY; ii++)
	for(int jj=0; jj<sizeX; jj++)
	{
		distances[ii][jj] = 32765;
	}
	distances[dest.y][dest.x] = 0;
	frontier.push_back(dest);
	
	while(frontier.size() > 0)
	{
		ICoord pos = *frontier.begin();
		frontier.pop_front();
		int dist = distances[pos.y][pos.x] + 1;
		
		int minX = pos.x-1, minY = pos.y-1,
		    maxX = pos.x+1, maxY = pos.y+1;
		if(minX<0) minX=0;
		if(minY<0) minY=0;
		if(maxX>=sizeX) maxX=sizeX-1;
		if(maxY>=sizeY) maxY=sizeY-1;
		
		if(model->getTile(pos.x, pos.y)->type->passable) {
			for(int yi=minY; yi<=maxY; yi++)
			for(int xi=minX; xi<=maxX; xi++)
			{
				if(model->getTile(xi, yi)->type->passable && distances[yi][xi] > dist) {
					frontier.push_back(ICoord(xi, yi));
					distances[yi][xi] = dist;
				}
			}
		} else {
			for(int yi=minY; yi<=maxY; yi++)
			for(int xi=minX; xi<=maxX; xi++)
			{
				if(distances[yi][xi] > dist) {
					frontier.push_back(ICoord(xi, yi));
					distances[yi][xi] = dist;
				}
			}
		}
	}
}

PathMap::PathMap(const PathMap &copy)
{
	refcount = copy.refcount;
	distances = copy.distances;
	(*refcount)++;
}

PathMap::~PathMap()
{
	(*refcount)--;
	
	if(*refcount == 0) {
		delete refcount;
		for(int ii=0; ii<sizeY; ii++)
			delete[] distances[ii];
		delete[] distances;
	}
}

//////////////////////////////////////////////////////////////////////////////

void ServerController::turnAndShoot(Model::Unit *shooter, Model::Unit *target)
{
	shooter->destAngle = angleHeading(shooter->x, shooter->y, target->nextX, target->nextY);
	shooter->turningToShoot = true;
	if(network) {
		Packet *pathfindMsg = NEW Packet(msg_pathfind);
			pathfindMsg->putInt(shooter->getId());
			pathfindMsg->putShort(shooter->nextX);
			pathfindMsg->putShort(shooter->nextY);
			pathfindMsg->putShort(shooter->destAngle);
		network->sendToAll(pathfindMsg);
	}
}

void ServerController::pathfindUnit(unitID id)
{
	Model::Unit *u = model->getUnit(id);
	
	u->moving = false;
	
	if(u->state == Model::Unit::fighting) {
		u->moving = (u->nextX != (int)u->x || u->nextY != (int)u->y);
		return;
	}
	if(u->fuel == 0 && u->type->usesFuel) {
		return;
	}
	
	int u_x = (int)(u->x + .01),
	    u_y = (int)(u->y + .01);
	
	int minX, maxX, minY, maxY;
	int nextX=u->nextX, nextY=u->nextY;
	mapRect(model, ICoord(u_x,u_y), ICoord(1,1), minX, minY, maxX, maxY);
	
	// If this is a unit which has to turn towards its target to shoot, and
	// it's in a state where it should shoot (attack state or idle), then turn
	// towards the target.
	if(u->type->facesTarget && u->target && u->nextShotTime<=getTime()
	 && (u->state==Model::Unit::attack || u->state==Model::Unit::fighting ||
	    (u->destX==u->x && u->destY==u->y))
	 && !u->turningToShoot)
	{
		Model::Unit *target = model->getUnit(u->target);
		if(!target) {
			u->target = 0;
		} else {
			turnAndShoot(u, target);
			return;
		}
	}
	
	if(u->type->flying)
	{
		// For flying units, use super-simple pathfinding: Always go to the adjacent
		// open tile which is nearest the destination (Cartesian distance).
		int distSq = (u_x-u->destX)*(u_x-u->destX) + (u_y-u->destY)*(u_y-u->destY);
		
		for(int ii=minY; ii<=maxY; ii++)
		for(int jj=minX; jj<=maxX; jj++)
		{
			if(!model->getTile(jj, ii)->passable(true))
				continue;
			
			int newDistSq = ((jj-u->destX)*(jj-u->destX) + (ii-u->destY)*(ii-u->destY));
			if(newDistSq < distSq) {
				distSq = newDistSq;
				nextX = jj;
				nextY = ii;
			}
		}
	}
	else
	{
		// For non-flying units, use path-finding distance instead of Cartesian
		// distance.
		PathMap *pathfinding = lookupPath(ICoord(u->destX, u->destY), model);
		int dist = pathfinding->distances[u_y][u_x];
		int origDist = dist;
		
		for(int ii=minY; ii<=maxY; ii++)
		for(int jj=minX; jj<=maxX; jj++)
		{
			if(!model->getTile(jj, ii)->passable(false))
				continue;
			int newDist = pathfinding->distances[ii][jj];
			if( // If this is closer in pathfinding distance or equal in
			    // pathfinding distance and closer in Manhattan distance, go there
			    newDist < dist ||
			    (newDist==dist && abs(u->destX-jj)+abs(u->destY-ii) < abs(u->destX-nextX)+abs(u->destY-nextY))
			    // Favor lateral moves over diagonal
			  ||(newDist == dist && (ii==u_y||jj==u_x) && newDist<origDist))
			{
				dist = newDist;
				nextX = jj;
				nextY = ii;
			}
		}
	}
	
	if(u->type->getSpeed() > 0) // Don't let immobile units move
	{
		u->moving = (nextX != u_x || nextY != u_y);
		if(u->moving)
			model->setUnitPath(id, nextX, nextY);
		model->getTile(u->nextX, u->nextY)->unitAt(u->type->flying) = id;
	}
	
	if(u->x == u->nextX && u->y == u->nextY)
		return;
	
	updateUnitFacing(id);
	
	if(u->type->usesFuel) {
		u->fuel--;
		if(u->fuel < 0)
			u->fuel = 0;
	}
	
	viewNotifyMoveUnit(id, ICoord(u->x, u->y), ICoord(u->nextX, u->nextY));
	
	// Update the unit's position over the network at this time.
	if(network) {
		Packet *pathfindMsg = NEW Packet(msg_pathfind);
			pathfindMsg->putInt(id);
			pathfindMsg->putShort(u->nextX);
			pathfindMsg->putShort(u->nextY);
			pathfindMsg->putShort(u->destAngle);
		network->sendToAll(pathfindMsg);
	}
}

