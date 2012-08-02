#include "widgetactiontable.hpp"
#include "widgetaction.hpp"

WidgetActionTable *widgetActions = 0;

WidgetActionTable::WidgetActionTable()
{
}

WidgetActionTable::~WidgetActionTable()
{
	for (WidgetActionPool::iterator it = actions.begin(); it != actions.end(); it++)
		delete it->second;
}

void WidgetActionTable::addAction(
	int actionId, 
	WidgetAction *action)
{
	if (actions.find(actionId) == actions.end())
		actions.insert(std::make_pair(actionId, action));
}

void WidgetActionTable::removeAction(int actionId)
{
	if (actions.find(actionId) != actions.end())
		actions.erase(actionId);
}

WidgetAction *WidgetActionTable::getAction(int actionId)
{
	if (actions.find(actionId) != actions.end())
		return actions[actionId];
	else
		return 0;
}

void initWidgets()
{
	widgetActions = new WidgetActionTable();

	widgetActions->addAction(TestAction::actionId, new TestAction());
}