#pragma once
#include "cvirwindow.h"

#define MAX_EDIT_GOTO_ADDRESS_CHARS (256)

class IEnterGotoAddress
{
public:
	virtual bool OnEnterGotoAddress(LPTSTR pszAddress) = 0;
};

class CToolItemAddress : public CVirWindow
{
public:
	CToolItemAddress(HFONT hFont);
	virtual ~CToolItemAddress();

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	void GetMinWindowSize(int &w, int &h);

	static const TCHAR ClassName[];

	bool OnEnterGotoAddress();
	void SetInterface(IEnterGotoAddress *pIEnterGotoAddress);

	HRESULT GetDefaultTextBoxSize(HWND hWnd, SIZE& sizeText);
	int GetAddressText(int linenumber, LPTSTR szBuffer, int cchBufferLength);
private:
	IEnterGotoAddress *m_pIEnterGotoAddress;
	HWND m_hWndTxtAddress;
	WNDPROC m_wpOrigEditProc;
	HWND m_hWndButGoAddress;

	HWND m_hWndParent;

	HFONT m_hFont;
	TCHAR m_tempAddressTextBuffer[MAX_EDIT_GOTO_ADDRESS_CHARS+1];
	HWND CreateTextBox(HWND hWndParent, int id, int x, int y, int w, int h);

	HRESULT GetSizeRectTextBox(RECT &rc);
	HRESULT GetSizeRectButton(RECT &rc);

	HRESULT OnCreate(HWND hWnd);
	void OnDestroy(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void Cleanup();
	virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
