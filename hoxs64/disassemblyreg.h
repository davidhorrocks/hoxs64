#ifndef __DISSASSEMBLYREG_H__
#define __DISSASSEMBLYREG_H__

class CDisassemblyReg : public CVirWindow
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
		HRESULT Init(HDC hdc, HFONT hFont, int x, int y, int cpuid);
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

		void UpdateCaret(HWND hWnd);
		void ClearCaret(HWND hWnd);

		TEXTMETRIC TextMetric;

		int m_iShowCaretCount;

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
};

#endif