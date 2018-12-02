#ifndef __HCONFIG_H__
#define __HCONFIG_H__

#include "bits.h"
#include "viciipalette.h"

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
		Right
	} C64JoystickButtonNumber;
};

class JoyKeyName
{
public:
	typedef enum tagJoystickKey
	{
		JoynEnabled = 0,
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
};


struct joyconfig
{
	static const unsigned int MAXBUTTONS = 32;
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
	DWORD dwOfs_Up;
	DWORD dwOfs_Down;
	DWORD dwOfs_Left;
	DWORD dwOfs_Right;
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
	unsigned int fire2ButtonCount;
	unsigned int upButtonCount;
	unsigned int downButtonCount;
	unsigned int leftButtonCount;
	unsigned int rightButtonCount;
	DWORD fire1ButtonOffsets[MAXBUTTONS];
	DWORD fire2ButtonOffsets[MAXBUTTONS];
	DWORD upButtonOffsets[MAXBUTTONS];
	DWORD downButtonOffsets[MAXBUTTONS];
	DWORD leftButtonOffsets[MAXBUTTONS];
	DWORD rightButtonOffsets[MAXBUTTONS];
	DWORD povAvailable[4];
	int povIndex[4];
	void LoadDefault();
	ICLK joyNotAcquiredClock;
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
	bool m_bShowFloppyLed;
	HCFG::EMUFPS m_fps;
	HCFG::ETRACKZEROSENSORSTYLE m_TrackZeroSensorStyle;
	HCFG::CIAMODE m_CIAMode;
	bool m_bTimerBbug;
};

#endif