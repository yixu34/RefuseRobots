#include "main.hpp"
#include <cmath>

Image imageMissile("missile.png"),
      missileShadow("missile-shadow.png"),
	  imageTankShell("bullet.png");

void ScreenView::drawProjectile(const Model::Projectile *p)
{
	double distance = std::sqrt((p->targetX-p->startX)*(p->targetX-p->startX) +
		                        (p->targetY-p->startY)*(p->targetY-p->startY));
	double travelTime = distance/p->speed;
	double weight = ((getTime()-p->startTime)/travelTime);
	double x = p->startX + weight * (p->targetX-p->startX);
	double y = p->startY + weight * (p->targetY-p->startY);
	double z = (1-weight)*p->startZ + weight * p->targetZ;
	
	float heading = angleHeading(p->startX, p->startY, p->targetX, p->targetY)+180;
	glColor3ub(255, 255, 255);
	
	switch(p->type)
	{
		case Model::Projectile::artilleryShell:
			z = travelTime * 6.0 * weight*(weight-1); // Add a parabolic arc
			if(z<0) z=-z;
			
			glPushMatrix();
				glTranslatef(x, y, 0);
				glRotatef(heading, 0, 0, -1);
				drawImage(-0.5, -0.5-z, 1, 1, imageMissile);
				drawImage(-0.5, -0.5, 1, 1, missileShadow);
			glPopMatrix();
			break;
		case Model::Projectile::tankShell:
			glPushMatrix();
				glTranslatef(x, y, 0);
				glRotatef(heading, 0, 0, -1);
				drawImage(-.5, -.5, 1, 1, imageTankShell);
			glPopMatrix();
			break;
		default:
			glPushMatrix();
				glTranslatef(x, y, 0);
				glRotatef(heading, 0, 0, -1);
				drawImage(-.5, -.5, 1, 1, imageMissile);
			glPopMatrix();
			glPushMatrix();
				glTranslatef(x-z*.2, y+z, 0);
				glRotatef(heading, 0, 0, -1);
				drawImage(-.5, -.5, 1, 1, missileShadow);
			glPopMatrix();
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////

Model::Projectile::Projectile(Model *model, unitID shooter, ProjectileType type, int effect,
    double targetX, double targetY, float speed, int damage, float splash, bool hitsAir)
	: targetX(targetX), targetY(targetY), speed(speed), damage(damage), splash(splash), hitsAir(hitsAir),
	  type(type), explosionType(effect)
{
	Model::Unit *source = model->getUnit(shooter);
	source->shootAnimTime = 0;
	startX = source->x + 0.5;
	startY = source->y + 0.5;
	
	float dirVecX = targetX-startX,
	      dirVecY = targetY-startY,
	      dirVecLen = sqrt(dirVecX*dirVecX+dirVecY*dirVecY);
	dirVecX /= dirVecLen, dirVecY /= dirVecLen;
	
	startX += dirVecX * source->type->barrelLength;
	startY += dirVecY * source->type->barrelLength;
	
	if(source->type->flying) startZ = 1.25;
	else                     startZ = 0.05;
	if(hitsAir) targetZ = 1.25;
	else        targetZ = 0.05;
	startTime = getTime();
}

Model::Projectile::ProjectileType Model::Projectile::getProjectileType(std::string str)
{
	if(str=="missile")
		return missile;
	else if(str=="artillery")
		return artilleryShell;
	else if(str=="tank")
		return tankShell;
	else
		return none;
}

