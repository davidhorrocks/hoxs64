#ifndef __APPSTATUS_H__
#define __APPSTATUS_H__

class CAppStatus : public CConfig
{
public:
	CAppStatus();
	bool					m_bBusy;
	bool                    m_bSoundOK;
	bool                    m_bSoundMute;
	bool					m_bFilterOK;
	bool                    m_bActive;				// App is running/active
	bool                    m_bReady;				// App is ready for updates
	bool                    m_bWindowed;			// App is in windowed mode
	bool					m_bRunning;
	bool					m_bDebug;
	bool					m_bBreak;
	bool					m_bPaused;
	bool					m_bWindowSizing;
	bool					m_bClosing;

	signed int				m_fskip;
	DWORD					m_displayFormat;	//Direct 3D9 Display format
	DWORD					m_blitFilterDX;	//Direct 3D9 Blit filter
	//eScreenType m_ScreenType;
	long					m_ScreenDepth;
	bool					m_bUseCPUDoubler;

	HCFG::AUDIOSPEED		m_audioSpeedStatus;
	ULARGE_INTEGER			m_systemfrequency;
	ULARGE_INTEGER			m_framefrequency;

	bit8                    m_bAutoload;
	bool					m_bInitDone;
	bool					m_bUpdateWindowTitle;
	bool					m_bDiskLedMotor;
	bool					m_bDiskLedDrive;
	bool					m_bDiskLedWrite;
	bool					m_bSerialTooBusyForSeparateThread;

	virtual LPTSTR GetAppTitle() = 0;
	virtual LPTSTR GetAppName() = 0;
	virtual LPTSTR GetMonitorTitle() = 0;
	virtual void SoundHalt() = 0;
	virtual void SoundResume() = 0;
	virtual void TogglePause() = 0;
	virtual void ToggleSoundMute() = 0;
	virtual void ToggleMaxSpeed() = 0;	
	virtual void FreeDirectX() = 0;
	virtual void InsertTape(HWND hWnd) = 0;
	virtual void LoadCrtFile(HWND hWnd) = 0;
	virtual void LoadC64Image(HWND hWnd) = 0;
	virtual void LoadT64(HWND hWnd) = 0;
	virtual void AutoLoad(HWND hWnd) = 0;
	virtual void InsertDiskImage(HWND hWnd) = 0;
	virtual void SaveD64Image(HWND hWnd) = 0;
	virtual void SaveFDIImage(HWND hWnd) = 0;
	virtual void SaveC64State(HWND hWnd) = 0;
	virtual void LoadC64State(HWND hWnd) = 0;
	virtual VS_FIXEDFILEINFO *GetVersionInfo() = 0;
	virtual void RestoreUserSettings() = 0;
	virtual void RestoreDefaultSettings() = 0;
	virtual void SaveCurrentSetting() = 0;
	virtual void GetUserConfig(CConfig& cfg) = 0;
	virtual void SetUserConfig(const CConfig& newcfg) = 0;
	virtual void ApplyConfig(const CConfig& newcfg) = 0;

	void SaveSpeedSettings();
	void RestoreSpeedSettings();
private:
	bool m_bSaveSkipFrames;;
	bool m_bSaveLimitSpeed;
	bool m_bSaveUseBlitStretch;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncMode;
};

#endif