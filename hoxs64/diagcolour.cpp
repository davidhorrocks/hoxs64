#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <windowsx.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include "servicerelease.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "diagcolour.h"
#include "resource.h"

#define WM_SELECT_PALLETE_ITEM WM_USER

CDiagColour::CDiagColour()
{
	selectedpaletteitem = NULL;
	hPen_highlight = 0;
	hPen_shadow = 0;
	hPen_darkshadow = 0;
}

CDiagColour::~CDiagColour()
{
	if (hPen_highlight)
	{
		DeleteObject(hPen_highlight);
		DeleteObject(hPen_shadow);
		DeleteObject(hPen_darkshadow);
	}
}

HRESULT CDiagColour::Init(const CConfig *currentCfg)
{
	ClearError();
	this->currentCfg = *currentCfg;
	this->newCfg = *currentCfg;
	if (NULL == (hPen_highlight = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT)))) return E_FAIL;

	if (NULL == (hPen_shadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW)))) return E_FAIL;

	if (NULL == (hPen_darkshadow = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW)))) return E_FAIL;
	
	return S_OK;
}

HRESULT CDiagColour::InitVicColourControls(HWND hwndDlg)
{
HWND hWnd;
int id;
static const int controlIdList[] = { IDC_VICCOLOR0, IDC_VICCOLOR1, IDC_VICCOLOR2, IDC_VICCOLOR3, IDC_VICCOLOR4, IDC_VICCOLOR5, IDC_VICCOLOR6, IDC_VICCOLOR7, IDC_VICCOLOR8, IDC_VICCOLOR9, IDC_VICCOLOR10, IDC_VICCOLOR11, IDC_VICCOLOR12, IDC_VICCOLOR13, IDC_VICCOLOR14, IDC_VICCOLOR15 };
	for (int i =0; i < NumColours; i++)
	{
		id = controlIdList[i];
		hWnd = GetDlgItem(hwndDlg, id);
		if (hWnd==NULL)
		{
			return E_FAIL;
		}
		controlpalette[i].IsFocused = false;
		controlpalette[i].IsSelected = false;
		controlpalette[i].VicColour = i;
		controlpalette[i].HWnd = hWnd;
		controlpalette[i].ControlId = id;
		controlpalette[i].RGBColour = this->newCfg.m_colour_palette[i];
		controlpalette[i].IsPalette = true;
		#pragma warning(disable:4244)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
		#pragma warning(default:4244)
	}

	this->SelectPaletteItem(&controlpalette[0], false);
	id = IDC_VICCOLORSAMPLE;
	hWnd = GetDlgItem(hwndDlg, id);
	if (hWnd==NULL)
	{
		return E_FAIL;
	}

	largepaletteitem.IsFocused = false;
	largepaletteitem.IsSelected = false;
	largepaletteitem.HWnd = hWnd;
	largepaletteitem.ControlId = id;
	largepaletteitem.VicColour = 0;
	largepaletteitem.IsPalette = false;
	#pragma warning(disable:4244)
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)this);
	#pragma warning(default:4244)
	InitSlider(hwndDlg, IDC_SLIDERRGBCOLORRED, IDC_EDITRGBCOLORRED);
	InitSlider(hwndDlg, IDC_SLIDERRGBCOLORGREEN, IDC_EDITRGBCOLORGREEN);
	InitSlider(hwndDlg, IDC_SLIDERRGBCOLORBLUE, IDC_EDITRGBCOLORBLUE);

	UpdateSliders(&controlpalette[0]);
	return S_OK;
}

void CDiagColour::InitSlider(HWND hwndDlg, int trackBarId, int editId)
{
	SendDlgItemMessage(hwndDlg, trackBarId, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG(0x0, 0xff));
	SendDlgItemMessage(hwndDlg, trackBarId, TBM_SETPAGESIZE, (WPARAM) TRUE, (LPARAM) 0x10); 
	SendDlgItemMessage(hwndDlg, editId, EM_SETLIMITTEXT, (WPARAM) 3, 0); 
}

