#include "main.hpp"
#include <vector>
#include <cassert>
#include <cmath>

const float scrollSpeed = 50.0;
const float mapScrollSpeed = 120.0;
const float transitionSpeed = 6.0;

const float selectionBoxRed   = 0.0,
            selectionBoxGreen = 1.0,
            selectionBoxBlue  = 0.0;

Image selectionCircle("selection.png");

ScreenView::ScreenView(Model *model, CommandQueue *commandQueue, playerID playerId)
	: View(model, commandQueue, playerId), EventListener(1)
{
	cameraX = cameraY = 0;
	mapCameraX = mapCameraY = 0;
	arrowHeldLeft = arrowHeldRight = arrowHeldDown = arrowHeldUp = false;
	hotkeysHeld = 0;
	dragging = false;
	hudButtonPressedX = -1;
	hudButtonPressedY = -1;
	input = NULL;
	messages = NEW TextDisplay(40, 50, 900, 450, fontDefault, true);
	messages->setTimeout(20.0);
	mapView = false;
	mapViewTransition = 0.0;
	showUnitStatus = false;
	pendingCommand = Command::none;
	doubleClickTime = 0;
	draggingMinimap = false;
	onSetPlayerId(playerId);
	nextSpeechTime = 0;
	
	for(int ii=0; ii<10; ii++)
		scrapyardHotkeys[ii] = -1;
	selectedScrapyard = -1;
	
	generateMinimap();
}

ScreenView::~ScreenView()
{
	delete messages;
	messages = NULL;
}


void ScreenView::onSetPlayerId(playerID id)
{
	std::vector<Model::Scrapyard*> vec;
	model->getScrapyardList(vec);
	for(unsigned ii=0; ii<vec.size(); ii++) {
		if(vec[ii]->owner == playerId)
			centerCameraAt(vec[ii]->center.x, vec[ii]->center.y);
	}
}


void ScreenView::redraw()
{
	cullSelected();
	if(mapViewTransition >= 1.0 && mapView)
		drawMapView();
	else if(mapViewTransition <= 0.0 && !mapView)
		drawMain();
	else
	{
		float normalTop   = mapViewTileSize * (cameraY-mapCameraY) * mapViewTransition,
		      normalLeft  = mapViewTileSize * (cameraX-mapCameraX) * mapViewTransition;
		float smallSize = (float)mapViewTileSize / tileSize;
		float normalWidth = smallSize + (1.0f-mapViewTransition)*(1.0f-smallSize),
		      normalHeight = smallSize + (1.0f-mapViewTransition)*(1.0f-smallSize);
		
		if(mapView) // Transitioning from main->mapview
			mapViewTransition += transitionSpeed*getDt();
		else // Transitioning from mapview->main
			mapViewTransition -= transitionSpeed*getDt();
		
		drawMapView();
		glPushMatrix();
			glViewport(normalLeft, screenHeight-normalTop-normalHeight*screenHeight,
			           screenWidth*normalWidth, screenHeight*normalHeight);
			drawMain();
			glViewport(0, 0, screenWidth, screenHeight);
			
			glTranslatef(normalLeft, normalTop, 0);
			glScalef(normalWidth, normalHeight, 1.0);
			
			glColor3f(1.0, 1.0, 1.0);
			glLineWidth(2.0);
			glBegin(GL_LINE_LOOP);
				glVertex2i(0,             0);
				glVertex2i(screenWidth-1, 0);
				glVertex2i(screenWidth-1, screenHeight-1);
				glVertex2i(0,             screenHeight-1);
			glEnd();
		glPopMatrix();
	}
	drawHUD();
}

void ScreenView::cullSelected()
{
	for(UnitSet::iterator ii=selection.begin(); ii!=selection.end(); )
	{
		Model::Unit *u = model->getUnit(*ii);
		if(u==NULL || u->inTransport) {
			ii = selection.erase(ii);
		} else {
			ii++;
		}
	}
}

