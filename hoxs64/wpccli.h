#ifndef __WPCCLI_H__
#define __WPCCLI_H__

#include <RichEdit.h>
#include <Richole.h>
#include <Tom.h>

DEFINE_GUID(IID_ITextDocument,0x8CC497C0,0xA1DF,0x11CE,0x80,0x98, 0x00,0xAA,0x00,0x47,0xBE,0x5D);

class WpcCli : public CVirWindow
{
public:
	WpcCli(IC64 *c64, IAppCommand *pIAppCommand, HFONT hFont);
	virtual ~WpcCli();

	static const TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	HRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, bool &handled);
	void OnDestory(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnCommandResultCompleted(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnTimer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	LRESULT SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnCommandEnterKey();
private:
	enum CommandState
	{
		Idle,
		Busy
	};
	bool m_bClosing;
	CommandState m_commandstate;
	DBGSYM::CliCpuMode::CliCpuMode m_cpumode;
	int m_iCommandNumber;
	shared_ptr<ICommandResult> m_pICommandResult;
	bool m_bIsTimerActive;
	CDPI m_dpi;
	IC64 *c64;
	IAppCommand *m_pIAppCommand;
	bool m_bHexDisplay;
	HMODULE m_hinstRiched;
	HWND m_hWndEdit;
	WNDPROC m_wpOrigEditProc;
	IRichEditOle *m_pIRichEditOle;
	ITextDocument *m_pITextDocument;
	ITextRange *m_pRange;
	HFONT m_hFont;
	BSTR m_bstrFontName;

	HRESULT StartCommand(LPCTSTR pszCommand);
	void StopCommand();
	HRESULT GetCurrentParagraphText(LPTSTR psBuffer, long *pcchBuffer, long *piStartCharIndex, long *piEndCharIndex);
	HRESULT SetCharInsertionPoint(long iCharIndex);
	HRESULT WriteCommandResponse(long iCharIndex, LPCTSTR pText);
	HRESULT WriteCommandResponse(ITextRange *pRange, LPCTSTR pText);	
};

#endif