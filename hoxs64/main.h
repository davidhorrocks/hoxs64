#ifndef __MAIN_H__
#define __MAIN_H__

class CApp : public CConfig, public CAppStatus, public IAppCommand, public IC64Event, public ErrorMsg
{
public:
	CApp();
	~CApp();
	int Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow);
	HRESULT InitApplication(HINSTANCE hInstance);
	HRESULT InitInstance(int nCmdShow, const CCommandArgArray *pArgs);
	HRESULT RegisterKeyPressWindow(HINSTANCE hInstance);
#if _WIN32_WINNT >= 0x400
	LRESULT LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam );
#else
	LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam );
#endif
	void AllowAccessibilityShortcutKeys( bool bAllowKeys );

	// CAppStatus
	virtual void CAppStatus::FreeDirectX();
	virtual void CAppStatus::SoundHalt();
	virtual void CAppStatus::SoundResume();
	virtual LPTSTR CAppStatus::GetAppTitle();
	virtual LPTSTR CAppStatus::GetAppName();
	virtual LPTSTR CAppStatus::GetMonitorTitle();
	virtual VS_FIXEDFILEINFO *CAppStatus::GetVersionInfo();
	virtual void CAppStatus::TogglePause();
	virtual void CAppStatus::ToggleSoundMute();
	virtual void CAppStatus::ToggleMaxSpeed();
	virtual void CAppStatus::InsertTape(HWND hWnd);
	virtual void CAppStatus::LoadC64Image(HWND hWnd);
	virtual void CAppStatus::LoadT64(HWND hWnd);
	virtual void CAppStatus::AutoLoad(HWND hWnd);
	virtual void CAppStatus::InsertDiskImage(HWND hWnd);
	virtual void CAppStatus::SaveD64Image(HWND hWnd);
	virtual void CAppStatus::SaveFDIImage(HWND hWnd);
	virtual void CAppStatus::RestoreUserSettings();
	virtual void CAppStatus::RestoreDefaultSettings();
	virtual void CAppStatus::SaveCurrentSetting();
	virtual void CAppStatus::GetUserConfig(CConfig& cfg);
	virtual void CAppStatus::SetUserConfig(const CConfig& newcfg);
	virtual void CAppStatus::ApplyConfig(const CConfig& newcfg);
	// CAppStatus

	// IAppCommand
	virtual void IAppCommand::Resume();
	virtual void IAppCommand::Trace();
	virtual void IAppCommand::TraceFrame();
	virtual void IAppCommand::ExecuteC64Clock();
	virtual void IAppCommand::ExecuteDiskClock();
	virtual void IAppCommand::ExecuteC64Instruction();
	virtual void IAppCommand::ExecuteDiskInstruction();
	virtual void IAppCommand::UpdateApplication();
	virtual HWND IAppCommand::ShowDevelopment();
	virtual bool IAppCommand::IsRunning();
	virtual void IAppCommand::SoundOff();
	virtual void IAppCommand::SoundOn();
	virtual void ShowCpuDisassembly(int cpuid, DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address);
	virtual HWND GetMainFrameWindow();
	virtual void DisplayVicCursor(bool bEnabled);
	virtual void SetVicCursorPos(int iCycle, int iLine);
	virtual void GetVicCursorPos(int *piCycle, int *piLine);
	// IAppCommand

	
	shared_ptr<CAppWindow> m_pWinAppWindow;
	

	//IC64Event
	virtual void BreakExecuteCpu64();
	virtual void BreakExecuteCpuDisk();
	virtual void BreakVicRasterCompare();
	virtual void BreakpointChanged();
	virtual void SetBusy(bool bBusy);
	virtual void DiskMotorLed(bool bOn);
	virtual void DiskDriveLed(bool bOn);
	virtual void DiskWriteLed(bool bOn);
	virtual void ShowErrorBox(LPCTSTR title, LPCTSTR message);

	C64 c64;

	//mainCfg is a CConfig that holds the users preferred settings (when Quick Speed is off).
	//CApp also derives from CConfig and hold the "in-play" setting. These "in-play" setting in (CConfig)CApp can differ from the normal user setting in CApp::mainCfg.
	//such as when Quick Speed is toggled on. Quick speed keep the users preferred settings intact and makes a change to the temporary "in play" setting to 
	//help optimise the function.
	CConfig mainCfg;
	HHOOK m_hKeyboardHook;
	EXECUTION_STATE m_Prevstate;
	STICKYKEYS m_StartupStickyKeys;
	TOGGLEKEYS m_StartupToggleKeys;
	FILTERKEYS m_StartupFilterKeys; 
	bool m_bBusy;
	HCURSOR hCursorBusy;
	HCURSOR hOldCursor;
	HACCEL m_hAccelTable;
	HINSTANCE m_hInstance;
	TCHAR m_szAppName[MAX_STR_LEN];  // Name of the app
	TCHAR m_szTitle[MAX_STR_LEN];    // The title bar text
	TCHAR m_szMonitorTitle[MAX_STR_LEN];    // The title bar text
	TCHAR m_szMainWndClsName[MAX_STR_LEN];  // Main window class
	TCHAR m_szMonitorWndClsName[MAX_STR_LEN];  // Monitor window class
	TCHAR m_szMonitorDisassembleWndClsName[MAX_STR_LEN];  //Disassemble child window class 
	TCHAR m_szAppConfigPath[MAX_PATH+1];
	TCHAR m_szAppFullPath[MAX_PATH+1];
	TCHAR m_szAppDirectory[MAX_PATH+1];
	VS_FIXEDFILEINFO m_Vinfo;

	CDX9 dx;
};


#endif