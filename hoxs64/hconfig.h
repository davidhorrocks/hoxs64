#ifndef __HCONFIG_H__
#define __HCONFIG_H__

struct joyconfig
{
	bool bEnabled;
	bool bPovEnabled;
	bool bValid;
	GUID joystickID;
	bool bXReverse;
	bool bYReverse;
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
	DWORD dwOfs_firebutton;
	DWORD povAvailable[4];
	int povIndex[4];
};

class CConfig
{
public:
	CConfig();
	HRESULT SaveWindowSetting(HWND);
	static HRESULT SaveMDIWindowSetting(HWND hWnd);
	HRESULT LoadWindowSetting(POINT& pos, bool& bWindowedCustomSize, int& winWidth, int& winHeight);
	static HRESULT LoadMDIWindowSetting(POINT& pos, SIZE& size);
	static int GetKeyScanCode(UINT ch);

	HRESULT LoadCurrentSetting();
	HRESULT SaveCurrentSetting();
	void LoadDefaultSetting();
	void SetCiaNewOldMode(bool isNew);
	void SetRunFast();
	void SetRunNormal();

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
	HCFG::FULLSCREENSYNCMODE m_syncMode;
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