#include "main.hpp"
#include <cassert>
#include <fstream>
#include <map>
using namespace std;

const char *bonusUnits[] = {
	"transport",
	"heavytank",
	"infantry",
	"infantry",
	};

Model::Model(const char *filename)
{
	using Parser::Section;
	using Parser::Line;
	
	std::string filenameStr = std::string("maps/")+filename;
	Parser parser(filenameStr.c_str());
	
	const Section *attributes = parser.getSection("attributes");
	const Section *starts     = parser.getSection("players");
	const Section *resources  = parser.getSection("resources");
	const Section *content    = parser.getSection("content");
	
	sizeX = sizeY = 0;
	std::string tilesetName = "tileset_default.txt";
	
	std::map<std::string, TileType*> tileset;
	
	// Load map attributes
	for(Section::const_iterator ii=attributes->begin(); ii!=attributes->end(); ii++)
	{
		Line L = *ii;
		if(L.size()<2)
			continue;
		
		if(L[0]=="tileset")
			tilesetName = L[1];
		else if(L[0]=="size") {
			sizeX = atoi(L[1].c_str());
			sizeY = atoi(L[2].c_str());
		}
		else if(L[0]=="description") {
		}
		else {
			throw MapLoadError(retprintf("Unrecognized attribute: '%s'", L[0].c_str()));
		}
	}
	
	//setRandomPlayers(starts->size());

	// Load 'resources' section
	for(Section::const_iterator ii=resources->begin(); ii!=resources->end(); ii++)
	{
		Line L = *ii;
		if(L[0]=="landfill") {
			if(L.size() < 6)
				throw MapLoadError("Not enough arguments to resource 'landfill'");

			scrapyards.push_back(Scrapyard(
				//randomPlayerIDs[atoi(L[1].c_str())],
				atoi(L[1].c_str()),
				atoi(L[2].c_str()), atoi(L[3].c_str()),
				atoi(L[4].c_str()), atoi(L[5].c_str()),
				scrapyards.size()));
		} else if(L[0]=="oil") {
			if(L.size() < 5)
				throw MapLoadError("Not enough arguments to resource 'oil'");
			
			oilWells.push_back(OilWell(
				atoi(L[1].c_str()), atoi(L[2].c_str()),
				atoi(L[3].c_str()), atoi(L[4].c_str())
				));
		} else {
			throw MapLoadError(retprintf("Invalid resource type: '%s'", L[0].c_str()));
		}
	}
	
	loadTileset(tilesetName, tileset);
	
	// Sanity-check map parameters
	if(sizeX==0 || sizeY==0)
		throw MapLoadError("Given map size is 0 or no size given.");
	if(sizeY != content->size() || sizeX != (*content)[0].size()) {
		throw MapLoadError(retprintf("Map size mismatch: Stated size is (%i,%i) but calculated size is (%i,%i)",
			(int)sizeX, (int)sizeY, (*content)[0].size(), content->size()));
	}
	
	// Allocate map
	map = NEW Tile*[sizeY];
	for(unsigned ii=0; ii<sizeY; ii++)
		map[ii] = NEW Tile[sizeX];
	
	// Mark scrapyard tiles as such
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		for(int xi =  scrapyards[ii].center.x - scrapyards[ii].size.x/2;
		        xi <= scrapyards[ii].center.x + scrapyards[ii].size.x/2; xi++)
		for(int yi =  scrapyards[ii].center.y - scrapyards[ii].size.y/2;
		        yi <= scrapyards[ii].center.y + scrapyards[ii].size.y/2; yi++)
		{
			map[yi][xi].scrapyard = ii;
		}
	}
	// Mark oil well tiles as giving fuel
	for(unsigned ii=0; ii<oilWells.size(); ii++)
	{
		for(int xi =  oilWells[ii].center.x - oilWells[ii].size.x/2;
		        xi <= oilWells[ii].center.x + oilWells[ii].size.x/2; xi++)
		for(int yi =  oilWells[ii].center.y - oilWells[ii].size.y/2;
		        yi <= oilWells[ii].center.y + oilWells[ii].size.y/2; yi++)
		{
			map[yi][xi].providesFuel = 1;
		}
	}
	
	// Load map tiles
	for(unsigned ii=0; ii<content->size(); ii++)
	{
		const Line *L = &(*content)[ii];
		
		if(L->size() != sizeX) {
			throw MapLoadError(retprintf("Line %i of map content is the wrong length", ii));
		}
		
		for(unsigned jj=0; jj<L->size(); jj++)
		{
			if(tileset.find((*L)[jj]) == tileset.end()) {
				throw MapLoadError(retprintf("Tile '%s' not in tileset %s",
					(*L)[jj].c_str(), tilesetName.c_str()));
			}
			map[ii][jj].type = tileset[ (*L)[jj] ];
		}
	}
	
	maxPlayers = starts->size();
	for(unsigned ii=1; ii<=maxPlayers; ii++)
	{
		fogOfWar[ii] = new unsigned short*[sizeY+1];
		for(unsigned yi=0; yi<=sizeY; yi++) {
			fogOfWar[ii][yi] = new unsigned short[sizeX+1];
			
			for(unsigned xi=0; xi<=sizeX; xi++)
				fogOfWar[ii][yi][xi] = 0;
		}
	}
	
	// Reveal fog of war around scrapyards
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		if(scrapyards[ii].owner) {
			revealFog(scrapyards[ii].owner, scrapyardSightRadius,
			          scrapyards[ii].center.x, scrapyards[ii].center.y);
		}
	}
	
	maxUnitID   = 0;
	maxPlayerID = 0;
	
	// Give starting units
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		if(scrapyards[ii].owner) {
			for(unsigned jj=0; jj<(sizeof bonusUnits)/sizeof(const char*); jj++)
			{
				ICoord pos = spaceNearestTo(scrapyards[ii].center.x, scrapyards[ii].center.y, false);
				addUnit(scrapyards[ii].owner, bonusUnits[jj], pos.x, pos.y);
			}
		}
	}
}
void Model::setRandomPlayers(int size)
{
	randomPlayerIDs.push_back(0);
	for(int x = 0; x < size; x++)
	{
		bool resolved = false;
		int randomNumber = 0;
		while(!resolved)
		{
			bool found = false;
			randomNumber = rand()%(size + 1);
			for(int y = 0; y < randomPlayerIDs.size(); y++)
			{
				if(randomPlayerIDs[y] == randomNumber)
					found = true;
			}
			if(!found) 
				resolved = true;
		}
		randomPlayerIDs.push_back(randomNumber);
	}
}
void Model::loadTileset(std::string filename, std::map<std::string, TileType*> &result)
{
	Parser tileset(filename.c_str());
	const Parser::Section *tiles = tileset.getSection("tiles");
	for(Parser::Section::const_iterator ii=tiles->begin(); ii!=tiles->end(); ii++)
	{
		Parser::Line L = *ii;
		if(L.size() < 3) continue;
		TileType *type = NEW TileType(L[0].c_str(), Image(L[1]), atoi(L[2].c_str())?true:false);
		result[L[0]] = type;
	}	
}

Model::~Model()
{
	for(unsigned ii=0; ii<sizeY; ii++)
		delete map[ii];
	delete map;
}

unsigned short** Model::getFog(playerID player)
{
	return fogOfWar[player];
}

Model::Unit *Model::getUnitAt(unsigned x, unsigned y, bool air)
{
	// Must check for a valid corresponding tile or crash!
	Tile *tile = getTile(x, y);
	if (!tile)
		return NULL;

	unitID id = tile->unitAt(air);
	if(!id)
		return NULL;
	else {
		if(units.find(id) == units.end()) {
			// This shouldn't happen, but it's been seen happening enough to not make it just an
			// assertion condition
			static bool didErrorMsg = false;
			if(!didErrorMsg) {
				didErrorMsg = true;
				printChatText(retprintf("Unit missing from tile at (%i,%i)%s", x, y, (air?" (air)":"")), 0);
			}
			return NULL;
		}
		return units[id];
	}
}

Model::Unit *Model::getUnit(unitID id)
{
	if(units.find(id) == units.end())
		return NULL;
	return units[id];
}

void Model::deleteUnit(unitID id)
{
	assert(units.find(id) != units.end());
	
	// Put it firmly on a tile, so it can then be removed from it
	Unit *u = units[id];
	concealFog(u->owner, u->type->vision, u->nextX, u->nextY);
	setUnitPosition(id, u->nextX, u->nextY);
	if(map[u->nextY][u->nextX].unitAt(u->type->flying) == id)
		map[u->nextY][u->nextX].unitAt(u->type->flying) = 0;
	
	units.erase(id);
	delete u;
}

void Model::putUnitInTransport(unitID unitId, unitID transportId)
{
	Model::Unit *unit = getUnit(unitId),
	            *transport = getUnit(transportId);
	
	concealFog(unit->owner, unit->type->vision, unit->nextX, unit->nextY);
	setUnitPosition(unitId, unit->nextX, unit->nextY);
	if(map[unit->nextY][unit->nextX].unitAt(unit->type->flying) == unitId)
		map[unit->nextY][unit->nextX].unitAt(unit->type->flying) = 0;
	
	
	transport->loadedUnits.push_back(unitId);
	unit->state = Model::Unit::move;
	unit->inTransport = true;
	unit->x = unit->y = -1;
}

/// Precondition: (x,y) is an EMPTY tile
unitID Model::addUnit(playerID player, std::string type, double x, double y, unitID unitId)
{
	// If the server is creating the unit for the first time, then 
	// unitId will be zero.  
	// If the client is being notified of the unit, then keep the unitID.
	int nextId = (unitId == 0) ? (newUnitID()) : (unitId);
	int clampX = static_cast<int>(x);
	int clampY = static_cast<int>(y);
	
	Unit *newUnit = units[nextId] = NEW Unit(type.c_str());
	map[clampY][clampX].unitAt(newUnit->type->flying) = nextId;
	
	newUnit->id      = nextId;
	newUnit->x       = x;
	newUnit->y       = y;
	newUnit->destX   = clampX;
	newUnit->destY   = clampY;
	newUnit->nextX   = clampX;
	newUnit->nextY   = clampY;
	newUnit->owner   = player;

	revealFog(player, newUnit->type->vision, newUnit->nextX, newUnit->nextY);
	
	return nextId;
}

playerID Model::addPlayer()
{
	// The server's model is adding its own player id to the game.
	playerID newId = ++maxPlayerID;
	//players[newId] = new Player("TEST1", newId);
	return newId;
}

playerID Model::addPlayer(playerID id)
{
	// From a client's model's perspective, some new player id showed up, 
	// so add him to the game.
	//players[id] = new Player("TEST2", id);
	return id;
}

unitID Model::newUnitID()
{
	return (++maxUnitID);
}

void Model::setUnitPosition(unitID unit, double x, double y)
{
	assert(units.find(unit) != units.end());
	
	Unit *u = units[unit];
	Tile *oldTile;
	Tile *newTile = getTile((unsigned)(x+0.5), (unsigned)(y+0.5));
	
	if(u->inTransport)
		oldTile = NULL;
	else
		oldTile = getTile((unsigned)(u->getX()+0.5), (unsigned)(u->getY()+0.5));
	
	u->setX(x);
	u->setY(y);
	
	if(oldTile != newTile && oldTile && oldTile->unitAt(u->type->flying) == unit) {
		oldTile->unitAt(u->type->flying) = 0;
	}
	newTile->unitAt(u->type->flying) = unit;
}

void Model::setUnitDestination(unitID unit, int x, int y)
{
	if (units.find(unit) == units.end())
		return;

	Unit *u = units[unit];
	u->destX = x;
	u->destY = y;
}

void Model::setUnitPath(unitID unit, int x, int y)
{
	if (units.find(unit) == units.end())
		return;

	Unit *u = units[unit];
	
	concealFog(u->owner, u->type->vision, u->nextX, u->nextY);
	revealFog(u->owner, u->type->vision, x, y);
	
	u->nextX = x;
	u->nextY = y;
}

void Model::getUnitList(std::vector<unitID> &vec)
{
	for(UnitMap::iterator ii=units.begin(); ii!=units.end(); ii++) {
		unitID u = ii->first;
		if(units[u]->inTransport) continue; // Don't show units inside transports
		vec.push_back(u);
	}
}

void Model::getScrapyardList(std::vector<Scrapyard*> &vec)
{
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
		vec.push_back(&scrapyards[ii]);
}

ICoord Model::nearestScrapyardTo(int x, int y)
{
	ICoord bestPos;
	double bestDist = LONG_MAX;
	
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		const Model::Scrapyard *s = &scrapyards[ii];
		double dist = (s->center.x-x)*(s->center.x-x) + (s->center.y-y)*(s->center.y-y);
		if(dist<bestDist) {
			bestDist = dist;
			bestPos = s->center;
		}
	}
	return bestPos;
}

