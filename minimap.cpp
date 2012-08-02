#include "main.hpp"

const int minimapX=13,
          minimapY=582;
const int minimapWidth=188,
          minimapHeight=176;

void ScreenView::generateMinimap()
{
	unsigned sizeX = model->getSizeX(),
	         sizeY = model->getSizeY();
	
	SDL_Surface *minimap = SDL_CreateRGBSurface(
		SDL_SWSURFACE, sizeX, sizeY, 32,
		textureFormat.Rmask, textureFormat.Gmask, textureFormat.Bmask, textureFormat.Amask);
	unsigned char *pos = (unsigned char*)minimap->pixels;
	
	for(unsigned yi=0; yi<sizeY; yi++)
	for(unsigned xi=0; xi<sizeX; xi++)
	{
		Color c = model->getTile(xi,yi)->image().averageColor();
		pos[0] = c.r;
		pos[1] = c.g;
		pos[2] = c.b;
		pos[3] = 255;
		pos += 4;
	}
	
	glGenTextures(1, &minimapTexID);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, minimapTexID);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	gluBuild2DMipmaps(GL_TEXTURE_2D,      //type of texture
	                  4,                  //channels
	                  sizeX, sizeY,       //dimensions
	                  GL_RGBA,            //format
	                  GL_UNSIGNED_BYTE,   //channel type
	                  minimap->pixels);   //data
	
	SDL_FreeSurface(minimap);
}


void ScreenView::drawMinimap()
{
	// Draw terrain
	glBindTexture(GL_TEXTURE_2D, minimapTexID);
	glColor3f(1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(minimapX,              minimapY              );
		glTexCoord2f(1.0, 0.0); glVertex2f(minimapX+minimapWidth, minimapY              );
		glTexCoord2f(1.0, 1.0); glVertex2f(minimapX+minimapWidth, minimapY+minimapHeight);
		glTexCoord2f(0.0, 1.0); glVertex2f(minimapX,              minimapY+minimapHeight);
	glEnd();
}

void ScreenView::drawMinimapBox()
{
	// Draw view rectangle
	float boxTop, boxLeft;
	float boxWidth, boxHeight;
	
	if(mapView) {
		boxLeft = minimapX + (float)minimapWidth  * mapCameraX/model->getSizeX();
		boxTop  = minimapY + (float)minimapHeight * mapCameraY/model->getSizeY(),
		boxWidth = (float)minimapWidth  * (float)screenWidth  / (float)mapViewTileSize / (float)model->getSizeX(),
		boxHeight = (float)minimapHeight * (float)screenMainHeight / (float)mapViewTileSize / (float)model->getSizeY();
	} else {
		boxTop  = minimapY + (float)minimapHeight * cameraY/model->getSizeY(),
		boxLeft = minimapX + (float)minimapWidth  * cameraX/model->getSizeX();
		boxWidth = (float)minimapWidth  * (float)screenWidth  / (float)tileSize / (float)model->getSizeX(),
		boxHeight = (float)minimapHeight * (float)screenMainHeight / (float)tileSize / (float)model->getSizeY();
	}
	if(boxWidth  > minimapWidth)  boxWidth  = minimapWidth;
	if(boxHeight > minimapHeight) boxHeight = minimapHeight;
	
	float boxBottom = boxTop + boxHeight,
	      boxRight = boxLeft + boxWidth;
	
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor3f(0.0, 1.0, 0.0);
	glLineWidth(1.0);
	glBegin(GL_LINE_LOOP);
		glVertex2f(boxLeft,  boxTop);
		glVertex2f(boxRight, boxTop);
		glVertex2f(boxRight, boxBottom);
		glVertex2f(boxLeft,  boxBottom);
	glEnd();
}


bool ScreenView::isMinimapArea(int x, int y)
{
	return x>=minimapX && y>=minimapY && x<(minimapX+minimapWidth) && y<(minimapY+minimapHeight);
}

bool ScreenView::mouseDownMinimap(int x, int y, int button, int mod)
{
	if(button == SDL_BUTTON_LEFT)
	{
		draggingMinimap = true;
		mouseDragMinimap(x, y);
	}
	else if(button == SDL_BUTTON_RIGHT)
	{
		float w_x=(x-minimapX)*((double)model->getSizeX()/minimapWidth),
		      w_y=(y-minimapY)*((double)model->getSizeY()/minimapWidth);
		
		if(mod & KMOD_SHIFT) {
			issueCommand(selection, Command::attack, w_x, w_y);
		} else if(mod & KMOD_CTRL) {
			mapView = !mapView;
			centerCameraAt(w_x, w_y);
		} else {
			issueCommand(selection, Command::move, w_x, w_y);
		}
	}
	return true;
}


void ScreenView::mouseDragMinimap(int x, int y)
{
	if(draggingMinimap) {
		centerCameraAt( (x-minimapX)*((double)model->getSizeX()/minimapWidth),
						(y-minimapY)*((double)model->getSizeY()/minimapWidth) );
		clipCamera();
	}
}

void ScreenView::mouseUpMinimap(int x, int y, int button)
{
	draggingMinimap = false;
}
