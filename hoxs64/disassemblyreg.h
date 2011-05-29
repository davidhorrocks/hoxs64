#ifndef __DISSASSEMBLYREG_H__
#define __DISSASSEMBLYREG_H__

class CDisassemblyReg : public CVirWindow
{
public:
	static const int MARGIN_TOP = 0;
	static const int PADDING_LEFT = 4;
	static const int PADDING_RIGHT = 4;
	static const int PADDING_TOP = 4;
	static const int PADDING_BOTTOM = 4;

	struct RegLineBuffer
	{
		TCHAR PC_Text[Monitor::BUFSIZEWORDTEXT];
		TCHAR A_Text[Monitor::BUFSIZEBYTETEXT];	
		TCHAR X_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Y_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR SR_Text[Monitor::BUFSIZEBITBYTETEXT];
		TCHAR SP_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Ddr_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Data_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Output_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Input_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR VicLine_Text[Monitor::BUFSIZEWORDTEXT];
		TCHAR VicCycle_Text[Monitor::BUFSIZEBYTETEXT];
		TCHAR Mmu_Text[Monitor::BUFSIZEMMUTEXT];

		void ClearBuffer();
	};
	CDisassemblyReg();
	virtual ~CDisassemblyReg();

	static TCHAR ClassName[];

	HRESULT Init(CVirWindow *parent, IMonitorEvent *monitorEvent, IMonitorCpu *cpu, IMonitorVic *vic, HFONT hFont);

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID);
	void GetMinWindowSize(int &w, int &h);
	void UpdateDisplay();
	void InvalidateBuffer();

private:
	CVirWindow *m_pParent;
	HFONT m_hFont;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;

	IMonitorCpu *m_cpu;
	IMonitorVic *m_vic;
	Monitor m_mon;
	IMonitorEvent *m_monitorEvent;

	RegLineBuffer m_TextBuffer;

	void UpdateBuffer(RegLineBuffer& b);
	HRESULT OnCreate(HWND hWnd);
	void DrawDisplay(HWND hWnd, HDC hdc);
	void DrawDisplay2(HWND hWnd, HDC hdc);

	void Cleanup();
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif