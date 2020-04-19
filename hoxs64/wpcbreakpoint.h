#pragma once
#include "cvirwindow.h"

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

class WpcBreakpoint_EventSink_OnBreakpointVicChanged : public EventSink<BreakpointVicChangedEventArgs>
{
protected:

	virtual int Sink(void *sender, BreakpointVicChangedEventArgs& e)
	{
		OnBreakpointVicChanged (sender, e);
		return 0;
	}

	virtual void OnBreakpointVicChanged (void *sender, BreakpointVicChangedEventArgs& e)=0;
};

class WpcBreakpoint_EventSink_OnBreakpointChanged : public EventSink<BreakpointChangedEventArgs>
{
protected:

	virtual int Sink(void *sender, BreakpointChangedEventArgs& e)
	{
		OnBreakpointChanged (sender, e);
		return 0;
	}

	virtual void OnBreakpointChanged (void *sender, BreakpointChangedEventArgs& e)=0;
};

class WpcBreakpoint_EventSink_OnRadixChanged : public EventSink<RadixChangedEventArgs>
{
protected:

	virtual int Sink(void *sender, RadixChangedEventArgs& e)
	{
		OnRadixChanged (sender, e);
		return 0;
	}

	virtual void OnRadixChanged (void *sender, RadixChangedEventArgs& e)=0;
};

class WpcBreakpoint_EventSink : 
	public WpcBreakpoint_EventSink_OnBreakpointC64ExecuteChanged,
	public WpcBreakpoint_EventSink_OnBreakpointDiskExecuteChanged,
	public WpcBreakpoint_EventSink_OnBreakpointVicChanged,
	public WpcBreakpoint_EventSink_OnBreakpointChanged,
	public WpcBreakpoint_EventSink_OnRadixChanged
{
};

class WpcBreakpoint : public CVirWindow, protected WpcBreakpoint_EventSink
{
public:
	typedef vector<BreakpointItem> LstBrk;

	WpcBreakpoint(IC64* c64, IAppCommand* pAppCommand);
	virtual ~WpcBreakpoint();
	WpcBreakpoint() = delete;
	WpcBreakpoint(const BreakpointItem&) = delete;
	virtual WpcBreakpoint& operator=(const WpcBreakpoint&) = delete;
	WpcBreakpoint(BreakpointItem&&) = delete;
	WpcBreakpoint& operator=(BreakpointItem&&) = delete;

	static const TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
protected:
	LstBrk m_lstBreak;

	LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	LRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, LRESULT &lresult);
	bool LvBreakPoint_OnDispInfo(NMLVDISPINFO *pnmh, LRESULT &lresult);
	bool LvBreakPoint_OnLClick(NMITEMACTIVATE *pnmh, LRESULT &lresult);
	bool LvBreakPoint_OnRClick(NMITEMACTIVATE *pnmh, LRESULT &lresult);
	bool LvBreakPoint_OnKeyDown(NMLVKEYDOWN *pnmh, LRESULT &lresult);
	
	HRESULT LvBreakPoint_RowCol_GetData(int iRow, BreakpointItem& bp);
	HRESULT LvBreakPoint_RowCol_GetText(int iRow, int iCol, LPTSTR pText, int cch);
	int LvBreakPoint_RowCol_State(int iRow, int iCol);

private:
	static LPCTSTR sVic;
	static LPCTSTR sDisk;
	static LPCTSTR sC64;
	static LPCTSTR sExecute;
	static LPCTSTR sRasterCompare;
	static LPCTSTR sLine;
	static LPCTSTR sCycle;
	static LPCTSTR sAddress;
	class LvBreakColumnIndex
	{
	public:
		enum tagEnumBreakColumnIndex
		{
			Cpu = 0,
			Type = 1,
			Address = 2,
			Line = 3,
			Cycle = 4
		};
	};

	HWND m_hLvBreak;
	HMENU m_hMenuBreakPoint;
	CDPI m_dpi;
	IC64* c64;
	IAppCommand* m_pAppCommand;

	HRESULT Init();
	HWND CreateListView(CREATESTRUCT *pcs, HWND hWndParent);
	HRESULT InitListViewColumns(HWND hWndListView);
	HRESULT FillListView(HWND hWndListView);
	int GetTextWidth(HWND hWnd, LPCTSTR szText, int fallbackWidthOnError);
	void SetMenuState();
	void RefreshBreakpointListView();
	void RedrawBreakpointListItems();

	void OnBreakpointC64ExecuteChanged(void *sender, BreakpointC64ExecuteChangedEventArgs& e) override;
	void OnBreakpointDiskExecuteChanged(void *sender, BreakpointDiskExecuteChangedEventArgs& e) override;
	void OnBreakpointVicChanged(void *sender, BreakpointVicChangedEventArgs& e) override;
	void OnBreakpointChanged(void *sender, BreakpointChangedEventArgs& e) override;
	void OnRadixChanged(void *sender, RadixChangedEventArgs& e) override;

	void OnShowAssembly();
	void DeleteBreakpoint(const BreakpointItem& bp);
	void EnableBreakpoint(const BreakpointItem& bp);
	void DisableBreakpoint(const BreakpointItem& bp);

	void OnDeleteSelectedBreakpoint();
	void OnEnableSelectedBreakpoint();
	void OnDisableSelectedBreakpoint();
	void OnSetRadixHexadecimal();
	void OnSetRadixDecimal();

	HRESULT AdviseEvents();
	void UnadviseEvents() noexcept;
	void Cleanup() noexcept;
};
