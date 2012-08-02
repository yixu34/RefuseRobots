#include "main.hpp"

float edgeScrollSensitivity = 50.0,
      mapEdgeScrollSensitivity = 120.0;
const int scrollBorder = 10;

void ScreenView::mouseToWorld(float *x, float *y)
{
	if(mapView)
	{
		*x += mapCameraX*mapViewTileSize;
		*y += mapCameraY*mapViewTileSize;
		*x /= mapViewTileSize;
		*y /= mapViewTileSize;
	} else {
		*x += cameraX*tileSize;
		*y += cameraY*tileSize;
		*x /= tileSize;
		*y /= tileSize;
	}
}

bool ScreenView::keyDown(SDL_keysym sym)
{
	switch(sym.sym)
	{
		case SDLK_0: case SDLK_1:
		case SDLK_2: case SDLK_3:
		case SDLK_4: case SDLK_5:
		case SDLK_6: case SDLK_7:
		case SDLK_8: case SDLK_9: {
			int hotkey = sym.sym - SDLK_0;
			
			if(sym.mod & KMOD_SHIFT) {
				if(selectedScrapyard < 0) {
					if(scrapyardHotkeys[hotkey] >= 0) // Can't add to a HK group which already contains a scrapyard
						{}
					else
						hotkeys[hotkey].insert(selection.begin(), selection.end());
				} else if(hotkeys[hotkey].size()==0 && scrapyardHotkeys[hotkey]<0) {
					scrapyardHotkeys[hotkey] = selectedScrapyard;
				}
			} else if(sym.mod & KMOD_CTRL) {
				if(selectedScrapyard >= 0) {
					scrapyardHotkeys[hotkey] = selectedScrapyard;
					hotkeys[hotkey].clear();
				} else {
					scrapyardHotkeys[hotkey] = -1;
					hotkeys[hotkey] = selection;
				}
			} else {
				if(selection == hotkeys[hotkey] && scrapyardHotkeys[hotkey]==selectedScrapyard) {
					// Double-pressed hotkey: center on the group
					if(selectedScrapyard >= 0) {
						Model::Scrapyard *s = model->getScrapyard(selectedScrapyard);
						centerCameraAt(s->center.x, s->center.y);
					} else {
						float sumX=0, sumY=0;
						for(UnitSet::iterator ii=hotkeys[hotkey].begin(); ii!=hotkeys[hotkey].end(); ii++) {
							sumX += model->getUnit(*ii)->getX() + 0.5;
							sumY += model->getUnit(*ii)->getY() + 0.5;
						}
						centerCameraAt(sumX/hotkeys[hotkey].size(), sumY/hotkeys[hotkey].size());
					}
				}
				else if(hotkeysHeld > 0) {
					// Multiple hotkeys held together -> select both groups together
					addToSelection(hotkeys[hotkey].begin(), hotkeys[hotkey].end());
					playAcknowledge(*(hotkeys[hotkey].begin()));
				}
				else {
					selectedScrapyard = scrapyardHotkeys[hotkey];
					selection = hotkeys[hotkey];
				}
				hotkeysHeld++;
			}
			break;
		}
		case SDLK_KP4: case SDLK_LEFT : arrowHeldLeft  = true; break;
		case SDLK_KP6: case SDLK_RIGHT: arrowHeldRight = true; break;
		case SDLK_KP8: case SDLK_UP   : arrowHeldUp    = true; break;
		case SDLK_KP2: case SDLK_DOWN : arrowHeldDown  = true; break;
		
		case SDLK_RALT:
		case SDLK_LALT:
			showUnitStatus = true;
			break;
		
		case SDLK_RETURN:
			input = NEW TextInput(40, 503,fontDefault);
			break;
		case SDLK_ESCAPE:
			//exit(0);
			Frame::changeFrame(mainMenu);
			shouldDestroyEngine = true;
			break;
		
		case SDLK_TAB: {
			if(mapView) {
				cameraX = mouseX - ((float)screenWidth/tileSize)/2.0;
				cameraY = mouseY - ((float)screenMainHeight/tileSize)/2.0;
			} else {
				mapCameraX = mouseX - ((float)screenWidth/mapViewTileSize)/2.0;
				mapCameraY = mouseY - ((float)screenMainHeight/mapViewTileSize)/2.0;
			}
			clipCamera();
			mapView = !mapView;
			break;
		}
		
		default:
			pressHotkey(sym.sym);
			break;
	}
	return false;
}

bool ScreenView::keyUp(SDL_keysym sym)
{
	switch(sym.sym)
	{
		case SDLK_0: case SDLK_1:
		case SDLK_2: case SDLK_3:
		case SDLK_4: case SDLK_5:
		case SDLK_6: case SDLK_7:
		case SDLK_8: case SDLK_9: {
			if(hotkeysHeld>0)
				hotkeysHeld--;
		}
		case SDLK_KP4: case SDLK_LEFT : arrowHeldLeft  = false; break;
		case SDLK_KP6: case SDLK_RIGHT: arrowHeldRight = false; break;
		case SDLK_KP8: case SDLK_UP   : arrowHeldUp    = false; break;
		case SDLK_KP2: case SDLK_DOWN : arrowHeldDown  = false; break;
		
		case SDLK_RALT:
		case SDLK_LALT:
			showUnitStatus = false;
			break;
	}
	return false;
}

