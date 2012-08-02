#include "main.hpp"
#include <cassert>
#include <climits>

const int deathSplashDamage = 12;

ServerController::ServerController(
	Model *model,
	CommandQueue *commandQueue, 
	MessageQueue *messageQueue)
	: Controller(model,commandQueue, messageQueue)
{
	nextIdleFuelTime = nextRegenTime = getTime();
	
	frame = 0;
	lastSent = 0;
	window = 0.5;
	gameOver = false;
}

// 1) Pick a player id
// 2) Create some units (based on map parameters?) with unit id's
// 3) Tell that client his playerId, so that he can control his own units.
// 4) Notify everyone
playerID ServerController::addPlayer(int nodeId, const char *name)
{
	playerID newPlayerId = model->addPlayer();
	players[newPlayerId] = PlayerStats();
	players[newPlayerId].lastHeard = 0;
	players[newPlayerId].node = nodeId;
	if(name)
		players[newPlayerId].name = name;
	else
		players[newPlayerId].name = network->getNode(nodeId)->username;
	players[newPlayerId].alive = true;

	// Find the client from the nodeId, and tell him his playerId
	if(network) {
		Packet *packet = new Packet(msg_assign_playerid);
			packet->putInt(newPlayerId);
		network->sendTo(packet, nodeId);
	}
	return newPlayerId;
}

void ServerController::disconnected(NetworkNode *net)
{
	playerID who = -1;
	for(std::map<playerID,PlayerStats>::iterator ii=players.begin(); ii!=players.end(); ii++)
	{
		if(ii->second.node == net->getNodeId()) {
			who = ii->first;
			break;
		}
	}
	if(who==-1)
		return;
	
	sendChatMessage(retprintf("%s has left the game.", players[who].name.c_str()), who);
	players[who].alive = false;
}

void ServerController::checkVictoryConditions()
{
	int livingPlayers = 0;
	
	for(std::map<playerID, PlayerStats>::iterator it = players.begin(); it != players.end(); it++)
	{
		if(!it->second.alive)
			continue;
		int currId = it->first;
		std::vector<Model::Scrapyard *> scrapyards;
		model->getScrapyardList(scrapyards);
		bool hasScrapyard = false;
		for(unsigned ii=0; ii<scrapyards.size(); ii++)
		{
			if (scrapyards[ii]->owner == currId)
			{
				hasScrapyard = true;
				break;
			}
		}
		if(!hasScrapyard)
			playerLost(currId);
		else
			livingPlayers++;
	}
	
	if(livingPlayers==1) // Last man standing! You win!
	{
		for(std::map<playerID, PlayerStats>::iterator it = players.begin(); it != players.end(); it++)
		{
			if(it->second.alive)
				playerWon(it->first);
		}
		gameOver = true;
	}
}

// Convert incoming packets into commands.  
// Use these commands to update state and 
// send corresponding state update messages to clients.
void ServerController::convertPackets()
{
	if(!network) return;
	
	Packet *packet = network->getNextPacket();
	while (packet != 0)
	{
		int packetType = packet->getInt();
		if (packetType == msg_command)
		{
			Command command;
			command.readFromPacket(packet);
			
			incomingCommands->push_back(command);
		}
		else
		{
			incomingMessages->push_back(packet->clone());
		}

		// Move on to the next packet.
		delete packet;
		packet = network->getNextPacket();
	}
}

void ServerController::timepass(float dt)
{
	convertPackets();
	processMessages();
	processCommands();
	sendHeartbeat();
	
	if(!waiting) {
		produceUnits(dt);
		resolveProjectiles();
		while(getTime() > nextIdleFuelTime) {
			nextIdleFuelTime += 1.0;
			drainUnitFuel();
		}
		while(getTime() > nextRegenTime) {
			nextRegenTime += 1.0;
			regenerateUnits();
		}
		thinkUnits();
		moveUnits(dt);
		cleanPathCache();
		checkVictoryConditions();
	}
}

double lastUpdateTime = 0;

