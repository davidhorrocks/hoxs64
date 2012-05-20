#ifndef __WPCCLI_H__
#define __WPCCLI_H__

class WpcCli : public CVirWindow
{
public:
	WpcCli(C64 *c64, IAppCommand *pIAppCommand);
	virtual ~WpcCli();

	static const TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
protected:

	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, bool &handled);

private:
	CDPI m_dpi;
	C64 *c64;
	IAppCommand *m_pIAppCommand;
	bool m_bHexDisplay;
	HMODULE m_hinstRiched;
	HWND m_hWndEdit;
};

#endif