#ifndef __MDIDEGUGGERFRAME_H__
#define __MDIDEGUGGERFRAME_H__

class CMDIDebuggerFrame_EventSink_OnTrace : public EventSink<EventArgs>
{
protected:

	virtual int Sink(void *sender, EventArgs& e)
	{
		OnTrace(sender, e);
		return 0;
	}
	virtual void OnTrace(void *sender, EventArgs& e)=0;
};

class CMDIDebuggerFrame_EventSink_OnShowDevelopment : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnShowDevelopment(sender, e);
		return 0;
	}
	virtual void OnShowDevelopment(void *sender, EventArgs& e)=0;
};

class CMDIDebuggerFrame_EventSink : 
	public CMDIDebuggerFrame_EventSink_OnTrace,
	public CMDIDebuggerFrame_EventSink_OnShowDevelopment
{
};


class CMDIDebuggerFrame : public CVirWindow, CMDIDebuggerFrame_EventSink// , public ErrorMsg
{
public:
	static const int ID_RERBAR = 2000;
	static const int ID_TOOLBAR = 2001;

	static const int TOOLBUTTON_PICTURE_INDEX = 0;
	static const int TOOLBUTTON_MASK_INDEX = 1;
	static const ButtonInfo TB_StepButtons[];
	static const ImageInfo TB_ImageList[];

	static const TCHAR ClassName[];
	static const TCHAR MenuName[];

	CMDIDebuggerFrame();
	virtual ~CMDIDebuggerFrame();


	static HRESULT RegisterClass(HINSTANCE hInstance);
	HRESULT Init(IMonitorCommand *monitorCommand, CConfig *cfg, CAppStatus *appStatus, C64 *c64);
	HWND Create(HINSTANCE hInstance, HWND parent, const TCHAR title[], int x,int y, int w, int h);
	HWND Show(CVirWindow *pParentWindow);

	void ShowDebugCpuC64(bool bSeekPC);
	void ShowDebugCpuDisk(bool bSeekPC);

	void GetMinWindowSize(int &w, int &h);
	void EnsureWindowPosition(int x, int y, int w, int h);
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual HRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnDestroy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	CVirWindow *m_pParentWindow;
	HWND m_hWndRebar;
	HWND m_hWndTooBar;
	HIMAGELIST m_hImageListToolBarNormal;
	CDPI m_dpi;
	WPanelManager m_WPanelManager;

	CDisassemblyFrame m_debugCpuC64;
	CDisassemblyFrame m_debugCpuDisk;

	C64 *c64;
	CAppStatus *appStatus;
	CConfig *cfg;
	IMonitorCommand *m_monitorCommand;
	Monitor m_monitorC64;
	Monitor m_monitorDisk;

	virtual void OnTrace(void *sender, EventArgs& e);
	virtual void OnShowDevelopment(void *sender, EventArgs& e);

	HRESULT CreateMDIToolBars();
	HWND CreateRebar(HWND hwndTB);
	HWND CreateToolBar(HIMAGELIST hImageListToolBarNormal, const ButtonInfo buttonInfo[], int length, int buttonWidth, int buttonHeight);
	HIMAGELIST CreateImageListNormal(HWND hWnd);
	void OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Cleanup();
	HRESULT AdviseEvents();
	void UnadviseEvents();
	void SetMenuState();
};

#endif