void ServerController::sendHeartbeat()
{
	if(!network)
		return;
	
	waiting = false;
	for(std::map<playerID,PlayerStats>::iterator ii=players.begin(); ii!=players.end(); ii++)
	{
		if(ii->second.node < 0)
			continue;
		if(ii->second.alive && ii->second.lastHeard+window*getFramerate() < lastSent)
			waiting = true;
	}
	
	if(waiting)
		return;
	if(getTime() < lastUpdateTime+window/4)
		return;
	lastUpdateTime = getTime();
	
	frame++;
	lastSent++;
	Packet *heartbeat = new Packet(msg_heartbeat);
		heartbeat->putInt(lastSent);
	network->sendToAll(heartbeat);
}

std::string ServerController::waitingFor()
{
	return "client";
}


void ServerController::sendChatMessage(std::string text, int player)
{
	printChatText(text, player);
	
	if(network) {
		Packet *textPacket = new Packet(msg_game_chat);
			textPacket->putString(text);
			textPacket->putInt(player);
		network->sendToAll(textPacket);
	}
}


void ServerController::processMessages()
{
	while (!incomingMessages->empty())
	{
		Packet *packet  = incomingMessages->front();
		int messageType = packet->getInt();
		switch (messageType)
		{
		case msg_game_chat: {
			std::string text       = packet->getString();
			playerID id            = packet->getInt();
			
			// Print and forward the chat text to other clients
			sendChatMessage(text, id);
			break;
		}

		case msg_join_lobby: {
			int destNodeId = network->getLastNodeId();
			Packet *joinFailed = new Packet(msg_join_failed);
			network->sendTo(joinFailed, destNodeId);
			network->getNode(destNodeId)->disconnect();
			break;
		}
		
		case msg_heartbeat_reply: {
			int player = packet->getShort();
			int time = packet->getInt();
			updateHeartbeat(player, time);
			break;
		}

		default:
			logger.log("Unhandled server msg type:  %d\n", messageType);
			break;
		}

		// FIXME:  shared pointers would be REALLY nice...
		// This should be the original copy
		delete packet;	
		incomingMessages->pop_front();
	}
}

void ServerController::updateHeartbeat(int player, int time)
{
	if(players.find(player) == players.end())
		return;
	players[player].lastHeard = time;
	
	// If waiting, only stop when everyone's fully caught up
	if(waiting) {
		waiting = false;
		for(std::map<playerID,PlayerStats>::iterator ii=players.begin(); ii!=players.end(); ii++) {
			if(ii->second.lastHeard != lastSent)
				waiting = true;
		}
	}
}



void ServerController::onTileReached(unitID id)
{
	pathfindUnit(id);
}
void ServerController::onAngleReached(unitID id)
{
	Model::Unit *u = model->getUnit(id);
	if(u->turningToShoot)
	{
		u->turningToShoot = false;
		u->moving = false;
		Model::Unit *target = model->getUnit(u->target);
		if(canTarget(u, target))
			fireBurst(u, target);
		else
			u->target = 0;
	}
}

unitID ServerController::createUnit(playerID owner, std::string unit, unsigned x, unsigned y)
{
	unitID id = model->addUnit(owner, unit, x, y);
	viewNotifyNewUnit(id);
	
	if(network) {
		Packet *newUnitMsg = NEW Packet(msg_create_unit);
			newUnitMsg->putInt(owner);
			newUnitMsg->putInt(id);
			newUnitMsg->putString(unit);
			newUnitMsg->putFloat(x);
			newUnitMsg->putFloat(y);
		network->sendToAll(newUnitMsg);
	}
	
	return id;
}

//TODO: This player lost, needs to be removed
void ServerController::playerLost(playerID loser)
{
	sendChatMessage(retprintf("%s was eliminated.", players[loser].name.c_str()), loser);
	players[loser].alive = false;
	
	if(players[loser].node==-1) { // Local user
		postMortem->set("You were defeated.", gameOver);
		Frame::changeFrame(postMortem);
	}
	
	std::vector<unitID> units;
	model->getUnitList(units);
	for(unsigned ii=0; ii<units.size(); ii++)
	{
		Model::Unit *u = model->getUnit(units[ii]);
		if(u->owner == loser)
			killUnit(units[ii], u->type->deathExplosion);
	}
	
	if(network && players[loser].node>=0) {
		Packet *playerLostMsg = NEW Packet(msg_player_lost);
		network->sendTo(playerLostMsg, players[loser].node);
	}	
}
void ServerController::playerWon(playerID winner)
{
	if(players[winner].node==-1) { // Local user
		postMortem->set("You won!", true);
		Frame::changeFrame(postMortem);
	}
	
	if(network && players[winner].node>=0) {
		Packet *playerWonMsg = NEW Packet(msg_player_won);
		network->sendTo(playerWonMsg, players[winner].node);
		postMortem->set(true);
	}	
}

