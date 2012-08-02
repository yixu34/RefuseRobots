#include "main.hpp"

const ICoord statusPos(229, 610), statusSize(493, 146);
const char *scrapyardMessage = NULL;
int scrapyardMessageCount = 0;

void ScreenView::drawStatus()
{
	if(selectedScrapyard >= 0) {
		drawScrapyardStatus();
		return;
	}
	scrapyardMessage = NULL;
	
	if(selection.size() > 1)
		drawSelectedUnits();
	else if(selection.size())
		drawUnitStatus();
	else
		drawGeneralStatus();
}

bool ScreenView::isStatusArea(int x, int y)
{
	return x>=statusPos.x &&
	       y>=statusPos.y &&
	       x<statusPos.x+statusSize.x &&
	       y<statusPos.y+statusSize.y;
}

bool ScreenView::mouseDownStatus(int x, int y, int button, int mod)
{
	if(selectedScrapyard >= 0)
		mouseDownScrapyardStatus(x, y, button, mod);
	else if(selection.size() > 1)
		return mouseDownSelectedUnits(x, y, button, mod);
	else if(selection.size())
		return mouseDownUnitStatus(x, y, button, mod);
	else
	{
	}
	return false;
}

const ICoord progressBarPos(260, 708),
             progressBarSize(250, 38);
const ICoord buildQueuePos(260, 653);
const int buildQueueSpacing = 50;
const int buildQueueIconSize = 48;
Image progressFull("status_full.png"),
      progressEmpty("status_empty.png");

/// Draw the status (current production, progress bar, queue) of the currently
/// selected scrapyard on the HUD.
// Precondition: There is a scrapyard selected.
void ScreenView::drawScrapyardStatus()
{
	const Model::Scrapyard *s = model->getScrapyard(selectedScrapyard);
	const UnitInfo *producingType = UnitInfo::getUnitType(s->currentProduction());
	
	for(unsigned ii=0; ii<s->buildQueue.size(); ii++)
	{
		const UnitInfo *unit = UnitInfo::getUnitType(s->buildQueue[ii]);
		
		// If this is the currently-active unit, highlight it and draw a line
		// connecting it to the progress bar
		if(ii==s->queuePos)
		{
			glLineWidth(3.0);
			glColor3ub(56, 145, 255);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBegin(GL_LINES);
			{
				float x = buildQueuePos.x + (float)buildQueueSpacing*((float)ii+0.5f);
				glVertex2f(x, buildQueuePos.y + buildQueueIconSize);
				glVertex2f(x, progressBarPos.y);
			}
			glEnd();
			glColor3ub(255, 255, 255);
		}
		else
		{
			glColor3ub(150, 150, 150);
		}
		
		drawImage(buildQueuePos.x + buildQueueSpacing*ii, buildQueuePos.y,
		          buildQueueIconSize, buildQueueIconSize, unit->productionIcon);
	}
	
	if(s->repeating)
		screenPrintf(buildQueuePos.x + buildQueueSpacing*s->buildQueue.size(), buildQueuePos.y, fontSmaller, "(repeating)");
	
	// Draw caption
	if(!scrapyardMessage) {
		if(rand()%6 || (scrapyardMessageCount++)<4)
			scrapyardMessage="Producing:";
		else if(rand()%2)
			scrapyardMessage="Assembling:";
		else switch(rand()%5)
		{
			case 0: scrapyardMessage="Wiping goop off of:"; break;
			case 1: scrapyardMessage="Disassembling:"; break;
			case 2: scrapyardMessage="From the ashes of toasters, we summon:"; break;
			case 3: scrapyardMessage="Reticulating splines for:"; break;
			case 4: scrapyardMessage="Intelligently Designing:"; break;
		}
	}
	screenPrintf(245, 623, fontDefault, scrapyardMessage);
	
	float percentDone = (getTime() - s->startTime) / producingType->getCost();
	
	// Draw progress bar
	glColor3ub(255,255,255);
	progressFull.bind();
	glBegin(GL_QUADS);
		glTexCoord2f(0,           0); glVertex2f(progressBarPos.x,                               progressBarPos.y);
		glTexCoord2f(percentDone, 0); glVertex2f(progressBarPos.x+progressBarSize.x*percentDone, progressBarPos.y);
		glTexCoord2f(percentDone, 1); glVertex2f(progressBarPos.x+progressBarSize.x*percentDone, progressBarPos.y+progressBarSize.y);
		glTexCoord2f(0,           1); glVertex2f(progressBarPos.x,                               progressBarPos.y+progressBarSize.y);
	glEnd();
	progressEmpty.bind();
	glBegin(GL_QUADS);
		glTexCoord2f(percentDone, 0); glVertex2f(progressBarPos.x+progressBarSize.x*percentDone, progressBarPos.y);
		glTexCoord2f(1,           0); glVertex2f(progressBarPos.x+progressBarSize.x,             progressBarPos.y);
		glTexCoord2f(1,           1); glVertex2f(progressBarPos.x+progressBarSize.x,             progressBarPos.y+progressBarSize.y);
		glTexCoord2f(percentDone, 1); glVertex2f(progressBarPos.x+progressBarSize.x*percentDone, progressBarPos.y+progressBarSize.y);
	glEnd();
}

