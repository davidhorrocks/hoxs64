#ifndef __DISSASSEMBLYCHILD_H__
#define __DISSASSEMBLYCHILD_H__

class CDisassemblyChild : public CVirWindow
{
public:
	CDisassemblyChild();
	virtual ~CDisassemblyChild();

	static const int ID_SCROLLBAR = 2000;
	static const int ID_DISASSEMBLY = 2001;
	static TCHAR ClassName[];

	HRESULT Init(CVirWindow *parent, IMonitorCommand *monitorCommand, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID);
	void GetMinWindowSize(int &w, int &h);
	void SetTopAddress(bit16 address);
	void UpdateDisplay(bool bEnsurePC);
	void InvalidateBuffer();
private:
	HWND m_hWndScroll;

	CVirWindow *m_pParent;
	CDisassemblyEditChild m_DisassemblyEditChild;

	HWND CreateScrollBar();
	HWND CreateEditWindow(int x, int y, int w, int h);

	HRESULT GetSizeRectEditWindow(RECT &rc);
	HRESULT GetSizeRectScrollBar(RECT &rc);

	HRESULT OnCreate(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent);
	void OnSizeScrollBar(HWND hWnd, int widthParent, int heightParent);
	void OnScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	void SetAddressScrollPos(int pos);	

	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif