
#include "main.hpp"

static int animationIndex(std::string str);

UnitInfo::UnitInfo(const char *filename)
{
	using Parser::Section;
	using Parser::Line;
	
	std::string filenameStr = std::string("units/")+filename+std::string(".unit");
	Parser parser(filenameStr.c_str());
	
	const Section *stats     = parser.getSection("stats");
	const Section *frames    = parser.getSection("frames");
	const Section *animation = parser.getSection("animation");
	const Section *sounds    = parser.getSection("sounds");
	
	// Default states
	cooldown = 1.0;
	damage = 1.0;
	hotkey = 'x';
	hudX = hudY = 0;
	name = filename;
	transports = 0;
	transportable = convertsScrapyard = false;
	usesFuel = true;
	startingFuel = -1;
	refueler = false;
	cost = 10;
	projectileSpeed = 15.0;
	inaccuracy = 0.2;
	flying = false;
	fuelCap = 15;
	mileage = 10;
	idleFuel = 0;
	minimumRange = 0;
	splashDamage = 0;
	turnSpeed = 180;
	range = 5;
	regeneration = 0;
	targetsGround = true;
	targetsAir = false;
	facesTarget = false;
	deathSplash = false;
	barrelLength = 0;
	burstFire = 0;
	burstFireInterval = 0;
	projectileType = Model::Projectile::none;
	explosionEffect = Explosion::smallShot;
	deathExplosion = Explosion::bigDeath;
	numStates = 0;
	
	// Parse stats
	for(Section::const_iterator ii=stats->begin(); ii!=stats->end(); ii++)
	{
		Line L = *ii;
		if(L.size()<2) {
			// TODO: Log this
			continue;
		}
		if(L[0]=="name")
			name = L[1];
		
		else if(L[0]=="speed")                  speed              = atof(L[1].c_str());
		else if(L[0]=="turn_speed")             turnSpeed          = atof(L[1].c_str());
		else if(L[0]=="cost")                   cost               = atof(L[1].c_str());
		else if(L[0]=="cooldown")               cooldown           = atof(L[1].c_str());
		else if(L[0]=="projectile_speed")       projectileSpeed    = atof(L[1].c_str());
		else if(L[0]=="inaccuracy")             inaccuracy         = atof(L[1].c_str());
		else if(L[0]=="splash_damage")          splashDamage       = atof(L[1].c_str());
		else if(L[0]=="idle_fuel")              idleFuel           = atof(L[1].c_str());
		else if(L[0]=="barrel_length")          barrelLength       = atof(L[1].c_str());
		else if(L[0]=="burst_fire")	            burstFire          = atof(L[1].c_str());
		else if(L[0]=="burst_fire_interval")    burstFireInterval  = atof(L[1].c_str());
		
		else if(L[0]=="production_icon")     productionIcon            = Image(L[1]);
		else if(L[0]=="mapview_icon")        mapViewIcon               = Image(L[1]);
		else if(L[0]=="loaded_icon")         mapViewFilledIcon         = Image(L[1]);
		else if(L[0]=="loaded_nanites_icon") mapViewFilledWNanitesIcon = Image(L[1]);
		else if(L[0]=="wireframe")           wireframeIcon             = Image(L[1]);
		
		else if(L[0]=="hotkey") hotkey = L[1][0];
		
		else if(L[0]=="hud_x")         hudX          = atoi(L[1].c_str());
		else if(L[0]=="hud_y")         hudY          = atoi(L[1].c_str());
		else if(L[0]=="hitpoints")     maxHP         = atoi(L[1].c_str());
		else if(L[0]=="damage")        damage        = atoi(L[1].c_str());
		else if(L[0]=="range")         range         = atoi(L[1].c_str());
		else if(L[0]=="minimum_range") minimumRange  = atoi(L[1].c_str());
		else if(L[0]=="transports")    transports    = atoi(L[1].c_str());
		else if(L[0]=="fuel_cap")      fuelCap       = atoi(L[1].c_str());
		else if(L[0]=="starting_fuel") startingFuel  = atoi(L[1].c_str());
		else if(L[0]=="mileage")       mileage       = atoi(L[1].c_str());
		else if(L[0]=="vision")        vision        = atoi(L[1].c_str());
		else if(L[0]=="regeneration")  regeneration  = atoi(L[1].c_str());
		
		
		else if(L[0]=="transportable")      transportable     = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="converts_scrapyard") convertsScrapyard = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="flying")             flying            = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="uses_fuel")          usesFuel          = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="refueler")           refueler          = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="targets_air")        targetsAir        = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="targets_ground")     targetsGround     = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="faces_target")       facesTarget       = (atoi(L[1].c_str()))?true:false;
		else if(L[0]=="death_splash")       deathSplash       = (atoi(L[1].c_str()))?true:false;
		
		else if(L[0]=="projectile_type") projectileType = Model::Projectile::getProjectileType(L[1]);
		else if(L[0]=="explosion_effect") explosionEffect = Effect::getEffectType(L[1]);
		else if(L[0]=="death_explosion") deathExplosion = Effect::getEffectType(L[1]);
		
		else if(L[0]=="commands") {
			commandCard = NEW CommandCard();
			for(unsigned ii=1; ii<L.size(); ii++)
				commandCard->addButtonName(L[ii]);
		}
	}
	if(startingFuel==-1) startingFuel = fuelCap;
	
	// Parse frames
	for(Section::const_iterator ii=frames->begin(); ii!=frames->end(); ii++)
	{
		Line L = *ii;
		if(L.size() < 8) {
			// TODO: Log this
			continue;
		}
		
		UnitFrame *frame = NEW UnitFrame();
		frame->image = Image(L[0]);
		frame->angle = atof(L[1].c_str());
		
		if(animStates.find(L[2]) != animStates.end())
			frame->frame = animStates[L[2]];
		else
			frame->frame = animStates[L[2]] = numStates++;
		
		frame->centerX = atof(L[3].c_str());
		frame->centerY = atof(L[4].c_str());
		frame->rotation = atof(L[5].c_str());
		frame->scaleX = atof(L[6].c_str()) * ((float)frame->image.getWidth())/tileSize;
		frame->scaleY = atof(L[7].c_str()) * ((float)frame->image.getHeight())/tileSize;
		this->frames.push_back(frame);
	}
	
	// Parse animations
	for(Section::const_iterator ii=animation->begin(); ii!=animation->end(); ii++)
	{
		Line L = *ii;
		animations[animationIndex(L[0])] = new Animation(this, L);
	}
	
	// Parse sounds
	for(Section::const_iterator ii=sounds->begin(); ii!=sounds->end(); ii++)
	{
		Line L = *ii;
		if(L.size() < 3)
			continue;
		std::string trigger = L[0];
		this->sounds[trigger] = new SoundSet();
		
		for(unsigned ii=1; ii+1<L.size(); ii+=2)
		{
			std::string sound = L[ii];
			int volume = atoi(L[ii+1].c_str());
			
			this->sounds[trigger]->push_back(new Sound(sound.c_str()));
		}
	}
}

