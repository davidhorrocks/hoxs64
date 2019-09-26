#ifndef __HCONFIG_H__
#define __HCONFIG_H__

#include <windows.h>
#include "dx_version.h"
#include "bits.h"
#include "C64Keys.h"
#include "viciipalette.h"
#include "gamecontrolleritem.h"

namespace C64JoystickButton
{
	typedef enum tagJoyItem
	{
		None = 0,
		Fire1,
		Fire2,
		Up,
		Down,
		Left,
		Right,
		AxisHorizontal,
		AxisVertical,
		ButtonAndAxisKey1,
		ButtonAndAxisKey2,
		ButtonAndAxisKey3,
		ButtonAndAxisKey4,
		ButtonAndAxisKey5,
		ButtonAndAxisKey6,
	} C64JoystickButtonNumber;
};

class JoyKeyName
{
public:
	typedef enum tagJoystickKey
	{
		JoynBlank = 0,
		JoynEnabled,
		JoynValid,
		JoynGUID,
		JoynIsValidAxisX,
		JoynIsValidAxisY,
		JoynAxisX,
		JoynAxisY,
		JoynRevX,
		JoynRevY,
		JoynPOV,
		JoynFire1,
		JoynFire1Mask,
		JoynFire1ButtonCount,
		JoynFire1ButtonList,
		JoynFire2,
		JoynFire2Mask,
		JoynFire2ButtonCount,
		JoynFire2ButtonList,
		JoynButtonUp,
		JoynButtonUpMask,
		JoynButtonUpCount,
		JoynButtonUpList,
		JoynButtonDown,
		JoynButtonDownMask,
		JoynButtonDownCount,
		JoynButtonDownList,
		JoynButtonLeft,
		JoynButtonLeftMask,
		JoynButtonLeftCount,
		JoynButtonLeftList,
		JoynButtonRight,
		JoynButtonRightMask,
		JoynButtonRightCount,
		JoynButtonRightList,
		JoynButtonKeyNoAssign,
		JoynButtonKeyNoAssignIsValid,
		JoynButtonKey1,
		JoynButtonKey1Mask,
		JoynButtonKey1Count,
		JoynButtonKey1List,
		JoynButtonKey2,
		JoynButtonKey2Mask,
		JoynButtonKey2Count,
		JoynButtonKey2List,
		JoynButtonKey3,
		JoynButtonKey3Mask,
		JoynButtonKey3Count,
		JoynButtonKey3List,
		JoynButtonKey4,
		JoynButtonKey4Mask,
		JoynButtonKey4Count,
		JoynButtonKey4List,
		JoynButtonKey5,
		JoynButtonKey5Mask,
		JoynButtonKey5Count,
		JoynButtonKey5List,
		JoynButtonKey6,
		JoynButtonKey6Mask,
		JoynButtonKey6Count,
		JoynButtonKey6List,
		JoynAxisKey1List,
		JoynAxisKey1ListDirection,
		JoynAxisKey1Count,
		JoynAxisKey2List,
		JoynAxisKey2ListDirection,
		JoynAxisKey2Count,
		JoynAxisKey3List,
		JoynAxisKey3ListDirection,
		JoynAxisKey3Count,
		JoynAxisKey4List,
		JoynAxisKey4ListDirection,
		JoynAxisKey4Count,
		JoynAxisKey5List,
		JoynAxisKey5ListDirection,
		JoynAxisKey5Count,
		JoynAxisKey6List,
		JoynAxisKey6ListDirection,
		JoynAxisKey6Count,
		JoynPovKey1List,
		JoynPovKey1ListDirection,
		JoynPovKey1Count,
		JoynPovKey2List,
		JoynPovKey2ListDirection,
		JoynPovKey2Count,
		JoynPovKey3List,
		JoynPovKey3ListDirection,
		JoynPovKey3Count,
		JoynPovKey4List,
		JoynPovKey4ListDirection,
		JoynPovKey4Count,
		JoynPovKey5List,
		JoynPovKey5ListDirection,
		JoynPovKey5Count,
		JoynPovKey6List,
		JoynPovKey6ListDirection,
		JoynPovKey6Count,
		JoynLast
	} JOYSTICKKEY;

	struct ButtonKeySet
	{
		JOYSTICKKEY single;
		JOYSTICKKEY mask;
		JOYSTICKKEY count;
		JOYSTICKKEY list;
	};

	static LPCTSTR Name[2][JoynLast];
	static ButtonKeySet Fire1;
	static ButtonKeySet Fire2;
	static ButtonKeySet Up;
	static ButtonKeySet Down;
	static ButtonKeySet Left;
	static ButtonKeySet Right;
	static ButtonKeySet ButtonKey1;
	static ButtonKeySet ButtonKey2;
	static ButtonKeySet ButtonKey3;
	static ButtonKeySet ButtonKey4;
	static ButtonKeySet ButtonKey5;
	static ButtonKeySet ButtonKey6;
	static ButtonKeySet AxisKey1;
	static ButtonKeySet AxisKey2;
	static ButtonKeySet AxisKey3;
	static ButtonKeySet AxisKey4;
	static ButtonKeySet AxisKey5;
	static ButtonKeySet AxisKey6;
	static ButtonKeySet PovKey1;
	static ButtonKeySet PovKey2;
	static ButtonKeySet PovKey3;
	static ButtonKeySet PovKey4;
	static ButtonKeySet PovKey5;
	static ButtonKeySet PovKey6;
};

struct joyconfig
{
	static const unsigned int MAXKEYMAPS = 6;
	static const unsigned int MAXV1BUTTONS = 32;
	static const unsigned int MAXBUTTONS = 128;
	static const unsigned int MAXBUTTONS32 = 32;
	static const unsigned int MAXAXIS = 32;
	static const unsigned int MAXPOV = 4;
	static const unsigned int MAXDIRECTINPUTPOVNUMBER = 3;
	static const int MaxUserKeyAssignCount = 256;
	joyconfig();
	GUID joystickID;
	bool IsValidId;
	bool IsEnabled;
	bool isPovEnabled;
	bool isXReverse;
	bool isYReverse;
	bool isValidXAxis;
	bool isValidYAxis;
	DWORD dwOfs_X;
	DWORD dwOfs_Y;
	HCFG::JOYOBJECTKIND joyObjectKindX;
	HCFG::JOYOBJECTKIND joyObjectKindY;	
	LONG xMin;
	LONG xMax;
	LONG xLeft;
	LONG xRight;
	LONG yMin;
	LONG yMax;
	LONG yUp;
	LONG yDown;
	unsigned int fire1ButtonCount;
	unsigned int fire1AxisCount;
	unsigned int fire1PovCount;
	unsigned int fire2ButtonCount;
	unsigned int fire2AxisCount;
	unsigned int fire2PovCount;
	unsigned int upButtonCount;
	unsigned int upAxisCount;
	unsigned int upPovCount;
	unsigned int downButtonCount;
	unsigned int downAxisCount;
	unsigned int downPovCount;
	unsigned int leftButtonCount;
	unsigned int leftAxisCount;
	unsigned int leftPovCount;
	unsigned int rightButtonCount;
	unsigned int rightAxisCount;
	unsigned int rightPovCount;
	unsigned int horizontalAxisButtonCount;
	unsigned int horizontalAxisAxisCount;
	unsigned int horizontalAxisPovCount;
	unsigned int verticalAxisButtonCount;
	unsigned int verticalAxisAxisCount;
	unsigned int verticalAxisPovCount;
	DWORD fire1ButtonOffsets[MAXBUTTONS];
	DWORD fire2ButtonOffsets[MAXBUTTONS];
	DWORD upButtonOffsets[MAXBUTTONS];
	DWORD downButtonOffsets[MAXBUTTONS];
	DWORD leftButtonOffsets[MAXBUTTONS];
	DWORD rightButtonOffsets[MAXBUTTONS];
	DWORD povAvailable[MAXDIRECTINPUTPOVNUMBER + 1];
	int povIndex[MAXDIRECTINPUTPOVNUMBER + 1];
	bool enableKeyAssign;
	C64Keys::C64Key keyNoAssign[MaxUserKeyAssignCount];
	bool isValidKeyNoAssign[MaxUserKeyAssignCount];

	// Counts of assigned buttons
	unsigned int keyNButtonCount[MAXKEYMAPS];

	// Buttons offsets
	DWORD keyNButtonOffsets[MAXKEYMAPS][MAXBUTTONS];

	// Counts of assigned axes
	unsigned int keyNAxisCount[MAXKEYMAPS];

	// Axes offsets
	DWORD keyNAxisOffsets[MAXKEYMAPS][MAXAXIS];

	// Axes directions
	GameControllerItem::ControllerAxisDirection keyNAxisDirection[MAXKEYMAPS][MAXAXIS];

	// Counts of assigned pov controls
	unsigned int keyNPovCount[MAXKEYMAPS];

	// Pov controls offsets
	DWORD keyNPovOffsets[MAXKEYMAPS][MAXAXIS];

	// Pov directions
	GameControllerItem::ControllerAxisDirection keyNPovDirection[MAXKEYMAPS][MAXAXIS];

	LPCDIDATAFORMAT inputDeviceFormat;
	DWORD sizeOfInputDeviceFormat;
	ICLK joyNotAcquiredClock;