void ScreenView::drawMain()
{
	typedef std::set<Model::Unit*> UnitList;
	UnitList airUnits, groundUnits;
	unsigned short **fog = model->getFog(playerId);
	
	unsigned left = (unsigned)cameraX;
	unsigned top  = (unsigned)cameraY;
	unsigned right = left+(screenWidth/tileSize)+1;
	unsigned bottom = top+(screenHeight/tileSize)+1;
	if(left<0) left=0;
	if(top<0) top=0;
	if(right  >= model->getSizeX()) right  = model->getSizeX()-1;
	if(bottom >= model->getSizeY()) bottom = model->getSizeY()-1;
	
	glPushMatrix();
	glTranslatef(-(int)(cameraX*tileSize), -(int)(cameraY*tileSize), 0);
	glScalef(tileSize, tileSize, 1.0);
	glColor3f(1.0, 1.0, 1.0);
	
	//
	// Draw all tiles top-to-bottom, taking note of relevant units along the way.
	//
	for(unsigned yi=top;  yi<=bottom; yi++)
	for(unsigned xi=left; xi<=right;  xi++)
	{
		Model::Tile *tile = model->getTile(xi, yi);
		
		tile->type->image.bind();
		
		unsigned char ulcolor = fog[yi  ][xi  ] ? 255 : 100;
		unsigned char urcolor = fog[yi  ][xi+1] ? 255 : 100;
		unsigned char blcolor = fog[yi+1][xi  ] ? 255 : 100;
		unsigned char brcolor = fog[yi+1][xi+1] ? 255 : 100;
		
		glBegin(GL_TRIANGLE_FAN);
			if(!!fog[yi][xi] == !!fog[yi+1][xi+1])
			{
				glColor3ub(ulcolor,ulcolor,ulcolor); glTexCoord2f(0.0, 0.0); glVertex2f(xi,     yi    );
				glColor3ub(urcolor,urcolor,urcolor); glTexCoord2f(1.0, 0.0); glVertex2f(xi+1.0, yi    );
				glColor3ub(brcolor,brcolor,brcolor); glTexCoord2f(1.0, 1.0); glVertex2f(xi+1.0, yi+1.0);
				glColor3ub(blcolor,blcolor,blcolor); glTexCoord2f(0.0, 1.0); glVertex2f(xi,     yi+1.0);
			}
			else
			{
				glColor3ub(urcolor,urcolor,urcolor); glTexCoord2f(1.0, 0.0); glVertex2f(xi+1.0, yi    );
				glColor3ub(brcolor,brcolor,brcolor); glTexCoord2f(1.0, 1.0); glVertex2f(xi+1.0, yi+1.0);
				glColor3ub(blcolor,blcolor,blcolor); glTexCoord2f(0.0, 1.0); glVertex2f(xi,     yi+1.0);
				glColor3ub(ulcolor,ulcolor,ulcolor); glTexCoord2f(0.0, 0.0); glVertex2f(xi,     yi    );
			}
		glEnd();
		
		if(model->playerCanSee(playerId, xi, yi))
		{
			Model::Unit *u;
			u = model->getUnitAt(xi, yi, false);
			if(u) groundUnits.insert(u);
			u = model->getUnitAt(xi, yi, true);
			if(u) airUnits.insert(u);
		}
	}
	
	// Draw scrapyard selection circle (if applicable)
	if(selectedScrapyard >= 0) {
		Model::Scrapyard *s = model->getScrapyard(selectedScrapyard);
		ICoord pos = s->center;
		ICoord size = s->size;
		
		glColor3f(1.0, 1.0, 1.0);
		drawImage(pos.x-size.x/2, pos.y-size.y/2, size.x, size.y, selectionCircle);
	}
	
	// Draw ground units
	for(UnitList::iterator ii=groundUnits.begin(); ii!=groundUnits.end(); ii++)
	{
		Model::Unit *u = *ii;
		drawUnit(u);
	}
	
	// Draw projectiles
	double time = getTime();
	glBindTexture(GL_TEXTURE_2D, 0);
	for(unsigned ii=0; ii<model->shotsPending.size(); ii++)
	{
		const Model::Projectile *p = &model->shotsPending[ii];
		drawProjectile(p);
	}
	
	// Draw air units
	for(UnitList::iterator ii=airUnits.begin(); ii!=airUnits.end(); ii++)
	{
		Model::Unit *u = *ii;
		drawUnit(u);
	}
	
	drawEffects();
	
	// Draw a selection box
	if(dragging) {
		glColor3f(selectionBoxRed, selectionBoxGreen, selectionBoxBlue);
		glBindTexture(GL_TEXTURE_2D, 0);
		glLineWidth(2.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(mouseX,     mouseY);
			glVertex2f(dragStartX, mouseY);
			glVertex2f(dragStartX, dragStartY);
			glVertex2f(mouseX,     dragStartY);
		glEnd();
	}
	
	glPopMatrix();
}

void ScreenView::drawUnit(Model::Unit *unit)
{
	float x = unit->getX(),
	      y = unit->getY();
	
	// Draw selection circle
	if(selection.find(unit->getId()) != selection.end()) {
		glColor3f(1.0, 1.0, 1.0);
		drawImage(x, y, 1.0, 1.0, selectionCircle);
	}
	
	// Draw the unit itself
	std::vector<UnitFrame*> frames;
	if(unit->moving)
		frames = unit->getFrame(Animation::animationMove);
	else
		frames = unit->getFrame(Animation::animationIdle);
	
	std::vector<UnitFrame*> turretFrames = unit->getFrame(Animation::animationTurret);
	frames.insert(frames.end(), turretFrames.begin(), turretFrames.end());
	
	// Draw unit meta-info
	if((showUnitStatus || selection.find(unit->getId()) != selection.end()) && playerId == unit->owner)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
		glColor3f(0.0, 0.0, 0.0);
		glBegin(GL_QUADS);
			// Draw HP bar
			glColor3f(0.0, 0.0, 0.0);
			glVertex2f(x+0.05, y+0.92);
			glVertex2f(x+0.95, y+0.92);
			glVertex2f(x+0.95, y+1.0);
			glVertex2f(x+0.05, y+1.0);
			
			glColor3f(0.0, 1.0, 0.0);
			glVertex2f(x+0.05,                                           y+0.92);
			glVertex2f(x+0.05+0.9*((float)unit->hp / unit->type->maxHP), y+0.92);
			glVertex2f(x+0.05+0.9*((float)unit->hp / unit->type->maxHP), y+1.0);
			glVertex2f(x+0.05,                                           y+1.0);
			
			// Draw fuel bar
			glColor3f(1.0, 1.0, 0.3);
			glVertex2f(x+0.05,                                         y+0.85);
			glVertex2f(x+0.05+0.9*((float)unit->fuel / unit->fuelMax), y+0.85);
			glVertex2f(x+0.05+0.9*((float)unit->fuel / unit->fuelMax), y+0.92);
			glVertex2f(x+0.05,                                         y+0.92);
		glEnd();
		
		// Draw loaded-units bar
		if(unit->loadedUnits.size()) {
			glColor3f(1.0, 1.0, 0.0);
			float d = 0.9/unit->type->transports;
			float width = (double)unit->loadedUnits.size()/unit->type->transports;;
			
			glBegin(GL_QUADS);
				glVertex2f(x+0.05,           y);
				glVertex2f(x+0.05+0.9*width, y);
				glVertex2f(x+0.05+0.9*width, y+0.1);
				glVertex2f(x+0.05,           y+0.1);
			glEnd();
			glLineWidth(1.0);
			glBegin(GL_LINES);
				glColor3f(0.0, 0.0, 0.0);
				for(unsigned ii=1; ii<unit->loadedUnits.size(); ii++)
				{
					glVertex2f(x+0.05+ii*d, y);
					glVertex2f(x+0.05+ii*d, y+0.11);
				}
			glEnd();
		}
	}
	
	// Draw the unit itself
	for(unsigned jj=0; jj<frames.size(); jj++)
	{
		UnitFrame *frame = frames[jj];
		glPushMatrix();
			glTranslatef(x+0.5, y+0.5, 0);
			glRotatef(frame->rotation, 0, 0, -1);
			glTranslatef(0.5-frame->centerX, 0.5-frame->centerY, 0);
			glScalef(frame->scaleX, frame->scaleY, 1.0);
			
			frame->image.bind();
			
			switch(unit->owner)
			{
				default:
				case 1: glColor3f(1.0, 1.0, 1.0); break;
				case 2: glColor3f(1.0, 0.7, 0.7); break;
				case 3: glColor3f(0.7, 1.0, 0.7); break;
				case 4: glColor3f(0.7, 0.7, 1.0); break;
			}
			
			if(unit->z >= 0)
			{
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0); glVertex2f(-0.5, -0.5);
					glTexCoord2f(1.0, 0.0); glVertex2f(+0.5, -0.5);
					glTexCoord2f(1.0, 1.0); glVertex2f(+0.5, +0.5);
					glTexCoord2f(0.0, 1.0); glVertex2f(-0.5, +0.5);
				glEnd();
			}
			else
			{
				glBegin(GL_QUADS);
					glTexCoord2f(0.0, 0.0        ); glVertex2f(-0.5, -0.5-unit->z);
					glTexCoord2f(1.0, 0.0        ); glVertex2f(+0.5, -0.5-unit->z);
					glTexCoord2f(1.0, 1.0+unit->z); glVertex2f(+0.5, +0.5        );
					glTexCoord2f(0.0, 1.0+unit->z); glVertex2f(-0.5, +0.5        );
				glEnd();
			}
			
			glColor3f(1.0, 1.0, 1.0);
		glPopMatrix();
	}
}

void ScreenView::centerCameraAt(float x, float y)
{
	mapCameraX = x - ((float)screenWidth)/(mapViewTileSize*2);
	mapCameraY = y - ((float)screenHeight)/(mapViewTileSize*2);
	cameraX = x - ((float)screenWidth)/(tileSize*2);
	cameraY = y - ((float)screenHeight)/(tileSize*2);
	clipCamera();
}

bool ScreenView::isOnScreen(int x, int y)
{
	if(mapView) {
		return x+5 > mapCameraX && x-5 < (mapCameraX + ((float)screenWidth)/mapViewTileSize) &&
		       y+5 > mapCameraY && y-5 < (mapCameraY + ((float)screenHeight)/mapViewTileSize);
	} else {
		return x+5 > cameraX && x-5 < (cameraX + ((float)screenWidth)/tileSize) &&
		       y+5 > cameraY && y-5 < (cameraY + ((float)screenHeight)/tileSize);
	}
}


void ScreenView::timepass(float dt)
{
	int scrollDirectionX = 0, scrollDirectionY = 0;
	if(arrowHeldLeft)  scrollDirectionX--;
	if(arrowHeldRight) scrollDirectionX++;
	if(arrowHeldUp)    scrollDirectionY--;
	if(arrowHeldDown)  scrollDirectionY++;
	mouseScroll(dt);
	
	cameraX += scrollSpeed * scrollDirectionX * dt;
	cameraY += scrollSpeed * scrollDirectionY * dt;
	mapCameraX += mapScrollSpeed * scrollDirectionX * dt;
	mapCameraY += mapScrollSpeed * scrollDirectionY * dt;
	clipCamera();
	
	if(input) {
		switch(input->getResult()) {
			case -1:
				delete input;
				input = NULL;
				break;
			case 1:
				cheat(input->getString());
				writeChat(input->getString());
				delete input;
				input = NULL;
				break;
			default:
			case 0:
				break;
		}
	}
	messages->timepass(dt);
}

void ScreenView::clipCamera()
{
	float deadSpaceX = (float)screenWidth / mapViewTileSize - model->getSizeX(),
	      deadSpaceY = (float)screenMainHeight / mapViewTileSize - model->getSizeY();
	float mapClipLeft = deadSpaceX>0 ? -(deadSpaceX/2) : 0,
	      mapClipTop = deadSpaceY>0 ? -(deadSpaceY/2) : 0;
		
	if(mapCameraX + (double)screenWidth/mapViewTileSize > model->getSizeX())
		mapCameraX = model->getSizeX() - ((double)screenWidth)/mapViewTileSize;
	if(mapCameraY + (double)screenMainHeight/mapViewTileSize > model->getSizeY())
		mapCameraY = model->getSizeY() - ((double)screenMainHeight)/mapViewTileSize;
	if(mapCameraX < mapClipLeft)
		mapCameraX = mapClipLeft;
	if(mapCameraY < mapClipTop)
		mapCameraY = mapClipTop;
	
	if(cameraX + (double)screenWidth/tileSize > model->getSizeX())
		cameraX = model->getSizeX() - ((double)screenWidth)/tileSize;
	if(cameraY + (double)screenMainHeight/tileSize > model->getSizeY())
		cameraY = model->getSizeY() - ((double)screenMainHeight)/tileSize;
	if(cameraX < 0) cameraX = 0;
	if(cameraY < 0) cameraY = 0;
}


void ScreenView::writeChat(const std::string &str)
{
	messages->println(formatChatText(playerName, str));

	// Fixme? The view really shouldn't be interacting with the network
	// directly.
	if(network)
	{
		// Send the message to others.
		Packet *textPacket = new Packet(msg_game_chat);
			textPacket->putString(formatChatText(playerName, str));
			textPacket->putInt(playerId);
		network->sendToAll(textPacket);
	}
	//TODO: SOUND add msgsent
}

void ScreenView::receiveChat(const std::string &str, playerID id)
{
	if (id != playerId)
		messages->println(str);
	//TODO: SOUND add msgreceived
}

void ScreenView::addToSelection(unitID id)
{
	Model::Unit *unit = model->getUnit(id);
	selectedScrapyard = -1;
	if (unit->owner == playerId)
		selection.insert(id);
}

void ScreenView::addToSelection(UnitSet::iterator unitsBegin, UnitSet::iterator unitsEnd)
{
	for (UnitSet::iterator it = unitsBegin;
		 it != unitsEnd;
		 it++)
	{
		addToSelection(*it);
	}
	selectedScrapyard = -1;
}

void ScreenView::removeFromSelection(unitID id)
{
	Model::Unit *unit = model->getUnit(id);
	if (unit->owner == playerId)
		selection.erase(id);
}

int ScreenView::getSelectionSize() {
	return selection.size();
}

void printChatText(const std::string &str, playerID id)
{
	if (engine && engine->view)
		engine->view->receiveChat(str, id);
}

void ScreenView::playSoundAt(int x, int y, Sound *s) {
	int volume = 100;
	//TODO: Rewrite this entire function from scratch. --Jim
	if(mapView)
		volume=70;
	else {
		double dist= std::abs((x - cameraX)/(screenWidth/tileSize)) + (std::abs((y - cameraY)/(screenHeight/tileSize)));
		if (dist>35)
			volume=30;
		else if (dist>25)
			volume= 50;
		else if (dist>15)
			volume= 70;
		else
			volume= 100;
	}
	
	/*else {
		if (x>cameraX) 
		{
			if ((x - cameraX)/(screenWidth/tileSize) < 1.0) {
				if (y>cameraY) {
					if ((y - cameraY)/(screenHeight/tileSize) < 1.0)
						volume=98;
				}else {
					if ((cameraY - y)/(screenHeight/tileSize) < 1.0)
						volume=98;
				}
			}
		} else if (true) {
			if ((x - cameraX)/(screenWidth/tileSize) < 1.0) {
				if (y>cameraY) {
					if ((y - cameraY)/(screenHeight/tileSize) < 1.0)
						volume=98;
				}else {
					if ((cameraY - y)/(screenHeight/tileSize) < 1.0)
						volume=98;
				}
			}
		}else {
			//TODO assert ((x - cameraX)/screenWidth * tileSize) + (y - cameraY)/screenHeight*tileSize) != 0
			volume = 54 / ((int) ((x - cameraX)/screenWidth * tileSize) + (y - cameraY)/screenHeight*tileSize);
		}
	}*/
	s->play(0, volume);	
}

void ScreenView::playMove(unitID id) {
	unitSpeech(model->getUnit(id)->type->getSound("move"));
}

void ScreenView::playAcknowledge(unitID id){
	unitSpeech(model->getUnit(id)->type->getSound("select"));
}

void ScreenView::playRetreat(unitID id){
	unitSpeech(model->getUnit(id)->type->getSound("retreat"));
}

void ScreenView::playAttack(unitID id){
	unitSpeech(model->getUnit(id)->type->getSound("attack"));
}

void ScreenView::unitSpeech(Sound *sound)
{
	if(sound && getTime() >= nextSpeechTime) {
		sound->play();
		nextSpeechTime = getTime()+sound->getLength();
	}
}