ICoord Model::nearestFriendlyScrapyardTo(int x, int y, playerID owner)
{
	ICoord bestPos;
	double bestDist = LONG_MAX;
	
	for(unsigned ii=0; ii<scrapyards.size(); ii++)
	{
		const Model::Scrapyard *s = &scrapyards[ii];
		double dist = (s->center.x-x)*(s->center.x-x) + (s->center.y-y)*(s->center.y-y);
		if(dist<bestDist && s->owner == owner) {
			bestDist = dist;
			bestPos = s->center;
		}
	}
	return bestPos;
}

Model::Scrapyard *Model::getScrapyard(int id)
{
	assert((int)scrapyards.size() > id);
	return &scrapyards[id];
}



ICoord Model::spaceNearestTo(unsigned x, unsigned y, bool flying) const
{
	// Algorithm: Start with the space requested, then check surrounding
	// spaces in a clockwise spiral pattern.
	//   789a
	//   612b
	//   543.
	//     ..
	// This corresponds to
	//   1 right, 1 down, 2 left, 2 up, 3 right, 3 down, 4 left, etc
	
	int dx=1, dy=0;     // Starting direction: right
	int xpos=x, ypos=y; // Starting position: (x,y)
	int runs=0;         // Count the number of 'runs' (eg '4 left' is a run) made
	int displacement=0;
	
	do
	{
		if(xpos>=0 && ypos>=0 && xpos<(int)sizeX && ypos<(int)sizeY && getTile(xpos, ypos)->passable(flying))
			return ICoord(xpos, ypos);
		
		xpos += dx;
		ypos += dy;
		displacement++;
		if(displacement > runs/2) {
			runs++;
			displacement = 0;
			swap(dx, dy);
			dx = -dx;
		}
	} while(1);
}



Model::Unit *Model::getUnitType(const char *name)
{
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////

Image Model::Tile::image() const
{
	return type->image;
}

bool Model::Tile::passable(bool flying) const
{
	return (flying || type->passable) && unitAt(flying)==0;
}

Model::Tile::Tile()
{
	type = 0;
	unit = 0;
	airUnit = 0;
	scrapyard = -1;
	providesFuel = 0;
}

/////////////////////////////////////////////////////////////////////////////

const std::vector<UnitFrame*> Model::Unit::getFrame(int anim)
{
	if(anim==Animation::animationTurret || anim==Animation::animationTurretShoot)
	{
		if(shootAnimTime != -1) {
			if(shootAnimTime+1 >= type->getAnimDuration(Animation::animationTurretShoot)) {
				shootAnimTime = -1;
				return type->getFrame(turretAngle, Animation::animationTurret, animTime);
			}
			float t = shootAnimTime;
			shootAnimTime += getDt()*60;
			return type->getFrame(turretAngle, Animation::animationTurretShoot, t);
		} else {
			float t = animTime;
			animTime += getDt()*60;
			return type->getFrame(turretAngle, Animation::animationTurret, t);
		}
	}
	else
	{
		float t = animTime;
		animTime += getDt()*60;
		return type->getFrame(angle, anim, t);
	}
}

Image Model::Unit::mapViewImage()
{
	if(loadedUnits.size())
		return type->mapViewFilledIcon;
	else
		return type->mapViewIcon;
}

Model::Unit::Unit(const char *filename)
{
	type   = UnitInfo::getUnitType(filename);
	x = y = z = 0;
	id     = 0;
	owner  = 0;
	destAngle = angle  = 0;
	turretAngle = 0;
	moving = false;
	hp     = type->maxHP;
	target = 0;
	escort = 0;
	nextShotTime = 0;
	burstFireTimer = type->burstFire;
	explicitTarget = false;
	state = move;
	inTransport = false;
	animTime = 0;
	shootAnimTime = -1;
	
	fuelMax = type->fuelCap;
	fuel = type->startingFuel;
	if(type->usesFuel) {
		fuelMax *= type->mileage;
		fuel    *= type->mileage;
	}
}

/////////////////////////////////////////////////////////////////////////////

Model::Player::Player(const char *playerName, playerID playerId)
{
	name = playerName;
	id   = playerId;
}

/////////////////////////////////////////////////////////////////////////////

Model::OilWell::OilWell(unsigned x, unsigned y, unsigned sizeX, unsigned sizeY)
	: center(x,y), size(sizeX, sizeY)
{
}


