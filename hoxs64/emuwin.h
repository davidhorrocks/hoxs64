#ifndef __EMUWIN_H__
#define __EMUWIN_H__

class CEmuWindow : public CVirWindow , public ErrorMsg
{
public:
	CEmuWindow(CDX9 *dx, CConfig *cfg, CAppStatus *appStatus, C64 *c64);
	~CEmuWindow();
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU controlID);

	class INotify
	{
	public:
		virtual void VicCursorMove(int cycle, int line) = 0;
	};

	static void GetRequiredWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bDoubleSizedWindow, int *w, int *h);
	HRESULT UpdateC64Window();
	HRESULT RenderWindow();
	void DrawDriveSprites();
	void SetColours();
	void ClearSurfaces();

	void SetNotify(INotify *pINotify);

	void DisplayVicCursor(bool bEnabled);
	void SetVicCursorPos(int iCycle, int iLine);
	void GetVicCursorPos(int *piCycle, int *piLine);
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CAppStatus *appStatus;
	CConfig *cfg;
	CDX9 *dx;
	C64 *c64;
	const static LPTSTR lpszClassName;
	DWORD m_dwSolidColourFill;

	bool m_bDragMode;
	bool m_bDrawCycleCursor;
	RECT m_rcCycleCursor;
	int m_iVicCycleCursor;
	int m_iVicLineCursor;

	int m_iLastX;
	int m_iLastY;
	INotify *m_pINotify;

	void OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void DrawCursorAtClientPosition(int x, int y);
	void DrawCursorAtVicPosition(int cycle, int line);

	void GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line);
	void GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle);
	void GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle, RECT& rcDisplay);
	void GetDisplayRectFromVicRect(const RECT rcVicCycle, RECT& rcDisplay);

};

#endif