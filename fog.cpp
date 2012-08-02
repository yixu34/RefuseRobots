#include "main.hpp"

void Model::revealFog(playerID player, int radius, int x, int y)
{
	if(player==0)
		return;
	int r2 = radius*radius;
	int xi, yi;
	int minx = x-radius,
	    maxx = x+radius,
	    miny = y-radius,
	    maxy = y+radius;
	if(minx<0) minx=0;
	if(miny<0) miny=0;
	if(maxx>=(int)sizeX) maxx=sizeX-1;
	if(maxy>=(int)sizeY) maxy=sizeY-1;
	
	for(yi=miny; yi<=maxy; yi++)
	for(xi=minx; xi<=maxx; xi++)
	{
		if( (xi-x)*(xi-x) + (yi-y)*(yi-y) <= r2 )
			fogOfWar[player][yi][xi]++;
	}
}
void Model::concealFog(playerID player, int radius, int x, int y)
{
	if(player==0)
		return;
	int r2 = radius*radius;
	int xi, yi;
	int minx = x-radius,
	    maxx = x+radius,
	    miny = y-radius,
	    maxy = y+radius;
	if(minx<0) minx=0;
	if(miny<0) miny=0;
	if(maxx>=(int)sizeX) maxx=sizeX-1;
	if(maxy>=(int)sizeY) maxy=sizeY-1;
	
	for(yi=miny; yi<=maxy; yi++)
	for(xi=minx; xi<=maxx; xi++)
	{
		if( (xi-x)*(xi-x) + (yi-y)*(yi-y) <= r2 )
			fogOfWar[player][yi][xi]--;
	}
}

bool Model::playerCanSee(playerID player, unsigned x, unsigned y)
{
	return fogOfWar[player][y][x]>0 || fogOfWar[player][y+1][x+1]>0;
}

void Model::revealFogAround(unitID unit)
{
	Unit *u = getUnit(unit);
	revealFog(u->owner, u->type->vision, u->nextX, u->nextY);
}

