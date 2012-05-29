#include <windows.h>
#include <INITGUID.H>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
//include "hexconv.h"
#include "C64.h"

#include "utils.h"

#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpccli.h"
#include "resource.h"

const TCHAR WpcCli::ClassName[] = TEXT("WPCCLI");

WpcCli::WpcCli(C64 *c64, IAppCommand *pIAppCommand)
{
	m_hinstRiched = NULL;
	m_hWndEdit = NULL;
	m_wpOrigEditProc = NULL;
	m_pIRichEditOle = NULL;
	m_pITextDocument = NULL;
	this->c64 = c64;
	this->m_pIAppCommand = pIAppCommand;
}

WpcCli::~WpcCli()
{
}

HRESULT WpcCli::RegisterClass(HINSTANCE hInstance)
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

	m_hinstRiched = LoadLibrary(TEXT("Riched20.dll"));
	if (m_hinstRiched)
	{
		if (GetClientRect(hWnd, &rcClient))
		{
			m_hWndEdit = CreateWindowEx(0, RICHEDIT_CLASS, NULL, WS_CHILD | WS_VISIBLE | ES_MULTILINE, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, hWnd, (HMENU)1000, this->GetHinstance(), 0);
			if (m_hWndEdit)
			{
				lr = SendMessage(m_hWndEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&m_pIRichEditOle);
				if (lr)
				{
					hr = m_pIRichEditOle->QueryInterface(IID_ITextDocument, (void**)&m_pITextDocument);
					if (SUCCEEDED(hr))
					{
						m_wpOrigEditProc = this->SubclassChildWindow(m_hWndEdit);
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

void WpcCli::OnDestory(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_hWndEdit)
	{
		if (m_wpOrigEditProc!=NULL)
		{
			SubclassChildWindow(m_hWndEdit, m_wpOrigEditProc);
			m_wpOrigEditProc= NULL;
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
			return 0;
		else
			return -1;
	case WM_SIZE:
		OnSize(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_NOTIFY:
		pnmh = (LPNMHDR)lParam;
		if (pnmh != NULL)
		{
			lr = OnNotify(hWnd, (int)wParam, pnmh, bHandled);
			if (bHandled)
				return lr;
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		//switch (wmId) 
		//{
		//case IDM_BREAKPOINTOPTIONS_SHOWASSEMBLY:
		//	return 0;
		//}
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
		//case WM_CHAR:
		//	if (wParam == VK_ESCAPE)
		//	{				
		//		return 0;
		//	}
		//	else if (wParam == VK_RETURN)
		//	{				
		//		return 0;
		//	}
		//	break;
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

void WpcCli::OnCommandEnterKey()
{
long cb;
LPTSTR ps = NULL;
	if (SUCCEEDED(GetCurrentParagraphText(NULL, &cb)))
	{
		ps = (LPTSTR)malloc(cb);
		if (ps)
		{
			if (SUCCEEDED(GetCurrentParagraphText(ps, &cb)))
			{
				MessageBox(GetHwnd(), ps, TEXT("test"), MB_OK);
				SetFocus(m_hWndEdit);
			}

			free(ps);
			ps = NULL;
		}
	}
}

HRESULT WpcCli::GetCurrentParagraphText(LPTSTR psBuffer, long *pcch)
{
ITextSelection *pSel;
ITextRange *pRange;
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
				if (FAILED(pRange->Move(tomParagraph, -1, &k)))
					break;
				if (FAILED(pRange->MoveEnd(tomParagraph, 1, &k)))
					break;
			}
			if (ok)
			{
				if (SUCCEEDED(pRange->GetStart(&iStart)))
				{
					if (SUCCEEDED(pRange->GetEnd(&iEnd)))
					{
						cb = (iEnd - iStart + 1) * sizeof(TCHAR);
						if (psBuffer)
						{
							TEXTRANGE tr;
							ZeroMemory(&tr, sizeof(tr));
							tr.chrg.cpMin = iStart;
							tr.chrg.cpMax = iEnd;
							tr.lpstrText = psBuffer;
							long c = (long)SendMessage(this->m_hWndEdit, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
							if (pcch)
								*pcch = c;
							if (c<=0)
								psBuffer[0] = 0;
						}
						else
						{
							if (pcch)
								*pcch = cb;
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