void ServerController::convertScrapyard(int sid, playerID player)
{
	Model::Scrapyard *s = model->getScrapyard(sid);
	playerID oldOwner = s->owner;
	model->concealFog(s->owner, scrapyardSightRadius, s->center.x, s->center.y);
	model->revealFog(player, scrapyardSightRadius, s->center.x, s->center.y);
	s->owner = player;
	s->rally = s->center;
	s->clear();
	
	viewNotifyConversion(sid, oldOwner, player);
	updateScrapyard(sid);
}

void ServerController::drainUnitFuel()
{
	Controller::drainUnitFuel();
	
	if(network) {
		Packet *fuelMsg = NEW Packet(msg_drain_idle_fuel);
		network->sendToAll(fuelMsg);	
	}
}

void ServerController::regenerateUnits()
{
	std::vector<unitID> units;
	model->getUnitList(units);
	
	for(std::vector<unitID>::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = model->getUnit(*ii);
		if(u->type->regeneration > 0 && u->hp < u->type->maxHP)
			damageUnit(-u->type->regeneration, *ii);
	}
}

void ServerController::loadUnit(unitID unitId, unitID transportId)
{
	model->putUnitInTransport(unitId, transportId);
	
	if(network) {
		Packet *p = new Packet(msg_enter_transport);
			p->putInt(unitId);
			p->putInt(transportId);
		network->sendToAll(p);
	}
}

void ServerController::unloadUnit(unitID transport, unitID unit)
{
	Model::Unit *u = model->getUnit(unit);
	Model::Unit *t = model->getUnit(transport);
	
	ICoord pos = model->spaceNearestTo((int)t->x, (int)t->y, u->type->flying);
	
	model->setUnitPosition(unit, pos.x, pos.y);
	
	u->inTransport = false;
	u->state = Model::Unit::move;
	u->escort = 0;
	u->moving = false;
	u->explicitTarget = false;
	u->target = 0;
	u->destX = u->nextX = u->x;
	u->destY = u->nextY = u->y;
	
	model->revealFogAround(unit);
	
	if(network) {
		Packet *p = new Packet(msg_leave_transport);
			p->putInt(unit);
			p->putInt(transport);
			p->putInt(u->x);
			p->putInt(u->y);
		network->sendToAll(p);
	}
}

void ServerController::updateUnitState(unitID unit)
{
	if(network) {
		Model::Unit *u = model->getUnit(unit);
		Packet *p = new Packet(msg_unit_state);
			p->putInt(unit);
			p->putInt(u->state);
			p->putChar(u->moving);
		network->sendToAll(p);
	}
}

// Look for projectiles that have landed, and deal damage accordingly
void ServerController::resolveProjectile(unsigned index)
{
	Model::Projectile *p = &model->shotsPending[index];
	if(model->isInBounds((int)p->targetX, (int)p->targetY))
	{
		Model::Unit *u = model->getUnitAt((int)p->targetX, (int)p->targetY, p->hitsAir);
		if(u) {
			damageUnit(p->damage, u->getId());
		}
	}
	if(p->splash > 0) {
		int left, top, right, bottom;
		mapRect(model, ICoord(p->targetX, p->targetY), ICoord(1,1), left, top, right, bottom);
		for(int yi=top; yi<=bottom; yi++)
		for(int xi=left; xi<=right; xi++) {
			if(xi==p->targetX && yi==p->targetY)
				continue;
			Model::Unit *splashed = model->getUnitAt(xi, yi, p->hitsAir);
			if(splashed)
				damageUnit(p->damage*p->splash, splashed->getId());
		}
	}
	Controller::resolveProjectile(index);
}

