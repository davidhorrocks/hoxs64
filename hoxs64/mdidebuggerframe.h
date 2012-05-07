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

	CMDIDebuggerFrame(C64 *c64, IMonitorCommand *monitorCommand, CConfig *cfg, CAppStatus *appStatus);
	virtual ~CMDIDebuggerFrame();

	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	HRESULT Init();
	HWND Show(CVirWindow *pParentWindow);

	void ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address);
	void ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address);

	virtual void GetMinWindowSize(int &w, int &h);
	CDisassemblyFrame m_debugCpuC64;
	CDisassemblyFrame m_debugCpuDisk;
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
	virtual void OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND m_hWndRebar;
	HWND m_hWndTooBar;
	HIMAGELIST m_hImageListToolBarNormal;
	HBITMAP m_hBmpRebarNotSized;
	std::vector<HBITMAP> m_vec_hBmpRebarSized;
	CDPI m_dpi;
	WPanelManager m_WPanelManager;


	C64 *c64;
	CAppStatus *appStatus;
	CConfig *cfg;
	IMonitorCommand *m_pMonitorCommand;
	//Monitor m_monitorC64;
	//Monitor m_monitorDisk;
	bool m_bIsCreated;

	virtual void OnTrace(void *sender, EventArgs& e);
	virtual void OnShowDevelopment(void *sender, EventArgs& e);

	void ShowDlgBreakpointVicRaster();

	HRESULT CreateMDIToolBars(HDC hdc);
	HIMAGELIST CreateImageListNormal(HWND hWnd);
	void OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Cleanup();
	HRESULT AdviseEvents();
	void UnadviseEvents();
	void SetMenuState();
};

#endif