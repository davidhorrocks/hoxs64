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

class CDisassemblyReg_EventSink : 
	public CDisassemblyReg_EventSink_OnTextChanged
{
};


class CDisassemblyReg : public CVirWindow, public CDisassemblyReg_EventSink
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

	struct RegLineBuffer
	{
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

		int CurrentControlIndex;
		CArray<EdLn*> Controls;

		HRESULT ArrangeControls(HDC hdc, int x, int y, int cpuid);
		void ClearBuffer();

		void SelectControl(int i);
		void DeSelectControl(int i);

		void UpdateCaret(HWND hWnd, HDC hdc);
		void ClearCaret(HWND hWnd);
		bool ProcessChar(WPARAM wParam, LPARAM lParam);
		bool ProcessKeyDown(WPARAM wParam, LPARAM lParam);

		TEXTMETRIC TextMetric;

		int m_iShowCaretCount;
		HWND hWndParent;
	};
	CDisassemblyReg();
	virtual ~CDisassemblyReg();

	static TCHAR ClassName[];

	HRESULT Init(CVirWindow *parent, IMonitorCommand *monitorCommand, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID);
	void GetMinWindowSize(int &w, int &h);
	void UpdateDisplay();
	void InvalidateBuffer();

private:
	CVirWindow *m_pParent;
	HFONT m_hFont;
	HDC m_hdc;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;

	IMonitorCpu *m_cpu;
	IMonitorVic *m_vic;
	Monitor m_mon;
	IMonitorCommand *m_monitorCommand;

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

	HRESULT AdviseEvents();
	void UnadviseEvents();
};

#endif