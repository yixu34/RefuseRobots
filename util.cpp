#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDL.h>
#include <deque>
#include "main.hpp"

std::string retprintf(const char *fmt, ...)
{
	va_list args;
	char buf[1024];
	va_start(args, fmt);
	_vsnprintf(buf, 1024, fmt, args);
	va_end(args);
	return std::string(buf);
}

float angleDifference(float a, float b)
{
	float diff;
	if(a>b) diff = a-b;
	else    diff = b-a;
	if(diff > 180)
		return 360-diff;
	else
		return diff;
}

float angleTowards(float angle, float dest, float dist)
{
	float ret;
	
	if((angle>dest) ^ (abs(angle-dest)<180))
		ret = dest - (angleDifference(angle,dest)-dist);
	else
		ret = dest + (angleDifference(angle,dest)-dist);
	
	if(ret<0) ret += 360;
	if(ret>=360) ret -= 360;
	
	return ret;
}

float angleHeading(float sourceX, float sourceY, float destX, float destY)
{
	float ret = (180.0/M_PI) * std::atan2(sourceY-destY, destX-sourceX);
	if(ret<0) ret += 360;
	return ret;
}

float randFloat(float min, float max)
{
	return (float)(((double)rand() / (double)RAND_MAX) * (max-min) + min);
}

void mapRect(Model *model, ICoord center, ICoord size, int &left, int &top, int &right, int &bottom)
{
	left   = center.x - size.x;
	right  = center.x + size.x;
	top    = center.y - size.y;
	bottom = center.y + size.y;
	if(left<0) left=0;
	if(top <0) top=0;
	if(right >= (int)model->getSizeX())
		right = model->getSizeX() - 1;
	if(bottom >= (int)model->getSizeY())
		bottom = model->getSizeY() - 1;
}


std::string formatChatText(
	const std::string &playerName, 
	const std::string &text)
{
	if(playerName=="")
		return text;
	else
		return playerName + ":  " + text;
}


static std::deque<double> times;
static double currentTime, dt, framerate;
double timeOffset;

void updateTime()
{
	currentTime = ((double)SDL_GetTicks())/1000;
	times.push_front(currentTime);
	if(times.size() > 10)
		times.pop_back();
	if(times.size() > 1)
		framerate = (double)times.size() / (times[0] - times[times.size()-1]);
	else
		framerate = 60;
	
	if(framerate <= 0) // Weird wraparound effect
		framerate = 60;
	
	dt = 1.0/framerate;
}

void resetTime()
{
	timeOffset = -currentTime;
}

double getTime()
{
	return currentTime - timeOffset;;
}

double getDt()
{
	return dt;
}

double getFramerate()
{
	return framerate;
}
