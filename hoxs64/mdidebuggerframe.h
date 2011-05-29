#ifndef __MDIDEGUGGERFRAME_H__
#define __MDIDEGUGGERFRAME_H__


class CMDIDebuggerFrame : public CVirWindow, IMonitorEvent// , public ErrorMsg
{
public:
	static const int ID_RERBAR = 2000;
	static const int ID_TOOLBAR = 2001;
	static const int TOOLBUTTON_WIDTH = 16;
	static const int TOOLBUTTON_HEIGHT = 16;

	static const int TOOLBUTTON_PICTURE_INDEX = 0;
	static const int TOOLBUTTON_MASK_INDEX = 1;
	static const ButtonInfo TB_StepButtons[];
	static const ImageInfo TB_ImageList[];

	static const TCHAR ClassName[];
	static const TCHAR MenuName[];

	CMDIDebuggerFrame();
	virtual ~CMDIDebuggerFrame();


	static HRESULT RegisterClass(HINSTANCE hInstance);
	HRESULT Init(IMonitorEvent *monitorEvent, CConfig *cfg, CAppStatus *appStatus, C64 *c64);
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

private:
	CVirWindow *m_pParentWindow;
	HWND m_hWndRebar;
	HWND m_hWndTooBar;
	HIMAGELIST m_hImageListToolBarNormal;
	
	CDisassemblyFrame m_debugCpuC64;
	CDisassemblyFrame m_debugCpuDisk;

	C64 *c64;
	CAppStatus *appStatus;
	CConfig *cfg;
	IMonitorEvent *m_monitorEvent;

	//IMonitorEvent
	virtual void IMonitorEvent::Resume(IMonitorEvent *sender);
	virtual void IMonitorEvent::Trace(IMonitorEvent *sender);
	virtual void IMonitorEvent::TraceFrame(IMonitorEvent *sender);
	virtual void IMonitorEvent::ExecuteC64Clock(IMonitorEvent *sender);
	virtual void IMonitorEvent::ExecuteDiskClock(IMonitorEvent *sender);
	virtual void IMonitorEvent::ExecuteC64Instruction(IMonitorEvent *sender);
	virtual void IMonitorEvent::ExecuteDiskInstruction(IMonitorEvent *sender);
	virtual void IMonitorEvent::UpdateApplication(IMonitorEvent *sender);
	virtual HWND IMonitorEvent::ShowDevelopment(IMonitorEvent *sender);
	virtual bool IMonitorEvent::IsRunning(IMonitorEvent *sender);
	virtual HRESULT IMonitorEvent::Advise(IMonitorEvent *sink);
	virtual void IMonitorEvent::Unadvise(IMonitorEvent *sink);

	HRESULT CreateMDIToolBars();
	HWND CreateRebar(HWND hwndTB);
	HWND CreateToolBar(HIMAGELIST hImageListToolBarNormal, const ButtonInfo buttonInfo[], int length, int buttonWidth, int buttonHeight);
	HIMAGELIST CreateImageListNormal(const ImageInfo imageInfo[], int length, int imageWidth, int imageHeight);
	void OnGetMinMaxSizeInfo(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void Cleanup();
	void SetMenuState();
};

#endif