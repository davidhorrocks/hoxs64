#pragma once
#include "cvirwindow.h"

class CMDIChildCli : public CVirMdiChildWindow
{
public:
	CMDIChildCli(IC64 *c64, IAppCommand *pIAppCommand, HFONT hFont);
	static const TCHAR ClassName[];
	virtual void WindowRelease() override;

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(CVirMdiFrameWindow *pWinMdiFrame);
	shared_ptr<CMDIChildCli> keepAlive;
protected:
	CVirMdiFrameWindow *m_pWinMdiFrame;
	weak_ptr<WpcCli> m_pWinWpcCli; 
	IC64 *c64;
	IAppCommand *m_pIAppCommand;
	HFONT m_hFont;

	HRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
