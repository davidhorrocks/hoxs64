#pragma once
#include "cvirwindow.h"
#include "errormsg.h"

class CAppWindow : public CVirWindow, public ErrorMsg
{
public:
	CAppWindow(Graphics* pGx, CDX9 *dx, IAppCommand *pAppCommand, CAppStatus *, IC64 *);
	~CAppWindow();
	
	class INotify
	{
	public:
		virtual void VicCursorMove(int cycle, int line) = 0;
	};

	static HRESULT RegisterClass(HINSTANCE hInstance);
	void WindowRelease() override;
	void Cleanup();
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu) override;
	CAppStatus *appStatus;
	RECT m_rcMainWindow;				// Saves the window size & pos while in windowed mode.

	void SetColours();
	void UpdateMenu();
	void GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h);
	void GetMinimumWindowedSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h);
	void CalcEmuWindowPadding(int *padw, int *padh);
	void SaveMainWindowSize();
	void AspectSizing(int edge, RECT &rcDrag);
	void AspectMaxSizing(HWND hWnd, int edge, RECT &rcDrag);
	HRESULT ResetDirect3D();
	HRESULT ToggleFullScreen();
	void RestoreWindowPosition();
	HRESULT SetWindowedMode(bool wantWindowed);
	void UpdateWindowTitle(const TCHAR *szTitle, DWORD emulationSpeed);
	HWND ShowDevelopment();
	void CloseWindow();

	shared_ptr<CAppWindow> keepAlive;
	weak_ptr<CMDIDebuggerFrame> m_pMDIDebugger;

	//From old emu child window.
	void GetRequiredClientWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, int* w, int* h);
	HRESULT UpdateC64Window(bool refreshVicData);
	void UpdateC64WindowWithObjects();
	HRESULT Present();
	void SetNotify(INotify* pINotify);
	void DisplayVicCursor(bool bEnabled);
	void DisplayVicRasterBreakpoints(bool bEnabled);
	void SetVicCursorPos(int iCycle, int iLine);
	void GetVicCursorPos(int* piCycle, int* piLine);
	void SetWindowedStyle(bool bWindowed);
protected:
	const static LPTSTR lpszClassName;
	const static LPTSTR lpszMenuName;
	const static DWORD CAppWindow::WindowStyles;
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
private:
	LONG_PTR m_dwStyle;
	HMENU m_hMenuOld;
	HCURSOR m_hCursorBusy;
	HCURSOR m_hOldCursor;
	static struct tabpageitem m_tabPagesKeyboard[4];
	static struct tabpageitem m_tabPagesSetting[5];
	IAppCommand *appCommand;
	Graphics* pGx;
	CDX9 *dx;
	IC64 *c64;
	static const DWORD StylesWindowed = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX  | WS_MINIMIZEBOX | WS_SIZEBOX;
	static const DWORD StylesNonWindowed = WS_POPUP | WS_SYSMENU;

	//From old emu child window.
	bool m_bDragMode;
	bool m_bDrawCycleCursor;
	bool m_bDrawVicRasterBreakpoints;
	int m_iVicCycleCursor;
	int m_iVicLineCursor;
	INotify* m_pINotify;
	HBRUSH m_hBrushBrkInner;
	HBRUSH m_hBrushBrkOuter;
	bool isMouseTracking = false;

	void ResizeGraphics(HWND hWnd, int width, int height);
	bool OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	void OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnPaletteChanged(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnDisplayChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnWindowPositionChanged(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	bool OnSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);	
	bool OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnSizing(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	bool OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult);
	bool OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT& lresult);
	void OnTraceSystemClocks(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	//From old emu child window.
	HRESULT RenderWindow();
	void OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMouseHover(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnMouseLeave(HWND hWnd, WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetCursorAtClientPosition(int x, int y);
	void DrawCursorAtVicPosition(HDC hdc, int cycle, int line);
	void PrepareVicCursorsForDrawing();
	void GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line);
	void GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle);
	void GetDisplayRectFromVicRect(const RECT rcVicCycle, RECT& rcDisplay);
};
