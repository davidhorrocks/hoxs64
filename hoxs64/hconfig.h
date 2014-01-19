#ifndef __HCONFIG_H__
#define __HCONFIG_H__


struct joyconfig
{
	bool bEnabled;
	bool bValid;
	GUID joystickID;
	bool bXReverse;
	bool bYReverse;
	DWORD dwOfs_X;
	DWORD dwOfs_Y;
	
	//LPCDIDEVICEINSTANCE pDInst;
	LONG xMin;
	LONG xMax;
	LONG xLeft;
	LONG xRight;
	LONG yMin;
	LONG yMax;
	LONG yUp;
	LONG yDown;

	DWORD dwOfs_firebutton;
};

class CConfig
{
public:
	CConfig();
	static HRESULT SaveWindowSetting(HWND);
	static HRESULT SaveMDIWindowSetting(HWND hWnd);
	static HRESULT LoadWindowSetting(POINT& pos);
	static HRESULT LoadMDIWindowSetting(POINT& pos, SIZE& size);

	HRESULT LoadCurrentSetting();
	HRESULT SaveCurrentSetting();
	void LoadDefaultSetting();
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
	int m_windowedModeWidth;
	int m_windowedModeHeight;
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

	void ToggleMaxSpeed();

	static int GetKeyScanCode(UINT ch);

private:
	void SaveSpeedSettings();
	void RestoreSpeedSettings();
	bool m_bSaveSkipFrames;;
	bool m_bSaveLimitSpeed;
	bool m_bSaveUseBlitStretch;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncMode;
};

#endif