bool ScreenView::mouseDown(int x, int y, int button, int mod)
{
	if(getTime() < doubleClickTime && button==SDL_BUTTON_LEFT) {
		button = -1;
		doubleClickTime = 0;
	} else {
		doubleClickTime = getTime() + doubleClickDelay;
	}
	
	if(isMinimapArea(x, y))
		return mouseDownMinimap(x, y, button, mod);
	else if(isStatusArea(x, y))
		return mouseDownStatus(x, y, button, mod);
	else if(isHudArea(x, y))
		return mouseDownHud(x, y, button, mod);
	else
		return mouseDownWorld(x, y, button, mod);
}

/// Handle a mouseclick in the main world-area (not the HUD)
bool ScreenView::mouseDownWorld(int x, int y, int button, int mod)
{
	float w_x = x, w_y = y;
	mouseToWorld(&w_x, &w_y);
	if((int)w_x >= (int)model->getSizeX()) w_x = model->getSizeX()-0.5;
	if((int)w_y >= (int)model->getSizeY()) w_y = model->getSizeY()-0.5;
	if(w_x<0) w_x=0;
	if(w_y<0) w_y=0;
	Model::Unit *unit = model->getUnitAt( (int)w_x, (int)w_y, true );
	if(!unit) unit = model->getUnitAt( (int)w_x, (int)w_y, false );
	
	if(pendingUnitCommand != Command::none && unit) {
		if(pendingUnitCommand == Command::attack && selection.size())
			playAttack(*selection.begin());
		else if(selection.size())
			playMove(*selection.begin());
		issueCommand(selection, pendingUnitCommand, unit->getId(), 0);
		pendingCommand = pendingUnitCommand = Command::none;
		return true;
	}
	if(pendingCommand != Command::none) {
		if(pendingCommand == Command::attack && selection.size())
			playAttack(*selection.begin());
		else if(selection.size())
			playMove(*selection.begin());
		
		if(pendingCommand == Command::setRally)
			issueCommand(selectedScrapyard, pendingCommand, (int)w_x, (int)w_y);
		else
			issueCommand(selection, pendingCommand, (int)w_x, (int)w_y);
		
		pendingCommand = pendingUnitCommand = Command::none;
		return true;
	}
	
	switch (button)
	{
		case SDL_BUTTON_LEFT: {
			dragging = true;
			if(mod & KMOD_SHIFT)
				dragMode = 1;
			else if(mod & KMOD_CTRL)
				dragMode = -1;
			else
				dragMode = 0;
			
			float dragX=x, dragY=y;
			mouseToWorld(&dragX, &dragY);
			this->dragStartX = dragX;
			this->dragStartY = dragY;
			
			if(unit)
			{
				if(mod & (KMOD_SHIFT|KMOD_CTRL)) {
					if(selection.find(unit->getId())==selection.end())
						addToSelection(unit->getId());
					else
						removeFromSelection(unit->getId());
				} else if(mod & KMOD_ALT) {
					playRetreat(unit->getId());
					issueCommand(unit->getId(), Command::retreat, 0, 0);
				} else {
					selection.clear();
					addToSelection(unit->getId());
					playAcknowledge(unit->getId());
				}
			}
			else if(!(mod & KMOD_SHIFT))
			{
				std::vector<Model::Scrapyard*> vec;
				model->getScrapyardList(vec);
				selectedScrapyard = -1;
				for(unsigned ii=0; ii<vec.size(); ii++) {
					int topLeftX = vec[ii]->center.x - vec[ii]->size.x/2;
					int topLeftY = vec[ii]->center.y - vec[ii]->size.y/2;
					
					if( w_x>=topLeftX && w_y>=topLeftY &&
					    w_x < topLeftX+vec[ii]->size.x &&
					    w_y < topLeftY+vec[ii]->size.y &&
					    vec[ii]->owner == playerId)
					{
						selectedScrapyard = ii;
						selection.clear();
						break;
					}
				}
			}
			break;
		}
		case SDL_BUTTON_RIGHT:
			if(unit && !model->playerCanSee(playerId, unit->x, unit->y))
				unit = NULL;
			
			if(mod & KMOD_CTRL) {
				if(mapView) {
					cameraX = w_x - ((float)screenWidth/tileSize)/2.0;
					cameraY = w_y - ((float)screenMainHeight/tileSize)/2.0;
				} else {
					mapCameraX = w_x - ((float)screenWidth/mapViewTileSize)/2.0;
					mapCameraY = w_y - ((float)screenMainHeight/mapViewTileSize)/2.0;
				}
				clipCamera();
				mapView = !mapView;
			} else if(unit && unit->owner != playerId) { // TODO: Handle teams here
				if(selection.size()) {
					playAttack(*selection.begin());
					issueCommand(selection, Command::attack, unit->getId(), 0);
				}
			} else if(unit) {
				if(selection.size()) {
					playMove(*selection.begin());
					issueCommand(selection, Command::follow, unit->getId(), 0);
				}
			} else if(mod & KMOD_SHIFT) {
				if(selection.size()) {
					playAttack(*selection.begin());
					issueCommand(selection, Command::attackMove, (int)w_x, (int)w_y);
				}
			} else {
				if(selection.size()) {
					playMove(*selection.begin());
					issueCommand(selection, Command::move, (int)w_x, (int)w_y);
				}
			}
			break;
		
		case buttonDoubleClick:
			if(mapView) {
				cameraX = w_x - ((float)screenWidth/tileSize)/2.0;
				cameraY = w_y - ((float)screenMainHeight/tileSize)/2.0;
				mapView = false;
			} else if(unit && unit->owner == playerId) {
				std::vector<unitID> vec;
				model->getUnitList(vec);
				selection.clear();
				selection.insert(unit->getId());
				for(std::vector<unitID>::const_iterator ii=vec.begin(); ii!=vec.end(); ii++) {
					Model::Unit *u = model->getUnit(*ii);
					if(u->owner==unit->owner && u->type==unit->type && isOnScreen(u->nextX, u->nextY))
						selection.insert(*ii);
				}
			}
			break;
		
		default:
			break;
	}
	
	return true;
}