BOOL CDiagColour::DialogProc(HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
LPNMHDR pNotify;
ColourControlState* pcontrol;
int editCtrlID;

	switch (message) 
	{ 
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hWndDlg);
		hr=InitVicColourControls(hWndDlg);
		if (FAILED(hr))
		{
			return FALSE;
		}

		return TRUE;
	case WM_COMMAND:
		editCtrlID = LOWORD(wParam);
		switch (LOWORD(editCtrlID))
		{
		case IDC_BUTTON_LOADPEPTO:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				LoadPepto();
				return TRUE;
			}
			break;
		case IDC_BUTTON_LOADPREVIOUSCOLOURS:
			if (HIWORD(wParam) == BN_CLICKED)
			{
				LoadPreviousColors();
				return TRUE;
			}
			break;
		case IDC_EDITRGBCOLORRED:
			EditNotifications(wParam, lParam, editCtrlID, IDC_SLIDERRGBCOLORRED);
			return TRUE;
		case IDC_EDITRGBCOLORGREEN:
			EditNotifications(wParam, lParam, editCtrlID, IDC_SLIDERRGBCOLORGREEN);
			return TRUE;
		case IDC_EDITRGBCOLORBLUE:
			EditNotifications(wParam, lParam, editCtrlID, IDC_SLIDERRGBCOLORBLUE);
			return TRUE;
		case IDOK:
			SaveNewColours();
			EndDialog(hWndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWndDlg, wParam);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		pNotify = (LPNMHDR)lParam;
		if (pNotify)
		{
			switch(pNotify->code)
			{
			case NotifyColour::SelectionChanged:
				pcontrol = this->FindPaletteControlByWindow(pNotify->hwndFrom);
				if (pcontrol)
				{
					UpdateSliders(pcontrol);
				}	
				return TRUE;
			}
		}

		return FALSE; 
	case WM_HSCROLL:
		if (lParam)
		{
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			switch(ctrlID)
			{
			case IDC_SLIDERRGBCOLORRED:
				TBNotifications(wParam, lParam, ctrlID, IDC_EDITRGBCOLORRED);
				return TRUE;
			case IDC_SLIDERRGBCOLORGREEN:
				TBNotifications(wParam, lParam, ctrlID, IDC_EDITRGBCOLORGREEN);
				return TRUE;
			case IDC_SLIDERRGBCOLORBLUE:
				TBNotifications(wParam, lParam, ctrlID, IDC_EDITRGBCOLORBLUE);
				return TRUE;
			}
		}
		return FALSE; 
	}

	return FALSE; 
}

void CDiagColour::LoadPepto()
{
unsigned int i;

	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		this->controlpalette[i].RGBColour = VicIIPalette::Pepto[i];
		::InvalidateRect(this->controlpalette[i].HWnd, NULL, FALSE);
	}

	if (this->selectedpaletteitem)
	{
		this->largepaletteitem.RGBColour = this->selectedpaletteitem->RGBColour;
	}

	::InvalidateRect(this->largepaletteitem.HWnd, NULL, FALSE);
	this->UpdateSliders(this->selectedpaletteitem);
}

void CDiagColour::LoadPreviousColors()
{
unsigned int i;

	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		this->controlpalette[i].RGBColour = this->currentCfg.m_colour_palette[i];
		::InvalidateRect(this->controlpalette[i].HWnd, NULL, FALSE);
	}

	if (this->selectedpaletteitem)
	{
		this->largepaletteitem.RGBColour = this->selectedpaletteitem->RGBColour;
	}

	::InvalidateRect(this->largepaletteitem.HWnd, NULL, FALSE);
	this->UpdateSliders(this->selectedpaletteitem);
}

void CDiagColour::SaveNewColours()
{
unsigned int i;

	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		this->newCfg.m_colour_palette[i] = this->controlpalette[i].RGBColour;		
	}	
}

void CDiagColour::EditNotifications(WPARAM wParam, LPARAM lParam, int editId, int trackBarId)
{
DWORD code = HIWORD(wParam);
BOOL isTranslated;
DWORD dwPosRed;
DWORD dwPosGreen;
DWORD dwPosBlue;

	if (code == EN_CHANGE)
	{
		dwPosRed = ::GetDlgItemInt(this->m_hWnd, IDC_EDITRGBCOLORRED, &isTranslated, FALSE);
		dwPosGreen = ::GetDlgItemInt(this->m_hWnd, IDC_EDITRGBCOLORGREEN, &isTranslated, FALSE);
		dwPosBlue = ::GetDlgItemInt(this->m_hWnd, IDC_EDITRGBCOLORBLUE, &isTranslated, FALSE);

		dwPosRed = min(dwPosRed, 0xff);
		dwPosGreen = min(dwPosGreen, 0xff);
		dwPosBlue = min(dwPosBlue, 0xff);

		DWORD pos = 0;
		bool ok = true;
		switch (editId)
		{
		case IDC_EDITRGBCOLORRED:
			pos = dwPosRed;
			break;
		case IDC_EDITRGBCOLORGREEN:
			pos = dwPosGreen;
			break;
		case IDC_EDITRGBCOLORBLUE:
			pos = dwPosBlue;
			break;
		default:
			ok = false;
		}

		if (ok)
		{
			SendDlgItemMessage(this->m_hWnd, trackBarId, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
		}

		UpdateSelectedPaletteItem();
	}
}

void CDiagColour::TBNotifications(WPARAM wParam, LPARAM lParam, int trackBarId, int editId)
{
DWORD dwPos;
TCHAR numstr[10];
int r;
	switch (LOWORD(wParam))
	{
	case TB_BOTTOM:
	case TB_ENDTRACK:
	case TB_LINEDOWN:
	case TB_LINEUP:
	case TB_PAGEDOWN:
	case TB_PAGEUP:
	case TB_THUMBPOSITION:
	case TB_THUMBTRACK:
	case TB_TOP:
		dwPos = (DWORD)::SendDlgItemMessage(this->m_hWnd, trackBarId, TBM_GETPOS, 0, 0);
		if (dwPos <= 0xff)
		{
			r = _stprintf_s(numstr, _countof(numstr), TEXT("%d"), (int)dwPos);
			if (r >= 0)
			{
				::SetDlgItemText(this->m_hWnd, editId, numstr);
			}
		}

		UpdateSelectedPaletteItem();
	}
}

void CDiagColour::UpdateSelectedPaletteItem()
{
DWORD dwPosRed;
DWORD dwPosGreen;
DWORD dwPosBlue;

	dwPosRed = (DWORD)::SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORRED, TBM_GETPOS, 0, 0);
	dwPosGreen = (DWORD)::SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORGREEN, TBM_GETPOS, 0, 0);
	dwPosBlue = (DWORD)::SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORBLUE, TBM_GETPOS, 0, 0);

	dwPosRed = min(dwPosRed, 0xff);
	dwPosGreen = min(dwPosGreen, 0xff);
	dwPosBlue = min(dwPosBlue, 0xff);

	bit32 cl = (dwPosRed << 16) | (dwPosGreen << 8) | (dwPosBlue);
	this->largepaletteitem.RGBColour = cl;
	if (this->selectedpaletteitem)
	{
		selectedpaletteitem->RGBColour = cl;
		if (this->selectedpaletteitem->IsPalette)
		{
			::InvalidateRect(this->selectedpaletteitem->HWnd, NULL, FALSE);
		}
	}

	::InvalidateRect(this->largepaletteitem.HWnd, NULL, FALSE);	
}