// Click on the scrapyard status display
// (if you click on a unit in production or in the queue, it will be cancelled)
// Precondition: There is a scrapyard selected.
bool ScreenView::mouseDownScrapyardStatus(int x, int y, int button, int mod)
{
	const Model::Scrapyard *s = model->getScrapyard(selectedScrapyard);
	
	for(unsigned ii=0; ii<s->buildQueue.size(); ii++)
	{
		int iconX = buildQueuePos.x + buildQueueSpacing*ii,
		    iconY = buildQueuePos.y;
		
		if(x>=iconX && y>=iconY && x<iconX+buildQueueIconSize && y<iconY+buildQueueIconSize)
		{
			issueCommand(s->getId(), Command::cancelBuild, ii);
			break;
		}
	}
	return true;
}


const int selectDisplayNumRows = 2;
const ICoord selectDisplayPos(255, 650);
const ICoord selectDisplayCellSize(96, 48);

// Draw all of the units which are currently selected
// You can click on this to pick a selected group
void ScreenView::drawSelectedUnits()
{
	typedef std::map<const UnitInfo*, int> UnitInfoMap;
	UnitInfoMap selectedCounts;
	
	for(UnitSet::iterator ii=selection.begin(); ii!=selection.end(); ii++)
	{
		Model::Unit *unit = model->getUnit(*ii);
		if(!unit) continue;
		
		if(selectedCounts.find(unit->type) != selectedCounts.end())
			selectedCounts[unit->type]++;
		else
			selectedCounts[unit->type] = 1;
	}
	
	int column=0,
	    row=0;
	
	for(UnitInfoMap::iterator ii=selectedCounts.begin(); ii!=selectedCounts.end(); ii++)
	{
		int x = selectDisplayPos.x + selectDisplayCellSize.x*column,
		    y = selectDisplayPos.y + selectDisplayCellSize.y*row;
		Image icon = ii->first->wireframeIcon;
		glColor3ub(255,255,255);
		drawImage(x, y, 48, 48, icon);
		screenPrintf(x+48, y+5, fontDefault, "%i", (int)ii->second);
		
		row++;
		if(row >= selectDisplayNumRows)
			column++, row=0;
	}
}

bool ScreenView::mouseDownSelectedUnits(int x, int y, int button, int mod)
{
	typedef std::map<const UnitInfo*, int> UnitInfoMap;
	UnitInfoMap selectedCounts;
	
	for(UnitSet::iterator ii=selection.begin(); ii!=selection.end(); ii++)
	{
		Model::Unit *unit = model->getUnit(*ii);
		if(!unit) continue;
		
		if(selectedCounts.find(unit->type) != selectedCounts.end())
			selectedCounts[unit->type]++;
		else
			selectedCounts[unit->type] = 1;
	}
	
	int column=0,
	    row=0;
	
	for(UnitInfoMap::iterator ii=selectedCounts.begin(); ii!=selectedCounts.end(); ii++)
	{
		int iconx = selectDisplayPos.x + selectDisplayCellSize.x*column,
		    icony = selectDisplayPos.y + selectDisplayCellSize.y*row;
		
		if(x>=iconx && y>=icony && x<iconx+selectDisplayCellSize.x && y<icony+selectDisplayCellSize.y)
		{
			const UnitInfo *type = ii->first;
			for(UnitSet::iterator jj=selection.begin(); jj!=selection.end(); )
			{
				Model::Unit *sel = model->getUnit(*jj);
				if(!sel || sel->type != type)
					jj = selection.erase(jj);
				else
					jj++;
			}
			break;
		}
		
		row++;
		if(row >= selectDisplayNumRows)
			column++, row=0;
	}
	
	return true;
}


const int loadedDisplayRows = 2;
const ICoord loadedDisplayPos(500, 630);
const ICoord loadedDisplayCellSize(48, 48);

