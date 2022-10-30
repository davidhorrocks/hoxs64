#pragma once
#include "cvirwindow.h"

class CMDIDebuggerFrame_EventSink_OnTrace : public EventSink<EventArgs>
{
protected:

	int Sink(void *sender, EventArgs& e) override
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

class CMDIDebuggerFrame_EventSink_OnRadixChanged : public EventSink<RadixChangedEventArgs>
{
protected:
	virtual int Sink(void *sender, RadixChangedEventArgs& e)
	{
		OnRadixChanged(sender, e);
		return 0;
	}

	virtual void OnRadixChanged(void *sender, RadixChangedEventArgs& e)=0;
};

class CMDIDebuggerFrame_EventSink : 
	public CMDIDebuggerFrame_EventSink_OnTrace,
	public CMDIDebuggerFrame_EventSink_OnShowDevelopment,
	public CMDIDebuggerFrame_EventSink_OnRadixChanged
{
};

class CMDIDebuggerFrame : public CVirMdiFrameWindow, CMDIDebuggerFrame_EventSink
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

	CMDIDebuggerFrame(IC64 *c64, IAppCommand *pAppCommand, CConfig *cfg, CAppStatus *appStatus);
	virtual ~CMDIDebuggerFrame();
	CMDIDebuggerFrame(const CMDIDebuggerFrame&) = default;
	CMDIDebuggerFrame& operator=(const CMDIDebuggerFrame&) = default;
	CMDIDebuggerFrame(CMDIDebuggerFrame&&) = default;
	CMDIDebuggerFrame& operator=(CMDIDebuggerFrame&&) = default;

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu) override;
	void WindowRelease() override;
	void GetMinWindowSize(int &w, int &h) override;
	//HWND Show(CVirWindow *pParentWindow);
	void ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address) noexcept;
	void ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address) noexcept;
	bool IsWinDlgModelessBreakpointVicRaster();
	void OpenNewCli();

	shared_ptr<CMDIDebuggerFrame> keepAlive;
	weak_ptr<CDisassemblyFrame> m_pWinDebugCpuC64;
	weak_ptr<CDisassemblyFrame> m_pWinDebugCpuDisk;
	Wp_CDiagBreakpointVicRaster m_pdlgModelessBreakpointVicRaster;
protected:
	virtual LRESULT MdiFrameWindowProc(HWND hWnd, HWND hWndMDIClient, UINT uMsg, WPARAM wParam, LPARAM lParam);
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
	vector<HBITMAP> m_vec_hBmpRebarSized;
	CDPI m_dpi;
	WPanelManager m_WPanelManager;

	IC64 *c64;
	CAppStatus *appStatus;
	CConfig *cfg;
	IAppCommand *m_pAppCommand;
	bool m_bIsCreated;
	HFONT m_hFont;

	HRESULT Init();
	HRESULT InitFonts();
	void CloseFonts();
	virtual void OnTrace(void *sender, EventArgs& e);
	virtual void OnShowDevelopment(void *sender, EventArgs& e);
	virtual void OnRadixChanged(void *sender, RadixChangedEventArgs& e);

	void ShowDlgBreakpointVicRaster();
	void ShowModelessDlgBreakpointVicRaster();

	HRESULT CreateMDIToolBars(HDC hdc) noexcept;
	HIMAGELIST CreateImageListNormal(HWND hWnd);
	void OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Cleanup();
	HRESULT AdviseEvents();
	void UnadviseEvents();
	void SetMenuState();
};
