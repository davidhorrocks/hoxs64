#ifndef __DISSASSEMBLYREG_H__
#define __DISSASSEMBLYREG_H__

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

class CDisassemblyReg_EventSink_OnEscControl : public EventSink<EdLnEscControlEventArgs>
{
protected:
	virtual int Sink(void *sender, EdLnEscControlEventArgs& e)
	{
		OnEscControl(sender, e);
		return 0;
	}
	virtual void OnEscControl(void *sender, EdLnEscControlEventArgs& e) =0;
};

class CDisassemblyReg_EventSink : 
	public CDisassemblyReg_EventSink_OnTextChanged,
	public CDisassemblyReg_EventSink_OnTabControl,
	public CDisassemblyReg_EventSink_OnEscControl
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

	class RegLineBuffer
	{
	public:
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

		HRESULT Init(HWND hWndParent, HDC hdc, HFONT hFont, int x, int y, int cpuid);
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

		int CurrentControlIndex;
		CArray<EdLn*> Controls;

		HRESULT ArrangeControls(HDC hdc, int x, int y, int cpuid);
		void ClearBuffer();

		void SelectControl(int i);
		void DeSelectControl(int i);
		void CancelEditing();
		void UpdateCaret(HWND hWnd, HDC hdc);
		void ClearCaret(HWND hWnd);
		bool ProcessChar(WPARAM wParam, LPARAM lParam);
		bool ProcessKeyDown(WPARAM wParam, LPARAM lParam);
		bool ProcessLButtonDown(WPARAM wParam, LPARAM lParam);

		EdLn *GetFocusedControl();
		int GetTabFirstControlIndex();
		int GetTabNextControlIndex();
		int GetTabPreviousControlIndex();
		int GetTabLastControlIndex();

		TEXTMETRIC TextMetric;

		int m_iShowCaretCount;
		HWND m_hWndParent;
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
private:
	CVirWindow *m_pParent;
	HFONT m_hFont;
	HDC m_hdc;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;
	IAppCommand *m_pAppCommand;

	RegLineBuffer m_RegBuffer;

	void UpdateBuffer(RegLineBuffer& b);
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
	void OnEscControl(void *sender, EdLnEscControlEventArgs& e);
	bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT AdviseEvents();
	void UnadviseEvents();
};

#endif