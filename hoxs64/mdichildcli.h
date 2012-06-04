#ifndef __MDICHILDCLI_H__
#define __MDICHILDCLI_H_

class CMDIChildCli : public CVirMdiChildWindow
{
public:
	CMDIChildCli(C64 *c64, IAppCommand *pIAppCommand, HFONT hFont);
	static const TCHAR ClassName[];

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(shared_ptr<CVirMdiFrameWindow> pWinMdiFrame);

protected:
	shared_ptr<CVirMdiFrameWindow> m_pWinMdiFrame;
	weak_ptr<WpcCli> m_pWinWpcCli; 
	C64 *c64;
	IAppCommand *m_pIAppCommand;
	HFONT m_hFont;

	HRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif