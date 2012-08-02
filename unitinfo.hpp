#ifndef UNITINFO_HPP
#define UNITINFO_HPP

#include "command.hpp"
#include "sound.hpp"
#include <vector>

class Animation;
struct UnitFrame;
class CommandCard;
class CommandButton;

class UnitInfo
{
public:
	static const UnitInfo *getUnitType(const std::string &filename);
	const std::vector<UnitFrame*> getFrame(float angle, int animationState, float time) const;
	int getAnimDuration(int anim) const;
	int getAnimState(std::string str) { return animStates[str]; }
	Sound *getSound(std::string trigger) const;
	
	std::string name;
	
	float cooldown;
	float damage;
	float projectileSpeed;
	float inaccuracy;
	float splashDamage;
	float barrelLength;
	int explosionEffect;
	int deathExplosion;
	int range;
	int minimumRange;
	int maxHP;
	int regeneration;
	int fuelCap, startingFuel;  // In gallons
	float idleFuel; // In gallons/second
	float turnSpeed; // Degrees/sec
	int mileage;  // In tiles/gallon
	int burstFire;
	float burstFireInterval;
	unsigned transports;
	bool transportable, convertsScrapyard;
	bool usesFuel, refueler;
	bool targetsAir, targetsGround;
	bool facesTarget;
	bool flying;
	bool deathSplash;
	int vision;
	Image mapViewIcon, mapViewFilledIcon, mapViewFilledWNanitesIcon, wireframeIcon;
	CommandCard *commandCard;
	int projectileType;
	
	Image productionIcon;
	char hotkey;
	int hudX, hudY;
	
	float getCost() const;
	float getSpeed() const;
	
protected:
	float cost;
	float speed;
	
	UnitInfo(const char *filename);
	std::map<int, Animation*> animations;
	std::vector<UnitFrame*> frames;
	
	int numStates;
	std::map<std::string, int> animStates;
	
	typedef std::vector<Sound*> SoundSet;
	typedef std::map<std::string, SoundSet*> SoundPool;
	SoundPool sounds;
	std::map<std::string, int> lastPlayed;
};

class Animation
{
public:
	enum AnimationType {
		animationIdle,
		animationMove,
		animationTurret,
		animationTurretShoot,
	};
	Animation(UnitInfo *context, Parser::Line &L);
	int getFrame(float time);
	int getDuration() { return duration; }
	
protected:
	struct AnimationStep {
		AnimationStep(int frame, int duration) :frame(frame),duration(duration) {}
		int frame;
		int duration;
	};
	int duration;
	std::vector<AnimationStep> frames;
};

struct UnitFrame
{
	Image image;
	int frame;
	float angle;
	float centerX, centerY;
	float rotation;
	float scaleX, scaleY;
};


class CommandButton
{
public:
	CommandButton(Image image, char hotkey, unsigned x, unsigned y,
	              Command::CommandType cmd, Command::CommandType unitcmd,
	              std::string tooltip);
	CommandButton(Image image, char hotkey, unsigned x, unsigned y,
	              Command::CommandType cmd,
	              std::string tooltip);
	CommandButton(Image image, char hotkey, unsigned x, unsigned y, std::string unit,
	              std::string tooltip);
	char hotkey;
	Image image;
	unsigned preferredX, preferredY;
	std::string tooltip;
	
	Command::CommandType command;
	
	std::string unitType; // For produce-a-unit command buttons
	
	bool targetted, isProduction;
	Command::CommandType unitCommand;   // Different commands issued depending on target (eg attack vs. attack-move)
};

class CommandCard
{
public:
	CommandCard();
	void addButton(CommandButton *button);
	void addButtonName(const std::string &name);
	CommandButton *getButton(int x, int y);
	
	static const unsigned width=4, height=3;
	
protected:
	CommandButton *buttons[height][width];
};
void initCommandCard();
extern CommandCard scrapyardCommands;
extern CommandCard blankCommandCard;

#endif
