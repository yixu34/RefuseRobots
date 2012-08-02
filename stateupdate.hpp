#ifndef STATEUPDATE_HPP
#define STATEUPDATE_HPP

#include "model.hpp"
#include <vector>

class Command;

// Objects sent from a controller to the model, telling the model to 
// update its state.  (Move some units, blah blah blah)
class StateUpdate
{
public:
	StateUpdate();
	StateUpdate(const Command &command);
	~StateUpdate();

	enum UpdateType
	{
		none,
		position,       
	};

	// Might need to subclass if the parameters of the updates differ a lot
	const std::vector<unitID> *units;
	UpdateType type;
	float x;
	float y;
};

#endif
