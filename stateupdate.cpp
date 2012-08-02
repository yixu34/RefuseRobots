#include "stateupdate.hpp"
#include "command.hpp"

StateUpdate::StateUpdate()
{
	type  = none;
	units = 0;
	x     = 0;
	y     = 0;
}

StateUpdate::StateUpdate(const Command &command)
{
	x = command.x;
	y = command.y;
	units = &command.units;

	switch (command.type)
	{
	case Command::teleport:
		type = position;
		break;

	default:
		type = none;
		break;
	}
}

StateUpdate::~StateUpdate()
{
}