#ifndef MODEL_HPP
#define MODEL_HPP

#include "types.hpp"
#include "image.hpp"
#include "unitinfo.hpp"
#include "util.hpp"
#include "effects.hpp"
#include <vector>
#include <list>
#include <stdexcept>

//////////////////////////////////////////////////////////////////////////////

class Model
{
public:
	class Tile;
	struct TileType;
	class Unit;
	class Player;
	class Scrapyard;
	class OilWell;
	class Projectile;
	
	Model(const char *filename);  ///< Initialize the game state from the map given by #filename
	~Model();
	Tile *getTile(unsigned x, unsigned y); ///< Return the tile at (x,y)
	const Tile *getTile(unsigned x, unsigned y) const;
	
	Unit *getUnitAt(unsigned x, unsigned y, bool air); ///< Return the unit on the tile at (x, y), or NULL if that tile is empty
	Unit *getUnit(unitID id);                ///< Return the unit with the given unit ID
	Unit *getUnitType(const char *type);     ///< Get a unit representative of #type to construct a new unit with
	void getUnitList(std::vector<unitID> &vec); ///< Get a list of unitIDs of all living units
	void getScrapyardList(std::vector<Scrapyard*> &vec);
	Scrapyard *getScrapyard(int id);
	ICoord nearestScrapyardTo(int x, int y);
	ICoord nearestFriendlyScrapyardTo(int x, int y, playerID owner);
	ICoord spaceNearestTo(unsigned x, unsigned y, bool flying) const; ///< The nearest empty space, with a spiral search
	inline unsigned getSizeX() const { return sizeX; }
	inline unsigned getSizeY() const { return sizeY; }
	bool isInBounds(int x, int y) { return x>0 && y>0 && x<(int)sizeX && y<(int)sizeY; }
	void setRandomPlayers(int size);
	
	unitID newUnitID();
	void setUnitPosition(unitID unit, double x, double y);
	void setUnitDestination(unitID unit, int x, int y);
	void setUnitPath(unitID unit, int x, int y);
	void putUnitInTransport(unitID unitId, unitID transport);
	void deleteUnit(unitID id);              ///< Remove unit #id (because it was killed)
	
	unitID addUnit(playerID player, std::string unit, double x, double y, unitID unitId = 0);
	playerID addPlayer();
	playerID addPlayer(playerID id);
	
	std::vector<Projectile> shotsPending;
	std::list<Effect> effects;
	std::vector<OilWell> oilWells;
	
	unsigned short** getFog(playerID player);
	void revealFogAround(unitID unit);
	void revealFog(playerID player, int radius, int x, int y);
	void concealFog(playerID player, int radius, int x, int y);
	bool playerCanSee(playerID player, unsigned x, unsigned y);
	std::map<playerID, unsigned short**> fogOfWar;
	
	unsigned maxPlayers;  // Max. players this map can have
	unsigned maxPlayerID; // Highest player ID of a player who has actually joined
	
protected:
	typedef std::map<unitID, Unit*> UnitMap;

	typedef std::map<playerID, Player *> PlayerMap;
	PlayerMap players;
	
	unsigned sizeX, sizeY;
	Tile **map;
	std::vector<Scrapyard> scrapyards;
	UnitMap units;
	unsigned maxUnitID;

	std::vector<int> randomPlayerIDs;
	
	class MapLoadError : std::runtime_error {
		public: MapLoadError(const std::string &message) : std::runtime_error(message) {}
	};
	void loadTileset(std::string filename, std::map<std::string, TileType*> &result);
};

//////////////////////////////////////////////////////////////////////////////

class Model::Tile
{
public:
	Tile();
	
	bool passable(bool air) const;
	Image image() const;
	
	/// This tile's type (eg dirt, grass, dirt-grass transition, etc.)
	/// \sa Model::TileType
	Model::TileType *type;
	
	/// The index of any scrapyard this tile is a part of, or -1 for none.
	int scrapyard;
	
	int providesFuel :1;
	
	/// The ID of any unit which is on this tile, or 0 if unoccupied.
	/// (If a unit is moving, it may occupy more than one tile.)
	unitID &unitAt(bool air) { return air?airUnit:unit; }
	const unitID &unitAt(bool air) const { return air?airUnit:unit; }
	
protected:
	unitID unit;
	unitID airUnit;
};

struct Model::TileType
{
	TileType(const char* name, Image image, bool passable)
		: name(name), image(image), passable(passable) {}
	const char* name;
	Image image;
	bool passable;
};

//////////////////////////////////////////////////////////////////////////////

class Model::Unit
{
public:
	Unit(const char *filename);
	
	const std::vector<UnitFrame*> getFrame(int frame);
	Image mapViewImage();
	inline unitID getId()  const { return id; }
	inline double getX()   const { return x;  }
	inline double getY()   const { return y;  }
	inline void setX(double x)   { this->x = x; }
	inline void setY(double y)   { this->y = y; }
	float getSpeed()       const { return type->getSpeed(); }
	
//protected:
	enum State {
		move,     // Will move to (destX,destY), shooting along the way
		attack,   // Will move to (destX,destY) and chase any targets encountered
		fighting, // Attack-moved and encountered a target, now chasing it
		patrol,
		follow,
		sinking,  // Singing into the ground to convert a scrapyard (z goes from 0 to -1)
	};
	
	unitID id;
	playerID owner;
	const UnitInfo *type;
	
	int hp;
	int fuelMax; // measured in tiles
	double fuel;
	
	double x, y;
	float z;
	int nextX, nextY;
	int destX, destY;
	int destAngle, turretDestAngle; // What direction should this unit be facing? (degrees)
	float angle, turretAngle;
	bool turningToShoot;
	bool inTransport;
	float animTime;
	float shootAnimTime;
	
	State state;
	bool moving;
	unitID target, escort;
	bool explicitTarget; // Whether this target was given by the player (otherwise it was assigned automatically)
	double nextShotTime;
	int burstFireTimer;

	std::vector<unitID> loadedUnits;
};

//////////////////////////////////////////////////////////////////////////////

class Model::Player
{
public:
	Player(const char *playerName, playerID playerId);

	// Could add some more stuff, like score, or whatever
	std::string name;
	playerID id;
};

//////////////////////////////////////////////////////////////////////////////

const int scrapyardSightRadius = 10;

class Model::Scrapyard
{
public:
	Scrapyard(playerID owner, unsigned x, unsigned y, unsigned sizeX, unsigned sizeY, int scrapyardId);
	
	void produceNext();
	std::string currentProduction() const;
	void enqueue(std::string unit);
	void cancel(int index);
	void clear();
	
	int getId() const { return id; }
	playerID owner;
	double startTime;
	ICoord center;
	ICoord size;
	ICoord rally;
	
	//std::string unitType, nextType;
	std::vector<std::string> buildQueue;
	int queuePos;
	bool repeating;
	
protected:
	int id;
};

//////////////////////////////////////////////////////////////////////////////

class Model::OilWell
{
public:
	OilWell(unsigned x, unsigned y, unsigned sizeX, unsigned sizeY);
	ICoord center, size;
};

//////////////////////////////////////////////////////////////////////////////

class Model::Projectile
{
public:
	enum ProjectileType {
		none = 0,
		tankShell,
		missile,
		artilleryShell,
	};
	static ProjectileType getProjectileType(std::string str);
	
	Projectile(Model *model, unitID shooter, ProjectileType type, int explosionType,
	           double targetX, double targetY, float speed, int damage, float splash, bool hitsAir);
	
	float splash;
	double startTime;
	double startX, startY, startZ;
	double targetX, targetY, targetZ;
	float speed;
	int damage;
	bool hitsAir;
	ProjectileType type;
	int explosionType;
};

//////////////////////////////////////////////////////////////////////////////

inline Model::Tile *Model::getTile(unsigned x, unsigned y) { return &map[y][x]; }
inline const Model::Tile *Model::getTile(unsigned x, unsigned y) const { return &map[y][x]; }

#endif