int UnitInfo::getAnimDuration(int anim) const
{
	std::map<int, Animation*>::const_iterator ii=animations.find(anim);
	if(ii == animations.end())
		return 0;
	return ii->second->getDuration();
}

static int animationIndex(std::string str)
{
	if(str=="idle")
		return Animation::animationIdle;
	else if(str=="move")
		return Animation::animationMove;
	else if(str=="turret")
		return Animation::animationTurret;
	else if(str=="turret_shoot")
		return Animation::animationTurretShoot;
	else
		return Animation::animationIdle;
}

Animation::Animation(UnitInfo *context, Parser::Line &L)
{
	for(int ii=1; ii<L.size(); ii+=2)
	{
		int frame = context->getAnimState(L[ii]);
		int duration = atoi(L[ii+1].c_str());
		frames.push_back(AnimationStep(frame,duration));
	}
	
	duration = 0;
	for(int ii=0; ii<frames.size(); ii++)
		duration += frames[ii].duration;
}


static std::map<std::string, UnitInfo*> unitTypes;

const UnitInfo *UnitInfo::getUnitType(const std::string &filename)
{
	std::map<std::string, UnitInfo*>::iterator ii=unitTypes.find(filename);
	
	if(ii != unitTypes.end())
		return ii->second;
	unitTypes[filename] = NEW UnitInfo(filename.c_str());
	return unitTypes[filename];
}


int Animation::getFrame(float ftime)
{
	int time = (int)ftime;
	time %= duration;
	for(int ii=0; ii<frames.size(); ii++)
	{
		time -= frames[ii].duration;
		if(time <= 0)
			return frames[ii].frame;
	}
	return frames[0].frame;
}


const std::vector<UnitFrame*> UnitInfo::getFrame(float angle, int animationState, float ftime) const
{
	std::vector<UnitFrame*> ret;
	
	std::map<int,Animation*>::const_iterator it = animations.find(animationState);
	if(it==animations.end())
		return ret;
	
	int frame = it->second->getFrame(ftime);
	
	int bestMatch = 0;
	bool matchingAnim = (frames[0]->frame==frame);
	float bestAngleDifference = 360;
	
	for(unsigned ii=0; ii<frames.size(); ii++)
	{
		if(frames[ii]->frame != frame)
			continue;
		
		if(angleDifference(angle, frames[ii]->angle) < bestAngleDifference) {
			bestMatch = ii;
			bestAngleDifference = angleDifference(frames[ii]->angle, angle);
			matchingAnim = true;
		}
	}
	
	if(!matchingAnim) return ret; // Don't return no-match
	for(unsigned ii=0; ii<frames.size(); ii++)
	{
		if(frames[ii]->frame == frames[bestMatch]->frame
		 &&frames[ii]->angle == frames[bestMatch]->angle)
			ret.push_back(frames[ii]);
	}
	
	return ret;
}

Sound *UnitInfo::getSound(std::string trigger) const
{
	if(sounds.find(trigger)==sounds.end())
		return NULL;
	if(sounds.find(trigger)==sounds.end())
		return NULL;
	SoundSet *candidates = sounds.find(trigger)->second;
	int num = candidates->size();
	int which = 0;
	if(num > 1) {
		int last;
		if(lastPlayed.find(trigger) == lastPlayed.end())
			last = -1;
		else
			last = lastPlayed.find(trigger)->second;

		do {
			which = rand()%num;
		} while(which == last);
		std::map<std::string, int> *stripconst = (std::map<std::string,int>*)&lastPlayed; //HACK
		(*stripconst)[trigger] = which;
	}
	return (*candidates)[which];
}

float UnitInfo::getCost() const
{
	if(cheatFastBuild)
		return cost/10.0;
	else
		return cost;
}

float UnitInfo::getSpeed() const
{
	if(cheatFastMove)
		return speed*10;
	else
		return speed;
}
