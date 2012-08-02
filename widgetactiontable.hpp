#ifndef WIDGETACTIONTABLE
#define WIDGETACTIONTABLE

#include <map>

class WidgetAction;

class WidgetActionTable
{
public:
	WidgetActionTable();
	~WidgetActionTable();

	void addAction(int actionId, WidgetAction *action);
	void removeAction(int actionId);
	WidgetAction *getAction(int actionId);

private:
	typedef std::map<int, WidgetAction *> WidgetActionPool;
	WidgetActionPool actions;
};

extern WidgetActionTable *widgetActions;

void initWidgets();

#endif