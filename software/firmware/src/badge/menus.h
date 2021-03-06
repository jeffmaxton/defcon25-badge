#ifndef BADGE_MENUS_H
#define BADGE_MENUS_H

#include "gui.h"
#include "KeyStore.h"
#include "Keyboard.h"

class StateBase;
class DisplayST7735;
class ContactStore;

struct ReturnStateContext {
	ReturnStateContext(StateBase *next, const ErrorType &er) :
			NextMenuToRun(next), Err(er) {
	}
	ReturnStateContext(StateBase *n) :
			NextMenuToRun(n), Err() {
	}
	StateBase *NextMenuToRun;
	ErrorType Err;
};

class RFM69;
class LedDC25;

class RunContext {
public:
	RunContext(DisplayST7735 *display, QKeyboard *kb, ContactStore *cs, RFM69 *r, LedDC25 *ledControl);
	DisplayST7735 &getDisplay();
	const GUI &getGUI();
	QKeyboard &getKB();
	ContactStore &getContactStore();
	RFM69 &getRadio();
	LedDC25 &getLedControl();
private:
	DisplayST7735 *dp; //should just be DisplayDevice rather than specific display //TODO
	GUI GuiDisplay;
	QKeyboard *KeyB;
	ContactStore *CS;
	RFM69 *Transciever;
	LedDC25 *LedControl;
};

class StateBase {
public:
	StateBase();
	ReturnStateContext run(RunContext &rc);
	uint32_t timeInState();
	ErrorType shutdown();
	virtual ~StateBase();
protected:
	static const uint32_t INIT_BIT = 0x01;
	static const uint32_t DONT_RESET = 0x02;
	static const uint32_t SHIFT_FROM_BASE = 8;
	virtual ErrorType onInit(RunContext &rc)=0;
	virtual ReturnStateContext onRun(RunContext &rc)=0;
	virtual ErrorType onShutdown()=0;
	void setState(uint32_t n) {
		StateData |= n;
	}
	void clearState(uint32_t n) {
		StateData = (StateData & ~n);
	}
	bool checkState(uint32_t n) {
		return (StateData & n) !=0;
	}
	bool hasBeenInitialized() {
		return (StateData & INIT_BIT) != 0;
	}
	bool shouldReset() {
		return (StateData & DONT_RESET) == 0;
	}
	uint32_t getTimesRunCalledAllTime() { return TimesRunCalledAllTime;}
	uint32_t getTimesRunCalledSinceLastReset() {return TimesRunCalledSinceLastReset;}
private:
	ErrorType init(RunContext &rc);
private:
	uint32_t StateData : 8;
	uint32_t TimesRunCalledAllTime : 24;
	uint32_t TimesRunCalledSinceLastReset;
	uint32_t StateStartTime;
};

class DisplayMessageState: public StateBase {
public:
	DisplayMessageState(uint16_t timeInState, StateBase *nextState);
	virtual ~DisplayMessageState();
	void setMessage(const char *msg);
	void setTimeInState(uint16_t t) {
		TimeInState = t;
	}
	void setNextState(StateBase *b) {
		NextState = b;
	}
	StateBase *getNextState() {
		return NextState;
	}
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
private:
	char Message[64];
	uint16_t TimeInState;
	StateBase *NextState;
};

class MenuState: public StateBase {
public:
	MenuState();
	virtual ~MenuState();
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
private:
	GUI_ListData MenuList;
#ifdef GATEWAY
	GUI_ListItemData Items[11];
#else
	GUI_ListItemData Items[10];
#endif
};

class SettingState: public StateBase {
public:
	SettingState();
	virtual ~SettingState();
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
private:
	GUI_ListData SettingList;
	GUI_ListItemData Items[3];
	char AgentName[ContactStore::AGENT_NAME_LENGTH];
	uint8_t InputPos;
	uint8_t SubState;
};

class BadgeInfoState: public StateBase {
public:
	BadgeInfoState();
	virtual ~BadgeInfoState();
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
	const char *getRegCode(ContactStore &cs);
private:
	GUI_ListData BadgeInfoList;
	GUI_ListItemData Items[9];
	char ListBuffer[9][64]; //height then width
	char RegCode[18];
};

class RadioInfoState: public StateBase {
public:
	RadioInfoState();
	virtual ~RadioInfoState();
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
private:
	GUI_ListData RadioInfoList;
	GUI_ListItemData Items[6];
	char ListBuffer[6][20];
};

class KeyBoardTest : public StateBase {
public:
	static const uint16_t NUMBER_DIALED = 7;
	static const uint16_t HEX_STRING=16;
public:
	KeyBoardTest();
	virtual ~KeyBoardTest();
protected:
	virtual ErrorType onInit(RunContext &rc);
	virtual ReturnStateContext onRun(RunContext &rc);
	virtual ErrorType onShutdown();
private:
	static const int8_t NOT_A_NUMBER = -1;
	uint8_t LastKey;
	int8_t NumberDialed[NUMBER_DIALED+1];
	int8_t FinalHexHash[HEX_STRING+1];
	uint8_t Pos;
	int8_t SelectedNumber;
	bool ReleaseEnter;
};

class SendMsgState;
class AddressState;
class Menu3D;
class MessageState;
class Gateway;
class StateFactory {
public:
	static bool init();
	static StateBase *getDisplayMessageState(StateBase *bm, const char *message, uint16_t timeToDisplay);
	static StateBase *getMenuState();
	static StateBase *getSettingState();
	static StateBase *getGameOfLifeState();
	static StateBase *getKeyBoardTest();
	static MessageState *getMessageState();
	static StateBase* getBadgeInfoState();
	static StateBase *getRadioInfoState();
	static StateBase *getIRPairingState();
	static SendMsgState* getSendMessageState();
	static AddressState* getAddressBookState();
	static Menu3D *get3DState();
	static StateBase *getGateway();
};

#endif
