#include "main.hpp"

CommandCard::CommandCard()
{
	for(unsigned ii=0; ii<height; ii++)
	for(unsigned jj=0; jj<width;  jj++)
		buttons[ii][jj] = NULL;
}
void CommandCard::addButton(CommandButton *button)
{
	int xpos=button->preferredX,
	    ypos=button->preferredY;
	
	while(buttons[ypos][xpos] != NULL) {
		xpos++;
		if(xpos>=width) {
			xpos=0;
			ypos++;
			if(ypos>=height) ypos=0;
		}
		if(xpos==button->preferredX && ypos==button->preferredY)
			break;
	}
	
	buttons[ypos][xpos] = button;
}
CommandButton *CommandCard::getButton(int x, int y)
{
	return buttons[y][x];
}


CommandButton::CommandButton(Image image, char hotkey, unsigned x, unsigned y,
	            Command::CommandType cmd, Command::CommandType unitcmd, std::string tooltip)
	: image(image), hotkey(hotkey), preferredX(x), preferredY(y), command(cmd),
	  unitCommand(unitcmd), targetted(true), isProduction(false), tooltip(tooltip)
{
}
CommandButton::CommandButton(Image image, char hotkey, unsigned x, unsigned y,
	            Command::CommandType cmd, std::string tooltip)
	: image(image), hotkey(hotkey), preferredX(x), preferredY(y), command(cmd),
	  unitCommand(Command::none), targetted(false), isProduction(false), tooltip(tooltip)
{
}
CommandButton::CommandButton(Image image, char hotkey, unsigned x, unsigned y, std::string unit, std::string tooltip)
	: image(image), hotkey(hotkey), preferredX(x), preferredY(y), command(Command::buildUnit),
	  unitCommand(Command::none), targetted(false), unitType(unit), isProduction(true), tooltip(tooltip)
{
}




CommandButton commandMove   (Image("buttons/button_move.png"),   'm', 1, 0,  Command::move, Command::follow,       "(M) Move");
CommandButton commandAttack (Image("buttons/button_attack.png"), 'a', 0, 0,  Command::attackMove, Command::attack,  "(A) Attack");

CommandButton commandStop   (Image("buttons/button_cancel.png"), 's', 2, 0,  Command::stop,    "(S) Stop");
CommandButton commandUnload (Image("buttons/button_unload.png"), 'u', 0, 1,  Command::unload,  "(U) Unload");
CommandButton commandRetreat(Image("buttons/button_retreat.png"),   'r', 3, 0,  Command::retreat, "(R) Retreat");

void CommandCard::addButtonName(const std::string &name)
{
	if(name=="attack")
		addButton(&commandAttack);
	else if(name=="move")
		addButton(&commandMove);
	else if(name=="stop")
		addButton(&commandStop);
	else if(name=="retreat")
		addButton(&commandRetreat);
	else if(name=="unload_all")
		addButton(&commandUnload);
}


CommandCard scrapyardCommands;
CommandCard blankCommandCard;

CommandButton buildRepeat(Image("buttons/button_repeat.png"), 'r', 2, 2, "repeat", "(R) Repeat production");
CommandButton buildRally(Image("buttons/button_rally.png"), 'y', 3, 2, Command::setRally, "(Y) Set rally point");

void initCommandCard()
{
	Parser unitListFile("units/unitlist.dat");
	const Parser::Section *units = unitListFile.getSection("units");
	
	for(Parser::Section::const_iterator ii=units->begin(); ii!=units->end(); ii++)
	{
		const Parser::Line &L = *ii;
		const UnitInfo *type = UnitInfo::getUnitType(L[0]);
		
		CommandButton *button = new CommandButton(type->productionIcon, type->hotkey, type->hudX, type->hudY, L[0],
			retprintf("(%c) Build %s", toupper(type->hotkey), type->name.c_str()));
		scrapyardCommands.addButton(button);
	}
	scrapyardCommands.addButton(&buildRepeat);
	scrapyardCommands.addButton(&buildRally);
}

