#pragma once
#include "cvirwindow.h"
#include "errormsg.h"

class CDisassemblyFrame_EventSink_OnResume : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnResume(sender, e);
		return 0;
	}
	virtual void OnResume(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnTrace : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnTrace(sender, e);
		return 0;
	}
	virtual void OnTrace(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnTraceFrame : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnTraceFrame(sender, e);
		return 0;
	}
	virtual void OnTraceFrame(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnExecuteC64Clock : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnExecuteC64Clock(sender, e);
		return 0;
	}
	virtual void OnExecuteC64Clock(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnExecuteDiskClock : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnExecuteDiskClock(sender, e);
		return 0;
	}
	virtual void OnExecuteDiskClock(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnExecuteC64Instruction : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnExecuteC64Instruction(sender, e);
		return 0;
	}
	virtual void OnExecuteC64Instruction(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnExecuteDiskInstruction : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnExecuteDiskInstruction(sender, e);
		return 0;
	}
	virtual void OnExecuteDiskInstruction(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnShowDevelopment : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnShowDevelopment(sender, e);
		return 0;
	}
	virtual void OnShowDevelopment(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnMemoryChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnMemoryChanged(sender, e);
		return 0;
	}

	virtual void OnMemoryChanged(void *sender, EventArgs& e) =0;
};

class CDisassemblyFrame_EventSink_OnRadixChanged : public EventSink<RadixChangedEventArgs>
{
protected:
	virtual int Sink(void *sender, RadixChangedEventArgs& e)
	{
		OnRadixChanged(sender, e);
		return 0;
	}

	virtual void OnRadixChanged(void *sender, RadixChangedEventArgs& e) =0;
};

class CDisassemblyFrame_EventSink : 
	public CDisassemblyFrame_EventSink_OnResume,
	public CDisassemblyFrame_EventSink_OnTrace,
	public CDisassemblyFrame_EventSink_OnTraceFrame,
	public CDisassemblyFrame_EventSink_OnExecuteC64Clock,
	public CDisassemblyFrame_EventSink_OnExecuteDiskClock,
	public CDisassemblyFrame_EventSink_OnExecuteC64Instruction,
	public CDisassemblyFrame_EventSink_OnExecuteDiskInstruction,
	public CDisassemblyFrame_EventSink_OnShowDevelopment,
	public CDisassemblyFrame_EventSink_OnMemoryChanged,
	public CDisassemblyFrame_EventSink_OnRadixChanged
{
};

class CDisassemblyFrame : public CVirWindow, public CDisassemblyFrame_EventSink, public DefaultCpu, public ErrorMsg, IEnterGotoAddress
{
public:
	CDisassemblyFrame(int cpuid, IC64 *c64, IAppCommand *pAppCommand, LPCTSTR pszCaption, HFONT hFont);
	virtual ~CDisassemblyFrame();
	static const int ID_RERBAR = 2000;
	static const int ID_TOOLBAR = 2001;
	static const int ID_DISASSEMBLEY = 2002;
	static const int ID_DISASSEMBLEYREG = 2003;

	static const int TOOLBUTTON_PICTURE_INDEX = 0;
	static const int TOOLBUTTON_MASK_INDEX = 1;
	static const ButtonInfo TB_ButtonsStep[];
	static const ButtonInfo TB_ButtonsAddress[];
	static const ImageInfo TB_ImageList[];
	static const TCHAR ClassName[];
	static const TCHAR MenuName[];
	
	shared_ptr<CDisassemblyFrame> keepAlive;

	virtual void WindowRelease() override;
	HRESULT Show(HWND hWndParent);
	void UpdateDisplay(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	void GetMinWindowSize(int &w, int &h);
	HRESULT UpdateMetrics();
	void Refresh();
	bool OnEnterGotoAddress();
	virtual bool OnEnterGotoAddress(LPTSTR pszAddress);

private:
	CDPI m_dpi;
	DBGSYM::MonitorOption::Radix radix;
	int m_MinSizeW;
	int m_MinSizeH;
	int m_iCurrentControlIndex;
	IAppCommand *m_pAppCommand;
	LPCTSTR m_pszCaption;
	HWND m_hWndRebar;
	HWND m_hWndTooBarStep;
	HWND m_hWndTooBarAddress;
	HBITMAP m_hBmpRebarNotSized;
	vector<HBITMAP> m_vec_hBmpRebarSized;
	HIMAGELIST m_hImageListToolBarNormal;
	HFONT m_hFont;
	shared_ptr<CDisassemblyChild> m_pWinDisassemblyChild;
	shared_ptr<CDisassemblyReg> m_pWinDisassemblyReg;
	shared_ptr<CToolItemAddress> m_pWinToolItemAddress;
	HWND m_hWndToolItemAddress;
	TCHAR m_tempAddressBuffer[MAX_EDIT_GOTO_ADDRESS_CHARS + 1];
	int m_wheel_current;

	HRESULT Init();
	HIMAGELIST CreateImageListStepNormal(HWND hWnd);
	HWND CreateDisassemblyChild(int x, int y, int w, int h);
	HWND CreateDisassemblyReg(int x, int y, int w, int h);
	shared_ptr<CToolItemAddress> CreateToolItemAddress(HWND hWndParent);
	HRESULT RebarAddAddressBar(HWND hWndRebar, HWND hWndToolbar);
	void SetRadix(DBGSYM::MonitorOption::Radix radix);
	HRESULT GetSizeRectReg(RECT &rc);
	HRESULT GetSizeRectDisassembly(RECT &rc);
	HRESULT GetSizeRectToolBar(RECT &rc);
	void SetMenuState();
	void SetHome();
	void CancelEditing();
	void OnResume(void *sender, EventArgs& e);
	void OnTrace(void *sender, EventArgs& e);
	void OnTraceFrame(void *sender, EventArgs& e);
	void OnExecuteC64Clock(void *sender, EventArgs& e);
	void OnExecuteDiskClock(void *sender, EventArgs& e);
	void OnExecuteC64Instruction(void *sender, EventArgs& e);
	void OnExecuteDiskInstruction(void *sender, EventArgs& e);
	void OnShowDevelopment(void *sender, EventArgs& e);
	void OnMemoryChanged(void *sender, EventArgs& e);
	void OnRadixChanged(void *sender, RadixChangedEventArgs& e);
	HRESULT AdviseEvents();
	void UnadviseEvents();
	HRESULT OnCreate(HWND hWnd);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSizeToolBar(HWND hWnd, int widthParent, int heightParent);
	void OnSizeDisassembly(HWND hWnd, int widthParent, int heightParent);
	void OnSizeScrollBar(HWND hWnd, int widthParent, int heightParent);
	void OnSizeRegisters(HWND hWnd, int widthParent, int heightParent);
	bool OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnMouseWheel(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnToolBarInfo(LPNMTBGETINFOTIP info);
	bool OnReBarHeightChange(LPNMHDR notify);
	void Cleanup();
	//virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