	// Axes with additional info.
	ButtonItemData axes[MAXKEYMAPS][MAXAXIS];

	// Pov with additional info.
	ButtonItemData pov[MAXKEYMAPS][MAXPOV];
	void LoadDefault();
	static void defaultClearAxisDirection(GameControllerItem::ControllerAxisDirection offsets[], GameControllerItem::ControllerAxisDirection axisDirection, unsigned int count);
};

class CConfig
{
public:
	CConfig();
	static LONG RegReadDWordOrStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD dwValue);
	static LONG RegReadStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	HRESULT SaveWindowSetting(HWND);
	static HRESULT SaveMDIWindowSetting(HWND hWnd);
	HRESULT LoadWindowSetting(POINT& pos, bool& bWindowedCustomSize, int& winWidth, int& winHeight);
	static HRESULT LoadMDIWindowSetting(POINT& pos, SIZE& size);
	static int GetKeyScanCode(UINT ch);
	HRESULT LoadCurrentSetting();
	HRESULT LoadCurrentJoystickSetting(int joystickNumber, struct joyconfig& jconfig);
	HRESULT WriteJoystickButtonList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pButtonOffsets, const unsigned int &buttonCount);
	HRESULT ReadJoystickButtonList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pButtonOffsets, unsigned int &buttonCount);
	HRESULT WriteJoystickAxisList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pAxisOffsets, const GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int axisCount);
	HRESULT WriteJoystickPovList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, const DWORD *pPovOffsets, const GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int povCount);
	HRESULT ReadJoystickAxisList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pAxisOffsets, GameControllerItem::ControllerAxisDirection *pAxisDirection, unsigned int maxAxisBufferCount, unsigned int* pAxisCount);
	HRESULT ReadJoystickPovList(HKEY hKey1, int joystickNumber, JoyKeyName::ButtonKeySet regnames, DWORD *pPovOffsets, GameControllerItem::ControllerAxisDirection *pPovDirection, unsigned int maxPovBufferCount, unsigned int* pPovCount);
	HRESULT SaveCurrentSetting();
	HRESULT SaveCurrentJoystickSetting(int joystickNumber, const struct joyconfig& jconfig);
	void LoadDefaultSetting();
	void SetPalettePepto();
	void SetCiaNewOldMode(bool isNew);
	void SetRunFast();
	void SetRunNormal();

	bit32 m_colour_palette[VicIIPalette::NumColours];
	unsigned char m_KeyMap[256];
	struct joyconfig m_joy1config;
	struct joyconfig m_joy2config;
	bool m_bMaxSpeed;
	bool m_bSwapJoysticks;
	bool m_bCPUFriendly;
	bool m_bAudioClockSync;
	bool m_bSidDigiBoost;

	bool m_bSIDResampleMode;
	bool m_bSIDStereo;
	bool m_bD1541_Emulation_Enable;
	bool m_bD1541_Thread_Enable;
	bool m_bAllowOpposingJoystick;
	bool m_bDisableDwmFullscreen;
	bool m_bSID_Emulation_Enable;
	bool m_bShowSpeed;
	bool m_bLimitSpeed;
	HCFG::FULLSCREENSYNCMODE m_syncModeFullscreen;
	HCFG::FULLSCREENSYNCMODE m_syncModeWindowed;
	bool m_bUseKeymap;
	bool m_bSkipFrames;
	bool m_bDoubleSizedWindow;
	bool m_bWindowedCustomSize;
	bool m_bUseBlitStretch;
	//POINT m_winpos;
	//POINT m_mdidebuggerwinpos;
	//SIZE m_mdidebuggerwinsize;
	GUID m_fullscreenAdapterId;
	DWORD m_fullscreenAdapterNumber;
	DWORD m_fullscreenWidth;
	DWORD m_fullscreenHeight;
	DWORD m_fullscreenRefresh;
	DWORD m_fullscreenFormat;
	HCFG::EMUWINDOWSTRETCH m_fullscreenStretch;
	HCFG::EMUWINDOWFILTER m_blitFilter;
	HCFG::EMUBORDERSIZE m_borderSize;
	int m_numberOfExtraSIDs;
	bit16 m_Sid2Address;
	bit16 m_Sid3Address;
	bit16 m_Sid4Address;
	bit16 m_Sid5Address;
	bit16 m_Sid6Address;
	bit16 m_Sid7Address;
	bit16 m_Sid8Address;
	bool m_bShowFloppyLed;
	HCFG::EMUFPS m_fps;
	HCFG::ETRACKZEROSENSORSTYLE m_TrackZeroSensorStyle;
	HCFG::CIAMODE m_CIAMode;
	bool m_bTimerBbug;
};

#endif