void ServerController::damageUnit(int damage, unitID id)
{
	Model::Unit *u = model->getUnit(id);
	if(!u) return;
	if(damage >= u->hp) {
		u->hp = 0;
		killUnit(id, u->type->deathExplosion);
	} else {
		if(damage<0) {
			u->hp -= damage;
			if(u->hp > u->type->maxHP)
				u->hp = u->type->maxHP;
		} else {
			u->hp -= damage;
		}
		if(network) {
			Packet *msg = NEW Packet(msg_set_hps);
				msg->putInt(id);
				msg->putInt(u->hp);
			network->sendToAll(msg);
		}
	}
}

void ServerController::killUnit(unitID id, int explosion)
{
	Model::Unit *u = model->getUnit(id);
	
	if(u->type->deathSplash) {
		int minX, minY, maxX, maxY;
		mapRect(model, ICoord(u->nextX, u->nextY), ICoord(2,2), minX, minY, maxX, maxY);
		for(int yi=minY; yi<=maxY; yi++)
		for(int xi=minX; xi<=maxX; xi++)
		{
			Model::Unit *splashed = model->getUnitAt(xi, yi, false);
			if(splashed && splashed != u && splashed->hp>0)
				damageUnit(deathSplashDamage, splashed->getId());
		}
	}
	
	if(network) {
		Packet *msg = NEW Packet(msg_kill_unit);
			msg->putInt(id);
			msg->putInt(explosion);
		network->sendToAll(msg);
	}
	
	Controller::killUnit(id, explosion);
}


bool ServerController::fireBurst(Model::Unit *shooter, Model::Unit *target)
{
	float targetX = target->nextX+0.5,
	      targetY = target->nextY+0.5;
	float distSq = (shooter->x+0.5-targetX)*(shooter->x+0.5-targetX) + (shooter->y+0.5-targetY)*(shooter->y+0.5-targetY);
	bool airTarget = target->type->flying;
	
	if(distSq > shooter->type->range*shooter->type->range ||
	   distSq < shooter->type->minimumRange*shooter->type->minimumRange)
	{
		if(shooter->moving) // Out of range and already moving, so nothing to do
			return false;
		
		int oldDestX = shooter->destX,
		    oldDestY = shooter->destY;
		bool resetPathing = shooter->destX!=shooter->nextX || shooter->destY!=shooter->nextY;
		
		if(distSq < shooter->type->minimumRange*shooter->type->minimumRange)
			return false;
		
		// We're out of range and NOT moving, so it's time to CHARGE!
		shooter->destX = (int)targetX;
		shooter->destY = (int)targetY;
		pathfindUnit(shooter->getId());
		
		// Don't then KEEP moving unless it's really necessary
		if(resetPathing) {
			shooter->destX = oldDestX;
			shooter->destY = oldDestY;
		} else {
			shooter->destX = shooter->nextX;
			shooter->destY = shooter->nextY;
		}
		
		return false;
	}
	
	if(shooter->nextShotTime > getTime())
		return false;

	if(shooter->burstFireTimer > 1)
	{
		shooter->nextShotTime = getTime() + shooter->type->burstFireInterval;
		shooter->burstFireTimer--;
	}
	else
	{
		shooter->burstFireTimer = shooter->type->burstFire;
		shooter->nextShotTime = getTime() + shooter->type->cooldown;
	}
	
	targetX += randFloat(-shooter->type->inaccuracy, +shooter->type->inaccuracy);
	targetY += randFloat(-shooter->type->inaccuracy, +shooter->type->inaccuracy);
	
	shooter->turretDestAngle = shooter->turretAngle = angleHeading(shooter->x, shooter->y, targetX, targetY);
	
	Model::Projectile projectile(
		model, shooter->getId(),
		(Model::Projectile::ProjectileType)shooter->type->projectileType,
		(Effect::EffectType)shooter->type->explosionEffect,
		targetX, targetY, shooter->type->projectileSpeed,
		shooter->type->damage, shooter->type->splashDamage, airTarget);
	
	if(network) {
		Packet *shotPacket = new Packet(msg_create_projectile);
			shotPacket->putInt(shooter->getId());
			shotPacket->putInt(projectile.type);
			shotPacket->putFloat(projectile.targetX);
			shotPacket->putFloat(projectile.targetY);
			shotPacket->putChar(airTarget);
		network->sendToAll(shotPacket);
	}
	shooter->type->getSound("fire")->play();
	
	model->shotsPending.push_back(projectile);
	
	return true;
}