// Draw the status of the currently selected unit
// Precondition: There is currently only one unit selected
//
// For transports, the status shows what units are inside the transport
// (and you can click on those units to unload them).
// For everything else, it shows things like HP, range, damage, etc.
void ScreenView::drawUnitStatus()
{
	unitID selectedId = *selection.begin();
	Model::Unit *unit = model->getUnit(selectedId);
	
	if(unit->type->transports > 0)
	{
		// Draw loaded units
		int slots = unit->type->transports;
		int columns = (unit->type->transports + loadedDisplayRows-1) / loadedDisplayRows;
		int row=0, column=0;
		
		for(int ii=0; ii<slots; ii++)
		{
			Model::Unit *loaded = NULL;
			if((int)unit->loadedUnits.size() > ii)
			{
				unitID id = unit->loadedUnits[ii];
				loaded = model->getUnit(id);
			}
			int x = loadedDisplayPos.x + loadedDisplayCellSize.x*column,
				y = loadedDisplayPos.y + loadedDisplayCellSize.y*row;
			
			// Draw the unit in the slot
			if(loaded) {
				glColor3ub(255, 255, 255);
				drawImage(x, y, 48, 48, loaded->type->wireframeIcon);
			}
			
			// Draw the box marking the slot
			glColor3ub(100, 100, 100);
			glLineWidth(2.0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glBegin(GL_LINE_LOOP);
				glVertex2f(x,    y   );
				glVertex2f(x+48, y   );
				glVertex2f(x+48, y+48);
				glVertex2f(x,    y+48);
			glEnd();
			
			// Advance to the next position
			column++;
			if(column >= columns) {
				column = 0;
				row++;
			}
		}
	}
	
	glColor3ub(255, 255, 255);
	drawImage(245, 647, 64, 64, unit->type->wireframeIcon);
	screenPrintf(240, 628, fontDefault, "%s", unit->type->name.c_str());
	screenPrintf(250, 710, fontSmaller, "%i/%i hp", (int)unit->hp, (int)unit->type->maxHP);
	if(unit->type->usesFuel)
		screenPrintf(250, 722, fontSmaller, "%.0f%% fuel", 100 * ((float)unit->fuel / (float)unit->fuelMax));
}

// Click on the status of the currently selected unit
// Precondition: There is currently only one unit selected
//
// If the unit is a transport, you can click on loaded units
// to unload them. Otherwise, nothing happens.
bool ScreenView::mouseDownUnitStatus(int x, int y, int button, int mod)
{
	unitID selectedId = *selection.begin();
	Model::Unit *unit = model->getUnit(selectedId);
	
	if(unit->type->transports == 0)
		return false;
	
	int slots = unit->type->transports;
	int columns = (unit->type->transports + loadedDisplayRows-1) / loadedDisplayRows;
	int row=0, column=0;
	
	for(int ii=0; ii<slots; ii++)
	{
		Model::Unit *loaded = NULL;
		if((int)unit->loadedUnits.size() > ii)
		{
			unitID id = unit->loadedUnits[ii];
			loaded = model->getUnit(id);
			if(!loaded)
				return false;
		}
		int boxx = loadedDisplayPos.x + loadedDisplayCellSize.x*column,
			boxy = loadedDisplayPos.y + loadedDisplayCellSize.y*row;
		
		if(x>boxx && y>boxy && x<boxx+48 && y<boxy+48)
		{
			issueCommand(selectedId, Command::unloadSingle, unit->loadedUnits[ii], 0);
			return true;
		}
		
		// Advance to the next position
		column++;
		if(column >= columns) {
			column = 0;
			row++;
		}
	}
	return false;
}

void ScreenView::drawGeneralStatus()
{
#ifdef SHOW_DEBUG
	screenPrintf(650, 720, fontDefault, "%.1ffps", (float)getFramerate());
	
	if (network)
		drawNetworkStatus();
#endif
}


void ScreenView::drawNetworkStatus()
{
	// Figure out what node type you are.
	std::string selfNodeType;
	int numServers = 0;
	int numClients = 0;
	int numRemoteClients = 0;
	switch (network->getSelf()->getType())
	{
	case NetworkNode::NET_SERVER:
		selfNodeType = "Server";
		numServers = 1;
		break;
	case NetworkNode::NET_CLIENT:
		selfNodeType = "Client";
		numClients = 1;
		break;
	case NetworkNode::NET_REMOTE_CLIENT:
		selfNodeType = "Remote client";
		numRemoteClients = 1;
		break;
	}

	screenPrintf(250, 630, fontDefault, "Me = %s", selfNodeType.c_str());

	for (Network::NetworkNodePool::iterator it = network->networkNodes.begin();
		 it != network->networkNodes.end();
		 it++)
	{
		// Don't count yourself again.
		if (it->second == network->getSelf())
			continue;

		NetworkNode *currNode             = it->second;
		NetworkNode::NetNodeType currType = currNode->getType();
		if (currType == NetworkNode::NET_SERVER)
			numServers++;
		else if (currType == NetworkNode::NET_CLIENT)
			numClients++;
		else
			numRemoteClients++;
	}

	screenPrintf(250, 650, fontDefault, "Servers:  %d", numServers);
	screenPrintf(250, 670, fontDefault, "Clients:  %d", numClients);
	screenPrintf(250, 690, fontDefault, "Remote clients:  %d", numRemoteClients);
	
	int bytesSent, bytesRecv, packetsSent, packetsRecv;
	logTrafficStats(bytesSent, bytesRecv, packetsSent, packetsRecv);
	
	screenPrintf(400, 630, fontDefault, "Sent: %i bytes in %i packets",
		bytesSent, packetsSent);
	screenPrintf(400, 650, fontDefault, "Received: %i bytes in %i packets",
		bytesRecv, packetsRecv);
}
