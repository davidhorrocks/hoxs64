#ifndef __EMUWIN_H__
#define __EMUWIN_H__

class CEmuWindow : public CVirWindow , public ErrorMsg
{
public:
	CEmuWindow(CDX9 *dx, CConfig *cfg, CAppStatus *appStatus, IC64 *c64);
	~CEmuWindow();
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU controlID);

	class INotify
	{
	public:
		virtual void VicCursorMove(int cycle, int line) = 0;
	};

	static void GetRequiredWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bDoubleSizedWindow, int *w, int *h);
	void SetColours();
	void ClearSurfaces();
	HRESULT UpdateC64Window(bool refreshVicData);
	void UpdateC64WindowWithObjects();
	HRESULT Present(DWORD dwFlags);

	void SetNotify(INotify *pINotify);

	void DisplayVicCursor(bool bEnabled);
	void DisplayVicRasterBreakpoints(bool bEnabled);
	void SetVicCursorPos(int iCycle, int iLine);
	void GetVicCursorPos(int *piCycle, int *piLine);
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CAppStatus *appStatus;
	CConfig *cfg;
	CDX9 *dx;
	IC64 *c64;
	const static LPTSTR lpszClassName;
	DWORD m_dwSolidColourFill;

	bool m_bDragMode;
	bool m_bDrawCycleCursor;
	bool m_bDrawVicRasterBreakpoints;
	int m_iVicCycleCursor;
	int m_iVicLineCursor;

	int m_iLastX;
	int m_iLastY;
	INotify *m_pINotify;

	HBRUSH m_hBrushBrkInner;
	HBRUSH m_hBrushBrkOuter;

	HRESULT RenderWindow();
	
	void OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetCursorAtClientPosition(int x, int y);
	void DrawCursorAtVicPosition(HDC hdc, int cycle, int line);
	void DrawAllCursors(HDC hdc);

	void GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line);
	void GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle);
	void GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle, RECT& rcDisplay);
	void GetDisplayRectFromVicRect(const RECT rcVicCycle, RECT& rcDisplay);

};

#endif