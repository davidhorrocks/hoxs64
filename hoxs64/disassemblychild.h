#ifndef __DISSASSEMBLYCHILD_H__
#define __DISSASSEMBLYCHILD_H__

class CDisassemblyChild_EventSink_OnCpuRegPCChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegPCChanged(sender, e);
		return 0;
	}
	virtual void OnCpuRegPCChanged(void *sender, EventArgs& e) =0;
};

class CDisassemblyChild_EventSink : 
	public CDisassemblyChild_EventSink_OnCpuRegPCChanged
{
};

class CDisassemblyChild : public CVirWindow, public CDisassemblyChild_EventSink, public DefaultCpu
{
public:
	CDisassemblyChild(int cpuid, C64 *c64, IAppCommand *pAppCommand, HFONT hFont);
	virtual ~CDisassemblyChild();

	static const int ID_SCROLLBAR = 2000;
	static const int ID_DISASSEMBLY = 2001;
	static TCHAR ClassName[];

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	void GetMinWindowSize(int &w, int &h);
	bit16 GetTopAddress();
	bit16 GetNthAddress(bit16 startaddress, int linenumber);
	int GetNumberOfLines();
	void SetHome();
	void UpdateDisplay(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address);
	void InvalidateBuffer();
	void CancelEditing();
private:
	HWND m_hWndScroll;
	IAppCommand *m_pAppCommand;

	shared_ptr<CDisassemblyEditChild> m_pWinDisassemblyEditChild;

	HRESULT Init();
	virtual void OnCpuRegPCChanged(void *sender, EventArgs& e);

	HRESULT AdviseEvents();
	void UnadviseEvents();

	HWND CreateScrollBar();
	HWND CreateEditWindow(int x, int y, int w, int h);

	HRESULT GetSizeRectEditWindow(RECT &rc);
	HRESULT GetSizeRectScrollBar(RECT &rc);
	void SetTopAddress(bit16 address, bool bSetScrollBarPage);

	HRESULT OnCreate(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent);
	void OnSizeScrollBar(HWND hWnd, int widthParent, int heightParent);
	void OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void SetAddressScrollPos(int pos);	

	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif