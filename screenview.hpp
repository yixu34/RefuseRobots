#ifndef SCREENVIEW_HPP
#define SCREENVIEW_HPP

#include "view.hpp"
#include "event.hpp"
#include "text.hpp"
#include "sound.hpp"

const unsigned int tileSize = 48;
const unsigned int mapViewTileSize = 8;

extern const float selectionBoxRed, selectionBoxGreen, selectionBoxBlue;

class TextDisplay;
class TextInput;

void printChatText(const std::string &str, playerID id);


class ScreenView :public EventListener, public View
{
public:
	ScreenView(Model *model, CommandQueue *commandQueue, playerID playerId);
	~ScreenView();
	
	void redraw();   // Draw the model, UI, etc on the screen
	void timepass(float dt);

	void receiveChat(const std::string &str, playerID id);
	
	void playSoundAt(int x, int y, Sound *s);

	// User input
	bool keyDown(SDL_keysym sym);
	bool keyUp(SDL_keysym sym);
	bool mouseDown(int x, int y, int button, int mod);
	bool mouseUp(int x, int y, int button);
	bool mouseMotion(int x, int y, int xrel, int yrel);

	int getSelectionSize();
	
	void onSetPlayerId(playerID id);
	
protected:
	bool mouseDownWorld(int x, int y, int button, int mod);
	bool mouseDownHud(int x, int y, int button, int mod);
	bool mouseDownMinimap(int x, int y, int button, int mod);
	bool mouseDownStatus(int x, int y, int button, int mod);
	bool mouseDownUnitStatus(int x, int y, int button, int mod);
	bool mouseDownScrapyardStatus(int x, int y, int button, int mod);
	bool mouseDownSelectedUnits(int x, int y, int button, int mod);
	void mouseUpHud(int x, int y, int button);
	void mouseUpMinimap(int x, int y, int button);
	void mouseDragMinimap(int x, int y);
	void mouseScroll(float dt);
	CommandCard *getCommandCard();
	void pressCommandButton(CommandButton *button);
	bool isOnScreen(int x, int y);
	
	// Drawing
	void drawHUD();
	void drawTooltip(float x, float y, const char *str);
	void drawMain();
	void drawUnit(Model::Unit *unit);
	void drawProjectile(const Model::Projectile *p);
	void drawMapView();
	void drawMinimap();
	void drawMinimapBox();
	void drawStatus();
	void drawCommandCard();
	void drawScrapyardStatus();
	void drawSelectedUnits();
	void drawUnitStatus();
	void drawGeneralStatus();
	void drawEffects();

	// Debug
	void drawNetworkStatus();
	
	// Utility
	void mouseToWorld(float *x, float *y);
	void cullSelected();
	bool pressHotkey(SDLKey key);
	void generateMinimap();
	
	// Selections and hotkeys
	typedef std::set<unitID> UnitSet;
	UnitSet selection;
	UnitSet hotkeys[10];
	int scrapyardHotkeys[10];
	int hotkeysHeld;
	int selectedScrapyard;
	double nextSpeechTime;
	
	// Update the selection, but restrict to units that match this player's id.
	void addToSelection(unitID id);
	void addToSelection(UnitSet::iterator unitsBegin, UnitSet::iterator unitsEnd);
	void removeFromSelection(unitID id);
	
	void playMove(unitID id);
	void playAcknowledge(unitID id);
	void playRetreat(unitID id);
	void playAttack(unitID id);
	void unitSpeech(Sound *sound);
	
	bool isHudArea(int x, int y);
	bool isMinimapArea(int x, int y);
	bool isStatusArea(int x, int y);
	void writeChat(const std::string &str);
	
	void clipCamera();
	void centerCameraAt(float x, float y);
	
	bool mapView;
	bool showUnitStatus;
	
	// Camera position and movement
	float cameraX, cameraY;
	float mapCameraX, mapCameraY;
	bool arrowHeldLeft, arrowHeldRight, arrowHeldUp, arrowHeldDown;
	float mapViewTransition;
	
	// Mouse tracking and dragging
	float dragStartX, dragStartY; // In world coordinates
	float mouseX, mouseY; // In world coordinates
	float screenMouseX, screenMouseY;
	bool dragging;
	double doubleClickTime;
	int dragMode; // -1 remove, 0 normal, 1 add
	Command::CommandType pendingCommand, pendingUnitCommand;
	
	// HUD state
	int hudButtonPressedX, hudButtonPressedY;  // Location of the currently pressed button, or -1 for none
	TextDisplay *messages;
	TextInput *input;
	
	// Minimap
	unsigned minimapTexID;
	bool draggingMinimap;
};

const int buttonDoubleClick = -1;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class TextDisplay : public EventListener
{
public:
	TextDisplay(float x, float y, float width, float height, TextParams textParams, bool alignToBottom, bool startEnabled = true);
	void redraw();
	bool isOpaque();
	void timepass(float dt);
	
	void setTimeout(float timeout);
	void println(std::string line);
	void clear();
	
protected:
	float timeout;
	float x, y, width, height;
	bool alignToBottom;
	typedef std::deque< std::pair<float,std::string> > LineList; //(expiration,string) pairs
	LineList lines;
	TextParams textParams;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

class TextInput :public EventListener
{
public:
	TextInput(float x, float y, TextParams fontToUse, bool startEnabled = true);
	bool keyDown(SDL_keysym sym);
	bool keyUp(SDL_keysym sym);
	void redraw();
	bool isOpaque();
	//void setFont(TextParams fontToUse) {fontName= fontToUse;)
	std::string getString();
	int getResult(); // 0=still inputting; 1=OK; -1=cancelled
	void reset();

protected:
	float x, y;
	int cursorPos;
	std::string str;
	int result;
	TextParams fontName;
};

#endif
