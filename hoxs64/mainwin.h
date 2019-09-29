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
	void GetMinimumWindowedSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h);
	bool CalcEmuWindowSize(RECT rcMainWindow, int *w, int *h);
	void SaveMainWindowSize();
	void AspectSizing(HWND hWnd, int edge, RECT &rcDrag);
	HRESULT ResetDirect3D();
	HRESULT SetCoopLevel(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	HRESULT InitSurfaces(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	void ClearSurfaces();
	HRESULT ToggleFullScreen();
	HRESULT SetWindowedMode(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch);
	void SetWindowedStyle(bool bWindowed);
	void UpdateWindowTitle(TCHAR *szTitle, DWORD emulationSpeed);
	HWND ShowDevelopment();
	void CloseWindow();

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

	bool OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	void OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaletteChanged(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnDisplayChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	bool OnSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);	
	bool OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnSizing(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	bool OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	void OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif