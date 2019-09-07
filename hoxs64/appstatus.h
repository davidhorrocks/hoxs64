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
	bool                    m_bActive;				// App window is either active or not minimised
	bool                    m_bReady;				// DirectX is ready for use.
	bool                    m_bWindowed;			// App is in windowed mode
	bool					m_bRunning;
	bool					m_bDebug;
	bool					m_bBreak;
	bool					m_bPaused;
	bool					m_bWindowSizing;
	bool					m_bClosing;
	bool                    m_bIsDebugCart;
	signed int				m_fskip;
	DWORD					m_displayFormat;	//Direct 3D9 Display format
	DWORD					m_blitFilterDX;	//Direct 3D9 Blit filter
	//eScreenType m_ScreenType;
	long					m_ScreenDepth;
	bool					m_bUseCPUDoubler;

	HCFG::AUDIOSPEED		m_audioSpeedStatus;
	ULARGE_INTEGER			m_systemfrequency;
	ULARGE_INTEGER			m_framefrequency;

	bool                    m_bAutoload;
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
	virtual void SaveP64Image(HWND hWnd) = 0;	
	virtual void SaveC64State(HWND hWnd) = 0;
	virtual void LoadC64State(HWND hWnd) = 0;
	virtual VS_FIXEDFILEINFO *GetVersionInfo() = 0;
	virtual void RestoreUserSettings() = 0;
	virtual void RestoreDefaultSettings() = 0;
	virtual void SaveCurrentSetting() = 0;
	virtual void GetUserConfig(CConfig& cfg) = 0;
	virtual void SetUserConfig(const CConfig& newcfg) = 0;
	virtual void ApplyConfig(const CConfig& newcfg) = 0;
	virtual void SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID) = 0;
	virtual void ResetSidChipAddressMap() = 0;
	virtual void UpdateUserConfigFromSid() = 0;

	void SaveSpeedSettings();
	void RestoreSpeedSettings();
private:
	bool m_bSaveSkipFrames;;
	bool m_bSaveLimitSpeed;
	bool m_bSaveUseBlitStretch;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncModeFullscreen;
	HCFG::FULLSCREENSYNCMODE m_SaveSyncModeWindowed;
};

#endif