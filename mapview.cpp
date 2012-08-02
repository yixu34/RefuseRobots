#include "main.hpp"
#include <cmath>

struct { float r,g,b; } hotkeyColors[10] = {
		{ 1.0, 0.6, 0.6 },
		{ 0.6, 0.6, 1.0 },
		{ 0.7, 1.0, 0.7 },
		{ 0.5, 1.0, 1.0 },
		{ 1.0, 1.0, 0.4 },
		{ 1.0, 0.6, 1.0 },
		{ 0.1, 1.0, 0.1 },
		{ 1.0, 0.7, 0.1 },
		{ 0.5, 1.0, 0.0 },
		{ 0.7, 0.7, 0.0 },
	};

Image mapviewSelectedScrapyard("selection.png"),
      mapviewFriendlyScrapyard("mapview_scrapyard_friendly.png"),
      mapviewNeutralScrapyard("mapview_scrapyard_neutral.png"),
      mapviewHostileScrapyard("mapview_scrapyard_hostile.png");
//	  mapviewRallyPoint("explode1.png");

void ScreenView::drawMapView()
{
	typedef std::set<Model::Unit*> UnitList;
	UnitList units;
	const float pixel = 0.125;
	unsigned short **fog = model->getFog(playerId);
	
	int left = mapCameraX;
	int top  = mapCameraY;
	int right = left+(screenWidth/mapViewTileSize)+1;
	int bottom = top+(screenHeight/mapViewTileSize)+1;
	if(left<0) left=0;
	if(top<0) top=0;
	if(right  >= (int)model->getSizeX()) right  = model->getSizeX()-1;
	if(bottom >= (int)model->getSizeY()) bottom = model->getSizeY()-1;
	
	glPushMatrix();
	glScalef(mapViewTileSize, mapViewTileSize, 1.0);
	glTranslatef(-(int)mapCameraX, -(int)mapCameraY, 0);
	
	// Draw map tiles at less-than-full brightness, so that the map will
	// fade into the background and make unit symbols &c easier to see.
	
	// Draw all tiles top-to-bottom, taking note of relevant units along the way.
	Image prevImage;
	glBegin(GL_QUADS);
	for(int yi=top;  yi<=bottom; yi++)
	for(int xi=left; xi<=right;  xi++)
	{
		Model::Tile *tile = model->getTile(xi, yi);
		
		if(fog[yi][xi] > 0)
		{
			glColor3ub(170,170,170);
			Model::Unit *u = model->getUnitAt(xi, yi, true);
			if(u) units.insert(u);
			u = model->getUnitAt(xi, yi, false);
			if(u) units.insert(u);
		} else {
			glColor3ub(100,100,100);
		}
		//drawImage(xi, yi, 1.0, 1.0, tile->image());
		Image texture = tile->image();
		if(!(texture == prevImage)) {
			glEnd();
			texture.bind();
			glBegin(GL_QUADS);
		}
		glTexCoord2f(0.0, 0.0); glVertex2f(xi,      yi     );
		glTexCoord2f(1.0, 0.0); glVertex2f(xi+1.0f, yi     );
		glTexCoord2f(1.0, 1.0); glVertex2f(xi+1.0f, yi+1.0f);
		glTexCoord2f(0.0, 1.0); glVertex2f(xi,      yi+1.0f);
	}
	glEnd();
	
	// Draw scrapyards
	std::vector<Model::Scrapyard*> vec;
	model->getScrapyardList(vec);
	for(unsigned ii=0; ii<vec.size(); ii++) {
		const Model::Scrapyard *s = vec[ii];
		int topLeftX = s->center.x - s->size.x/2;
		int topLeftY = s->center.y - s->size.y/2;
		if(ii==selectedScrapyard) {
			glColor3ub(255,255,255);
			drawImage(topLeftX, topLeftY, s->size.x, s->size.y, mapviewSelectedScrapyard);
			
			/*int rallyX = s->rally.x - 2;
			int rallyY = s->rally.y - 2;
			drawImage(rallyX,rallyY,4,4, mapviewRallyPoint);*/

		} else if(s->owner == playerId) {
			glColor3ub(130,130,130);
			drawImage(topLeftX, topLeftY, s->size.x, s->size.y, mapviewFriendlyScrapyard);
		} else if(s->owner == 0) {
			glColor3ub(130,130,130);
			drawImage(topLeftX, topLeftY, s->size.x, s->size.y, mapviewNeutralScrapyard);
		} else {
			glColor3ub(130,130,130);
			drawImage(topLeftX, topLeftY, s->size.x, s->size.y, mapviewHostileScrapyard);
		}
		if(s->owner==playerId && !(s->rally == s->center)) {
			glColor3f(0.0, 0.7, 0.0);
			glLineWidth(1.0);
			glEnable(GL_LINE_SMOOTH);
			glBindTexture(GL_TEXTURE_2D, 0);
			float vertexX = s->rally.x - s->center.x,
			      vertexY = s->rally.y - s->center.y,
			      vertexLen = sqrt(vertexX*vertexX + vertexY*vertexY);
			float perpendicularX = -vertexY/vertexLen,
			      perpendicularY = vertexX/vertexLen;
			glBegin(GL_LINES);
				glVertex2f(s->center.x, s->center.y);
				glVertex2f(s->rally.x, s->rally.y);
				
				glVertex2f(s->rally.x, s->rally.y);
				glVertex2f(s->rally.x-vertexX/vertexLen+perpendicularX, s->rally.y-vertexY/vertexLen+perpendicularY);
				glVertex2f(s->rally.x, s->rally.y);
				glVertex2f(s->rally.x-vertexX/vertexLen-perpendicularX, s->rally.y-vertexY/vertexLen-perpendicularY);
			glEnd();
			glDisable(GL_LINE_SMOOTH);
		}
	}
	
	// Draw unit selection boxes
	glLineWidth(1.0);
	glColor3f(selectionBoxRed, selectionBoxGreen, selectionBoxBlue);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	glBegin(GL_LINES);
	for(UnitList::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = *ii;
		Model::Unit *border;
		float x = u->getX(),
			  y = u->getY();
		
		if(selection.find(u->getId())==selection.end())
			continue;
		
		x = pixel*std::floor(x*8.0); // Round (x,y) to the nearest pixel
		y = pixel*std::floor(y*8.0);
		
		// The goal here is to draw a bounding shape around groups of selected
		// units; that is, something like
		//  +----+
		//  |UUUU|
		//  |U|U++
		//  +-+-+
		// where the line is one pixel outside the unit's boundaries.
		//
		// For moving units, this is too complex to deal with; un that case,
		// simply draw a border all around every unit.
		//
		// For stationary units, look at each of the unit's borders. If there
		// is no unit on that border, that unit is moving, or that unit is
		// not selected, a line needs to be drawn on that border.
		
		int left, right, top, bottom;
		
		if(u->moving)
		{
			left = right = top = bottom = 1;
		} else
		{
			border = model->getUnitAt((unsigned)x, (unsigned)(y-0.5), u->type->flying);
			top = !border || border->moving || selection.find(border->getId())==selection.end();
			
			border = model->getUnitAt((unsigned)(x+1.5), (unsigned)y, u->type->flying);
			right = !border || border->moving || selection.find(border->getId())==selection.end();
			
			border = model->getUnitAt((unsigned)x, (unsigned)(y+1.5), u->type->flying);
			bottom = !border || border->moving || selection.find(border->getId())==selection.end();
			
			border = model->getUnitAt((unsigned)(x-0.5), (unsigned)y, u->type->flying);
			left = !border || border->moving || selection.find(border->getId())==selection.end();
		}
		
		// Corner pixels complicate things. If the rule was 'a corner pixel is
		// drawn if either of the adjoining edges is drawn' (as would happen if
		// we simply handled each of the four edges independently), then this
		// would happen:
		//   --------
		//   |UUUuuu|
		//   |UUUuuu|
		//   |UUU|uu|
		//   |uu-----
		//   |uuu|
		//   |uuu|
		//   -----
		// The correct answer is to include a corner pixel iff _both_ the
		// adjoining edges are drawn. And to do this, the most efficient
		// solution turns out to be to just permute every possible
		// combination of edges to draw, and write separate code for each.
		// (switch is O(1), so there is no performance penalty for this.)
		int mode = (top) | (right<<1) | (bottom<<2) | (left<<3);
		
		switch(mode)
		{
			case 0: // none
				break;
				
			case 1: // top
				glVertex2f(x,     y); //top
				glVertex2f(x+1.0, y);
				break;
				
			case 2: // right
				glVertex2f(x+1.0, y    ); //right
				glVertex2f(x+1.0, y+1.0);
				break;
				
			case 3: // right, top
				glVertex2f(x+1.0, y    ); //right
				glVertex2f(x+1.0, y+1.0);
				
				glVertex2f(x,           y); //top
				glVertex2f(x+1.0+pixel, y);
				break;
				
			case 4: // bottom
				glVertex2f(x+1.0, y+1.0+pixel); //bottom
				glVertex2f(x,     y+1.0+pixel);
				break;
				
			case 5: // bottom, top
				glVertex2f(x,     y); //top
				glVertex2f(x+1.0, y);
				
				glVertex2f(x+1.0, y+1.0+pixel); //bottom
				glVertex2f(x,     y+1.0+pixel);
				break;
				
			case 6: // bottom, right
				glVertex2f(x+1.0+pixel, y+1.0+pixel); //bottom
				glVertex2f(x,           y+1.0+pixel);
			
				glVertex2f(x+1.0, y    ); //right
				glVertex2f(x+1.0, y+1.0);
				break;
				
			case 7: // bottom, right, top
				glVertex2f(x+1.0+pixel, y+1.0+pixel); //bottom
				glVertex2f(x,           y+1.0+pixel);
			
				glVertex2f(x+1.0, y-pixel); //right
				glVertex2f(x+1.0, y+1.0);
				
				glVertex2f(x,           y); //top
				glVertex2f(x+1.0+pixel, y);
				break;
				
			case 8: // left
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				break;
				
			case 9: // left, top
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				glVertex2f(x-pixel, y); //top
				glVertex2f(x+1.0,   y);
				break;
				
			case 10: // left, right
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0, y); //right
				glVertex2f(x+1.0, y+1.0);
				break;
				
			case 11: // left, right, top
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0, y-pixel); //right
				glVertex2f(x+1.0, y+1.0);
				
				glVertex2f(x-pixel,     y); //top
				glVertex2f(x+1.0+pixel, y);
				break;
				
			case 12: // left, bottom
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0,    y+1.0+pixel); //bottom
				glVertex2f(x-pixel,  y+1.0+pixel);
				break;
				
			case 13: // left, bottom, top
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0,   y+1.0+pixel); //bottom
				glVertex2f(x-pixel, y+1.0+pixel);
			
				glVertex2f(x-pixel, y); //top
				glVertex2f(x+1.0,   y);
				break;
				
			case 14: // left, bottom, right
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0+pixel, y+1.0+pixel); //bottom
				glVertex2f(x-pixel,     y+1.0+pixel);
			
				glVertex2f(x+1.0, y); //right
				glVertex2f(x+1.0, y+1.0);
				break;
				
			case 15: // left, bottom, right, top
				glVertex2f(x-pixel, y    ); //left
				glVertex2f(x-pixel, y+1.0);
				
				glVertex2f(x+1.0+pixel, y+1.0+pixel); //bottom
				glVertex2f(x-pixel,     y+1.0+pixel);
			
				glVertex2f(x+1.0, y-pixel); //right
				glVertex2f(x+1.0, y+1.0);
				
				glVertex2f(x-pixel,     y); //top
				glVertex2f(x+1.0+pixel, y);
				break;
		}
	}
	glEnd();
	
	bool blinkOff = ((int)(getTime()*3.0) % 2) ? true : false;
	// Draw units
	for(UnitList::iterator ii=units.begin(); ii!=units.end(); ii++)
	{
		Model::Unit *u = *ii;
		float x = u->getX(),
			  y = u->getY();
		x = pixel*std::floor(x*8.0); // Round (x,y) to the nearest pixel
		y = pixel*std::floor(y*8.0);
		
		if(showUnitStatus) {
			float fraction = (float)u->hp / u->type->maxHP;
			if(fraction > 0.5)
				glColor3f((1.0-fraction)*2, 1.0, 0.0);
			else
				glColor3f(1.0, fraction*2, 0.0);
		}
		else if(u->owner != playerId)
			glColor3f(1.0, 0.2, 0.2);
		else {
			glColor3f(1.0, 1.0, 1.0);
			
			// Check hotkeys
			for(unsigned ii=0; ii<10; ii++) {
				if(hotkeys[ii].find(u->getId()) != hotkeys[ii].end()) {
					glColor3f(hotkeyColors[ii].r, hotkeyColors[ii].g, hotkeyColors[ii].b);
					break;
				}
			}
			
			// Check fuel
			if(u->fuel == 0 && u->type->usesFuel && blinkOff)
				glColor3f(0.0, 0.0, 0.0);
		}
		
		drawImage(x, y, 1.0, 1.0, u->mapViewImage());
	}
	
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

