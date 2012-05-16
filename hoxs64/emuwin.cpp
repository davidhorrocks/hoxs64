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
#include "boost2005.h"
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
#include "register.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "tap.h"
#include "filter.h"
#include "sid.h"
#include "sidfile.h"
#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "diskinterface.h"
#include "t64.h"
#include "C64.h"
#include "emuwin.h"
#include "dchelper.h"
#include "resource.h"


const LPTSTR CEmuWindow::lpszClassName = HOXS_EMULATION_WND_CLASS;

CEmuWindow::CEmuWindow(CDX9 *dx, CConfig *cfg, CAppStatus *appStatus, C64 *c64)
{
	m_dwSolidColourFill=0;
	m_bDrawCycleCursor = false;
	m_iVicCycleCursor = 1;
	m_iVicLineCursor = 0;
	m_pINotify = 0;
	m_bDragMode = false;

	this->dx = dx;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;
}

CEmuWindow::~CEmuWindow()
{
	int i=0;
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
		return;
	if (dx->m_pd3dDevice == NULL)
		return;
	D3DDISPLAYMODE currentDisplayMode;
	HRESULT hr = dx->m_pd3dDevice->GetDisplayMode(0, &currentDisplayMode);
	if (SUCCEEDED(hr))
	{
		if (appStatus!=NULL)
		{
			appStatus->m_displayFormat = (DWORD)currentDisplayMode.Format;
			appStatus->m_ScreenDepth = CDX9::GetBitsPerPixel(currentDisplayMode.Format);

			if (c64!=NULL)
				c64->vic.setup_color_tables(currentDisplayMode.Format);

			switch(appStatus->m_ScreenDepth)
			{
			case 8:
				m_dwSolidColourFill = VIC6569::vic_color_array8[0];
				break;
			case 16:
				m_dwSolidColourFill = VIC6569::vic_color_array16[0];
				break;
			case 24:
				m_dwSolidColourFill = VIC6569::vic_color_array24[0];
				break;
			case 32:
				m_dwSolidColourFill = VIC6569::vic_color_array32[0];
				break;
			default:
				m_dwSolidColourFill = 0;
			}
		}

	}
}

void CEmuWindow::SetNotify(INotify *pINotify)
{
	m_pINotify = pINotify;
}

void CEmuWindow::GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line)
{
	int s = dx->m_displayStart;
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
	int s = dx->m_displayStart;
	int st = dx->m_displayFirstVicRaster;
	int sb = dx->m_displayLastVicRaster;
	int wc = dx->m_displayWidth;
	int hc = dx->m_displayHeight;
	int wp = dx->m_rcTargetRect.right - dx->m_rcTargetRect.left;
	int hp = dx->m_rcTargetRect.bottom - dx->m_rcTargetRect.top;
		
	rcDisplay.left = (rcVicCycle.left - s) * wp / wc;
	rcDisplay.right = (rcVicCycle.right - s) * wp / wc;

	rcDisplay.top = (rcVicCycle.top - st) * hp / hc;
	rcDisplay.bottom = (rcVicCycle.bottom - st) * hp / hc;

	OffsetRect(&rcDisplay, dx->m_rcTargetRect.left, dx->m_rcTargetRect.top);
}


void CEmuWindow::DisplayVicCursor(bool bEnabled)
{
	m_bDrawCycleCursor = bEnabled;
}

void CEmuWindow::SetVicCursorPos(int iCycle, int iLine)
{
	m_iVicCycleCursor = iCycle;
	m_iVicLineCursor = iLine;
	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		DrawCursorAtVicPosition(iCycle, iLine);
	}
}

void CEmuWindow::GetVicCursorPos(int *piCycle, int *piLine)
{
	if (piCycle)
		*piCycle = m_iVicCycleCursor;
	if (piLine)
		*piLine = m_iVicLineCursor;
}

void CEmuWindow::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);

		m_bDragMode = true;
		SetCapture(hWnd);
		DrawCursorAtClientPosition(x, y);
		if (m_pINotify)
			m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);

		m_iLastX = x;
		m_iLastY = y;
	}
}

void CEmuWindow::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		int x = GET_X_LPARAM(lParam);
		int y = GET_Y_LPARAM(lParam);
		ReleaseCapture();
		if (m_bDragMode)
		{
			DrawCursorAtClientPosition(x, y);
			if (m_pINotify)
				m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);

			m_iLastX = x;
			m_iLastY = y;
		}
		m_bDragMode = false;
	}
}

bool CEmuWindow::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		if (m_bDragMode)
		{
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);

			DrawCursorAtClientPosition(x, y);
			if (m_pINotify)
				m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);

			m_iLastX = x;
			m_iLastY = y;
		}

	}
	return true;
}

void CEmuWindow::DrawCursorAtClientPosition(int x, int y)
{
	GetVicRasterPositionFromClientPosition(x, y, m_iVicCycleCursor, m_iVicLineCursor);

	DrawCursorAtVicPosition(m_iVicCycleCursor, m_iVicLineCursor);
}

