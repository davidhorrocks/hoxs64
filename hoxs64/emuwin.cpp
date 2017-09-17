#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <assert.h>
#include "CDPI.h"
#include "dchelper.h"
#include "utils.h"

#include "C64.h"
#include "emuwin.h"
#include "resource.h"


const LPTSTR CEmuWindow::lpszClassName = HOXS_EMULATION_WND_CLASS;

CEmuWindow::CEmuWindow(CDX9 *dx, CConfig *cfg, CAppStatus *appStatus, IC64 *c64)
{
	m_dwSolidColourFill=0;
	m_bDrawCycleCursor = false;
	m_bDrawVicRasterBreakpoints = false;
	m_iVicCycleCursor = 1;
	m_iVicLineCursor = 0;
	m_pINotify = 0;
	m_bDragMode = false;
	m_hBrushBrkInner = NULL;
	m_hBrushBrkOuter = NULL;

	this->dx = dx;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;

	m_hBrushBrkInner = CreateSolidBrush(RGB(255, 220, 220));
	m_hBrushBrkOuter = CreateSolidBrush(RGB(10, 40, 40));
}

CEmuWindow::~CEmuWindow()
{
	if (m_hBrushBrkInner)
	{
		DeleteObject(m_hBrushBrkInner);
		m_hBrushBrkInner = NULL;
	}
	if (m_hBrushBrkOuter)
	{
		DeleteObject(m_hBrushBrkOuter);
		m_hBrushBrkOuter = NULL;
	}
}

HRESULT CEmuWindow::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

    // Fill in window class structure with parameters that describe
    // the main window.
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_PARENTDC | CS_SAVEBITS;
	wc.lpfnWndProc		= (WNDPROC)::WindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof(CEmuWindow *);
	wc.hInstance		= hInstance;
	wc.hIcon			= 0;//LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);//(COLOR_WINDOW+1);
	wc.lpszMenuName		= NULL;
    wc.lpszClassName	= lpszClassName;
	wc.hIconSm			= NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	else
		return S_OK;
}
HWND CEmuWindow::Create(HINSTANCE hInstance, HWND parentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU controlID)
{
	this->m_hInst = hInstance;
	return CVirWindow::CreateVirWindow(0L, lpszClassName, title, WS_CHILD | WS_VISIBLE, x, y, w, h, parentWindow, controlID, hInstance);
}


void CEmuWindow::ClearSurfaces()
{
	if (dx)
		dx->ClearSurfaces(m_dwSolidColourFill);
}

void CEmuWindow::SetColours()
{
	if (dx == NULL)
	{
		return;
	}

	if (dx->m_pd3dDevice == NULL)
	{
		return;
	}

	D3DDISPLAYMODE currentDisplayMode;
	dx->SetDefaultPalette();
	HRESULT hr = dx->m_pd3dDevice->GetDisplayMode(0, &currentDisplayMode);
	if (SUCCEEDED(hr))
	{
		if (appStatus != NULL)
		{
			appStatus->m_displayFormat = (DWORD)currentDisplayMode.Format;
			appStatus->m_ScreenDepth = CDX9::GetBitsPerPixel(currentDisplayMode.Format);

			if (c64!=NULL)
			{
				c64->SetupColorTables(currentDisplayMode.Format);
			}

			m_dwSolidColourFill = dx->ConvertColour2(currentDisplayMode.Format, RGB(0, 0, 0));
		}
	}
}

void CEmuWindow::SetNotify(INotify *pINotify)
{
	m_pINotify = pINotify;
}

void CEmuWindow::GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line)
{
	int s = dx->m_displayStart + DISPLAY_START;
	int st = dx->m_displayFirstVicRaster;
	int sb = dx->m_displayLastVicRaster;
	int wc = dx->m_displayWidth;
	int hc = dx->m_displayHeight;
	int wp = dx->m_rcTargetRect.right - dx->m_rcTargetRect.left;
	int hp = dx->m_rcTargetRect.bottom - dx->m_rcTargetRect.top;

	cycle = (s + ((x * wc) / wp)) & ~7;
	cycle = cycle / 8;
	cycle += 1;
	if (cycle < 1)
		cycle = 1;
	if (cycle > PAL_CLOCKS_PER_LINE)
		cycle = PAL_CLOCKS_PER_LINE;

	line = st + ((y * hc) / hp);
	line -= 1;		

	if (line < 0)
		line = 0;
	if (line > PAL_MAX_LINE)
		line = PAL_MAX_LINE;
}

void CEmuWindow::GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle)
{
	int cycle;
	int line;
	GetVicRasterPositionFromClientPosition(x, y, cycle, line);
	SetRect(&rcVicCycle, (cycle - 1) * 8, line, (cycle) * 8, line + 1);
}

void CEmuWindow::GetDisplayRectFromVicRect(const RECT rcVicCycle, RECT& rcDisplay)
{
	int s = dx->m_displayStart + DISPLAY_START;
	int st = dx->m_displayFirstVicRaster;
	int sb = dx->m_displayLastVicRaster;
	int wc = dx->m_displayWidth;
	int hc = dx->m_displayHeight;
	int wp = dx->m_rcTargetRect.right - dx->m_rcTargetRect.left;
	int hp = dx->m_rcTargetRect.bottom - dx->m_rcTargetRect.top;
		
	double left = ::ceil((double)(rcVicCycle.left - s) * (double)wp / (double)wc);
	double right = ::floor((double)(rcVicCycle.right - s) * (double)wp / (double)wc);
	rcDisplay.left = (LONG)left;
	rcDisplay.right = (LONG)right;
	if (right <= left)
		right = left + 1;

	double top = ::ceil((double)(rcVicCycle.top - st) * (double)hp / (double)hc);
	double bottom = ::floor((double)(rcVicCycle.bottom - st) * (double)hp / (double)hc);
	if (bottom <= top)
		bottom = top + 1;
	rcDisplay.top = (LONG)top;
	rcDisplay.bottom = (LONG)bottom;

	OffsetRect(&rcDisplay, dx->m_rcTargetRect.left, dx->m_rcTargetRect.top);
}


void CEmuWindow::DisplayVicCursor(bool bEnabled)
{
	m_bDrawCycleCursor = bEnabled;
}

void CEmuWindow::DisplayVicRasterBreakpoints(bool bEnabled)
{
	m_bDrawVicRasterBreakpoints = bEnabled;
}

void CEmuWindow::SetVicCursorPos(int iCycle, int iLine)
{
	m_iVicCycleCursor = iCycle;
	m_iVicLineCursor = iLine;
}

void CEmuWindow::GetVicCursorPos(int *piCycle, int *piLine)
{
	if (piCycle)
	{
		*piCycle = m_iVicCycleCursor;
	}

	if (piLine)
	{
		*piLine = m_iVicLineCursor;
	}
}

void CEmuWindow::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (G::IsHideWindow)
	{
		return;
	}

	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		m_bDragMode = true;
		SetCapture(hWnd);
		SetCursorAtClientPosition(x, y);
		UpdateC64WindowWithObjects();
		if (m_pINotify)
		{
			m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);
		}

		m_iLastX = x;
		m_iLastY = y;
	}
}

void CEmuWindow::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (G::IsHideWindow)
	{
		return;
	}

	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		ReleaseCapture();
		if (m_bDragMode)
		{
			SetCursorAtClientPosition(x, y);
			UpdateC64WindowWithObjects();
			if (m_pINotify)
			{
				m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);
			}

			m_iLastX = x;
			m_iLastY = y;
		}

		m_bDragMode = false;
	}
}

bool CEmuWindow::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	if (G::IsHideWindow)
	{
		return true;
	}

	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		if (m_bDragMode)
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			SetCursorAtClientPosition(x, y);
			UpdateC64WindowWithObjects();
			if (m_pINotify)
			{
				m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);
			}

			m_iLastX = x;
			m_iLastY = y;
		}

	}
	return true;
}

void CEmuWindow::SetCursorAtClientPosition(int x, int y)
{
int cycle, line;
	GetVicRasterPositionFromClientPosition(x, y, cycle, line);
	this->SetVicCursorPos(cycle, line);
}

void CEmuWindow::UpdateC64WindowWithObjects()
{
	if (G::IsHideWindow)
	{
		return;
	}

	HRESULT hr = this->UpdateC64Window();
	if (SUCCEEDED(hr))
	{
		this->Present(0);
	}

	if (appStatus->m_bWindowed && appStatus->m_bDebug && !appStatus->m_bRunning && m_bDrawVicRasterBreakpoints)
	{
		HWND hWnd = this->GetHwnd();
		HDC hdc = GetDC(hWnd);
		if (hdc)
		{
			DrawAllCursors(hdc);
			ReleaseDC(hWnd, hdc);
		}
	}
}

