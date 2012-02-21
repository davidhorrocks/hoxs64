#ifndef __BREAKPOINTFRAME_H__
#define __BREAKPOINTFRAME_H__

class WpcBreakpoint_EventSink_OnBreakpointC64ExecuteChanged : public EventSink<BreakpointC64ExecuteChangedEventArgs>
{
protected:

	virtual int Sink(void *sender, BreakpointC64ExecuteChangedEventArgs& e)
	{
		OnBreakpointC64ExecuteChanged(sender, e);
		return 0;
	}
	virtual void OnBreakpointC64ExecuteChanged(void *sender, BreakpointC64ExecuteChangedEventArgs& e)=0;
};

class WpcBreakpoint_EventSink_OnBreakpointDiskExecuteChanged : public EventSink<BreakpointDiskExecuteChangedEventArgs>
{
protected:

	virtual int Sink(void *sender, BreakpointDiskExecuteChangedEventArgs& e)
	{
		OnBreakpointDiskExecuteChanged(sender, e);
		return 0;
	}
	virtual void OnBreakpointDiskExecuteChanged(void *sender, BreakpointDiskExecuteChangedEventArgs& e)=0;
};

class WpcBreakpoint_EventSink : 
	public WpcBreakpoint_EventSink_OnBreakpointC64ExecuteChanged,
	public WpcBreakpoint_EventSink_OnBreakpointDiskExecuteChanged
{
};


class WpcBreakpoint : public CVirWindow, protected WpcBreakpoint_EventSink
{
public:
	typedef std::vector<Sp_BreakpointItem> LstBrk;
	WpcBreakpoint(C64 *c64);
	virtual ~WpcBreakpoint();
	static const TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);

	HRESULT Init();
protected:
	LstBrk m_lstBreak;

	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, LRESULT &lresult);
	bool LvBreakPoint_OnDispInfo(NMLVDISPINFO *pnmh, LRESULT &lresult);
	HRESULT LvBreakPoint_RowCol_GetText(int iRow, int iCol, LPTSTR pText, int cch);
	int LvBreakPoint_RowCol_State(int iRow, int iCol);

private:
	class LvBreakColumnIndex
	{
	public:
		enum tagEnumBreakColumnIndex
		{
			Cpu = 0,
			Address = 1,
			Type = 2
		};
	};
	HWND m_hLvBreak;
	HWND CreateListView(CREATESTRUCT *pcs, HWND hWndParent);
	HRESULT InitListViewColumns(HWND hWndListView);
	HRESULT FillListView(HWND hWndListView);

	void OnBreakpointC64ExecuteChanged(void *sender, BreakpointC64ExecuteChangedEventArgs& e);
	void OnBreakpointDiskExecuteChanged(void *sender, BreakpointDiskExecuteChangedEventArgs& e);

	CDPI m_dpi;
	C64 *c64;
};

#endif