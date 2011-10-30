#ifndef __DISSASSEMBLYEDITCHILD_H__
#define __DISSASSEMBLYEDITCHILD_H__

class CDisassemblyEditChild : public CVirWindow
{
public:
	static const int WIDTH_LEFTBAR = 16;
	int WIDTH_LEFTBAR2;
	int LINE_HEIGHT;
	static const int PADDING_LEFT = 4;
	static const int PADDING_RIGHT = 4;
	static const int PADDING_TOP = 4;
	static const int PADDING_BOTTOM = 4;
	static const int MARGIN_TOP = 5;
	static const int TAB_ADDRESS = 0;
	static const int TAB_BYTES = 6;
	static const int TAB_MNEMONIC = 6+10;

	static const int ID_EDITDISASSEMBLY = 2000;

	struct AssemblyLineBuffer
	{
		int index;
		bit16 Address;
		bit8 InstructionSize;
		TCHAR AddressText[Monitor::BUFSIZEADDRESSTEXT];
		TCHAR BytesText[Monitor::BUFSIZEINSTRUCTIONBYTESTEXT];
		TCHAR MnemonicText[Monitor::BUFSIZEMNEMONICTEXT];
		MEM_TYPE AddressReadAccessType;
		RECT MnemonicRect;
		bool IsUnDoc;
		bool IsPC;
		bool IsBreak;
		bool IsFocused;
		bool WantUpdate;
		int InstructionCycle;
		bool IsValid;

		AssemblyLineBuffer();
		bool GetIsReadOnly();
		void Clear();
		void WriteDisassemblyString(TCHAR *pszBuffer, int cchBuffer);
		bool IsEqual(AssemblyLineBuffer& other);
	};

	CDisassemblyEditChild();
	virtual ~CDisassemblyEditChild();
	static TCHAR ClassName[];
	HRESULT Init(CVirWindow *parent, IMonitorCommand *monitorCommand, Monitor *pMon, HFONT hFont);
	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Create(HINSTANCE hInstance, HWND parent, int x,int y, int w, int h, HMENU ctrlID);

	void InvalidateBuffer();
	void UpdateDisplay(bool bSeekPC);
	void UpdateBuffer(bool bEnsurePC);
	void SetTopAddress(bit16 address);
	bit16 GetNearestTopAddress(bit16 address);
	bit16 GetNthAddress(bit16 startaddress, int linenumber);
	bit16 GetTopAddress();
	bit16 GetBottomAddress();
	bit16 GetBottomAddress(int offset);
	bit16 GetNextAddress();
	bit16 GetPrevAddress();
	void GetMinWindowSize(int &w, int &h);
	void SetHome();
	void CancelAsmEditing();
	HRESULT SaveAsmEditing();

	static const int BUFFER_WIDTH = 50;
	static const int MAX_BUFFER_HEIGHT = 200;
private:
	CVirWindow *m_pParent;
	IMonitorCommand *m_monitorCommand;
	Monitor *m_pMon;
	HFONT m_hFont;
	bit16 m_FirstAddress;
	int m_NumLines;
	bool m_MinSizeDone;
	int m_MinSizeW;
	int m_MinSizeH;
	HBITMAP m_hBmpBreak;
	bool m_bHasLastDrawText;
	RECT m_rcLastDrawText;
	bit16 m_iFocusedAddress;
	bool m_bIsFocusedAddress;
	bool m_bMouseDownOnFocusedAddress;
	HWND m_hWndEditText;
	WNDPROC m_wpOrigEditProc;
	AssemblyLineBuffer *m_CurrentEditLineBuffer;
	bool m_bIsFocused;
	int xcol_Address;
	int xcol_Bytes;
	int xcol_Mnemonic;

	
	AssemblyLineBuffer *m_pFrontTextBuffer;
	AssemblyLineBuffer *m_pBackTextBuffer;

	void Cleanup();
	HWND CreateAsmEdit(HWND hWndParent);
	HRESULT AllocTextBuffer();
	void FreeTextBuffer();
	void ClearBuffer();
	void FlipBuffer();
	void UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, int numLines, int iEnsureAddress);
	void UpdateBuffer(AssemblyLineBuffer *assemblyLineBuffer, bit16 address, int startLine, int numLines);
	void InvalidateFocus();
	void InvalidateRectChanges();
	int GetLineFromYPos(int y);
	void DrawDisplay(HWND hWnd, HDC hdc);
	void DrawDisplay2(HWND hWnd, HDC hdc);
	int GetNumberOfLines(RECT& rc, int lineHeight);
	bool GetFocusedAddress(bit16 *address);
	void SetFocusedAddress(bit16 address);
	void ClearFocusedAddress();	
	void MoveNextLine();
	void GetRect_Bar(const RECT& rcClient, LPRECT rc);
	void GetRect_Status(const RECT& rcClient, LPRECT rc);
	void GetRect_Edit(const RECT& rcClient, LPRECT rc);
	void ShowEditMnemonic(AssemblyLineBuffer *pAlb);
	void HideEditMnemonic();
	bool IsEditing();
	HRESULT UpdateMetrics();
	HRESULT OnCreate(HWND hWnd);
	bool OnCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnKeyDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnChar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool OnNotify(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnVScroll(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnEditFocusedMnemonic();

	virtual LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif
