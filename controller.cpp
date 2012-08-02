#include "main.hpp"
#include <cassert>

const float sinkSpeed = 0.3;
Sound *explode = NULL;

Controller::Controller(
	Model *model, 
	CommandQueue *commandQueue, 
	MessageQueue *messageQueue)
{
	this->model = model;
	this->incomingCommands = commandQueue;
	this->incomingMessages = messageQueue;
	waiting = false;

	if(!explode) explode = new Sound("sounds/explode.wav");
}

Controller::~Controller()
{
}

bool Controller::isWaiting() { return waiting; }

void Controller::moveUnits(float dt)
{
	std::vector<unitID> units;
	float timeLeft;
	model->getUnitList(units);
	
	for(std::vector<unitID>::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		timeLeft = dt;
		Model::Unit *u = model->getUnit(*ii);
		
		if(u->state == Model::Unit::sinking)
			u->z -= dt*sinkSpeed;
		
		// If turret should be turning, turn it
		const float turretTurnSpeed = 300;
		if(u->turretAngle != u->turretDestAngle) {
			if(angleDifference(u->turretAngle, u->turretDestAngle) < u->type->turnSpeed * dt)
				u->turretAngle = u->turretDestAngle;
			else
				u->turretAngle = angleTowards(u->turretAngle, u->turretDestAngle, u->type->turnSpeed*dt);
		} else if(u->turningToShoot) {
			u->turningToShoot = false;
		}
		
		// If turning, update angle.
		if(u->angle != u->destAngle) {
			if(angleDifference(u->angle, u->destAngle) < u->type->turnSpeed * timeLeft) {
				timeLeft -= angleDifference(u->angle, u->destAngle)/u->type->turnSpeed;
				u->angle = u->destAngle;
				onAngleReached(*ii);
			} else {
				u->angle = angleTowards(u->angle, u->destAngle, u->type->turnSpeed*timeLeft);
				timeLeft = 0;
			}
		}
		
		bool reachedTile = false;
		double u_x = u->x,
		       u_y = u->y;
		double moveSpeed = u->getSpeed() * timeLeft;
		
		if(u->nextX != u_x && u->nextY != u_y) // Don't go faster if moving along both x and y
			moveSpeed /= 1.4142;
		
		if(u->nextX > u_x) {
			u_x += moveSpeed;
			if(u_x >= u->nextX) {
				u_x = u->nextX;
				reachedTile = true;
			}
		}
		if(u->nextX < u_x) {
			u_x -= moveSpeed;
			if(u_x <= u->nextX) {
				u_x = u->nextX;
				reachedTile = true;
			}
		}
		if(u->nextY > u_y) {
			u_y += moveSpeed;
			if(u_y >= u->nextY) {
				u_y = u->nextY;
				reachedTile = true;
			}
		}
		if(u->nextY < u_y) {
			u_y -= moveSpeed;
			if(u_y <= u->nextY) {
				u_y = u->nextY;
				reachedTile = true;
			}
		}
		
		model->setUnitPosition(*ii, u_x, u_y);
		
		if(!u->moving && (u->x != u->destX || u->y != u->destY))
			reachedTile = true;

		if(reachedTile)
			onTileReached(*ii);
	}
}

void Controller::updateUnitFacing(unitID id)
{
	Model::Unit *u = model->getUnit(id);
	int u_x = (int)(u->x + .1),
	    u_y = (int)(u->y + .1);
	int n_x = u->nextX,
	    n_y = u->nextY;
	if(u_x==n_x && u_y==n_y) return;
	switch(n_x-u_x) {
		case 1:
			switch(n_y-u_y) {
				case  1: u->turretDestAngle=u->destAngle=315; break;
				case  0: u->turretDestAngle=u->destAngle=0;   break;
				case -1: u->turretDestAngle=u->destAngle=45;  break;
			}
			break;
		case 0:
			switch(n_y-u_y) {
				case  1: u->turretDestAngle=u->destAngle=270; break;
				case  0:                                      break;
				case -1: u->turretDestAngle=u->destAngle=90;  break;
			}
			break;
		case -1:
			switch(n_y-u_y) {
				case  1: u->turretDestAngle=u->destAngle=225; break;
				case  0: u->turretDestAngle=u->destAngle=180; break;
				case -1: u->turretDestAngle=u->destAngle=135; break;
			}
			break;
	}
}

void Controller::resolveProjectiles()
{
	double time = getTime();
	
	for(unsigned ii=0; ii<model->shotsPending.size(); ii++)
	{
		Model::Projectile *p = &model->shotsPending[ii];
		double distanceSq = (p->targetX-p->startX)*(p->targetX-p->startX)
		                  + (p->targetY-p->startY)*(p->targetY-p->startY);
		double travelTimeSq = distanceSq / (p->speed*p->speed);
		if((time-p->startTime)*(time-p->startTime) >= travelTimeSq) {
			model->effects.insert(model->effects.begin(),
				Explosion(DCoord(p->targetX, p->targetY), p->explosionType));
			resolveProjectile(ii--);
		}
	}
}

void Controller::resolveProjectile(unsigned index)
{
	std::vector<Model::Projectile>::iterator ii = model->shotsPending.begin();
	ii += index;
	model->shotsPending.erase(ii);
}


void Controller::killUnit(unitID id, int explosion)
{
	viewNotifyDeath(id);
	Model::Unit *unit=engine->model->getUnit(id); 
	int x=unit->getX();
	int y=unit->getY();
	if(explosion != Explosion::none)
		playSound(x, y, unit->type->getSound("die"));
	model->effects.push_back(Explosion(DCoord(x+0.5, y+0.5), explosion));
	model->deleteUnit(id);
}

/////////////////////////////////////////////////////////////////////

ClientController::ClientController(
	Model *model, 
	CommandQueue *commandQueue, 
	MessageQueue *messageQueue)
	: Controller(model, commandQueue, messageQueue)
{
}

void ClientController::convertPackets()
{
	if(!network) return;
	
	Packet *packet = network->getNextPacket();
	while (packet != 0)
	{
		incomingMessages->push_back(packet);

		// Move on to the next packet.
		// Don't delete it like w/ the commands on the server side.
		packet = network->getNextPacket();
	}
}

void ClientController::timepass(float dt)
{
	convertPackets();
	processMessages();
	processCommands();
	resolveProjectiles();
	moveUnits(dt);
}

void ClientController::processMessages()
{
	while (!incomingMessages->empty())
	{
		Packet *packet  = incomingMessages->front();
		int messageType = packet->getInt();
		switch (messageType)
		{
		// There should be a leeter way to do this...
		case msg_create_unit: {
			playerID owner    = packet->getInt();
			unitID unitId     = packet->getInt();
			std::string type  = packet->getString();
			float x           = packet->getFloat();
			float y           = packet->getFloat();
			model->addUnit(owner, type, x, y, unitId);
			viewNotifyNewUnit(unitId);
			break;
		}
		
		case msg_heartbeat:
			if(network) {
				int player = playerId;
				int time = packet->getInt();
				Packet *response = NEW Packet(msg_heartbeat_reply);
					response->putShort(player);
					response->putInt(time);
				network->sendToServer(response);
			}
			break;
			
		case msg_pathfind: {
			unitID id = packet->getInt();
			Model::Unit *unit = model->getUnit(id);
			if (unit == 0)
				return;
			
			int nextX = packet->getShort();
			int nextY = packet->getShort();
			//int fuelLeft = packet->getInt();
			int destAngle = packet->getShort();
			
			// Moving drains one unit of fuel (unless it's to same-spot or not
			// a fuel-using unit.)
			if(unit->type->usesFuel && (unit->nextX!=nextX || unit->nextY!=nextY))
				unit->fuel--;
			unit->turretDestAngle = unit->destAngle = destAngle;
			model->setUnitPath(id, nextX, nextY);
			viewNotifyMoveUnit(id, ICoord(unit->x, unit->y), ICoord(unit->nextX, unit->nextY));
			break;
		}
		
		case msg_update_fuel: {
			unitID id = packet->getInt();
			int fuel = packet->getInt();
			
			Model::Unit *unit = model->getUnit(id);
			if(unit) unit->fuel = fuel;
			break;
		}
		
		case msg_drain_idle_fuel:
			drainUnitFuel();
			break;

		case msg_assign_playerid: {
			playerId = packet->getInt();
			logger.log("Got a player id:  %d\n", playerId);
			engine->view->setPlayerId(playerId);
			break;
		}
		
		case msg_create_projectile: {
			unitID source = packet->getInt();
			Model::Projectile::ProjectileType projectileType = (Model::Projectile::ProjectileType)packet->getInt();
			float destX  = packet->getFloat();
			float destY  = packet->getFloat();
			int airTarget = packet->getChar();
			
			Model::Unit *shooter = model->getUnit(source);
			if(!shooter) break;
			Effect::EffectType effectType = (Effect::EffectType)shooter->type->explosionEffect;
			shooter->turretDestAngle = shooter->turretAngle = angleHeading(shooter->x, shooter->y, destX, destY);
			model->shotsPending.push_back(
				Model::Projectile(model, source, projectileType, effectType,
				                  destX, destY, shooter->type->projectileSpeed,
				                  0, 0, airTarget?true:false));
			shooter->type->getSound("fire")->play();
			break;
		}

		case msg_game_chat: {
			std::string text       = packet->getString();
			playerID id            = packet->getInt();
			printChatText(text, id);

			break;
		}
		
		case msg_set_hps: {
			unitID id = packet->getInt();
			int newHP = packet->getInt();
			Model::Unit *u = model->getUnit(id);
			if(u) u->hp = newHP;
			break;
		}
		
		case msg_kill_unit: {
			unitID id = packet->getInt();
			int explosion = packet->getInt();
			killUnit(id, explosion);
			break;
		}
		
		case msg_enter_transport: {
			unitID unitId = packet->getInt();
			unitID transportId = packet->getInt();
			model->putUnitInTransport(unitId, transportId);
			break;
		}
		
		case msg_leave_transport: {
			unitID unit = packet->getInt();
			unitID transport = packet->getInt();
			int x = packet->getInt(),
			    y = packet->getInt();
			
			Model::Unit *u = model->getUnit(unit),
			            *t = model->getUnit(transport);
			for(std::vector<unitID>::iterator ii=t->loadedUnits.begin(); ii!=t->loadedUnits.end(); ii++) {
				if(*ii == unit) {
					t->loadedUnits.erase(ii);
					break;
				}
			}
			model->setUnitPosition(unit, x, y);
			u->inTransport = false;
			u->moving = false;
			u->state = Model::Unit::move;
			u->escort = 0;
			u->destX = u->nextX = u->x;
			u->destY = u->nextY = u->y;
			model->revealFogAround(unit);
			break;
		}
		
		case msg_unit_state: {
			unitID unit = packet->getInt();
			Model::Unit *u = model->getUnit(unit);
			if(u) {
				Model::Unit::State prevState = u->state;
				u->state = (Model::Unit::State)packet->getInt();
				if(prevState == Model::Unit::sinking && u->state != prevState)
					u->z = 0;
				u->moving = packet->getChar()?true:false;
			}
			break;
		}
		
		case msg_update_scrapyard: {
			int sid = packet->getInt();
			Model::Scrapyard *s = model->getScrapyard(sid);
			playerID prevOwner = s->owner;
			s->owner = packet->getInt();
			s->startTime = getTime() - packet->getFloat();
			s->queuePos = packet->getShort();
			s->repeating = packet->getChar()?true:false;
			s->buildQueue.resize(packet->getShort());
			short rallyX = packet->getShort(), rallyY = packet->getShort();
			s->rally = ICoord(rallyX, rallyY);
			for(unsigned ii=0; ii<s->buildQueue.size(); ii++)
				s->buildQueue[ii] = packet->getString();
			
			if(s->owner != prevOwner) {
				model->concealFog(prevOwner, scrapyardSightRadius, s->center.x, s->center.y);
				model->revealFog(s->owner, scrapyardSightRadius, s->center.x, s->center.y);
			}
			break;
		}

		case msg_player_lost: {
			// Show defeat screen
			postMortem->set("You were defeated.", true);
			Frame::changeFrame(postMortem);
			break;
		}
		case msg_player_won: {
			// Show victory screen
			postMortem->set("You won!", true);
			Frame::changeFrame(postMortem);
			break;
		}
		default:
			logger.log("Unhandled client msg type:  %d\n", messageType);
			break;
		}

		// FIXME:  shared pointers would be REALLY nice...
		delete packet;	
		incomingMessages->pop_front();
	}
}

// Don't update state directly; send commands to the server for that.
void ClientController::processCommands()
{
	while (!incomingCommands->empty())
	{
		Command currentCommand = incomingCommands->front();
		
		if(network) {
			Packet *packet = NEW Packet();
				currentCommand.writeToPacket(packet);
			network->sendToServer(packet);
		}
		
		incomingCommands->pop_front();
	}
}

void ClientController::onTileReached(unitID id)
{
	// Empty - clients shouldn't do anything.
}
void ClientController::onAngleReached(unitID id)
{
	// Empty - clients shouldn't do anything.
}

void ClientController::disconnected(NetworkNode *net)
{
	//TODO
}

void Controller::drainUnitFuel()
{
	std::vector<unitID> units;
	model->getUnitList(units);
	
	for(std::vector<unitID>::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = model->getUnit(*ii);
		if(u->type->idleFuel > 0 && u->fuel > 0) {
			u->fuel -= u->type->idleFuel;
			if(u->fuel < 0) u->fuel = 0;
		}
	}
}
