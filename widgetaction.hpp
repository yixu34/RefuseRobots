#ifndef WIDGETACTION_HPP
#define WIDGETACTION_HPP

#include <map>
#include <vector>
#include <string>
#include "widget.hpp"
#include "tracker.hpp"

class Frame;
class TextInputWidget;
class TextDisplay;
class Widget;
class ScrollPane;

// Encapsulates an action that should be triggered when a 
// widget is activated.  (Keypress or mousedown.)
class WidgetAction
{
public:
	WidgetAction();
	virtual ~WidgetAction();

	void activate();

	void enqueueAction(WidgetAction *action);

protected:
	virtual void activateSelf() = 0;

	typedef std::vector<WidgetAction *> ActionSequence;
	ActionSequence nextActions;
};

//////////////////////////////////////////////////////////////////////////

class TestAction:public WidgetAction
{
public:
	TestAction();
	~TestAction();

private:
	void activateSelf();
};

//////////////////////////////////////////////////////////////////////////

class ChangeFrameAction:public WidgetAction
{
public:
	ChangeFrameAction(Frame *newFrame);
	~ChangeFrameAction();

private:
	void activateSelf();
	
	Frame *nextFrame;
};

//////////////////////////////////////////////////////////////////////////

class ExitGameAction:public WidgetAction
{
public:
	ExitGameAction();
	~ExitGameAction();

private:
	void activateSelf();
};

//////////////////////////////////////////////////////////////////////////

class StartSinglePlayerAction :public WidgetAction
{
public:
	StartSinglePlayerAction(ScrollPane<MapDesc> *mapselect);
	~StartSinglePlayerAction();
	
private:
	ScrollPane<MapDesc> *mapselect;
	void activateSelf();
};

//////////////////////////////////////////////////////////////////////////

class StartMultiPlayerAction:public WidgetAction
{
public:
	StartMultiPlayerAction(const std::string &name);
	~StartMultiPlayerAction();

private:
	void activateSelf();
	
	std::string playerName;
};

//////////////////////////////////////////////////////////////////////////

class ShowHostLobbyAction:public WidgetAction
{
public:
	ShowHostLobbyAction(ScrollPane<MapDesc> *mapselect, TextBox *gamename, TextBox *user);
	~ShowHostLobbyAction();

private:
	ScrollPane<MapDesc> *mapselect;
	void activateSelf();
	TextBox *name, *user;
};

//////////////////////////////////////////////////////////////////////////

class SendChatTextAction:public WidgetAction
{
public:
	SendChatTextAction(
		TextBox *input, 
		TextDisplay *display, 
		std::string *name);

	~SendChatTextAction();

private:
	void activateSelf();

	TextBox *textInput;
	TextDisplay *textDisplay;
	std::string *playerName;
};

//////////////////////////////////////////////////////////////////////////

class ConnectToServerAction :public WidgetAction
{
public:
	ConnectToServerAction(ScrollPane<ServerDesc> *serverlist, TextBox *playername);
	~ConnectToServerAction();
	
private:
	void activateSelf();
	ScrollPane<ServerDesc> *serverlist;
	TextBox *playername;
};

//////////////////////////////////////////////////////////////////////////

class ConnectToManualServerAction:public WidgetAction
{
public:
	ConnectToManualServerAction(TextInputWidget *input);
	~ConnectToManualServerAction();

private:
	void activateSelf();

	TextInputWidget *textInput;
};

//////////////////////////////////////////////////////////////////////////

class RefreshListAction :public WidgetAction
{
private:
	void activateSelf();
};

//////////////////////////////////////////////////////////////////////////

class LeaveLobbyAction :public WidgetAction
{
private:
	void activateSelf();
};

//////////////////////////////////////////////////////////////////////////

void setUsername(std::string name);

#endif