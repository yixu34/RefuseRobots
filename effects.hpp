#ifndef EFFECTS_HPP
#define EFFECTS_HPP

#include "util.hpp"

class Effect
{
public:
	enum EffectType {
		smallShot,
		artilleryBoom,
		bigDeath,
		tankerDeath,
		none,
	};
	static EffectType getEffectType(std::string str);
	
	DCoord pos;
	int lifetime; // in frames
	
	EffectType type;
};

class Explosion : public Effect
{
public:
	Explosion(DCoord pos, int type);
};

#endif
