#ifndef __DISSASSEMBLYFRAME_H__
#define __DISSASSEMBLYFRAME_H__

class CDisassemblyFrame : public CVirWindow, public IMonitorEvent, public ErrorMsg
{
public:
	CDisassemblyFrame();
	virtual ~CDisassemblyFrame();
	static const int ID_RERBAR = 2000;
	static const int ID_TOOLBAR = 2001;
	static const int ID_DISSASSEMBLEY = 2002;
	static const int ID_DISSASSEMBLEYREG = 2003;

	static const int TOOLBUTTON_WIDTH = 16;
	static const int TOOLBUTTON_HEIGHT = 16;
	//static const int TOOLBUTTON_COUNT = 5;

	static const int TOOLBUTTON_PICTURE_INDEX = 0;
	static const int TOOLBUTTON_MASK_INDEX = 1;
	static const ButtonInfo TB_StepButtons[];
	static const ImageInfo TB_ImageList[];
	static const TCHAR ClassName[];
	static const TCHAR MenuName[];	

	HRESULT Init(CVirWindow *parent, IMonitorEvent *monitorEvent, IMonitorCpu *cpu, IMonitorVic *vic, LPCTSTR pszCaption);
	HRESULT Show();
	HRESULT Show(bool bSeekPC);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, const TCHAR title[], int x,int y, int w, int h);
	void GetMinWindowSize(int &w, int &h);
	void EnsureWindowPosition(int x, int y, int w, int h);

private:
	IMonitorEvent *m_monitorEvent;
	IMonitorCpu *m_cpu;
	IMonitorVic *m_vic;
	LPCTSTR m_pszCaption;
	HWND m_hWndRebar;
	HWND m_hWndTooBar;
	HIMAGELIST m_hImageListToolBarNormal;
	HFONT m_monitor_font;
	CVirWindow *m_pParentWindow;
	CDisassemblyChild m_DisassemblyChild;
	CDisassemblyReg m_DisassemblyReg;


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

	HWND CreateRebar(HWND hwndTB);
	HWND CreateToolBar(HIMAGELIST hImageListToolBarNormal);
	HIMAGELIST CreateImageListNormal();
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