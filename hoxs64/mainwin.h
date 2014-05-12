#ifndef __MAINWIN_H__
#define __MAINWIN_H__

class CEmuWindow;

class CAppWindow : public CVirWindow, public ErrorMsg
{
public:
	CAppWindow(CDX9 *dx, IAppCommand *pAppCommand, CAppStatus *, IC64 *);
	~CAppWindow();
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	CAppStatus *appStatus;
	RECT m_rcMainWindow;				// Saves the window size & pos while in windowed mode.
	int spareFillCounter;
	HWND m_hWndStatusBar;
	int m_iStatusBarHeight;

	void SetColours();
	void SetDriveMotorLed(bool bOn);
	void UpdateMenu();
	void GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bDoubleSizedWindow, int *w, int *h);
	void GetMinimumWindowedSize(int *w, int *h);
	bool CalcEmuWindowSize(RECT rcMainWindow, int *w, int *h);
	void SaveMainWindowSize();
	HRESULT ResetDirect3D();
	HRESULT SetCoopLevel(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	HRESULT InitSurfaces(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	void ClearSurfaces();
	HRESULT ToggleFullScreen();
	HRESULT SetWindowedMode(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	void SetWindowedStyle(bool bWindowed);
	void UpdateWindowTitle(TCHAR *szTitle, DWORD emulationSpeed);
	HWND ShowDevelopment();

	shared_ptr<CEmuWindow> m_pWinEmuWin;
	weak_ptr<CMDIDebuggerFrame> m_pMDIDebugger;

protected:
	const static LPTSTR lpszClassName;
	const static LPTSTR lpszMenuName;
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	LONG_PTR m_dwStyle;
	HMENU m_hMenuOld;
	HCURSOR m_hCursorBusy;
	HCURSOR m_hOldCursor;
	static struct tabpageitem m_tabPagesKeyboard[4];
	static struct tabpageitem m_tabPagesSetting[5];
	IAppCommand *m_pAppCommand;
	CDX9 *dx;
	IC64 *c64;
	static const DWORD StylesWindowed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX  | WS_MINIMIZEBOX | WS_SIZEBOX;
	static const DWORD StylesNonWindowed = WS_POPUP | WS_SYSMENU;


	void OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif