#include "main.hpp"

Image hudImage("hud.png");

const int numHudButtons = 12;
const FCoord hudButtonSize(64, 64);
const FCoord hudButtonPositions[3][4] = {
		{ FCoord(749, 563), FCoord(816, 563), FCoord(883, 563), FCoord(949, 563), },
		{ FCoord(749, 629), FCoord(816, 629), FCoord(883, 629), FCoord(949, 629), },
		{ FCoord(749, 695), FCoord(816, 695), FCoord(883, 695), FCoord(949, 695), },
	};

// Draw the HUD, and all the things on it (minimap, buttons, status area)
void ScreenView::drawHUD()
{
	glColor3ub(255, 255, 255); drawMinimap();
	glColor3ub(255, 255, 255); drawImage(0, screenHeight-256, screenWidth, 256, hudImage);
	drawMinimapBox();
	
	messages->redraw();
	
	drawStatus();
	drawCommandCard();
	
	if(engine && engine->controller && engine->controller->isWaiting()) {
		screenPrintf(200, 300, fontBigger, "Waiting for %s", engine->controller->waitingFor().c_str());
	}
}

void ScreenView::drawCommandCard()
{
	CommandCard *card = getCommandCard();
	CommandButton *button;
	
	for(int ii=0; ii<CommandCard::height; ii++)
	for(int jj=0; jj<CommandCard::width; jj++)
	{
		int index = jj + ii*CommandCard::width;
		
		button = NULL;
		if(card)
			button = card->getButton(jj, ii);
		
		if(button)
		{
			if(ii==hudButtonPressedY && jj==hudButtonPressedX)
				glColor3ub(130,130,130);
			else
				glColor3ub(255,255,255);
			
			drawImage(hudButtonPositions[ii][jj].x, hudButtonPositions[ii][jj].y, hudButtonSize.x, hudButtonSize.y, button->image);
		}
		else
		{
			glColor3ub(255,255,255);
			drawImage(hudButtonPositions[ii][jj].x, hudButtonPositions[ii][jj].y, hudButtonSize.x, hudButtonSize.y, Image("buttons/button_blank.png"));
		}
	}
	
	// "The shiny elbow drank the automobile. Write that down."
	//     -- Yi Xu
	
	for(int ii=0; ii<CommandCard::height; ii++)
	for(int jj=0; jj<CommandCard::width; jj++)
	{
		if( screenMouseX > hudButtonPositions[ii][jj].x
		 && screenMouseX < hudButtonPositions[ii][jj].x+hudButtonSize.x
		 && screenMouseY > hudButtonPositions[ii][jj].y
		 && screenMouseY < hudButtonPositions[ii][jj].y+hudButtonSize.y)
		{
			button = card->getButton(jj, ii);
			if(!button)
				break;
			
			drawTooltip(screenMouseX, screenMouseY, button->tooltip.c_str());
			
			break;
		}
	}

}


void ScreenView::drawTooltip(float x, float y, const char *str)
{
	int tooltipWidth, tooltipHeight;
	TTF_SizeText(fontDefault.font, str, &tooltipWidth, &tooltipHeight);
	
	float left = x-tooltipWidth,
	      top = y-tooltipHeight,
	      right=left+tooltipWidth,
	      bottom=top+tooltipHeight;
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0.3,0.3,0.3);
	glBegin(GL_QUADS);
		glVertex2f(left,  top);
		glVertex2f(right, top);
		glVertex2f(right, bottom);
		glVertex2f(left,  bottom);
	glEnd();
	
	glLineWidth(1.0);
	glColor3f(0,0,0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(left,  top);
		glVertex2f(right, top);
		glVertex2f(right, bottom);
		glVertex2f(left,  bottom);
	glEnd();
	
	screenPrintf(left, top, fontDefault, "%s", str);
}


CommandCard *ScreenView::getCommandCard()
{
	if(selection.size()) {
		Model::Unit *u = model->getUnit(*(selection.begin()));
		return u->type->commandCard;
	} else if(selectedScrapyard >= 0) {
		return &scrapyardCommands;
	} else {
		return &blankCommandCard;
	}	
}


bool ScreenView::isHudArea(int x, int y)
{
	if(y>560 && x>740)      return true;
	else if(y>560 && x<210) return true;
	else if(y>610)          return true;
	else return false;
}

bool ScreenView::mouseDownHud(int x, int y, int button, int mod)
{
	CommandCard *card = getCommandCard();
	CommandButton *hudbutton = NULL;
	
	for(int ii=0; ii<CommandCard::height; ii++)
	for(int jj=0; jj<CommandCard::width; jj++)
	{
		if( x > hudButtonPositions[ii][jj].x
		 && x < hudButtonPositions[ii][jj].x+hudButtonSize.x
		 && y > hudButtonPositions[ii][jj].y
		 && y < hudButtonPositions[ii][jj].y+hudButtonSize.y)
		{
			hudButtonPressedX = jj;
			hudButtonPressedY = ii;
			
			hudbutton = card->getButton(jj, ii);
			if(hudbutton)
				pressCommandButton(hudbutton);
			
			break;
		}
	}
	return true;
}

bool ScreenView::pressHotkey(SDLKey key)
{
	CommandCard *card = getCommandCard();
	CommandButton *button = NULL;
	
	for(int ii=0; ii<CommandCard::height; ii++)
	for(int jj=0; jj<CommandCard::width; jj++)
	{
		button = card->getButton(jj, ii);
		if(button && button->hotkey == key) {
			pressCommandButton(button);
			return true;
		}
	}
	return false;
}

void ScreenView::pressCommandButton(CommandButton *button)
{
	if(button->command == Command::retreat && selection.size()) {
		playRetreat(*selection.begin());
	}
	
	if(button->targetted) {
		pendingCommand = button->command;
		pendingUnitCommand = button->unitCommand;
	} else if(button->command == Command::setRally) {
		pendingCommand = button->command;
	} else if(button->isProduction) {
		issueCommand(selectedScrapyard, button->command, button->unitType);
	} else {
		issueCommand(selection, button->command, 0, 0);
	}
}

void ScreenView::mouseUpHud(int x, int y, int button)
{
	hudButtonPressedX = -1;
	hudButtonPressedY = -1;
}

