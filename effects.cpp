#include "main.hpp"

Image explosion[] = {
	/*Image("effects/explosion-frame1.png"),
	Image("effects/explosion-frame2.png"),
	Image("effects/explosion-frame3.png"),
	Image("effects/explosion-frame4.png"),
	Image("effects/explosion-frame5.png"),
	Image("effects/explosion-frame6.png"),
	Image("effects/explosion-frame7.png"),
	Image("effects/explosion-frame8.png"),
	Image("effects/explosion-frame9.png"),*/
	Image("effects/explosion3_frame1.png"),
	Image("effects/explosion3_frame2.png"),
	Image("effects/explosion3_frame3.png"),
	Image("effects/explosion3_frame4.png"),
	Image("effects/explosion3_frame5.png"),
	Image("effects/explosion3_frame6.png"),
	Image("effects/explosion3_frame7.png"),
	//Image("effects/explosion3_frame8.png"),
	Image("effects/explosion3_frame9.png"),
	//Image("effects/explosion3_frame10.png"),
	//Image("effects/explosion3_frame11.png"),
	//Image("effects/explosion3_frame12.png"),
	Image("effects/explosion3_frame13.png"),
	Image("effects/explosion3_frame14.png"),
	};
const int numExplosionFrames =(sizeof explosion)/(sizeof explosion[0]);

void ScreenView::drawEffects()
{
	for(std::list<Effect>::iterator ii=model->effects.begin(); ii!=model->effects.end(); )
	{
		Effect *e = &(*ii);
		int duration = 0;
		
		glColor3f(1.0, 1.0, 1.0);
		
		switch(e->type)
		{
			case Explosion::artilleryBoom:
				drawImage(e->pos.x-0.8, e->pos.y-0.8, 1.6, 1.6, explosion[e->lifetime/3]);
				duration = numExplosionFrames * 3;
				break;
			case Explosion::bigDeath:
				drawImage(e->pos.x-0.5, e->pos.y-0.5, 1.0, 1.0, explosion[e->lifetime/2]);
				duration = numExplosionFrames * 2;
				break;
			case Explosion::tankerDeath:
				drawImage(e->pos.x-1.5, e->pos.y-1.5, 3.0, 3.0, explosion[e->lifetime/2]);
				duration = numExplosionFrames * 2;
				break;
			case Explosion::smallShot:
				drawImage(e->pos.x-0.2, e->pos.y-0.2, 0.4, 0.4, explosion[e->lifetime]);
				duration = numExplosionFrames;
				break;
		}
		
		e->lifetime++;
		if(e->lifetime >= duration)
			ii = model->effects.erase(ii);
		else
			ii++;
	}
}

Explosion::Explosion(DCoord pos, int type)
{
	this->pos = pos;
	this->lifetime = 0;
	this->type = (EffectType)type;
}

Effect::EffectType Effect::getEffectType(std::string str)
{
	if(str=="artillery")
		return Effect::artilleryBoom;
	else if(str=="death")
		return Effect::bigDeath;
	else if(str=="tanker_death")
		return Effect::tankerDeath;
	else
		return Effect::smallShot;
}