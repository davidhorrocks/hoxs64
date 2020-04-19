#include <windows.h>
#include <INITGUID.H>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "hexconv.h"
#include "CDPI.h"
#include "IC64.h"
#include "utils.h"
#include "StringConverter.h"
#include "dchelper.h"
#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpccli.h"
#include "resource.h"

const TCHAR WpcCli::ClassName[] = TEXT("WPCCLI");

WpcCli::WpcCli(IC64 *c64, IAppCommand *pIAppCommand, HFONT hFont)
{
	m_bIsTimerActive = false;
	m_bClosing = false;
	m_iCommandNumber = 0;
	m_hinstRiched = nullptr;
	m_hWndEdit = nullptr;
	m_wpOrigEditProc = nullptr;
	m_pIRichEditOle = nullptr;
	m_pITextDocument = nullptr;
	m_pRange = nullptr;
	m_hFont = nullptr;
	m_bstrFontName = nullptr;
	m_nCliFontHeight = 0;
	m_commandstate = Idle;
	m_cpumode = DBGSYM::CliCpuMode::C64;
	m_mapmemory = DBGSYM::CliMapMemory::VIEWCURRENT;
	m_iDebuggerMmuIndex = -1;
	this->c64 = c64;
	this->m_pIAppCommand = pIAppCommand;
	m_iDefaultAddress = GetDefaultCpuPcAddress();
	m_hFont = hFont;
	if (hFont)
	{
		TCHAR fontname[100];
		fontname[0] = 0;
		HDC hdc = ::CreateCompatibleDC(nullptr);
		if (hdc)
		{
			SelectObject(hdc, hFont);
			const BOOL br = GetTextFace(hdc, _countof(fontname), fontname);
			if (br)
			{
				m_bstrFontName = StringConverter::AllocBStr(fontname);
			}

			DeleteDC(hdc);
		}
	}
}

WpcCli::~WpcCli()
{
	if (m_bstrFontName)
	{
		SysFreeString(m_bstrFontName);
		m_bstrFontName = nullptr;
	}
}

void WpcCli::WindowRelease()
{
	this->keepAlive.reset();
}

HRESULT WpcCli::RegisterClass(HINSTANCE hInstance) noexcept
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	//wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(WPanel *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);//
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
}

HWND WpcCli::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

bit16 WpcCli::GetDefaultCpuPcAddress()
{
	IMonitorCpu *pCpu = c64->GetMon()->GetMainCpu();
	CPUState reg;
	pCpu->GetCpuState(reg);
	return reg.PC_CurrentOpcode;
}

void WpcCli::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return;	
	if (wParam == SIZE_MINIMIZED)
		return;

	int w = LOWORD(lParam);
	int h = HIWORD(lParam);

	if (w < 0)
		w = 0;
	if (h < 0)
		h = 0;
	if (m_hWndEdit)
		SetWindowPos(m_hWndEdit, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOZORDER);	
}

HRESULT WpcCli::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LRESULT lr;
HRESULT hr;
	RECT rcClient;
	ZeroMemory(&m_textmetric, sizeof(m_textmetric));

	m_hinstRiched = LoadLibrary(TEXT("Riched20.dll"));
	if (m_hinstRiched)
	{
		if (GetClientRect(hWnd, &rcClient))
		{
			m_hWndEdit = CreateWindowEx(0, RICHEDIT_CLASS, NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_HSCROLL | WS_VSCROLL, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWnd, (HMENU)1000, this->GetHinstance(), 0);
			if (m_hWndEdit)
			{
				lr = SendMessage(m_hWndEdit, EM_SETTYPOGRAPHYOPTIONS, TO_ADVANCEDTYPOGRAPHY, (LPARAM)TO_ADVANCEDTYPOGRAPHY);
				 
				lr = SendMessage(m_hWndEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&m_pIRichEditOle);
				if (lr)
				{
					hr = m_pIRichEditOle->QueryInterface(IID_ITextDocument, (void**)&m_pITextDocument);
					if (SUCCEEDED(hr))
					{
						SendMessage(m_hWndEdit, WM_SETFONT, (WPARAM)m_hFont, (LPARAM)TRUE);

						HDC hdc = ::CreateCompatibleDC(NULL);
						if (hdc)
						{
							DcHelper dch(hdc);
							dch.UseFont(m_hFont);
							dch.UseMapMode(MM_TEXT);
							BOOL br = GetTextMetrics(hdc, &m_textmetric);
							if (br)
							{
								m_nCliFontHeight = m_textmetric.tmHeight;
							}
							dch.Restore();
							dch.m_hdc = 0;
							DeleteDC(hdc);
						}

						m_wpOrigEditProc = this->SubclassChildWindow(m_hWndEdit);
						
						if (m_pRange)
						{
							m_pRange->Release();
							m_pRange = 0;
						}
						long iStart = 0;
						long iEnd = 0;
						long iLen;
						if (SUCCEEDED(m_pITextDocument->Range(iStart, iEnd, &m_pRange)))
						{
							m_pRange->Collapse(tomEnd);
							m_pRange->GetStoryLength(&iLen);
							if (iEnd>=iLen)
							{
								WriteCommandResponse(m_pRange, TEXT("\r"));
								m_pRange->Collapse(tomEnd);
							}
							else
							{
								WriteCommandResponse(m_pRange, TEXT("\r"));
								m_pRange->Collapse(tomStart);
							}
							m_pRange->Select();
							StartCommand(TEXT("r"));
						}
						return S_OK;
					}
				}
			}
		}
	}
	if (m_pITextDocument)
	{
		m_pITextDocument->Release();
		m_pITextDocument = 0;
	}
	if (m_pIRichEditOle)
	{
		m_pIRichEditOle->Release();
		m_pIRichEditOle = 0;
	}
	return E_FAIL;	
}

LRESULT WpcCli::OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, bool &handled)
{
	handled = false;
	return 0;
}

void WpcCli::OnCommandResultCompleted(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
RunCommandMapMemory *pRunCommandMapMemory;
RunCommandDisassembly *pRunCommandDisassembly;
RunCommandReadMemory *pRunCommandReadMemory;
EventArgs argChanged;
	if (m_bClosing)
		return;
	if (!m_pICommandResult)
		return;
	if (!lParam)
		return;
	if (m_pICommandResult.get() != (ICommandResult *)lParam)
		return;
	if (m_pICommandResult->GetId() != (int)wParam)
		return;
	if (m_pICommandResult->GetStatus() == DBGSYM::CliCommandStatus::CompletedOK)
	{
		const CommandToken *pToken = m_pICommandResult->GetToken();
		if (pToken)
		{
			DBGSYM::CliCpuMode::CliCpuMode cpumode = DBGSYM::CliCpuMode::C64;
			switch(pToken->cmd)
			{
			case DBGSYM::CliCommand::SelectCpu:
				if (pToken->bViewCurrent)
				{
					cpumode = this->m_cpumode;
				}
				else
				{
					cpumode = pToken->cpumode;
					this->m_cpumode = pToken->cpumode;
				}

				switch (cpumode)
				{
					case DBGSYM::CliCpuMode::C64:
						m_pICommandResult->AddLine(TEXT("C64\r"));
						break;
					case DBGSYM::CliCpuMode::Disk:
						m_pICommandResult->AddLine(TEXT("Disk\r"));
						break;
				}

				m_iDefaultAddress = GetDefaultCpuPcAddress();
				break;
			case DBGSYM::CliCommand::ClearScreen:
				this->m_pITextDocument->New();
				break;
			case DBGSYM::CliCommand::MapMemory:
				pRunCommandMapMemory = (RunCommandMapMemory *)m_pICommandResult->GetRunCommand();
				if (pRunCommandMapMemory)
				{
					if (!pRunCommandMapMemory->m_bViewDebuggerC64Mmu)
					{
						if (pRunCommandMapMemory->m_bSetDebuggerToFollowC64Mmu)
						{
							this->m_iDebuggerMmuIndex = -1;
						}
						else
						{
							this->m_iDebuggerMmuIndex = pRunCommandMapMemory->m_iMmuIndex;
						}
					}
				}

				break;
			case DBGSYM::CliCommand::Disassemble:
				pRunCommandDisassembly = (RunCommandDisassembly *)m_pICommandResult->GetRunCommand();
				if (pRunCommandDisassembly)
				{
					this->m_iDefaultAddress = pRunCommandDisassembly->m_currentAddress;
				}

				break;
			case DBGSYM::CliCommand::ReadMemory:
				pRunCommandReadMemory = (RunCommandReadMemory *)m_pICommandResult->GetRunCommand();
				if (pRunCommandReadMemory)
				{
					this->m_iDefaultAddress = pRunCommandReadMemory->m_currentAddress;
				}

				break;
			case DBGSYM::CliCommand::Assemble:
				argChanged = EventArgs();
				this->m_pIAppCommand->EsMemoryChanged.Raise(this, argChanged);
				break;
			case DBGSYM::CliCommand::WriteMemory:
				argChanged = EventArgs();
				this->m_pIAppCommand->EsMemoryChanged.Raise(this, argChanged);
				break;
			}
		}
	}

	if (m_pICommandResult)
	{
		m_pICommandResult->SetDataTaken();
	}
}

void WpcCli::OnTimer(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	if (m_bClosing)
	{
		return;
	}

	if (!m_pICommandResult)
	{
		return;
	}

	if (m_pICommandResult->GetId() != (int)wParam)
	{
		return;
	}

	ITextSelection *pSel = 0;
	if (m_pRange)
	{
		if (SUCCEEDED(m_pITextDocument->GetSelection(&pSel)))
		{
			long tsflags = 0;
			pSel->GetFlags(&tsflags);
			pSel->SetFlags((tsflags & (tomSelOvertype | tomSelReplace)) | tomSelStartActive);//tomSelAtEOL
		}
		else
		{
			pSel = 0;
		}
	}

	LPCTSTR pline = 0;
	if (m_pICommandResult->IsQuit())
	{
		WriteCommandResponse(m_pRange, TEXT("*break*\r"));
		m_pRange->Collapse(tomEnd);
		if (!isRangeInView(m_pRange))
		{
			m_pRange->ScrollIntoView(tomEnd);
		}

		m_pRange->Select();				
		StopCommand();
	}
	else
	{
		hr = m_pICommandResult->GetNextLine(&pline);
		if (SUCCEEDED(hr))
		{
			if (m_pRange)
			{
				WriteCommandResponse(m_pRange, pline);
				m_pRange->Collapse(tomEnd);
				if (!isRangeInView(m_pRange))
				{
					m_pRange->ScrollIntoView(tomEnd);
				}

				m_pRange->Select();				
			}
		}

		DBGSYM::CliCommandStatus::CliCommandStatus status = m_pICommandResult->GetStatus();
		switch (status)
		{
			case DBGSYM::CliCommandStatus::Finished:
				if (m_pICommandResult->CountUnreadLines() == 0)
				{
					m_pICommandResult->SetAllLinesTaken();
					StopCommand();
				}
				break;
		}
	}

	if (pSel)
	{
		pSel->Release();
		pSel = 0;
	}
}

bool WpcCli::isRangeInView(ITextRange const *pIRange)
{
	if (m_nCliFontHeight <= 0 || pIRange == NULL)
	{
		return false;
	}

	long nCurrentLine;
	long nTotalLines;
	long nFirstLine;
	long nLastLine;
	RECT rcTxt;
	
	nTotalLines = (long)SendMessage(this->m_hWndEdit, EM_GETLINECOUNT, 0, 0);
	nFirstLine = (long)SendMessage(this->m_hWndEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
	SendMessage(this->m_hWndEdit, EM_GETRECT, 0, (LPARAM)&rcTxt);
	nLastLine = nFirstLine + (rcTxt.bottom - rcTxt.top) / m_nCliFontHeight - 1;	
	if (nLastLine >= 0)
	{
		if (SUCCEEDED(m_pRange->GetIndex(tomLine, &nCurrentLine)))
		{
			if (nCurrentLine >= nFirstLine && nCurrentLine <= nLastLine)
			{
				return true;
			}
		}
	}

	return false;
}

void WpcCli::OnClose(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	m_bClosing = true;
}

void WpcCli::OnDestory(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_pICommandResult)
	{
		m_pICommandResult->SetHwnd(NULL);
	}

	StopCommand();
	if (m_hWndEdit)
	{
		if (m_wpOrigEditProc!=NULL)
		{
			SubclassChildWindow(m_hWndEdit, m_wpOrigEditProc);
			m_wpOrigEditProc= NULL;
		}
	}

	if (m_pRange)
	{
		m_pRange->Release();
		m_pRange = 0;
	}

	if (m_pITextDocument)
	{
		m_pITextDocument->Release();
		m_pITextDocument = 0;
	}

	if (m_pIRichEditOle)
	{
		m_pIRichEditOle->Release();
		m_pIRichEditOle = 0;
	}
}

LRESULT WpcCli::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LPNMHDR pnmh;
LRESULT lr;
bool bHandled;
int wmId, wmEvent;

	switch (uMsg)
	{
	case WM_CREATE:
		if (SUCCEEDED(OnCreate(hWnd, uMsg, wParam, lParam)))
		{
			return 0;
		}
		else
		{
			return -1;
		}

	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_NOTIFY:
		pnmh = (LPNMHDR)lParam;
		if (pnmh != NULL)
		{
			lr = OnNotify(hWnd, (int)wParam, pnmh, bHandled);
			if (bHandled)
			{
				return lr;
			}
		}

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		break;
	case WM_TIMER:
		OnTimer(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_COMMANDRESULT_COMPLETED:
		OnCommandResultCompleted(m_hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_CLOSE:
		OnClose(m_hWnd, uMsg, wParam, lParam);
		break;
	case WM_DESTROY:
		OnDestory(m_hWnd, uMsg, wParam, lParam);
		break;
	}
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}


LRESULT WpcCli::SubclassWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
short nVirtKey;
	if (hWnd == m_hWndEdit)
	{
		switch(uMsg)
		{
		case WM_CHAR:
			if (this->m_commandstate == Busy || m_pIAppCommand->IsRunning())
			{
				return 0;
			}
			break;
		case WM_KEYDOWN:
			if (wParam == VK_RETURN)
			{
				nVirtKey = GetKeyState(VK_SHIFT); 
				if (nVirtKey>=0)
				{
					OnCommandEnterKey();
					return 0;
				}
			}
			else if (wParam == VK_ESCAPE)
			{
				if (this->m_commandstate == Busy)
				{
					if (this->m_pICommandResult)
					{
						this->m_pICommandResult->Quit();
					}
					return 0;
				}
			}
			break;
		}
	}
	if (m_wpOrigEditProc)
	{
		return ::CallWindowProc(m_wpOrigEditProc, hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return 0;
	}
}


HRESULT WpcCli::StartCommand(LPCTSTR pszCommand)
{
HRESULT hr;
	StopCommand();
	m_iCommandNumber++;
	hr = c64->GetMon()->BeginExecuteCommandLine(this->GetHwnd(), pszCommand, this->m_iCommandNumber, m_cpumode, m_iDebuggerMmuIndex, m_iDefaultAddress, &m_pICommandResult);
	if (SUCCEEDED(hr))
	{		
		UINT_PTR t = SetTimer(m_hWnd, m_pICommandResult->GetId(), 30, NULL);
		if (t)
		{
			m_bIsTimerActive = true;
			m_commandstate = Busy;
		}	
	}
	else
	{
		m_commandstate = Idle;
	}
	return hr;
}

void WpcCli::StopCommand()
{
	if (m_pICommandResult)
	{
		if (m_bIsTimerActive)
		{
			KillTimer(GetHwnd(), m_pICommandResult->GetId());
			m_bIsTimerActive = false;
		}

		m_pICommandResult->Quit();
		m_pICommandResult->WaitFinished(INFINITE);
	}

	m_commandstate = Idle;
}

void WpcCli::OnCommandEnterKey()
{
long cb;
LPTSTR ps = NULL;
long iStart = 0;
long iEnd = 0;
long iLen = 0;
	if (m_commandstate == Busy)
	{
		return;
	}

	if (m_pIAppCommand->IsRunning())
	{
		return;
	}

	if (SUCCEEDED(GetCurrentParagraphText(NULL, &cb, NULL, NULL)))
	{
		ps = (LPTSTR)malloc(cb);
		if (ps)
		{
			if (SUCCEEDED(GetCurrentParagraphText(ps, NULL, &iStart, &iEnd)))
			{
				if (cb > 0 && iEnd-iStart > 0)
				{
					if (m_pRange)
					{
						m_pRange->Release();
						m_pRange = 0;
					}
					ITextSelection *pSel = 0;
					if (SUCCEEDED(m_pITextDocument->GetSelection(&pSel)))
					{
						long tsflags = 0;
						pSel->GetFlags(&tsflags);
						pSel->SetFlags((tsflags & (tomSelOvertype | tomSelReplace)) | tomSelStartActive);//tomSelAtEOL

						if (SUCCEEDED(m_pITextDocument->Range(iStart, iEnd, &m_pRange)))
						{
							m_pRange->Collapse(tomEnd);
							m_pRange->GetStoryLength(&iLen);
							if (iEnd>=iLen)
							{
								WriteCommandResponse(m_pRange, TEXT("\r"));
								m_pRange->Collapse(tomEnd);
							}
							else
							{
								WriteCommandResponse(m_pRange, TEXT("\r"));
								m_pRange->Collapse(tomStart);
							}
							m_pRange->Select();

							if (!G::IsStringBlank(ps))
							{
								if (FAILED(StartCommand(ps)))
								{
									WriteCommandResponse(m_pRange, TEXT("Command failed to start.\r"));
									m_pRange->Collapse(tomEnd);
									m_pRange->Select();
								}
							}
						}

						pSel->Release();
						pSel = 0;
					}
				}
			}

			free(ps);
			ps = NULL;
		}
	}
}

HRESULT WpcCli::SetCharInsertionPoint(long iCharIndex)
{
ITextSelection *pSel = 0;
	if (SUCCEEDED(m_pITextDocument->GetSelection(&pSel)))
	{
		pSel->SetRange(iCharIndex, iCharIndex);
		pSel->Release();
	}

	return S_OK;
}

HRESULT WpcCli::WriteCommandResponse(ITextRange *pRange, LPCTSTR pText)
{
	if (!pRange || !pText)
	{
		return E_POINTER;
	}

	BSTR bstr;
	bstr = StringConverter::AllocBStr(pText);
	if (bstr)
	{
		pRange->SetText(bstr);
		SysFreeString(bstr);
	}

	return S_OK;
}

HRESULT WpcCli::WriteCommandResponse(long iCharIndex, LPCTSTR pText)
{
ITextRange * pRange = nullptr;
HRESULT hr;

	hr = m_pITextDocument->Range(iCharIndex, iCharIndex, &pRange);
	if (SUCCEEDED(hr))
	{
		hr = WriteCommandResponse(pRange, pText);
		pRange->Release();
	}

	return hr;
}

HRESULT WpcCli::GetCurrentParagraphText(LPTSTR psBuffer, long *pcchBuffer, long *piStartCharIndex, long *piEndCharIndex)
{
ITextSelection *pSel = 0;
ITextRange *pRange = 0;
long iStart = 0;
long iEnd = 0;
long cb;
HRESULT hrRet = E_FAIL;
long k;
	if (SUCCEEDED(m_pITextDocument->GetSelection(&pSel)))
	{
		if (SUCCEEDED(pSel->GetDuplicate(&pRange)))
		{		
			bool ok;
			for (ok = false; !ok ; ok = true)
			{
				if (FAILED(pRange->StartOf(tomParagraph, 0, &k)))
				{
					break;
				}

				if (FAILED(pRange->MoveEnd(tomParagraph, 1, &k)))
				{
					break;
				}
			}

			if (ok)
			{
				if (SUCCEEDED(pRange->GetStart(&iStart)))
				{
					if (SUCCEEDED(pRange->GetEnd(&iEnd)))
					{
						if (piStartCharIndex)
							*piStartCharIndex = iStart; 
						if (piEndCharIndex)
							*piEndCharIndex = iEnd; 
						cb = (iEnd - iStart + 1) * sizeof(TCHAR);
						if (psBuffer)
						{
							TEXTRANGE tr;
							ZeroMemory(&tr, sizeof(tr));
							tr.chrg.cpMin = iStart;
							tr.chrg.cpMax = iEnd;
							tr.lpstrText = psBuffer;
							long c = (long)SendMessage(this->m_hWndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
							if (pcchBuffer)
							{
								*pcchBuffer = c;
							}

							if (c<=0)
							{
								psBuffer[0] = 0;
							}
						}
						else
						{
							if (pcchBuffer)
							{
								*pcchBuffer = cb;
							}
						}

						hrRet = S_OK;
					}
				}
			}

			pRange->Release();
		}

		pSel->Release();
	}
	return hrRet;
}
