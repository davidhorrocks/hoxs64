#ifndef __MDICHILDCLI_H__
#define __MDICHILDCLI_H_

class CMDIChildCli : public CVirMdiChildWindow
{
public:

	static const TCHAR ClassName[];

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(shared_ptr<CVirMdiFrameWindow> pWinMdiFrame);

protected:

	shared_ptr<CVirMdiFrameWindow> m_pWinMdiFrame; 

	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif