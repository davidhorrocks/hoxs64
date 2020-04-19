#pragma once
#include "cvirwindow.h"

class CDisassemblyReg_EventSink_OnTextChanged : public EventSink<EdLnTextChangedEventArgs>
{
protected:
	virtual int Sink(void *sender, EdLnTextChangedEventArgs& e)
	{
		OnTextChanged(sender, e);
		return 0;
	}
	virtual void OnTextChanged(void *sender, EdLnTextChangedEventArgs& e) =0;
};

class CDisassemblyReg_EventSink_OnTabControl : public EventSink<EdLnTabControlEventArgs>
{
protected:
	virtual int Sink(void *sender, EdLnTabControlEventArgs& e)
	{
		OnTabControl(sender, e);
		return 0;
	}

	virtual void OnTabControl(void *sender, EdLnTabControlEventArgs& e) =0;
};

class CDisassemblyReg_EventSink_OnCancelControl : public EventSink<EdLnCancelControlEventArgs>
{
protected:
	virtual int Sink(void *sender, EdLnCancelControlEventArgs& e)
	{
		OnCancelControl(sender, e);
		return 0;
	}

	virtual void OnCancelControl(void *sender, EdLnCancelControlEventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnSaveControl : public EventSink<EdLnSaveControlEventArgs>
{
protected:
	virtual int Sink(void *sender, EdLnSaveControlEventArgs& e)
	{
		OnSaveControl(sender, e);
		return 0;
	}

	virtual void OnSaveControl(void *sender, EdLnSaveControlEventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegPCChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegPCChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegPCChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegAChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegAChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegAChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegXChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegXChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegXChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegYChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegYChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegYChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegSRChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegSRChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegSRChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegSPChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegSPChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegSPChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegDdrChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegDdrChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegDdrChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink_OnCpuRegDataChanged : public EventSink<EventArgs>
{
protected:
	virtual int Sink(void *sender, EventArgs& e)
	{
		OnCpuRegDataChanged(sender, e);
		return 0;
	}

	virtual void OnCpuRegDataChanged(void *sender, EventArgs& e) = 0;
};

class CDisassemblyReg_EventSink : 
	public CDisassemblyReg_EventSink_OnTextChanged,
	public CDisassemblyReg_EventSink_OnTabControl,
	public CDisassemblyReg_EventSink_OnCancelControl,
	public CDisassemblyReg_EventSink_OnSaveControl,
	public CDisassemblyReg_EventSink_OnCpuRegPCChanged,
	public CDisassemblyReg_EventSink_OnCpuRegAChanged,
	public CDisassemblyReg_EventSink_OnCpuRegXChanged,
	public CDisassemblyReg_EventSink_OnCpuRegYChanged,
	public CDisassemblyReg_EventSink_OnCpuRegSRChanged,
	public CDisassemblyReg_EventSink_OnCpuRegSPChanged,
	public CDisassemblyReg_EventSink_OnCpuRegDdrChanged,
	public CDisassemblyReg_EventSink_OnCpuRegDataChanged
{
};

class CDisassemblyReg : public CVirWindow, public CDisassemblyReg_EventSink, public DefaultCpu
{
public:
	static const int MARGIN_TOP = 0;
	static const int MARGIN_RIGHT = 0;
	static const int MARGIN_BOTTOM = 0;
	static const int MARGIN_LEFT = 0;
	static const int PADDING_LEFT = 4;
	static const int PADDING_RIGHT = 4;
	static const int PADDING_TOP = 4;
	static const int PADDING_BOTTOM = 4;
	enum tagControlID
	{
		CTRLID_PC=1,
		CTRLID_A=2,
		CTRLID_X=3,
		CTRLID_Y=4,
		CTRLID_SR=5,
		CTRLID_SP=6,
		CTRLID_DDR=7,
		CTRLID_DATA=8,
		CTRLID_VICLINE=9,
		CTRLID_VICCYCLE=10,
		CTRLID_DISKTRACK=11
	};

	CDisassemblyReg(int cpuid, IC64 *c64, IAppCommand *pAppCommand, HFONT hFont);
	virtual ~CDisassemblyReg();

	static TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	void GetMinWindowSize(int &w, int &h);
	void UpdateDisplay();
	void InvalidateBuffer();
	void CancelEditing();
	void SaveEditing();
	bool SaveEditing(EdLn *pEdLnControl);
	void MoveTab(bool isNext);
	HRESULT UpdateMetrics();
	HRESULT UpdateMetrics(DBGSYM::MonitorOption::Radix radix);
	void SetRadix(DBGSYM::MonitorOption::Radix radix);
private:
	CVirWindow *m_pParent;
	HFONT m_hFont;
	HDC m_hdc;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;
	IAppCommand *m_pAppCommand;
	int CurrentControlID;
	int m_iShowCaretCount;
	int cpuid;
	int xpos;
	int ypos;
	EdLn PC;
	EdLn A;
	EdLn X;
	EdLn Y;
	EdLn SR;
	EdLn SP;
	EdLn Ddr;
	EdLn Data;
	EdLn VicLine;
	EdLn VicCycle;
	EdLn DiskTrack;
	bool IsEditing;
	CArray<EdLn*> Controls;
	DBGSYM::MonitorOption::Radix radix;
	TEXTMETRIC TextMetric;

	HRESULT UpdateLayout();
	void UpdateBuffer();
	HRESULT OnCreate(HWND hWnd);
	void DrawDisplay(HWND hWnd, HDC hdc);
	void DrawDisplay2(HWND hWnd, HDC hdc);
	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnTextChanged(void *sender, EdLnTextChangedEventArgs& e);
	void OnTabControl(void *sender, EdLnTabControlEventArgs& e);
	void OnCancelControl(void *sender, EdLnCancelControlEventArgs& e);
	void OnSaveControl(void *sender, EdLnSaveControlEventArgs& e);
	void OnCpuRegPCChanged(void *sender, EventArgs& e);
	void OnCpuRegAChanged(void *sender, EventArgs& e);
	void OnCpuRegXChanged(void *sender, EventArgs& e);
	void OnCpuRegYChanged(void *sender, EventArgs& e);
	void OnCpuRegSRChanged(void *sender, EventArgs& e);
	void OnCpuRegSPChanged(void *sender, EventArgs& e);
	void OnCpuRegDdrChanged(void *sender, EventArgs& e);
	void OnCpuRegDataChanged(void *sender, EventArgs& e);
	bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT AdviseEvents();
	void UnadviseEvents();

	HRESULT Init(int x, int y, int cpuid, DBGSYM::MonitorOption::Radix radix);
	HRESULT ArrangeControls();
	void SelectControl(EdLn *control);
	void StartEditing(EdLn *control, int insertionPoint);
	bool IsChanged();
	void UpdateCaret();
	void StartCaretCount();
	void ClearCaretCount();
	bool ProcessChar(WPARAM wParam, LPARAM lParam);
	bool ProcessKeyDown(WPARAM wParam, LPARAM lParam);
	bool ProcessLButtonDown(WPARAM wParam, LPARAM lParam);
	EdLn *GetSelectedControl();
	EdLn *GetControlByID(int id);
	EdLn *GetTabFirstControl();
	EdLn *GetTabNextControl();
	EdLn *GetTabPreviousControl();
	EdLn *GetTabLastControl();
	HRESULT UpdateDisplay(EdLn *control, int value);
};
