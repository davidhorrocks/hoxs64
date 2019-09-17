#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include <winuser.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "c64keys.h"
#include "diagkeyboard.h"
#include "resource.h"

#define IDT_TIMER1 1001

CDiagKeyboard::CDiagKeyboard()
{
}

CDiagKeyboard::~CDiagKeyboard()
{
}

HRESULT CDiagKeyboard::Init(CDX9 *pDX, const CConfig *currentCfg)
{
	ClearError();
	clear_keypress_contols();
	CDiagKeyboard::currentCfg = currentCfg;

	newCfg = *currentCfg;

	CDiagKeyboard::pDX = pDX;
	return S_OK;
}

void CDiagKeyboard::loadconfig(const CConfig *cfg)
{
	CopyMemory(&keymap[0], &cfg->m_KeyMap[0], sizeof(keymap));
}

void CDiagKeyboard::saveconfig(CConfig *cfg)
{
	CopyMemory(&cfg->m_KeyMap[0], &keymap[0], sizeof(cfg->m_KeyMap));
}

BOOL CDiagKeyboard::DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
HRESULT hr;
LPNMHDR lpnmhdr;
	switch (message) 
	{ 
	case WM_INITDIALOG:
		m_b_scanningkey = false;
		m_bBeginKeyScan = false;
		m_bKeyCapture = false;
        SetTimer( hwndDlg, IDT_TIMER1, 1000 / 15, NULL );
		hr = pDX->pKeyboard->Unacquire();
		hr = pDX->pKeyboard->SetCooperativeLevel(hwndDlg, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
		if (FAILED(hr))
		{
			return FALSE;
		}

		loadconfig(&newCfg);
		return OnTabbedDialogInit(hwndDlg);
    case WM_ACTIVATE:	
        if( WA_INACTIVE == wParam )
		{
            m_bActive = false;
		}
        else
		{
            m_bActive = true;
		}

        SetAcquire();
        return 0;
	case WM_DESTROY:
        KillTimer(hwndDlg, IDT_TIMER1);
		return TRUE;
	case WM_TIMER:
		if (wParam == IDT_TIMER1)
		{
			if (!m_bKeyCapture)
			{
				return 0;
			}

			if (m_bBeginKeyScan)
			{
				m_b_scanningkey=false;
				m_bBeginKeyScan=false;
			}

			if (m_b_scanningkey)
			{
				hr = ReadScanCode(&m_scancode);
				if (hr == S_OK)
				{
					lstrcpy(m_szkeyname, TEXT(""));
					GetKeyName(m_scancode, m_szkeyname, sizeof(m_szkeyname));
					AssignKey(keycontrol[m_current_c64key].control_id,m_current_c64key);
					SendMessage(keycontrol[m_current_c64key].hwnd, WM_KEYCAPTURE, FALSE, 0);
					m_bKeyCapture = false;
				}
				else
				{
					if (FAILED(hr))
					{
						SendMessage(keycontrol[m_current_c64key].hwnd, WM_KEYCAPTURE, FALSE , 0);
					}
				}
			}
			else
			{
				hr = ReadScanCode(&m_scancode);
				if (hr == S_FALSE)
				{
					m_b_scanningkey = true;
				}
				else if (FAILED(hr))
				{
					SetAcquire();
				}
			}

			return TRUE;
		}
		break;
	case WM_NOTIFY: 
		if (lParam == 0)
		{
			return FALSE;
		}

		lpnmhdr = (LPNMHDR) lParam;
		if (lpnmhdr->hwndFrom == m_hwndTab)
		{
			switch (lpnmhdr->code) 
			{ 
			case TCN_SELCHANGING: 
				return FALSE;
			case TCN_SELCHANGE: 
				OnSelChanged(hwndDlg);
				ResetKeyCapture();
				UpdatePage(m_current_page_index, hwndDlg);
				SetDefaultFocusForPage(m_current_page_index, hwndDlg);
				return TRUE;
			}
		} 
		break; 
	case WM_COMMAND: 
		switch (LOWORD(wParam))
		{
		case IDOK:
			saveconfig(&newCfg);
			EndDialog(hwndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		}
	}

	return FALSE;
}

void CDiagKeyboard::AssignKey(int label, int c64key)
{
int i;
BOOL bReplace;
	keymap[c64key] = m_scancode;			
	if (keymap[c64key] != 0)
	{
		bReplace=FALSE;
		for (i = 0; i<256; i++)
		{
			if (i!=c64key && keymap[i] == m_scancode)
			{
				keymap[i] =0;
				bReplace=TRUE;
				break;
			}
		}
	}

	if (IsWindow(m_hwndDisplay))
	{
		if (bReplace)
		{
			UpdatePage(m_current_page_index, m_hwndDisplay);
		}
		else
		{
			if (keymap[c64key]==0)
			{
				SetDlgItemText(m_hwndDisplay, label, TEXT(""));
			}
			else
			{
				SetDlgItemText(m_hwndDisplay, label, m_szkeyname);
			}
		}
	}
}

void CDiagKeyboard::SetDefaultFocusForPage(int pageno, HWND hwndDlg)
{
HWND hWnd = 0;
	switch (pageno)
	{
	case 0:
		hWnd = keycontrol[C64Keys::C64K_ARROWLEFT].hwnd;
		break;
	case 1:
		hWnd = keycontrol[C64Keys::C64K_A].hwnd;
		break;
	case 2:
		hWnd = keycontrol[C64Keys::C64K_1].hwnd;
		break;
	case 3:
		hWnd = keycontrol[C64Keys::C64K_JOY1FIRE].hwnd;
		break;
	}

	if (hWnd)
	{
		if (IsWindowVisible(hWnd))
		{
			SetFocus(hWnd);
		}
	}
}

void CDiagKeyboard::UpdatePage(int pageno, HWND hwndDlg)
{
	switch (pageno)
	{
	case 0:
		UpdatePage1(hwndDlg);
		break;
	case 1:
		UpdatePage2(hwndDlg);
		break;
	case 2:
		UpdatePage3(hwndDlg);
		break;
	case 3:
		UpdatePage4(hwndDlg);
		break;
	}
}

void CDiagKeyboard::UpdatePage1(HWND hwndDlg)
{
TCHAR szBuffer[30];

	GetKeyName(keymap[C64Keys::C64K_HOME], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_HOME, szBuffer);	

	GetKeyName(keymap[C64Keys::C64K_ASTERISK], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_ASTERISK, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_SLASH], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_SLASH, szBuffer);
	
	GetKeyName(keymap[C64Keys::C64K_ARROWUP], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_ARROWUP, szBuffer);
	
	GetKeyName(keymap[C64Keys::C64K_ARROWLEFT], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_ARROWLEFT, szBuffer);
	
	GetKeyName(keymap[C64Keys::C64K_CONTROL], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_CONTROL, szBuffer);
	
	GetKeyName(keymap[C64Keys::C64K_STOP], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_STOP, szBuffer);
	
	GetKeyName(keymap[C64Keys::C64K_COMMODORE], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_COMMODORE, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_DEL], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_DEL, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_PLUS], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_PLUS, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_MINUS], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_MINUS, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_EQUAL], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_EQUAL, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_POUND], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_POUND, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_AT], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_AT, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_COLON], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_COLON, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_SEMICOLON], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_SEMICOLON, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_CURSORRIGHT], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_CURSORRIGHT, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_CURSORDOWN], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_CURSORDOWN, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_LEFTSHIFT], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_LEFTSHIFT, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_RIGHTSHIFT], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_RIGHTSHIFT, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_RESTORE], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_RESTORE, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_SPACE], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_SPACE, szBuffer);

	GetKeyName(keymap[C64Keys::C64K_RETURN], szBuffer, sizeof(szBuffer));
	SetDlgItemText(hwndDlg, IDC_TXT_RETURN, szBuffer);
}


#define _updatekeypage(n) GetKeyName(keymap[C64Keys::C64K_##n], szBuffer, sizeof(szBuffer));\
	SetDlgItemText(hwndDlg, IDC_TXT_##n, szBuffer);

void CDiagKeyboard::UpdatePage2(HWND hwndDlg)
{
TCHAR szBuffer[30];

	_updatekeypage(A);
	_updatekeypage(B);
	_updatekeypage(C);
	_updatekeypage(D);
	_updatekeypage(E);
	_updatekeypage(F);
	_updatekeypage(G);
	_updatekeypage(H);
	_updatekeypage(I);
	_updatekeypage(J);
	_updatekeypage(K);
	_updatekeypage(L);
	_updatekeypage(M);
	_updatekeypage(N);
	_updatekeypage(O);
	_updatekeypage(P);
	_updatekeypage(Q);
	_updatekeypage(R);
	_updatekeypage(S);
	_updatekeypage(T);
	_updatekeypage(U);
	_updatekeypage(V);
	_updatekeypage(W);
	_updatekeypage(X);
	_updatekeypage(Y);
	_updatekeypage(Z);
}

void CDiagKeyboard::UpdatePage3(HWND hwndDlg)
{
TCHAR szBuffer[30];

	_updatekeypage(1);
	_updatekeypage(2);
	_updatekeypage(3);
	_updatekeypage(4);
	_updatekeypage(5);
	_updatekeypage(6);
	_updatekeypage(7);
	_updatekeypage(8);
	_updatekeypage(9);
	_updatekeypage(0);
	_updatekeypage(F1);
	_updatekeypage(F2);
	_updatekeypage(F3);
	_updatekeypage(F4);
	_updatekeypage(F5);
	_updatekeypage(F6);
	_updatekeypage(F7);
	_updatekeypage(F8);
	_updatekeypage(CURSORUP);
	_updatekeypage(CURSORLEFT);
}

void CDiagKeyboard::UpdatePage4(HWND hwndDlg)
{
TCHAR szBuffer[30];

	_updatekeypage(JOY1FIRE);
	_updatekeypage(JOY1FIRE2);
	_updatekeypage(JOY1UP);
	_updatekeypage(JOY1DOWN);
	_updatekeypage(JOY1LEFT);
	_updatekeypage(JOY1RIGHT);
	_updatekeypage(JOY2FIRE);
	_updatekeypage(JOY2FIRE2);
	_updatekeypage(JOY2UP);
	_updatekeypage(JOY2DOWN);
	_updatekeypage(JOY2LEFT);
	_updatekeypage(JOY2RIGHT);
}

void CDiagKeyboard::clear_keypress_contols()
{
int i;

	m_current_c64key=0;
	m_bKeyCapture=false;
	for(i=0 ; i < _countof(keycontrol) ; i++)
	{
		keycontrol[i].control_id=0;
		keycontrol[i].hwnd=0;
		keycontrol[i].state=kcs_display;
		keycontrol[i].bGotFocus=FALSE;
		keycontrol[i].text[0]=0;
	}
}

HRESULT CDiagKeyboard::initkeycapturecontrols(int pageno, HWND hwndDlg)
{
	switch (pageno)
	{
	case 0:
		return initkeycapturecontrols1(hwndDlg);
	case 1:
		return initkeycapturecontrols2(hwndDlg);
	case 2:
		return initkeycapturecontrols3(hwndDlg);
	case 3:
		return initkeycapturecontrols4(hwndDlg);
	}

	return E_FAIL;
}

LRESULT CALLBACK GetKeyPressWindowProc(
	HWND hWnd,      // handle to window
	UINT uMsg,      // message identifier
	WPARAM wParam,  // first message parameter
	LPARAM lParam   // second message parameter
)
{
HWND parentHwnd;

	CDiagKeyboard* pWin = (CDiagKeyboard*)(LONG_PTR) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_NCCREATE:
		parentHwnd = ((CREATESTRUCT *)lParam)->hwndParent;
		if (parentHwnd==0)
		{
			return FALSE;
		}

		parentHwnd = GetParent(parentHwnd);
		if (parentHwnd==0)
		{
			return FALSE;
		}

		SetWindowLongPtr(hWnd, GWLP_USERDATA, GetWindowLongPtr(parentHwnd, GWLP_USERDATA));				
		break;
	case WM_DESTROY:
		// This is our signal to destroy the window object.
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);
		pWin = NULL;
		break;
	default:
		break;
	}

	// Call its message proc method.
	if (NULL != pWin)
	{
		return (pWin->OnEventKeyControl(hWnd, uMsg, wParam, lParam));
	}
	else
	{
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
}

