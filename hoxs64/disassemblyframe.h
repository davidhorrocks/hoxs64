#ifndef __DISSASSEMBLYFRAME_H__
#define __DISSASSEMBLYFRAME_H__

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

class CDisassemblyFrame_EventSink : 
	public CDisassemblyFrame_EventSink_OnResume,
	public CDisassemblyFrame_EventSink_OnTrace,
	public CDisassemblyFrame_EventSink_OnTraceFrame,
	public CDisassemblyFrame_EventSink_OnExecuteC64Clock,
	public CDisassemblyFrame_EventSink_OnExecuteDiskClock,
	public CDisassemblyFrame_EventSink_OnExecuteC64Instruction,
	public CDisassemblyFrame_EventSink_OnExecuteDiskInstruction,
	public CDisassemblyFrame_EventSink_OnShowDevelopment
{
};

class CDisassemblyFrame : public CVirWindow, public CDisassemblyFrame_EventSink, public ErrorMsg
{
public:
	CDisassemblyFrame();
	virtual ~CDisassemblyFrame();
	static const int ID_RERBAR = 2000;
	static const int ID_TOOLBAR = 2001;
	static const int ID_DISASSEMBLEY = 2002;
	static const int ID_DISASSEMBLEYREG = 2003;

	static const int TOOLBUTTON_PICTURE_INDEX = 0;
	static const int TOOLBUTTON_MASK_INDEX = 1;
	static const ButtonInfo TB_StepButtons[];
	static const ImageInfo TB_ImageList[];
	static const TCHAR ClassName[];
	static const TCHAR MenuName[];	

	HRESULT Init(CVirWindow *parent, IMonitorCommand *monitorCommand, Monitor *pMon, LPCTSTR pszCaption);
	HRESULT Show();
	HRESULT Show(bool bSeekPC);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, const TCHAR title[], int x,int y, int w, int h);
	void GetMinWindowSize(int &w, int &h);
	void EnsureWindowPosition(int x, int y, int w, int h);

private:
	CDPI m_dpi;
	int m_iCurrentControlIndex;
	IMonitorCommand *m_monitorCommand;
	Monitor *m_pMon;
	LPCTSTR m_pszCaption;
	HWND m_hWndRebar;
	HWND m_hWndTooBar;
	HIMAGELIST m_hImageListToolBarNormal;
	HFONT m_monitor_font;
	CVirWindow *m_pParentWindow;
	CDisassemblyChild m_DisassemblyChild;
	CDisassemblyReg m_DisassemblyReg;

	HWND CreateRebar(HWND hwndTB);
	HWND CreateToolBar(HIMAGELIST hImageListToolBarNormal);
	HIMAGELIST CreateImageListNormal(HWND hWnd);
	HWND CreateDisassemblyChild(int x, int y, int w, int h);
	HWND CreateDisassemblyReg(int x, int y, int w, int h);
	HRESULT InitFonts();
	void CloseFonts();
	HRESULT GetSizeRectReg(RECT &rc);
	HRESULT GetSizeRectDisassembly(RECT &rc);
	HRESULT GetSizeRectToolBar(RECT &rc);
	void SetMenuState();

	void SetHome();
	void UpdateDisplay(bool bSeekPC);
	void CancelEditing();

	void OnResume(void *sender, EventArgs& e);
	void OnTrace(void *sender, EventArgs& e);
	void OnTraceFrame(void *sender, EventArgs& e);
	void OnExecuteC64Clock(void *sender, EventArgs& e);
	void OnExecuteDiskClock(void *sender, EventArgs& e);
	void OnExecuteC64Instruction(void *sender, EventArgs& e);
	void OnExecuteDiskInstruction(void *sender, EventArgs& e);
	void OnShowDevelopment(void *sender, EventArgs& e);

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

	bool OnToolBarInfo(LPNMTBGETINFOTIP info);
	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};


#endif