ColourControlState* CDiagColour::FindPaletteControlById(int id)
{
unsigned int i;
ColourControlState* pcontrol = NULL;

	if (id == largepaletteitem.ControlId)
	{
		//Found a match on the large palette item control.
		pcontrol=&largepaletteitem;
	}
	else
	{
		for (i=0 ; i < NumColours ; i++)
		{
			if (id == controlpalette[i].ControlId)
			{
				//Found a match on a small palette item control.
				pcontrol=&controlpalette[i];
				break;
			}
		}
	}
	return pcontrol;
}

ColourControlState* CDiagColour::FindPaletteControlByWindow(HWND hWnd)
{
unsigned int i;
ColourControlState* pcontrol = NULL;

	if (hWnd == largepaletteitem.HWnd)
	{
		//Found a match on the large palette item control.
		pcontrol=&largepaletteitem;
	}
	else
	{
		for (i=0 ; i < NumColours ; i++)
		{
			if (hWnd == controlpalette[i].HWnd)
			{
				//Found a match on a small palette item control.
				pcontrol=&controlpalette[i];
				break;
			}
		}
	}
	return pcontrol;
}

LRESULT CDiagColour::OnEventVicColorControl(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int id;
ColourControlState* pcontrol = NULL;
ColourControlState* pcontrolprevious = NULL;

	//Search which palette control this message belongs to.
	id = GetDlgCtrlID(hwnd);
	if (id == 0)
	{
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	pcontrol = FindPaletteControlById(id);
	if (pcontrol == NULL)
	{
		//Could not match a palette item control.
		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	}

	switch (uMsg)
	{
	case WM_SETTEXT:
		return 0;
	case WM_PAINT:
		PaintControl(pcontrol);
		break;
	case WM_GETDLGCODE:
		return DLGC_UNDEFPUSHBUTTON;
	case WM_NCHITTEST:
		return HTCLIENT;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			SendMessage(hwnd, WM_SELECT_PALLETE_ITEM, 1, 0);
			return DLGC_WANTCHARS | DLGC_WANTMESSAGE | DLGC_UNDEFPUSHBUTTON;
		}

		return DefWindowProc(hwnd,uMsg,wParam,lParam);
	case WM_LBUTTONDOWN:
		SendMessage(hwnd, WM_SELECT_PALLETE_ITEM, 1, 0);

		return 0;
	case WM_SELECT_PALLETE_ITEM:
		if (wParam == 1)
		{
			if (pcontrol->IsPalette)
			{
				pcontrolprevious = SelectPaletteItem(pcontrol, true);
				NotifyColour notify;
				ZeroMemory(&notify, sizeof(NotifyColour));
				notify.code = NotifyColour::SelectionChanged;
				notify.hwndFrom = hwnd;
				notify.idFrom = pcontrol->ControlId;
				notify.paletteitem = pcontrol;				
				if (pcontrolprevious != pcontrol)
				{
					SendMessage(GetParent(hwnd), WM_NOTIFY, pcontrol->ControlId, (LPARAM)&notify);
				}

				if (!pcontrol->IsFocused)
				{
					SetFocus(hwnd);
				}
			}
		}
		return 0;
	case WM_SETFOCUS:
		pcontrol->IsFocused = true;
		InvalidateRect(pcontrol->HWnd, NULL, TRUE);
		return 0;
	case WM_KILLFOCUS:
		pcontrol->IsFocused = false;
		InvalidateRect(pcontrol->HWnd, NULL, TRUE);
		return 0;
	}

	return DefWindowProc(hwnd,uMsg,wParam,lParam);
}

void CDiagColour::UpdateSliders(ColourControlState* pcontrol)
{
TCHAR numstr[10];
int r;
	if (pcontrol == NULL || !pcontrol->IsPalette)
	{
		return;
	}

	int red = (int)((pcontrol->RGBColour >> 16) & 0xff);
	int green = (int)((pcontrol->RGBColour >> 8) & 0xff);
	int blue = (int)((pcontrol->RGBColour) & 0xff);
	r = _stprintf_s(numstr, _countof(numstr), TEXT("%d"), red);
	if (r > 0)
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORRED, numstr);
	}
	else
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORRED, G::EmptyString);
	}

	r = _stprintf_s(numstr, _countof(numstr), TEXT("%d"), green);
	if (r > 0)
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORGREEN, numstr);
	}
	else
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORGREEN, G::EmptyString);
	}

	r = _stprintf_s(numstr, _countof(numstr), TEXT("%d"), blue);
	if (r > 0)
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORBLUE, numstr);
	}
	else
	{
		SetDlgItemText(this->m_hWnd, IDC_EDITRGBCOLORBLUE, G::EmptyString);
	}

	SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORRED, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) red);
	SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORGREEN, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) green);
	SendDlgItemMessage(this->m_hWnd, IDC_SLIDERRGBCOLORBLUE, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) blue);
}

ColourControlState *CDiagColour::SelectPaletteItem(ColourControlState* pcontrol, bool redraw)
{
unsigned int i;
ColourControlState *p;
bool isChanged = false;

	ColourControlState *pcurrent = selectedpaletteitem;
	if (pcurrent != pcontrol)
	{
		isChanged = true;
	}

	if (isChanged)
	{
		for (i = 0; i < NumColours; i++)
		{
			p = &controlpalette[i];
			if (pcontrol != p)
			{
				if (p->IsFocused)
				{
					if (redraw)
					{
						InvalidateRect(p->HWnd, NULL, TRUE);
					}
				}
				else if (p->IsSelected)
				{
					p->IsSelected = false;
					if (redraw)
					{
						InvalidateRect(p->HWnd, NULL, TRUE);
					}
				}
			}
		}

		if (selectedpaletteitem)
		{
			selectedpaletteitem->IsSelected = false;
		}

		selectedpaletteitem = pcontrol;
		if (selectedpaletteitem)
		{
			selectedpaletteitem->IsSelected = true;
			largepaletteitem.RGBColour = selectedpaletteitem->RGBColour;
		}


		if (redraw)
		{
			if (selectedpaletteitem)
			{
				InvalidateRect(selectedpaletteitem->HWnd, NULL, TRUE);
			}
			InvalidateRect(largepaletteitem.HWnd, NULL, TRUE);
		}
	}
	return pcurrent;
}

void CDiagColour::DrawSelectBox(HDC hdc, const RECT& rc)
{
	//Save old pen.
	HPEN hPenOld = (HPEN)SelectObject(hdc, hPen_shadow); 

	//Draw top and left.
	MoveToEx(hdc, rc.left,rc.top, NULL);
	LineTo(hdc,rc.right,rc.top);
	MoveToEx(hdc, rc.left,rc.top, NULL);
	LineTo(hdc,rc.left,rc.bottom);

	//Draw top and left inset 1 unit.
	SelectObject(hdc, hPen_darkshadow); 
	MoveToEx(hdc, rc.left+1,rc.top+1, NULL);
	LineTo(hdc,rc.right-1,rc.top+1);
	MoveToEx(hdc, rc.left+1,rc.top+1, NULL);
	LineTo(hdc,rc.left+1,rc.bottom-1);

	//Draw right and bottom.
	SelectObject(hdc, hPen_highlight); 
	MoveToEx(hdc, rc.right,rc.top, NULL);
	LineTo(hdc,rc.right,rc.bottom);
	MoveToEx(hdc, rc.left,rc.bottom, NULL);
	LineTo(hdc,rc.right,rc.bottom);

	//Restore old pen
	if (hPenOld)
	{
		SelectObject(hdc, hPenOld); 
	}
}

void CDiagColour::PaintControl(const ColourControlState* pcontrol)
{
HDC hdc;
PAINTSTRUCT ps;
RECT rc;
RECT clientRect;
HRGN bgRgn;
HBRUSH hBrushPalette;
HBRUSH hBrushHighLight;
HBRUSH hBrushButton;
COLORREF colorHighLight;
COLORREF colorButton;
TEXTMETRIC tm;
COLORREF oldBkColor;
HBRUSH oldBrush = NULL;

	if (GetUpdateRect(pcontrol->HWnd, &rc, FALSE)==0)
	{
		return;
	}

	bgRgn = 0;
	hBrushPalette = 0;
	hdc = BeginPaint(pcontrol->HWnd, &ps); 
	if (hdc != NULL)
	{
		LONG textHeight = 16;
		if (GetTextMetrics(hdc, &tm))
		{
			textHeight = tm.tmHeight;
		}

		oldBkColor = ::GetBkColor(hdc);
		const LONG padding = 2;
		GetClientRect(pcontrol->HWnd, &clientRect);
		RECT rcColorBlock;
		CopyRect(&rcColorBlock, &clientRect);
		if (pcontrol->IsPalette)
		{
			RECT rcColorTitle;
			TCHAR szTitle[10];

			hBrushHighLight = ::GetSysColorBrush(COLOR_WINDOW);
			hBrushButton = ::GetSysColorBrush(COLOR_BTNFACE);
			colorHighLight = ::GetSysColor(COLOR_WINDOW);
			colorButton = ::GetSysColor(COLOR_BTNFACE);
			CopyRect(&rcColorTitle, &clientRect);
			rcColorTitle.bottom = rcColorTitle.top + textHeight + padding * 2 + 1;
			if (rcColorTitle.bottom > clientRect.bottom)
			{
				rcColorTitle.bottom = clientRect.bottom;
			}

			if (pcontrol->IsSelected)
			{
				::SetBkColor(hdc, colorHighLight);
				::FillRect(hdc, &rcColorTitle, hBrushHighLight);
			}
			else
			{
				::SetBkColor(hdc, colorButton);
				::FillRect(hdc, &rcColorTitle, hBrushButton);
			}
			//hBrushHighLight

			rcColorBlock.top = rcColorTitle.bottom;			
			unsigned int colourIndex = pcontrol->VicColour & 0xf;
			int r = _stprintf_s(szTitle, _countof(szTitle), TEXT("%d"), colourIndex);
			if (r > 0)
			{
				DrawText(hdc, szTitle, lstrlen(szTitle), &rcColorTitle, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			}
		}

		if (rcColorBlock.top < rcColorBlock.bottom)
		{
			bgRgn = CreateRectRgnIndirect(&rcColorBlock);
			if (bgRgn)
			{
				bit32 cl = pcontrol->RGBColour;
				COLORREF color = RGB((cl >> 16) & 0xff, (cl >> 8)  & 0xff, cl & 0xff);
				hBrushPalette = CreateSolidBrush(color);
				if (hBrushPalette)
				{
					FillRgn(hdc, bgRgn, hBrushPalette);
				}
			}
		}

		if (pcontrol->IsSelected)
		{
			DrawSelectBox(hdc, clientRect);
		}
		else if (pcontrol->IsFocused)
		{
			::DrawFocusRect(hdc, &clientRect);
		}

		::SetBkColor(hdc, oldBkColor);
		if (oldBrush)
		{
			SelectObject(hdc, oldBrush);
		}
	}

    if (hBrushPalette)
	{
		DeleteObject(hBrushPalette);
	}

	if (bgRgn)
	{
		DeleteObject(bgRgn);
	}

	EndPaint(pcontrol->HWnd, &ps); 
}

LRESULT CALLBACK VicColorWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CDiagColour* pWin = (CDiagColour*)(LONG_PTR) GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (uMsg)
	{
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
		return (pWin->OnEventVicColorControl(hWnd, uMsg, wParam, lParam));
	}
	else
	{
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
}