LRESULT CDiagKeyboard::OnEventKeyControl(
HWND hwnd,      // handle to window
UINT uMsg,      // message identifier
WPARAM wParam,  // first message parameter
LPARAM lParam   // second message parameter
)
{
int id;
int i;
int c64key=-1;
LPMSG lpmsg;
HDC hdc;
RECT rc;
PAINTSTRUCT ps; 
LOGBRUSH lb; 
HPEN hPenOld,hPen,hPen_highlight,hPen_shadow,hPen_darkshadow;
COLORREF textcolor_old,backcolor_old;

	id=GetDlgCtrlID(hwnd);
	if (id==0)
	{
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	for (i=0 ; i < _countof(keycontrol) ; i++)
	{
		if (id == keycontrol[i].control_id)
		{
			c64key=i;
			break;
		}
	}

	if (c64key<0)
	{
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	switch (uMsg)
	{
	case WM_SETTEXT:
		lstrcpyn(keycontrol[c64key].text, (LPCTSTR) lParam, sizeof(keycontrol[c64key].text)-1);
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	case WM_LBUTTONDOWN:
		if (keycontrol[c64key].bGotFocus)
		{
			if (keycontrol[m_current_c64key].state == kcs_getkey)
			{
				SendMessage(hwnd, WM_KEYCAPTURE, FALSE, 0);
			}
			else
			{
				SendMessage(hwnd, WM_KEYCAPTURE, TRUE, 0);
			}
		}
		else
		{
			SetFocus(hwnd);
		}

		return 0;
	case WM_KEYCAPTURE:
		if (wParam!=FALSE)
		{
			SetKeyCapture(c64key);
		}
		else
		{
			ResetKeyCapture();
		}
		return 0;
	case WM_GETDLGCODE:
		if (keycontrol[c64key].state == kcs_getkey)
		{
			return DLGC_WANTCHARS | DLGC_WANTALLKEYS | DLGC_WANTMESSAGE | DLGC_UNDEFPUSHBUTTON ;
		}
		else
		{
			if (lParam)
			{
				lpmsg = (LPMSG)lParam;
				switch (lpmsg->message)
				{
				case WM_KEYDOWN:
					switch (lpmsg->wParam)
					{
					case VK_F2:
					case VK_SPACE:
						SendMessage(hwnd, WM_KEYCAPTURE, TRUE, 0);
						return DLGC_WANTCHARS | DLGC_WANTMESSAGE | DLGC_UNDEFPUSHBUTTON;
					}
				}
			}
			return DLGC_UNDEFPUSHBUTTON;
		}
	case WM_SETFOCUS:
		keycontrol[c64key].bGotFocus=TRUE;
		InvalidateRect(keycontrol[c64key].hwnd, NULL, TRUE);
		return 0;
	case WM_KILLFOCUS:
		keycontrol[c64key].bGotFocus=FALSE;
		InvalidateRect(keycontrol[c64key].hwnd, NULL, TRUE);
		SendMessage(hwnd, WM_KEYCAPTURE, FALSE, 0);
		return 0;
	case WM_PAINT:
		if (GetUpdateRect(hwnd, &rc, FALSE)==0)
		{
			return 0;
		}

		hdc = BeginPaint(hwnd, &ps); 
		if (hdc != NULL)
		{
			GetClientRect(hwnd, &rc); 
			hPen=0;
			hPen_highlight = 0;
			hPen_shadow = 0;
			hPen_darkshadow = 0;
			hPenOld=0;
			textcolor_old=0;
			backcolor_old=0;
			textcolor_old = SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
			backcolor_old = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
			if (keycontrol[c64key].state == kcs_display)
			{
				if (keycontrol[c64key].bGotFocus)
				{
					lb.lbStyle = BS_SOLID; 
					lb.lbColor = RGB(255,0,0); 
					lb.lbHatch = 0;
					hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_INSIDEFRAME, 
						2, &lb, 0, NULL); 
					if (hPen)
					{
						hPenOld = (HPEN) SelectObject(hdc, hPen); 
						Rectangle(hdc,rc.left, rc.top, rc.right, rc.bottom);
						InflateRect(&rc,-2,-2);
						DrawText(hdc,keycontrol[c64key].text,lstrlen(keycontrol[c64key].text),&rc,DT_CENTER);
					}
				}
				else
				{

					hPen_highlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
					hPen_shadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
					hPen_darkshadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));

					if (hPen_highlight && hPen_shadow)
					{
						hPenOld = (HPEN) SelectObject(hdc, hPen_shadow); 
						MoveToEx(hdc, rc.left,rc.top, NULL);
						LineTo(hdc,rc.right,rc.top);
						MoveToEx(hdc, rc.left,rc.top, NULL);
						LineTo(hdc,rc.left,rc.bottom);

						SelectObject(hdc, hPen_darkshadow); 
						MoveToEx(hdc, rc.left+1,rc.top+1, NULL);
						LineTo(hdc,rc.right-1,rc.top+1);
						MoveToEx(hdc, rc.left+1,rc.top+1, NULL);
						LineTo(hdc,rc.left+1,rc.bottom-1);

						SelectObject(hdc, hPen_highlight); 
						MoveToEx(hdc, rc.right,rc.top, NULL);
						LineTo(hdc,rc.right,rc.bottom);
						MoveToEx(hdc, rc.left,rc.bottom, NULL);
						LineTo(hdc,rc.right,rc.bottom);

						InflateRect(&rc,-2,-2);
						SetBkMode(hdc, TRANSPARENT);
						DrawText(hdc,keycontrol[c64key].text,lstrlen(keycontrol[c64key].text),&rc,DT_CENTER);
					}
				}
			}
			else
			{
				lb.lbStyle = BS_HATCHED; 
				lb.lbColor = RGB(255,0,0); 
				lb.lbHatch = HS_FDIAGONAL;
				hPen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | PS_INSIDEFRAME, 
					4, &lb, 0, NULL); 
				if (hPen)
				{
					hPenOld = (HPEN) SelectObject(hdc, hPen); 
					Rectangle(hdc,rc.left, rc.top, rc.right, rc.bottom);
					InflateRect(&rc,-2,-2);
					DrawText(hdc,G::GetStringRes(IDS_PRESSANYKEY),lstrlen(G::GetStringRes(IDS_PRESSANYKEY)),&rc,DT_CENTER);
				}
			}

			if (textcolor_old) SetTextColor(hdc, textcolor_old);
			if (backcolor_old) SetBkColor(hdc, backcolor_old);
			if (hPenOld) SelectObject(hdc, hPenOld); 
			if (hPen) DeleteObject(hPen); 
			if (hPen_highlight)	DeleteObject(hPen_highlight); 
			if (hPen_shadow) DeleteObject(hPen_shadow); 
			if (hPen_darkshadow) DeleteObject(hPen_darkshadow); 
			EndPaint(hwnd, &ps); 
		}
	default:
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	return 0;
}

void CDiagKeyboard::SetKeyCapture(int c64key)
{
int i;
	m_current_c64key = c64key;
	m_bKeyCapture = true;
	m_bBeginKeyScan = true;
	for (i=0 ; i < _countof(keycontrol) ; i++)
	{
		if (m_current_c64key!=i && keycontrol[i].state!=kcs_display)
		{
			keycontrol[i].state=kcs_display;
			InvalidateRect(keycontrol[i].hwnd, NULL, TRUE);
		}
	}

	keycontrol[m_current_c64key].state = kcs_getkey;
	if (keycontrol[m_current_c64key].hwnd != 0)
	{
		InvalidateRect(keycontrol[m_current_c64key].hwnd, NULL, TRUE);
	}
}

void CDiagKeyboard::ResetKeyCapture()
{
int i;
	m_bKeyCapture=false;
	for (i=0 ; i < _countof(keycontrol) ; i++)
	{
		if (keycontrol[i].state!=kcs_display)
		{
			keycontrol[i].state=kcs_display;
			if (keycontrol[i].hwnd != 0)
			{
				InvalidateRect(keycontrol[i].hwnd, NULL, TRUE);
			}
		}
	}
}

#define _initcapturectrls(n) hWnd = GetDlgItem(hwndDlg, IDC_TXT_##n);\
	if (hWnd==NULL) return E_FAIL;\
	keycontrol[C64Keys::C64K_##n].hwnd=hWnd;\
	keycontrol[C64Keys::C64K_##n].control_id=IDC_TXT_##n;

HRESULT CDiagKeyboard::initkeycapturecontrols1(HWND hwndDlg)
{
HWND hWnd;
	_initcapturectrls(HOME);
	_initcapturectrls(ASTERISK);
	_initcapturectrls(SLASH);
	_initcapturectrls(ARROWUP);
	_initcapturectrls(ARROWLEFT);
	_initcapturectrls(CONTROL);
	_initcapturectrls(STOP);
	_initcapturectrls(COMMODORE);
	_initcapturectrls(DEL);
	_initcapturectrls(PLUS);
	_initcapturectrls(MINUS);
	_initcapturectrls(EQUAL);
	_initcapturectrls(POUND);
	_initcapturectrls(AT);
	_initcapturectrls(COLON);
	_initcapturectrls(SEMICOLON);
	_initcapturectrls(CURSORRIGHT);
	_initcapturectrls(CURSORDOWN);
	_initcapturectrls(LEFTSHIFT);
	_initcapturectrls(RIGHTSHIFT);
	_initcapturectrls(RESTORE);
	_initcapturectrls(SPACE);
	_initcapturectrls(RETURN);
	return S_OK;
}

HRESULT CDiagKeyboard::initkeycapturecontrols2(HWND hwndDlg)
{
HWND hWnd;

	_initcapturectrls(A);
	_initcapturectrls(B);
	_initcapturectrls(C);
	_initcapturectrls(D);
	_initcapturectrls(E);
	_initcapturectrls(F);
	_initcapturectrls(G);
	_initcapturectrls(H);
	_initcapturectrls(I);
	_initcapturectrls(J);
	_initcapturectrls(K);
	_initcapturectrls(L);
	_initcapturectrls(M);
	_initcapturectrls(N);
	_initcapturectrls(O);
	_initcapturectrls(P);
	_initcapturectrls(Q);
	_initcapturectrls(R);
	_initcapturectrls(S);
	_initcapturectrls(T);
	_initcapturectrls(U);
	_initcapturectrls(V);
	_initcapturectrls(W);
	_initcapturectrls(X);
	_initcapturectrls(Y);
	_initcapturectrls(Z);

	return S_OK;
}

HRESULT CDiagKeyboard::initkeycapturecontrols3(HWND hwndDlg)
{
HWND hWnd;

	_initcapturectrls(1);
	_initcapturectrls(2);
	_initcapturectrls(3);
	_initcapturectrls(4);
	_initcapturectrls(5);
	_initcapturectrls(6);
	_initcapturectrls(7);
	_initcapturectrls(8);
	_initcapturectrls(9);
	_initcapturectrls(0);
	_initcapturectrls(F1);
	_initcapturectrls(F2);
	_initcapturectrls(F3);
	_initcapturectrls(F4);
	_initcapturectrls(F5);
	_initcapturectrls(F6);
	_initcapturectrls(F7);
	_initcapturectrls(F8);
	_initcapturectrls(CURSORUP);
	_initcapturectrls(CURSORLEFT);
	return S_OK;
}

HRESULT CDiagKeyboard::initkeycapturecontrols4(HWND hwndDlg)
{
HWND hWnd;
	_initcapturectrls(JOY1FIRE);
	_initcapturectrls(JOY1FIRE2);
	_initcapturectrls(JOY1UP);
	_initcapturectrls(JOY1DOWN);
	_initcapturectrls(JOY1LEFT);
	_initcapturectrls(JOY1RIGHT);
	_initcapturectrls(JOY2FIRE);
	_initcapturectrls(JOY2FIRE2);
	_initcapturectrls(JOY2UP);
	_initcapturectrls(JOY2DOWN);
	_initcapturectrls(JOY2LEFT);
	_initcapturectrls(JOY2RIGHT);

	return S_OK;
}

BOOL CDiagKeyboard::OnPageEvent(CTabPageDialog *page, HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
HRESULT hr;
	switch (message) 
	{ 
	case WM_INITDIALOG:
		hr=initkeycapturecontrols(page->m_pageindex, hwndDlg);
		if (FAILED(hr))
		{
			return FALSE;
		}

		UpdatePage(page->m_pageindex, hwndDlg);
		return OnChildDialogInit(hwndDlg);
	case WM_DESTROY:
		if (hwndDlg == m_hwndDisplay)
		{
			m_hwndDisplay = NULL;
		}

		return TRUE;
	} 
	return FALSE; 
}

/**************************************************************/


HRESULT CDiagKeyboard::Acquire()
{
HRESULT	hr=E_FAIL;

	if (pDX->pKeyboard)
	{
		hr = pDX->pKeyboard->Acquire();
	}

	return hr;
}

void CDiagKeyboard::Unacquire()
{
	pDX->pKeyboard->Unacquire();
}

void CDiagKeyboard::SetAcquire()
{
    if( m_bActive ) 
    {
		Acquire();
    } 
    else 
    {
		Unacquire();
    }
}

HRESULT CDiagKeyboard::ReadScanCode(BYTE *scancode)
{
char     buffer[256]; 
int i;
HRESULT  hr; 
#define KEYDOWN(name,key) (name[key] & 0x80) 

	hr = S_FALSE;
	hr = pDX->pKeyboard->GetDeviceState(sizeof(buffer),(LPVOID)&buffer); 
	if FAILED(hr) 
	{ 
		if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST )
		{
			hr = pDX->pKeyboard->Acquire();
			if ( FAILED(hr) )
			{
				return hr;
			}

			hr = pDX->pKeyboard->GetDeviceState(sizeof(buffer), (LPVOID)&buffer); 
			if FAILED(hr) 
			{
				return hr;
			}
		}
		else if (hr==DIERR_INVALIDPARAM)
		{
			return hr; 
		}
		else if (hr==DIERR_NOTINITIALIZED)
		{
			return hr;
		}
		else if (hr==E_PENDING)
		{
			return hr;
		}
		else
		{
			return hr;
		}
	} 

	hr = S_FALSE;// S_FALSE: no key was pressed. S_OK: A key was pressed.
	for (i=0 ; i<256 ; i++)
	{
		switch (i)
		{
		case DIK_ESCAPE:
		case DIK_1:
		case DIK_2:
		case DIK_3:
		case DIK_4:
		case DIK_5:
		case DIK_6:
		case DIK_7:
		case DIK_8:
		case DIK_9:
		case DIK_0:
		case DIK_MINUS:    /* - on main keyboard */
		case DIK_EQUALS:
		case DIK_BACK:/* backspace */
		case DIK_TAB:
		case DIK_Q:
		case DIK_W:
		case DIK_E:
		case DIK_R:
		case DIK_T:
		case DIK_Y:
		case DIK_U:
		case DIK_I:
		case DIK_O:
		case DIK_P:
		case DIK_LBRACKET:
		case DIK_RBRACKET:
		case DIK_RETURN:    /* Enter on main keyboard */
		case DIK_LCONTROL:
		case DIK_A:
		case DIK_S:
		case DIK_D:
		case DIK_F:
		case DIK_G:
		case DIK_H:
		case DIK_J:
		case DIK_K:
		case DIK_L:
		case DIK_SEMICOLON:
		case DIK_APOSTROPHE:
		case DIK_GRAVE:    /* accent grave */
		case DIK_LSHIFT:
		case DIK_BACKSLASH:
		case DIK_Z:
		case DIK_X:
		case DIK_C:
		case DIK_V:
		case DIK_B:
		case DIK_N:
		case DIK_M:
		case DIK_COMMA:
		case DIK_PERIOD:    /* . on main keyboard */
		case DIK_SLASH:    /* / on main keyboard */
		case DIK_RSHIFT:
		case DIK_MULTIPLY:    /* * on numeric keypad */
		case DIK_LMENU:    /* left Alt */
		case DIK_SPACE:
		case DIK_CAPITAL:
		case DIK_F1:
		case DIK_F2:
		case DIK_F3:
		case DIK_F4:
		case DIK_F5:
		case DIK_F6:
		case DIK_F7:
		case DIK_F8:
		case DIK_F9:
		case DIK_F10:
		case DIK_NUMLOCK:
		case DIK_SCROLL:    /* Scroll Lock */
		case DIK_NUMPAD7:
		case DIK_NUMPAD8:
		case DIK_NUMPAD9:
		case DIK_SUBTRACT:    /* - on numeric keypad */
		case DIK_NUMPAD4:
		case DIK_NUMPAD5:
		case DIK_NUMPAD6:
		case DIK_ADD:    /* + on numeric keypad */
		case DIK_NUMPAD1:
		case DIK_NUMPAD2:
		case DIK_NUMPAD3:
		case DIK_NUMPAD0:
		case DIK_DECIMAL:    /* . on numeric keypad */
		case DIK_OEM_102:    /* < > | on UK/Germany keyboards */
		case DIK_F11:
		case DIK_F12:

		case DIK_F13:    /*                     (NEC PC98) */
		case DIK_F14:    /*                     (NEC PC98) */
		case DIK_F15:    /*                     (NEC PC98) */

		case DIK_KANA:    /* (Japanese keyboard)            */
		case DIK_ABNT_C1:    /* / ? on Portugese (Brazilian) keyboards */
		case DIK_CONVERT:    /* (Japanese keyboard)            */
		case DIK_NOCONVERT:    /* (Japanese keyboard)            */
		case DIK_YEN:    /* (Japanese keyboard)            */
		case DIK_ABNT_C2:    /* Numpad . on Portugese (Brazilian) keyboards */
		case DIK_NUMPADEQUALS:    /* = on numeric keypad (NEC PC98) */
		case DIK_PREVTRACK:    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
		case DIK_AT:    /*                     (NEC PC98) */
		case DIK_COLON:    /*                     (NEC PC98) */
		case DIK_UNDERLINE:    /*                     (NEC PC98) */
		case DIK_KANJI:    /* (Japanese keyboard)            */
		case DIK_STOP:    /*                     (NEC PC98) */
		case DIK_AX:    /*                     (Japan AX) */
		case DIK_UNLABELED:    /*                        (J3100) */
		case DIK_NEXTTRACK:    /* Next Track */
		case DIK_NUMPADENTER:    /* Enter on numeric keypad */
		case DIK_RCONTROL:
		case DIK_MUTE:    /* Mute */
		case DIK_CALCULATOR:    /* Calculator */
		case DIK_PLAYPAUSE:    /* Play / Pause */
		case DIK_MEDIASTOP:    /* Media Stop */
		case DIK_VOLUMEDOWN:    /* Volume - */
		case DIK_VOLUMEUP:    /* Volume + */
		case DIK_WEBHOME:    /* Web home */
		case DIK_NUMPADCOMMA:    /* , on numeric keypad (NEC PC98) */
		case DIK_DIVIDE:    /* / on numeric keypad */
		case DIK_SYSRQ:
		case DIK_RMENU:    /* right Alt */
		case DIK_PAUSE:    /* Pause */
		case DIK_HOME:    /* Home on arrow keypad */
		case DIK_UP:    /* UpArrow on arrow keypad */
		case DIK_PRIOR:    /* PgUp on arrow keypad */
		case DIK_LEFT:    /* LeftArrow on arrow keypad */
		case DIK_RIGHT:    /* RightArrow on arrow keypad */
		case DIK_END:    /* End on arrow keypad */
		case DIK_DOWN:    /* DownArrow on arrow keypad */
		case DIK_NEXT:    /* PgDn on arrow keypad */
		case DIK_INSERT:    /* Insert on arrow keypad */
		case DIK_DELETE:    /* Delete on arrow keypad */
		case DIK_LWIN:    /* Left Windows key */
		case DIK_RWIN:    /* Right Windows key */
		case DIK_APPS:    /* AppMenu key */
		case DIK_POWER:    /* System Power */
		case DIK_SLEEP:    /* System Sleep */
		case DIK_WAKE:    /* System Wake */
		case DIK_WEBSEARCH:    /* Web Search */
		case DIK_WEBFAVORITES:    /* Web Favorites */
		case DIK_WEBREFRESH:    /* Web Refresh */
		case DIK_WEBSTOP:    /* Web Stop */
		case DIK_WEBFORWARD:    /* Web Forward */
		case DIK_WEBBACK:    /* Web Back */
		case DIK_MYCOMPUTER:    /* My Computer */
		case DIK_MAIL:    /* Mail */
		case DIK_MEDIASELECT:    /* Media Select */
			if (KEYDOWN(buffer , i))
			{
				*scancode = i;
				hr = S_OK;
				return hr;
			}
		default:
			continue;
		}
	}

	return hr;
}


void GetKeyName(BYTE scancode,TCHAR *buffer,int bufferTCharSize)
{
int vk,i;
BYTE KeyState[256];
BYTE ch[2];
	switch (scancode)
	{
	case 0:
		if (bufferTCharSize > 0)
			buffer[0]=0;
		return;
	case DIK_1:
	case DIK_2:
	case DIK_3:
	case DIK_4:
	case DIK_5:
	case DIK_6:
	case DIK_7:
	case DIK_8:
	case DIK_9:
	case DIK_0:
	case DIK_MINUS:    /* - on main keyboard */
	case DIK_EQUALS:
	case DIK_Q:
	case DIK_W:
	case DIK_E:
	case DIK_R:
	case DIK_T:
	case DIK_Y:
	case DIK_U:
	case DIK_I:
	case DIK_O:
	case DIK_P:
	case DIK_LBRACKET:
	case DIK_RBRACKET:
	case DIK_A:
	case DIK_S:
	case DIK_D:
	case DIK_F:
	case DIK_G:
	case DIK_H:
	case DIK_J:
	case DIK_K:
	case DIK_L:
	case DIK_SEMICOLON:
	case DIK_APOSTROPHE:
	case DIK_GRAVE:    /* accent grave */
	case DIK_BACKSLASH:
	case DIK_Z:
	case DIK_X:
	case DIK_C:
	case DIK_V:
	case DIK_B:
	case DIK_N:
	case DIK_M:
	case DIK_COMMA:
	case DIK_PERIOD:    /* . on main keyboard */
	case DIK_SLASH:    /* / on main keyboard */
		ZeroMemory(&KeyState[0], sizeof(KeyState));
		lstrcpyn(buffer,TEXT("Unknown"), bufferTCharSize);

		vk = MapVirtualKey(scancode, 1);
		if (vk!=0)
		{
			i = ToAscii(vk, scancode, KeyState, (LPWORD) &ch[0], 0);
			if (i==1)
			{
				ch[1] = 0;
				CharUpper((LPTSTR)&ch[0]);
#ifdef UNICODE
				int chOut;
				if (SUCCEEDED(G::AnsiToUc((LPCSTR) &ch[0], &buffer[0], 1, chOut)))
				{
					buffer[chOut] = 0;
				}
#else
				buffer[0] = ch[0];//Little endian
				buffer[1] = 0;
#endif
			}
		}
		break;
	case DIK_RETURN:
		lstrcpyn(buffer,TEXT("RETURN"), bufferTCharSize);
		break;
	case DIK_TAB:
		lstrcpyn(buffer,TEXT("TAB"), bufferTCharSize);
		break;
	case DIK_F1:
		lstrcpyn(buffer,TEXT("F1"), bufferTCharSize);
		break;
	case DIK_F2:
		lstrcpyn(buffer,TEXT("F2"), bufferTCharSize);
		break;
	case DIK_F3:
		lstrcpyn(buffer,TEXT("F3"), bufferTCharSize);
		break;
	case DIK_F4:
		lstrcpyn(buffer,TEXT("F4"), bufferTCharSize);
		break;
	case DIK_F5:
		lstrcpyn(buffer,TEXT("F5"), bufferTCharSize);
		break;
	case DIK_F6:
		lstrcpyn(buffer,TEXT("F6"), bufferTCharSize);
		break;
	case DIK_F7:
		lstrcpyn(buffer,TEXT("F7"), bufferTCharSize);
		break;
	case DIK_F8:
		lstrcpyn(buffer,TEXT("F8"), bufferTCharSize);
		break;
	case DIK_F9:
		lstrcpyn(buffer,TEXT("F9"), bufferTCharSize);
		break;
	case DIK_F10:
		lstrcpyn(buffer,TEXT("F10"), bufferTCharSize);
		break;
	case DIK_F11:
		lstrcpyn(buffer,TEXT("F11"), bufferTCharSize);
		break;
	case DIK_F12:
		lstrcpyn(buffer,TEXT("F12"), bufferTCharSize);
		break;
	case DIK_F13:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("F13"), bufferTCharSize);
		break;
	case DIK_F14:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("F14"), bufferTCharSize);
		break;
	case DIK_F15:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("F15"), bufferTCharSize);
		break;
	case DIK_BACK:/* backspace */
		lstrcpyn(buffer,TEXT("BACK"), bufferTCharSize);
		break;
	case DIK_ESCAPE:
		lstrcpyn(buffer,TEXT("ESCAPE"), bufferTCharSize);
		break;
	case DIK_LMENU:    /* left Alt */
		lstrcpyn(buffer,TEXT("LMENU"), bufferTCharSize);
		break;
	case DIK_LCONTROL:
		lstrcpyn(buffer,TEXT("LEFT CONTROL"), bufferTCharSize);
		break;
	case DIK_RCONTROL:
		lstrcpyn(buffer,TEXT("RIGHT CONTROL"), bufferTCharSize);
		break;
	case DIK_LSHIFT:
		lstrcpyn(buffer,TEXT("LEFT SHIFT"), bufferTCharSize);
		break;
	case DIK_RSHIFT:
		lstrcpyn(buffer,TEXT("RIGHT SHIFT"), bufferTCharSize);
		break;
	case DIK_MULTIPLY:    /* on numeric keypad */
		lstrcpyn(buffer,TEXT("NUM MULTIPLY"), bufferTCharSize);
		break;
	case DIK_SPACE:
		lstrcpyn(buffer,TEXT("SPACE"), bufferTCharSize);
		break;
	case DIK_CAPITAL:
		lstrcpyn(buffer,TEXT("CAPITAL"), bufferTCharSize);
		break;
	case DIK_NUMLOCK:
		lstrcpyn(buffer,TEXT("NUMLOCK"), bufferTCharSize);
		break;
	case DIK_SCROLL:    /* Scroll Lock */
		lstrcpyn(buffer,TEXT("SCROLL"), bufferTCharSize);
		break;
	case DIK_NUMPAD7:
		lstrcpyn(buffer,TEXT("NUMPAD 7"), bufferTCharSize);
		break;
	case DIK_NUMPAD8:
		lstrcpyn(buffer,TEXT("NUMPAD 8"), bufferTCharSize);
		break;
	case DIK_NUMPAD9:
		lstrcpyn(buffer,TEXT("NUMPAD 9"), bufferTCharSize);
		break;
	case DIK_SUBTRACT:    /* - on numeric keypad */
		lstrcpyn(buffer,TEXT("NUMPAD MINUS"), bufferTCharSize);
		break;
	case DIK_NUMPAD4:
		lstrcpyn(buffer,TEXT("NUMPAD 4"), bufferTCharSize);
		break;
	case DIK_NUMPAD5:
		lstrcpyn(buffer,TEXT("NUMPAD 5"), bufferTCharSize);
		break;
	case DIK_NUMPAD6:
		lstrcpyn(buffer,TEXT("NUMPAD 6"), bufferTCharSize);
		break;
	case DIK_ADD:    /* + on numeric keypad */
		lstrcpyn(buffer,TEXT("NUMPAD +"), bufferTCharSize);
		break;
	case DIK_NUMPAD1:
		lstrcpyn(buffer,TEXT("NUMPAD 1"), bufferTCharSize);
		break;
	case DIK_NUMPAD2:
		lstrcpyn(buffer,TEXT("NUMPAD 2"), bufferTCharSize);
		break;
	case DIK_NUMPAD3:
		lstrcpyn(buffer,TEXT("NUMPAD 3"), bufferTCharSize);
		break;
	case DIK_NUMPAD0:
		lstrcpyn(buffer,TEXT("NUMPAD 0"), bufferTCharSize);
		break;
	case DIK_DECIMAL:    /* . on numeric keypad */
		lstrcpyn(buffer,TEXT("NUMPAD ."), bufferTCharSize);
		break;
	case DIK_OEM_102:    /* < > | on UK/Germany keyboards */
		lstrcpyn(buffer,TEXT("OEM 102"), bufferTCharSize);
		break;
	case DIK_KANA:    /* (Japanese keyboard)            */
		lstrcpyn(buffer,TEXT("KANA"), bufferTCharSize);
		break;
	case DIK_ABNT_C1:    /* / ? on Portugese (Brazilian) keyboards */
		lstrcpyn(buffer,TEXT("ABNT_C1"), bufferTCharSize);
		break;
	case DIK_CONVERT:    /* (Japanese keyboard)            */
		lstrcpyn(buffer,TEXT("CONVERT"), bufferTCharSize);
		break;
	case DIK_NOCONVERT:    /* (Japanese keyboard)            */
		lstrcpyn(buffer,TEXT("NOCONVERT"), bufferTCharSize);
		break;
	case DIK_YEN:    /* (Japanese keyboard)            */
		lstrcpyn(buffer,TEXT("YEN"), bufferTCharSize);
		break;
	case DIK_ABNT_C2:    /* Numpad . on Portugese (Brazilian) keyboards */
		lstrcpyn(buffer,TEXT("ABNT_C2"), bufferTCharSize);
		break;
	case DIK_NUMPADEQUALS:    /* = on numeric keypad (NEC PC98) */
		lstrcpyn(buffer,TEXT("NUMPAD ="), bufferTCharSize);
		break;
	case DIK_PREVTRACK:    /* Previous Track (DIK_CIRCUMFLEX on Japanese keyboard) */
		lstrcpyn(buffer,TEXT("PREVTRACK"), bufferTCharSize);
		break;
	case DIK_AT:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("AT"), bufferTCharSize);
		break;
	case DIK_COLON:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("COLON"), bufferTCharSize);
		break;
	case DIK_UNDERLINE:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("UNDERLINE"), bufferTCharSize);
		break;
	case DIK_KANJI:    /* (Japanese keyboard)            */
		lstrcpyn(buffer,TEXT("KANJI"), bufferTCharSize);
		break;
	case DIK_STOP:    /*                     (NEC PC98) */
		lstrcpyn(buffer,TEXT("STOP"), bufferTCharSize);
		break;
	case DIK_AX:    /*                     (Japan AX) */
		lstrcpyn(buffer,TEXT("AX"), bufferTCharSize);
		break;
	case DIK_UNLABELED:    /*                        (J3100) */
		lstrcpyn(buffer,TEXT("UNLABELED"), bufferTCharSize);
		break;
	case DIK_NEXTTRACK:    /* Next Track */
		lstrcpyn(buffer,TEXT("NEXTTRACK"), bufferTCharSize);
		break;
	case DIK_NUMPADENTER:    /* Enter on numeric keypad */
		lstrcpyn(buffer,TEXT("NUMPAD ENTER"), bufferTCharSize);
		break;
	case DIK_MUTE:    /* Mute */
		lstrcpyn(buffer,TEXT("MUTE"), bufferTCharSize);
		break;
	case DIK_CALCULATOR:    /* Calculator */
		lstrcpyn(buffer,TEXT("CALC"), bufferTCharSize);
		break;
	case DIK_PLAYPAUSE:    /* Play / Pause */
		lstrcpyn(buffer,TEXT("PLAY"), bufferTCharSize);
		break;
	case DIK_MEDIASTOP:    /* Media Stop */
		lstrcpyn(buffer,TEXT("MEDIA STOP"), bufferTCharSize);
		break;
	case DIK_VOLUMEDOWN:    /* Volume - */
		lstrcpyn(buffer,TEXT("VOLUME DOWN"), bufferTCharSize);
		break;
	case DIK_VOLUMEUP:    /* Volume + */
		lstrcpyn(buffer,TEXT("VOLUME UP"), bufferTCharSize);
		break;
	case DIK_WEBHOME:    /* Web home */
		lstrcpyn(buffer,TEXT("WEB HOME"), bufferTCharSize);
		break;
	case DIK_NUMPADCOMMA:    /* , on numeric keypad (NEC PC98) */
		lstrcpyn(buffer,TEXT("NUMPAD COMMA"), bufferTCharSize);
		break;
	case DIK_DIVIDE:    /* / on numeric keypad */
		lstrcpyn(buffer,TEXT("NUMPAD DIVIDE"), bufferTCharSize);
		break;
	case DIK_SYSRQ:
		lstrcpyn(buffer,TEXT("SYSRQ"), bufferTCharSize);
		break;
	case DIK_RMENU:    /* right Alt */
		lstrcpyn(buffer,TEXT("RIGHT MENU"), bufferTCharSize);
		break;
	case DIK_PAUSE:    /* Pause */
		lstrcpyn(buffer,TEXT("PAUSE"), bufferTCharSize);
		break;
	case DIK_HOME:    /* Home on arrow keypad */
		lstrcpyn(buffer,TEXT("HOME"), bufferTCharSize);
		break;
	case DIK_UP:    /* UpArrow on arrow keypad */
		lstrcpyn(buffer,TEXT("UP"), bufferTCharSize);
		break;
	case DIK_PRIOR:    /* PgUp on arrow keypad */
		lstrcpyn(buffer,TEXT("PRIOR"), bufferTCharSize);
		break;
	case DIK_LEFT:    /* LeftArrow on arrow keypad */
		lstrcpyn(buffer,TEXT("LEFT"), bufferTCharSize);
		break;
	case DIK_RIGHT:    /* RightArrow on arrow keypad */
		lstrcpyn(buffer,TEXT("RIGHT"), bufferTCharSize);
		break;
	case DIK_END:    /* End on arrow keypad */
		lstrcpyn(buffer,TEXT("END"), bufferTCharSize);
		break;
	case DIK_DOWN:    /* DownArrow on arrow keypad */
		lstrcpyn(buffer,TEXT("DOWN"), bufferTCharSize);
		break;
	case DIK_NEXT:    /* PgDn on arrow keypad */
		lstrcpyn(buffer,TEXT("NEXT"), bufferTCharSize);
		break;
	case DIK_INSERT:    /* Insert on arrow keypad */
		lstrcpyn(buffer,TEXT("INSERT"), bufferTCharSize);
		break;
	case DIK_DELETE:    /* Delete on arrow keypad */
		lstrcpyn(buffer,TEXT("DELETE"), bufferTCharSize);
		break;
	case DIK_LWIN:    /* Left Windows key */
		lstrcpyn(buffer,TEXT("LWIN"), bufferTCharSize);
		break;
	case DIK_RWIN:    /* Right Windows key */
		lstrcpyn(buffer,TEXT("RWIN"), bufferTCharSize);
		break;
	case DIK_APPS:    /* AppMenu key */
		lstrcpyn(buffer,TEXT("APPS"), bufferTCharSize);
		break;
	case DIK_POWER:    /* System Power */
		lstrcpyn(buffer,TEXT("PWR"), bufferTCharSize);
		break;
	case DIK_SLEEP:    /* System Sleep */
		lstrcpyn(buffer,TEXT("SLEEP"), bufferTCharSize);
		break;
	case DIK_WAKE:    /* System Wake */
		lstrcpyn(buffer,TEXT("WAKE"), bufferTCharSize);
		break;
	case DIK_WEBSEARCH:    /* Web Search */
		lstrcpyn(buffer,TEXT("WEB SEARCH"), bufferTCharSize);
		break;
	case DIK_WEBFAVORITES:    /* Web Favorites */
		lstrcpyn(buffer,TEXT("WEB FAVORITES"), bufferTCharSize);
		break;
	case DIK_WEBREFRESH:    /* Web Refresh */
		lstrcpyn(buffer,TEXT("WEB REFRESH"), bufferTCharSize);
		break;
	case DIK_WEBSTOP:    /* Web Stop */
		lstrcpyn(buffer,TEXT("WEB STOP"), bufferTCharSize);
		break;
	case DIK_WEBFORWARD:    /* Web Forward */
		lstrcpyn(buffer,TEXT("WEB FORWARD"), bufferTCharSize);
		break;
	case DIK_WEBBACK:    /* Web Back */
		lstrcpyn(buffer,TEXT("WEB BACK"), bufferTCharSize);
		break;
	case DIK_MYCOMPUTER:    /* My Computer */
		lstrcpyn(buffer,TEXT("MY COMPUTER"), bufferTCharSize);
		break;
	case DIK_MAIL:    /* Mail */
		lstrcpyn(buffer,TEXT("MAIL"), bufferTCharSize);
		break;
	case DIK_MEDIASELECT:    /* Media Select */
		lstrcpyn(buffer,TEXT("MEDIA SELECT"), bufferTCharSize);
		break;
	default:
		lstrcpyn(buffer,TEXT("Unknown"), bufferTCharSize);
		break;
	}
}

