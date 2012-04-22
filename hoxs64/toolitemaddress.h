#ifndef __TOOLITEMADDRESS_H__
#define __TOOLITEMADDRESS_H__

#define MAX_EDIT_GOTO_ADDRESS_CHARS (256)

class IEnterGotoAddress
{
public:
	virtual void OnEnterGotoAddress(LPTSTR pszAddress) = 0;
};

class CToolItemAddress : public CVirWindow
{
public:
	CToolItemAddress();
	virtual ~CToolItemAddress();

	HRESULT Init(HFONT hFont);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	void GetMinWindowSize(int &w, int &h);

	static const TCHAR ClassName[];

	void OnEnterGotoAddress();
	void SetInterface(IEnterGotoAddress *pIEnterGotoAddress);

private:
	IEnterGotoAddress *m_pIEnterGotoAddress;
	HWND m_hWndTxtAddress;
	WNDPROC m_wpOrigEditProc;
	HWND m_hWndButGoAddress;

	HWND m_hWndParent;

	HFONT m_hFont;
	TCHAR m_tempAddressTextBuffer[MAX_EDIT_GOTO_ADDRESS_CHARS+1];
	HWND CreateTextBox(HWND hWndParent, int id, int x, int y, int w, int h);
	HWND CreateButton(HWND hWndParent, int id, int x, int y, int w, int h);

	HRESULT GetSizeRectTextBox(RECT &rc);
	HRESULT GetSizeRectButton(RECT &rc);

	HRESULT OnCreate(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Cleanup();
	virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif