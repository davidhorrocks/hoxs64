#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
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
#include "resource.h"



const LPTSTR CEmuWindow::lpszClassName = HOXS_EMULATION_WND_CLASS;

CEmuWindow::CEmuWindow()
{
	m_AutoDelete = false;
	m_hInstance = 0;
	m_dwSolidColourFill=0;
}

HRESULT CEmuWindow::Init(CDX9 *dx, CConfig *cfg, CAppStatus *appStatus, C64 *c64)
{
	this->dx = dx;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;
	return S_OK;
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
	this->m_hInstance = hInstance;
	return CVirWindow::Create(0L, lpszClassName, title, WS_CHILD | WS_VISIBLE, x, y, w, h, parentWindow, controlID, hInstance);
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

LRESULT CEmuWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
//HDC hdc;
//PAINTSTRUCT ps;
//RECT rc;
	switch (uMsg) 
	{
		case WM_CREATE:
			return 0;
		case WM_PAINT:
			hr = E_FAIL;
			if (appStatus->m_bWindowed)
			{
				hr = UpdateC64Window();
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
		case WM_ERASEBKGND:
			return 1;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}	
	return 0;
}

HRESULT CEmuWindow::RenderWindow()
{
HRESULT hr;
	if (!dx)
		return E_FAIL;
	if (!dx->m_pd3dDevice)
		return E_FAIL;	

	dx->ClearTargets(m_dwSolidColourFill);
	if(SUCCEEDED(hr = dx->m_pd3dDevice->BeginScene()))
	{
		hr = dx->UpdateBackbuffer((D3DTEXTUREFILTERTYPE)appStatus->m_blitFilter);
		if (cfg->m_bShowFloppyLed)
			DrawDriveSprites();
		dx->m_pd3dDevice->EndScene();
	}
	return hr;
}

HRESULT CEmuWindow::UpdateC64Window()
{
HRESULT hr = E_FAIL;

	if (!dx || !c64)
		return E_FAIL;
	if (!dx->m_pd3dDevice)
		return E_FAIL;
	hr = this->c64->vic.UpdateBackBuffer();
	if (SUCCEEDED(hr))
	{
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
