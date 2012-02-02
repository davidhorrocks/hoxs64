#ifndef __MAINWIN_H__
#define __MAINWIN_H__

class CEmuWindow;

class IMainWindowProc
{
public:
	virtual LRESULT MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)=0;
};

class CAppWindow : public CVirWindow , public ErrorMsg
{
public:
	CAppWindow();
	HRESULT Init(CDX9 *dx, IMonitorCommand *monitorCommand, CConfig *, CAppStatus *, C64 *);
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	CAppStatus *appStatus;
	CConfig *cfg;
	CEmuWindow emuWin;
	RECT m_rcMainWindow;				// Saves the window size & pos while in windowed mode.
	int spareFillCounter;
	HWND m_hWndStatusBar;
	int m_iStatusBarHeight;

	void SetColours();
	void SetDriveMotorLed(bool bOn);
	void UpdateMenu();
	void SetMainWindowSize(bool bDoubleSizedWindow);
	void GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bDoubleSizedWindow, int *w, int *h);
	void SaveMainWindowSize();
	HRESULT ResetDirect3D();
	HRESULT SetCoopLevel(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch);
	HRESULT InitSurfaces(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch);
	void ClearSurfaces();
	HRESULT ToggleFullScreen();
	HRESULT SetWindowedMode(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch);
	void UpdateWindowTitle(TCHAR *szTitle, DWORD emulationSpeed);
	HWND ShowDevelopment();
	CMDIDebuggerFrame *m_pMDIDebugger;

protected:
	const static LPTSTR lpszClassName;
	const static LPTSTR lpszMenuName;
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:

	HCURSOR hCursorBusy;
	HCURSOR hOldCursor;
	/*Dialogs*/
	CDiagKeyboard mDlgkey;
	CDiagJoystick mDlgjoy;
	CDiagEmulationSettingsTab mDlgSettingsTab;
	CDiagNewBlankDisk mDlgNewBlankDisk;
	CDiagAbout mDlgAbout;
	static struct tabpageitem m_tabPagesKeyboard[4];
	static struct tabpageitem m_tabPagesSetting[5];
	IMonitorCommand *m_monitorCommand;
	CDX9 *dx;
	C64 *c64;
	static const DWORD StylesWindowed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SYSMENU;
	static const DWORD StylesNonWindowed = WS_POPUP | WS_SYSMENU;


	void OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif