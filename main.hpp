#ifndef MAIN_HPP
#define MAIN_HPP

#include <SDL.h>
#include <time.h>
#include <stdlib.h>
#include <glut.h>
#include <string>
#include <map>
#include "msvcfix.hpp"
#include "event.hpp"
#include "memory.hpp"
#include "model.hpp"
#include "view.hpp"
#include "screenview.hpp"
#include "computerplayer.hpp"
#include "squad.hpp"
#include "controller.hpp"
#include "network.hpp"
#include "networknode.hpp"
#include "server.hpp"
#include "client.hpp"
#include "resourcepool.hpp"
#include "menu.hpp"
#include "frame.hpp"
#include "text.hpp"
#include "logger.hpp"
#include "packet.hpp"
#include "util.hpp"
#include "unitinfo.hpp"
#include "sound.hpp"

#define FULLSCREEN
#define USE_SOUND
//#define ENABLE_CHEATS
//#define SHOW_DEBUG

#define defaultMap "newformat.map"

class Engine
{
public:
	Engine(const char *mapName);
	~Engine();
	
	CommandQueue *commands;
	MessageQueue *messages;
	Model *model;
	ScreenView *view;
	Controller *controller;
	std::vector<AIPlayer *> aiPlayers;
};
extern Engine *engine;

extern Network *network;

const int screenWidth = 1024,
          screenHeight = 768;
const int screenMainHeight = 560;
const int hudHeight = 208;
const int screenCenterX = screenWidth/2,
          screenCenterY = screenHeight/2;

const float doubleClickDelay = 0.25; // Max time between clicks of a double click (seconds)

#endif
