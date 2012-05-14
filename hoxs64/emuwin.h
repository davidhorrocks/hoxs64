#ifndef __EMUWIN_H__
#define __EMUWIN_H__

class CEmuWindow : public CVirWindow , public ErrorMsg
{
public:
	CEmuWindow();
	HRESULT Init(CDX9 *dx, CConfig *, CAppStatus *, C64 *);
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU controlID);

	static void GetRequiredWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bDoubleSizedWindow, int *w, int *h);
	HRESULT UpdateC64Window();
	HRESULT RenderWindow();
	void DrawDriveSprites();
	void SetColours();
	void ClearSurfaces();
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	CAppStatus *appStatus;
	CConfig *cfg;
	CDX9 *dx;
	C64 *c64;
	const static LPTSTR lpszClassName;
	DWORD m_dwSolidColourFill;

	int m_iLastX;
	int m_iLastY;
	bool OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif