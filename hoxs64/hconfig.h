#pragma once
#include <windows.h>
#include <tchar.h>
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
	joyconfig() noexcept;
	GUID joystickID;
	bool IsValidId = false;
	bool IsEnabled = false;
	bool isPovEnabled = false;
	bool isXReverse = false;
	bool isYReverse = false;
	bool isValidXAxis = false;
	bool isValidYAxis = false;
	DWORD dwOfs_X = 0;
	DWORD dwOfs_Y = 0;
	HCFG::JOYOBJECTKIND joyObjectKindX = HCFG::JOYOBJECTKIND::JoyKindNone;
	HCFG::JOYOBJECTKIND joyObjectKindY = HCFG::JOYOBJECTKIND::JoyKindNone;
	LONG xMin = 0;
	LONG xMax = 0;
	LONG xLeft = 0;
	LONG xRight = 0;
	LONG yMin = 0;
	LONG yMax = 0;
	LONG yUp = 0;
	LONG yDown = 0;
	unsigned int fire1ButtonCount = 0;
	unsigned int fire1AxisCount = 0;
	unsigned int fire1PovCount = 0;
	unsigned int fire2ButtonCount = 0;
	unsigned int fire2AxisCount = 0;
	unsigned int fire2PovCount = 0;
	unsigned int upButtonCount = 0;
	unsigned int upAxisCount = 0;
	unsigned int upPovCount = 0;
	unsigned int downButtonCount = 0;
	unsigned int downAxisCount = 0;
	unsigned int downPovCount = 0;
	unsigned int leftButtonCount = 0;
	unsigned int leftAxisCount = 0;
	unsigned int leftPovCount = 0;
	unsigned int rightButtonCount = 0;
	unsigned int rightAxisCount = 0;
	unsigned int rightPovCount = 0;
	unsigned int horizontalAxisButtonCount = 0;
	unsigned int horizontalAxisAxisCount = 0;
	unsigned int horizontalAxisPovCount = 0;
	unsigned int verticalAxisButtonCount;
	unsigned int verticalAxisAxisCount = 0;
	unsigned int verticalAxisPovCount = 0;
	DWORD fire1ButtonOffsets[MAXBUTTONS] = {};
	DWORD fire2ButtonOffsets[MAXBUTTONS] = {};
	DWORD upButtonOffsets[MAXBUTTONS] = {};
	DWORD downButtonOffsets[MAXBUTTONS] = {};
	DWORD leftButtonOffsets[MAXBUTTONS] = {};
	DWORD rightButtonOffsets[MAXBUTTONS] = {};
	DWORD povAvailable[MAXDIRECTINPUTPOVNUMBER + 1] = {};
	int povIndex[MAXDIRECTINPUTPOVNUMBER + 1] = {};
	bool enableKeyAssign = false;
	C64Keys::C64Key keyNoAssign[MaxUserKeyAssignCount] = {};
	bool isValidKeyNoAssign[MaxUserKeyAssignCount] = {};

	// Counts of assigned buttons
	unsigned int keyNButtonCount[MAXKEYMAPS] = {};

	// Buttons offsets
	DWORD keyNButtonOffsets[MAXKEYMAPS][MAXBUTTONS] = {};

	// Counts of assigned axes
	unsigned int keyNAxisCount[MAXKEYMAPS] = {};

	// Axes offsets
	DWORD keyNAxisOffsets[MAXKEYMAPS][MAXAXIS] = {};

	// Axes directions
	GameControllerItem::ControllerAxisDirection keyNAxisDirection[MAXKEYMAPS][MAXAXIS] = {};

	// Counts of assigned pov controls
	unsigned int keyNPovCount[MAXKEYMAPS] = {};

	// Pov controls offsets
	DWORD keyNPovOffsets[MAXKEYMAPS][MAXAXIS] = {};

	// Pov directions
	GameControllerItem::ControllerAxisDirection keyNPovDirection[MAXKEYMAPS][MAXAXIS] = {};

	LPCDIDATAFORMAT inputDeviceFormat = nullptr;
	DWORD sizeOfInputDeviceFormat = 0;
	ICLK joyNotAcquiredClock = 0;

	// Axes with additional info.
	ButtonItemData axes[MAXKEYMAPS][MAXAXIS] = {};

	// Pov with additional info.
	ButtonItemData pov[MAXKEYMAPS][MAXPOV] = {};
	void LoadDefault() noexcept;
	static void defaultClearAxisDirection(GameControllerItem::ControllerAxisDirection offsets[], GameControllerItem::ControllerAxisDirection axisDirection, unsigned int count) noexcept;
};

