#pragma once
#include "cvirwindow.h"
#include "errormsg.h"
#include "c64Keys.h"

#define C_PAGES 1

#define WM_KEYCAPTURE (WM_USER)
#define WM_KEYCLEAR (WM_USER + 1)

enum ekeycontrolmode
{
	kcs_display=0,
	kcs_getkey=1
};

struct keycontrolstate
{
	int control_id;
	enum ekeycontrolmode state;
	HWND hwnd;
	TCHAR text[30];
	BOOL bGotFocus;
};

extern LRESULT CALLBACK GetKeyPressWindowProc(
	HWND hwnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
);

class CDiagKeyboard : public CTabDialog, public ErrorMsg
{
public:
	virtual void loadconfig(const CConfig *);
	virtual void saveconfig(CConfig *);
	CConfig newCfg;
	const CConfig *currentCfg;
	HRESULT Init(CDX9 *dx, const CConfig *);

private:
	void UpdatePage(int pageno, HWND hwndDlg);
	void UpdatePage1(HWND hwndDlg);
	void UpdatePage2(HWND hwndDlg);
	void UpdatePage3(HWND hwndDlg);
	void UpdatePage4(HWND hwndDlg);
	HRESULT initkeycapturecontrols(int pageno, HWND hwndDlg);
	HRESULT initkeycapturecontrols1(HWND hwndDlg);
	HRESULT initkeycapturecontrols2(HWND hwndDlg);
	HRESULT initkeycapturecontrols3(HWND hwndDlg);
	HRESULT initkeycapturecontrols4(HWND hwndDlg);
	void SetDefaultFocusForPage(int pageno, HWND hwndDlg);
	void ResetKeyCapture();
	void SetKeyCapture(int c64key);
	BYTE keymap[256];
	struct keycontrolstate keycontrol[C64Keys::C64K_COUNTOFKEYS];
	int m_current_c64key;
	bool m_bKeyCapture;
	int	m_c64key;
	UINT m_scancode;
	BYTE m_KeyboardState[256];
	TCHAR m_szkeyname[50];

	HRESULT Acquire();
	void Unacquire();
	void SetAcquire();
	HRESULT ReadScanCode(LPUINT scancode, LPBYTE keyboardState, SIZE_T sizeKeyboardState);
	void AssignKey(int label, int c64key, UINT scancode);
	static void GetKeyName(UINT scancode, TCHAR* buffer, int cchBuffersize, LPBYTE keyboardState = nullptr);
	bool m_b_scanningkey;
	bool m_bActive;
	bool m_bBeginKeyScan;

	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual BOOL OnPageEvent(CTabPageDialog *page, HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	virtual LRESULT OnEventKeyControl(
		HWND hwnd,      // handle to window
		UINT uMsg,      // message identifier
		WPARAM wParam,  // first message parameter
		LPARAM lParam   // second message parameter
		);
	void clear_keypress_contols();
	friend LRESULT CALLBACK ::GetKeyPressWindowProc(
		HWND hwnd,      // handle to window
		UINT uMsg,      // message identifier
		WPARAM wParam,  // first message parameter
		LPARAM lParam   // second message parameter
		);
	CDX9 *pDX;
};