bool ScreenView::mouseUp(int x, int y, int button)
{
	mouseUpHud(x, y, button);
	mouseUpMinimap(x, y, button);
	
	if(dragging)
	{
		dragging = false;
		
		int left, right, top, bottom;
		left   = (int)min(mouseX, dragStartX);
		top    = (int)min(mouseY, dragStartY);
		right  = (int)max(mouseX, dragStartX);
		bottom = (int)max(mouseY, dragStartY);
		if(left<0) left=0;
		if(top<0) top=0;
		if(right>=model->getSizeX()) right=model->getSizeX()-1;
		if(bottom>=model->getSizeY()) bottom=model->getSizeY()-1;
		
		if(left==right && top==bottom && (model->getUnitAt(left,top,true)||model->getUnitAt(left,top,false)))
			return true;
		
		if(dragMode == 0)
			selection.clear();
		
		for(unsigned yi=top;  yi<=bottom; yi++)
		for(unsigned xi=left; xi<=right;  xi++)
		{
			Model::Unit *u = model->getUnitAt(xi, yi, true);
			if(u) {
				if(dragMode != -1)
					addToSelection(u->getId());
				else if(dragMode == -1)
					removeFromSelection(u->getId());
			}
			u = model->getUnitAt(xi, yi, false);
			if(u) {
				if(dragMode != -1) {
					addToSelection(u->getId());
					if(getSelectionSize()==1)
						playAcknowledge(u->getId());
				}
				else if(dragMode == -1)
					removeFromSelection(u->getId());
			}
		}
		return true;
	}
	return false;
}

bool ScreenView::mouseMotion(int x, int y, int xrel, int yrel)
{
	float mX=x, mY=y;
	mouseDragMinimap(mX, mY);
	this->screenMouseX = mX;
	this->screenMouseY = mY;
	mouseToWorld(&mX, &mY);
	this->mouseX = mX;
	this->mouseY = mY;
	
/*	ICoord mouseScroll = cursor->getScroll();
	cursor->clearScroll();
	if(mapView) {
		cameraX += (double)mouseScroll.x * edgeScrollSensitivity / mapViewTileSize;
		cameraY += (double)mouseScroll.y * edgeScrollSensitivity / mapViewTileSize;
	} else {
		cameraX += (double)mouseScroll.x * edgeScrollSensitivity / tileSize;
		cameraY += (double)mouseScroll.y * edgeScrollSensitivity / tileSize;
	}
	clipCamera();*/
	
	return false;
}

void ScreenView::mouseScroll(float dt)
{
	int dx=0, dy=0;
	ICoord scroll = cursor->getScroll();
	cursor->clearScroll();
	
	if     (screenMouseX <= scrollBorder               ) dx = -1;
	else if(screenMouseX >= screenWidth -1-scrollBorder) dx = 1;
	if     (screenMouseY <= scrollBorder               ) dy = -1;
	else if(screenMouseY >= screenHeight-1-scrollBorder) dy = 1;
	
	if(!mapView) {
		cameraX += dx*dt*edgeScrollSensitivity;
		cameraY += dy*dt*edgeScrollSensitivity;
	} else {
		mapCameraX += dx*dt*mapEdgeScrollSensitivity;
		mapCameraY += dy*dt*mapEdgeScrollSensitivity;
	}
}