void CEmuWindow::DrawCursorAtVicPosition(int cycle, int line)
{
RECT rcVicCycle;

	this->UpdateC64Window();

	HWND hWnd = this->GetHwnd();
	SetRect(&rcVicCycle, (cycle - 1) * 8, line, (cycle) * 8, line + 1);
	GetDisplayRectFromVicRect(rcVicCycle, m_rcCycleCursor);

	HDC hdc = GetDC(hWnd);
	if (hdc)
	{
		{
			DcHelper dch(hdc);
			BOOL br = FillRect(hdc, &m_rcCycleCursor, (HBRUSH) (COLOR_WINDOW+1));
			br = br;
		}
		ReleaseDC(hWnd, hdc);
	}
}

LRESULT CEmuWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//HDC hdc;
//PAINTSTRUCT ps;
//RECT rc;
	switch (uMsg) 
	{
		case WM_CREATE:
			return 0;
		case WM_PAINT:
			if (appStatus->m_bWindowed)
			{
				UpdateC64Window();
				//if (GetUpdateRect(hWnd, &rc, FALSE)!=0)
				//{
				//	hdc = BeginPaint (hWnd, &ps);
				//	if (hdc)
				//	{
				//		BOOL br = FillRect(hdc, &rc, (HBRUSH) (COLOR_WINDOW+1));
				//	}
				//	EndPaint(hWnd, &ps);
				//}
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
Function: UpdateC64Window
Description:
Draws the small dx surface to the dx back buffer.
Draws the drive sprites to the dx back buffer.
*/
HRESULT CEmuWindow::RenderWindow()
{
HRESULT hr;
	if (!dx)
		return E_FAIL;
	if (!dx->m_pd3dDevice)
		return E_FAIL;	

	//Blank out any regions that are outside of the C64 display area.
	dx->ClearTargets(m_dwSolidColourFill);
	if(SUCCEEDED(hr = dx->m_pd3dDevice->BeginScene()))
	{
		//Draw from the dx small surface to the dx backbuffer
		hr = dx->UpdateBackbuffer((D3DTEXTUREFILTERTYPE)appStatus->m_blitFilter);
		if (cfg->m_bShowFloppyLed)
			DrawDriveSprites();
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

	if (!dx || !c64)
		return E_FAIL;
	if (!dx->m_pd3dDevice)
		return E_FAIL;
	//VIC6569::UpdateBackBuffer() draws pixels that have been buffered by the VIC class to the dx small surface.
	hr = this->c64->vic.UpdateBackBuffer();
	if (SUCCEEDED(hr))
	{
		//CEmuWindow::RenderWindow Stretch-blits from the dx small surface to the dx backbuffer.
		hr = RenderWindow();
		if (SUCCEEDED(hr))
		{
			hr = dx->m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
			if (FAILED(hr))// D3DERR_DEVICELOST
			{
				hr = dx->m_pd3dDevice->TestCooperativeLevel();
				if (FAILED(hr))// D3DERR_DEVICELOST)
				{
					appStatus->m_bReady = false;
					appStatus->SoundHalt();
				}
			}
		}
	}
	return hr;
}

void CEmuWindow::DrawDriveSprites()
{
HRESULT hr;

	if (SUCCEEDED(hr = dx->m_psprLedMotor->Begin(0)))
	{
		if (appStatus->m_bDiskLedMotor)
			dx->m_psprLedMotor->Draw(dx->m_ptxLedGreenOn, NULL, NULL, &dx->m_vecPositionLedMotor, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));
		else
			dx->m_psprLedMotor->Draw(dx->m_ptxLedGreenOff, NULL, NULL, &dx->m_vecPositionLedMotor, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));

		dx->m_psprLedMotor->End();
	}

	if (SUCCEEDED(hr = dx->m_psprLedDrive->Begin(0)))
	{
		if (appStatus->m_bDiskLedDrive)
			dx->m_psprLedDrive->Draw(dx->m_ptxLedBlueOn, NULL, NULL, &dx->m_vecPositionLedDrive, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));
		else
			dx->m_psprLedDrive->Draw(dx->m_ptxLedBlueOff, NULL, NULL, &dx->m_vecPositionLedDrive, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));

		dx->m_psprLedDrive->End();
	}

	if (SUCCEEDED(hr = dx->m_psprLedDrive->Begin(0)))
	{
		if (appStatus->m_bDiskLedWrite)
			dx->m_psprLedDrive->Draw(dx->m_ptxLedRedOn, NULL, NULL, &dx->m_vecPositionLedWrite, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));
		else
			dx->m_psprLedDrive->Draw(dx->m_ptxLedRedOff, NULL, NULL, &dx->m_vecPositionLedWrite, D3DCOLOR_RGBA(0xff, 0xff, 0xff, 0xff ));

		dx->m_psprLedDrive->End();
	}
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