class CConfig
{
public:
	CConfig() noexcept;
	virtual ~CConfig() = default;
	CConfig(const CConfig&) = default;
	CConfig(CConfig&&) = default;
	CConfig& operator=(const CConfig&) = default;
	CConfig& operator=(CConfig&&) = default;
	static LONG RegReadDWordOrStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD dwValue);
	static LONG RegReadStr(HKEY hKey, LPCTSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
	static HRESULT SaveMDIWindowSetting(HWND hWnd);
	static HRESULT LoadMDIWindowSetting(POINT& pos, SIZE& size);
	static int GetKeyScanCode(UINT ch);
	HRESULT SaveWindowSetting(HWND);
	HRESULT LoadWindowSetting(POINT& pos, int& winWidth, int& winHeight);
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
	void SetPalettePepto() noexcept;
	void SetCiaNewOldMode(bool isNew);
	void SetRunFast();
	void SetRunNormal();

private:
	HRESULT ReadRegKeyboardItem(HKEY hkey, LPCTSTR keyname, unsigned char keyvalue) noexcept;
	HRESULT WriteRegKeyboardItem(HKEY hkey, LPCTSTR keyname, unsigned char keyvalue) noexcept;
public:
	bit32 m_colour_palette[VicIIPalette::NumColours] = {};
	unsigned char m_KeyMap[256] = {};
	struct joyconfig m_joy1config = {};
	struct joyconfig m_joy2config = {};
	bool m_bMaxSpeed = false;
	bool m_bSwapJoysticks = false;
	bool m_bCPUFriendly = true;
	bool m_bAudioClockSync = true;
	bool m_bSidDigiBoost = false;
	bool m_bSIDResampleMode = true;
	bool m_bSIDStereo = true;
	bool m_bD1541_Emulation_Enable = true;
	bool m_bD1541_Thread_Enable = true;
	bool m_bAllowOpposingJoystick = false;
	bool m_bDisableDwmFullscreen = false;
	bool m_bEnableImGuiWindowed = true;
	bool m_bSID_Emulation_Enable = true;
	bool m_bShowSpeed = true;
	bool m_bLimitSpeed = true;
	HCFG::FULLSCREENSYNCMODE m_syncModeFullscreen = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	HCFG::FULLSCREENSYNCMODE m_syncModeWindowed = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	bool m_bSkipFrames = false;
	bool m_bWindowedLockAspectRatio = true;
	bool m_fullscreenAdapterIsDefault = true;
	DWORD m_fullscreenAdapterNumber = 0;
	DWORD m_fullscreenOutputNumber = 0;
	DWORD m_fullscreenWidth = 0;
	DWORD m_fullscreenHeight = 0;
	DWORD m_fullscreenRefreshNumerator = 0;
	DWORD m_fullscreenRefreshDenominator = 0;
	DWORD m_fullscreenFormat = 0;
	DXGI_MODE_SCALING m_fullscreenDxGiModeScaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	DXGI_MODE_SCANLINE_ORDER m_fullscreenDxGiModeScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	HCFG::EMUWINDOWSTRETCH m_fullscreenStretch = HCFG::EMUWINDOWSTRETCH::EMUWINSTR_ASPECTSTRETCH;
	HCFG::EMUWINDOWFILTER m_blitFilter = HCFG::EMUWINDOWFILTER::EMUWINFILTER_AUTO;
	HCFG::EMUBORDERSIZE m_borderSize = HCFG::EMUBORDERSIZE::EMUBORDER_TV;
	int m_numberOfExtraSIDs = 1;
	bit16 m_Sid2Address = 0;
	bit16 m_Sid3Address = 0;
	bit16 m_Sid4Address = 0;
	bit16 m_Sid5Address = 0;
	bit16 m_Sid6Address = 0;
	bit16 m_Sid7Address = 0;
	bit16 m_Sid8Address = 0;
	bool m_bShowFloppyLed = true;
	HCFG::EMUFPS m_fps = HCFG::EMUFPS::EMUFPS_50_12;
	HCFG::ETRACKZEROSENSORSTYLE m_TrackZeroSensorStyle;
	HCFG::CIAMODE m_CIAMode = HCFG::CIAMODE::CM_CIA6526A;
	bool m_bTimerBbug = false;
};