void CEmuWindow::DrawCursorAtVicPosition(HDC hdc, int cycle, int line)
{
RECT rcVicCycle;
RECT rcDisplay;
RECT rcDisplay2;

	SetRect(&rcVicCycle, (cycle - 1) * 8, line, (cycle) * 8, line + 1);
	GetDisplayRectFromVicRect(rcVicCycle, rcDisplay);
	CopyRect(&rcDisplay2, &rcDisplay);
	int p = (rcDisplay.bottom - rcDisplay.top);
	int dx = p;
	int dy = p;
	InflateRect(&rcDisplay2, dx, dy);
	HBRUSH hOuter = m_hBrushBrkOuter;
	HBRUSH hInner = m_hBrushBrkInner;
	if (!hInner)
		hInner = (HBRUSH)GetStockObject(WHITE_BRUSH);
	if (!hOuter)
		hOuter = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	 
	FillRect(hdc, &rcDisplay2, (HBRUSH) hOuter);
	FillRect(hdc, &rcDisplay, (HBRUSH) hInner);
}

void CEmuWindow::DrawAllCursors(HDC hdc)
{
	DcHelper dch(hdc);
	if (m_bDrawCycleCursor)
	{
		DrawCursorAtVicPosition(hdc, m_iVicCycleCursor, m_iVicLineCursor);
	}
	IEnumBreakpointItem *p = c64->GetMon()->BM_CreateEnumBreakpointItem();
	if (p)
	{
		Sp_BreakpointItem v;
		while (p->GetNext(v))
		{
			if (v->bptype == DBGSYM::BreakpointType::VicRasterCompare)
			{
				DrawCursorAtVicPosition(hdc, v->vic_cycle, v->vic_line);
			}
		}
	}
}

LRESULT CEmuWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) 
	{
		case WM_CREATE:
			return 0;
		case WM_PAINT:
			if (appStatus->m_bWindowed)
			{
				UpdateC64WindowWithObjects();
			}
			ValidateRect(hWnd, NULL);
			return 0;
		case WM_LBUTTONDOWN:
			OnLButtonDown(hWnd, uMsg, wParam, lParam);
			return 0;
		case WM_MOUSEMOVE:
			OnMouseMove(hWnd, uMsg, wParam, lParam);
			return 0;
		case WM_LBUTTONUP:
			OnLButtonUp(hWnd, uMsg, wParam, lParam);
			return 0;
		case WM_ERASEBKGND:
			return 1;
	}	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*
Function: RenderWindow
Description:
Draws the small dx surface to the dx back buffer.
Draws the drive sprites to the dx back buffer.
*/
HRESULT CEmuWindow::RenderWindow()
{
HRESULT hr;
	if (!dx)
	{
		return E_FAIL;
	}

	if (!dx->m_pd3dDevice)
	{
		return E_FAIL;	
	}

	//Blank out any regions that are outside of the C64 display area.
	dx->ClearTargets(m_dwSolidColourFill);
	if(SUCCEEDED(hr = dx->m_pd3dDevice->BeginScene()))
	{
		//Draw from the dx small surface to the dx backbuffer
		hr = dx->UpdateBackBuffer((D3DTEXTUREFILTERTYPE)appStatus->m_blitFilterDX);
		if (cfg->m_bShowFloppyLed)
		{
			dx->DrawDriveSprites();
		}
		dx->DrawUi();
		dx->m_pd3dDevice->EndScene();
	}
	return hr;
}

/*
Function: UpdateC64Window
Description:
Draws a full C64 screen to the current DirectX backbuffer and presents the display. 
The display consists of two parts separated by the current VIC output cursor position. 
Pixels from the top of the display to the current VIC output cursor position are from 
the current C64 frame. Pixels from beyond the current VIC output cursor position to 
the bottom of the display are from the previous C64 frame.
*/
HRESULT CEmuWindow::UpdateC64Window()
{
HRESULT hr = E_FAIL;

	if (G::IsHideWindow)
	{
		return S_FALSE;
	}

	if (!dx || !c64)
	{
		return E_FAIL;
	}

	if (!dx->m_pd3dDevice)
	{
		return E_FAIL;
	}

	//VIC6569::UpdateBackBuffer() draws pixels that have been buffered by the VIC class to the dx small surface.
	hr = this->c64->UpdateBackBuffer();
	if (SUCCEEDED(hr))
	{
		//CEmuWindow::RenderWindow Stretch-blits from the dx small surface to the dx backbuffer.
		hr = RenderWindow();
	}
	return hr;
}

HRESULT CEmuWindow::Present(DWORD dwFlags)
{
	return dx->Present(dwFlags);
}

void CEmuWindow::GetRequiredWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bDoubleSizedWindow, int *w, int *h)
{
C64WindowDimensions dims;

	dims.SetBorder(borderSize);

	if (bDoubleSizedWindow)
	{
		*w=dims.Width*2;
		*h=dims.Height*2 + CDX9::GetToolBarHeight(bShowFloppyLed);
	}
	else
	{
		*w=dims.Width;
		*h=dims.Height +  CDX9::GetToolBarHeight(bShowFloppyLed);
	}
}
