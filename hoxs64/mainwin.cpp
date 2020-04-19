#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <winuser.h>
#include <Dwmapi.h>
#include <commctrl.h>
#include "dx_version.h"
#include <stdio.h>
#include <assert.h>
#include "IC64.h"
#include "CDPI.h"
#include "utils.h"
#include "besttextwidth.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "sidfile.h"
#include "p64.h"
#include "d64.h"
#include "t64.h"
#include "c64file.h"
#include "prgbrowse.h"
#include "diagkeyboard.h"
#include "diagjoystick.h"
#include "diagemulationsettingstab.h"
#include "diagnewblankdisk.h"
#include "diagabout.h"
#include "diagfilesaved64.h"
#include "diagcolour.h"
#include "edln.h"
#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpcbreakpoint.h"
#include "toolitemaddress.h"
#include "diagbreakpointvicraster.h"
#include "disassemblyreg.h"
#include "disassemblyeditchild.h"
#include "disassemblychild.h"
#include "disassemblyframe.h"
#include "wpccli.h"
#include "mdichildcli.h"
#include "mdidebuggerframe.h"
#include "mainwin.h"
#include "vicpixelbuffer.h"
#include "dchelper.h"
#include "resource.h"

const LPTSTR CAppWindow::lpszClassName = HOXS_MAIN_WND_CLASS;
const LPTSTR CAppWindow::lpszMenuName = APPMENUNAME;

CAppWindow::CAppWindow(Graphics* pGx, CDX9 *dx, IAppCommand *appCommand, CAppStatus *appStatus, IC64 *c64)
{
	this->pGx = pGx;
	this->dx = dx;
	this->appStatus = appStatus;
	this->c64 = c64;
	this->appCommand = appCommand;

	appStatus->m_bWindowed = true;
	SetRect(&m_rcMainWindow, 0, 0, 0, 0);
	m_hMenuOld = nullptr;
	m_hOldCursor = nullptr;

	m_bDrawCycleCursor = false;
	m_bDrawVicRasterBreakpoints = false;
	m_iVicCycleCursor = 1;
	m_iVicLineCursor = 0;
	m_pINotify = nullptr;
	m_bDragMode = false;
	m_hBrushBrkInner = nullptr;
	m_hBrushBrkOuter = nullptr;
	m_hBrushBrkInner = CreateSolidBrush(RGB(255, 220, 220));
	m_hBrushBrkOuter = CreateSolidBrush(RGB(10, 40, 40));
	m_dwStyle = 0;
	try
	{
		m_hCursorBusy = LoadCursor(0L, IDC_WAIT);
		{
			if (!m_hCursorBusy)
			{
				throw std::exception("LoadCursor failed.");
			}
		}
	}
	catch(...)
	{
		Cleanup();
		throw;
	}	
}

CAppWindow::~CAppWindow()
{
	Cleanup();
}

void CAppWindow::WindowRelease()
{
	this->keepAlive.reset();
}

void CAppWindow::Cleanup()
{
	if (m_hBrushBrkInner)
	{
		DeleteObject(m_hBrushBrkInner);
		m_hBrushBrkInner = nullptr;
	}
	if (m_hBrushBrkOuter)
	{
		DeleteObject(m_hBrushBrkOuter);
		m_hBrushBrkOuter = nullptr;
	}
}

HRESULT CAppWindow::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

    // Fill in window class structure with parameters that describe
    // the main window.
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_SAVEBITS;
	wc.lpfnWndProc		= (WNDPROC)::WindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= sizeof(CAppWindow *);
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);//(COLOR_WINDOW+1);
	wc.lpszMenuName		= lpszMenuName;
    wc.lpszClassName	= lpszClassName;
	wc.hIconSm			= NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	else
		return S_OK;
}

void CAppWindow::GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h)
{
C64WindowDimensions dims;

	dims.SetBorder(borderSize);
	*w=dims.Width + GetSystemMetrics(SM_CXSIZEFRAME)*2;
	*h=dims.Height + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) + pGx->c64display.GetToolBarHeight(bShowFloppyLed);
}

void CAppWindow::GetMinimumWindowedSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h)
{
	C64WindowDimensions dims;
	dims.SetBorder(borderSize);
	int width = dims.Width / 2 + GetSystemMetrics(SM_CXSIZEFRAME)*2;
	int height = dims.Height / 2 + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) + pGx->c64display.GetToolBarHeight(bShowFloppyLed);
	int v;
	v = GetSystemMetrics(SM_CXMINTRACK);	
	width = width >= v ? width : v;
	v = GetSystemMetrics(SM_CYMINTRACK);
	height = height >= v ? height : v;
	if (w)
	{
		*w = width;
	}

	if (h)
	{
		*h = height;
	}
}

void CAppWindow::CalcEmuWindowPadding(int *padw, int *padh)
{
	if (padw)
	{
		*padw = GetSystemMetrics(SM_CXSIZEFRAME) * 2;
	}

	if (padh)
	{
		*padh= GetSystemMetrics(SM_CYSIZEFRAME) * 2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION);
	}
}

HWND CAppWindow::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	this->m_hInst = hInstance;
	this->m_rcMainWindow.left = x;
	this->m_rcMainWindow.top = y;
	this->m_rcMainWindow.right = x + w;
	this->m_rcMainWindow.bottom = y + h;
	HWND hWnd = CVirWindow::CreateVirWindow(0L, lpszClassName, title, WindowStyles, x, y, w, h, hWndParent, hMenu, hInstance);
	return hWnd;
}

const DWORD CAppWindow::WindowStyles = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SIZEBOX;

void CAppWindow::AspectMaxSizing(HWND hWnd, int edge, RECT &rect)
{
	RECT rcWorkArea;
	CopyRect(&rcWorkArea, &rect);

	HMONITOR hMonitorInUse = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFOEX miMonitorInUse;
	ZeroMemory(&miMonitorInUse, sizeof(miMonitorInUse));
	miMonitorInUse.cbSize = sizeof(miMonitorInUse);
	if (GetMonitorInfo(hMonitorInUse, &miMonitorInUse))
	{			
		CopyRect(&rcWorkArea, &miMonitorInUse.rcWork);
		if (rect.top < miMonitorInUse.rcMonitor.top)
		{
			rcWorkArea.top -= (miMonitorInUse.rcMonitor.top - rect.top);
		}

		if (rect.bottom > miMonitorInUse.rcMonitor.bottom)
		{				
			rcWorkArea.bottom += (rect.bottom - miMonitorInUse.rcMonitor.bottom);
		}

		if (rect.left < miMonitorInUse.rcMonitor.left)
		{
			rcWorkArea.left -= (miMonitorInUse.rcMonitor.left - rect.left);
		}

		if (rect.right > miMonitorInUse.rcMonitor.right)
		{
			rcWorkArea.right += (rect.right - miMonitorInUse.rcMonitor.right);
		}
	}

	RECT rcMaxAspect;
	CopyRect(&rcMaxAspect, &rcWorkArea);
	this->AspectSizing(WMSZ_BOTTOM, rcMaxAspect);
	if (rcMaxAspect.right - rcMaxAspect.left > rcWorkArea.right - rcWorkArea.left)
	{
		CopyRect(&rcMaxAspect, &rcWorkArea);
		this->AspectSizing(WMSZ_RIGHT, rcMaxAspect);
	}

	this->AspectSizing(WMSZ_BOTTOMRIGHT, rect);
	if (rect.right - rect.left > rcMaxAspect.right - rcMaxAspect.left || rect.bottom - rect.top > rcMaxAspect.bottom - rcMaxAspect.top)
	{
		CopyRect(&rect, &rcMaxAspect);
	}
}

void CAppWindow::AspectSizing(int edge, RECT &rect)
{
	HCFG::EMUBORDERSIZE borderSize = this->appStatus->m_borderSize;
	C64WindowDimensions dims;
	dims.SetBorder(borderSize);
	int window_ratio_x = dims.Width;
	int window_ratio_y = dims.Height;
	int padw;
	int padh;
	CalcEmuWindowPadding(&padw, &padh);
	int window_adjust_x = padw;
	int window_adjust_y = padh + pGx->c64display.GetToolBarHeight(this->appStatus->m_bShowFloppyLed);
	int size_x_desired = (rect.right - rect.left) - window_adjust_x;
	int size_y_desired = (rect.bottom - rect.top) - window_adjust_y;
	switch (edge)
	{
		case WMSZ_BOTTOM:
		case WMSZ_TOP:
			{
				int size_x = window_adjust_x + (size_y_desired * window_ratio_x) / window_ratio_y;

				rect.left = (rect.left + rect.right) / 2 - size_x / 2;
				rect.right = rect.left + size_x;
			}

			break;
		case WMSZ_BOTTOMLEFT:
			{
				int size_x, size_y;

				if (size_x_desired * window_ratio_y > size_y_desired * window_ratio_x)
				{
					size_x = rect.right - rect.left;
					size_y = window_adjust_y + ((size_x - window_adjust_x) * window_ratio_y) / window_ratio_x;
				}
				else
				{
					size_y = rect.bottom - rect.top;
					size_x = window_adjust_x + ((size_y - window_adjust_y) * window_ratio_x) / window_ratio_y;
				}

				rect.left = rect.right - size_x;
				rect.bottom = rect.top + size_y;
			}
			break;
		case WMSZ_BOTTOMRIGHT:
			{
				int size_x, size_y;

				if (size_x_desired * window_ratio_y > size_y_desired * window_ratio_x)
				{
					size_x = rect.right - rect.left;
					size_y = window_adjust_y + ((size_x - window_adjust_x) * window_ratio_y) / window_ratio_x;

					rect.right = rect.left + size_x;
					rect.bottom = rect.top + size_y;
				}
				else
				{
					size_y = rect.bottom - rect.top;
					size_x = window_adjust_x + ((size_y - window_adjust_y) * window_ratio_x) / window_ratio_y;
				}

				rect.right = rect.left + size_x;
				rect.bottom = rect.top + size_y;
			}
			break;
		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			{
				int size_y = window_adjust_y + (size_x_desired * window_ratio_y) / window_ratio_x;
				rect.top = (rect.top + rect.bottom) / 2 - size_y / 2;
				rect.bottom = rect.top + size_y;
			}
			break;
		case WMSZ_TOPLEFT:
			{
				int size_x, size_y;

				if (size_x_desired * window_ratio_y > size_y_desired * window_ratio_x)
				{
					size_x = rect.right - rect.left;
					size_y = window_adjust_y + ((size_x - window_adjust_x) * window_ratio_y) / window_ratio_x;
				}
				else
				{
					size_y = rect.bottom - rect.top;
					size_x = window_adjust_x + ((size_y - window_adjust_y) * window_ratio_x) / window_ratio_y;
				}

				rect.left = rect.right - size_x;
				rect.top = rect.bottom - size_y;
			}
			break;
		case WMSZ_TOPRIGHT:
			{
				int size_x, size_y;

				if (size_x_desired * window_ratio_y > size_y_desired * window_ratio_x)
				{
					size_x = rect.right - rect.left;
					size_y = window_adjust_y + ((size_x - window_adjust_x) * window_ratio_y) / window_ratio_x;
				}
				else
				{
					size_y = rect.bottom - rect.top;
					size_x = window_adjust_x + ((size_y - window_adjust_y) * window_ratio_x) / window_ratio_y;
				}

				rect.right = rect.left + size_x;
				rect.top = rect.bottom - size_y;
			}
			break;
	}
}

bool CAppWindow::OnMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int xPos = (int)(short)LOWORD(lParam);   // horizontal position 
	int yPos = (int)(short)HIWORD(lParam);   // vertical position 

	if (!pGx->IsWantingFullscreen() && !pGx->IsWentFullscreen())
	{
		SaveMainWindowSize();
		return true;
	}

	else
	{
		return false;
	}
}

bool CAppWindow::OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int w, h;
	if (ErrorLogger::HideWindow)
	{
		return 0;
	}

    // Check to see if we are losing our window...
	w = (int)(DWORD)LOWORD(lParam);
	h = (int)(DWORD)HIWORD(lParam);
    if (SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam)
	{
		appCommand->SoundHalt();
		appStatus->m_bActive = false;
	}
	else
	{
		if (appStatus->m_bReady && !appStatus->m_bClosing)
		{
			appStatus->m_bActive = true;
			if (wParam == SIZE_RESTORED)
			{
				if (!pGx->IsWantingFullscreen() && !pGx->IsWentFullscreen())
				{
					SaveMainWindowSize();
				}
			}
		}

		ResizeGraphics(hWnd, w, h);
	}

	return 0;	 
}

void CAppWindow::ResizeGraphics(HWND hWnd, int width, int height)
{
	if (appStatus->m_bReady && !appStatus->m_bClosing)
	{
		RECT rcClient;
		if (GetClientRect(hWnd, &rcClient))
		{
			const int reserveHeight = 0;
			int y = rcClient.top;
			int x = rcClient.left;
			int clientWidth = rcClient.right - rcClient.left;
			int clientHeight = rcClient.bottom - rcClient.top - reserveHeight;
			if (clientWidth > 0 && clientHeight > 0)
			{
				UINT showWindowFlags;
				if (ErrorLogger::HideWindow)
				{
					showWindowFlags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW;
				}
				else
				{
					showWindowFlags = SWP_NOZORDER | SWP_NOMOVE;
				}

				//SetWindowPos(m_pWinEmuWin->GetHwnd(), HWND_NOTOPMOST, 0, 0, clientWidth, clientHeight, showWindowFlags);
				pGx->ResizeBuffers(clientWidth, clientHeight);
			}
		}
	}
}

bool CAppWindow::OnSizing(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (lParam != NULL)
	{
		if (appStatus->m_bWindowedLockAspectRatio && !pGx->IsWantingFullscreen() && !pGx->IsWentFullscreen())
		{
			if (!appStatus->m_bClosing)
			{
				AspectSizing((int)wParam, *((LPRECT)lParam));
				return true;
			}
		}
	}

	return false;
}

bool CAppWindow::OnGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MINMAXINFO* pMinMax;
	pMinMax = (MINMAXINFO*)lParam;
	if (!pGx->IsWantingFullscreen() && !pGx->IsWentFullscreen())
	{
		int w;
		int h;
		GetMinimumWindowedSize(appStatus->m_borderSize, appStatus->m_bShowFloppyLed, &w, &h);
		if (w > pMinMax->ptMinTrackSize.x)
		{
			pMinMax->ptMinTrackSize.x = w;
		}

		if (h > pMinMax->ptMinTrackSize.y)
		{
			pMinMax->ptMinTrackSize.y = h;
		}

		if (appStatus->m_bWindowedLockAspectRatio)
		{
			// Apply aspect ratio to the normal window dimensions.
			RECT rcWorkArea;
			rcWorkArea.left = 0;
			rcWorkArea.top = 0;
			rcWorkArea.right = pMinMax->ptMaxTrackSize.x;
			rcWorkArea.bottom = pMinMax->ptMaxTrackSize.y;
			RECT rcAspect;
			CopyRect(&rcAspect, &rcWorkArea);
			this->AspectMaxSizing(hWnd, WMSZ_BOTTOMRIGHT, rcAspect);
			if (rcAspect.right - rcAspect.left <= rcWorkArea.right - rcWorkArea.left)
			{
				pMinMax->ptMaxTrackSize.x = rcAspect.right - rcAspect.left;
			}

			if (rcAspect.bottom - rcAspect.top <= rcWorkArea.bottom - rcWorkArea.top)
			{
				pMinMax->ptMaxTrackSize.y = rcAspect.bottom - rcAspect.top;
			}

			// Apply aspect ratio to the maximized window dimensions.
			rcWorkArea.left = pMinMax->ptMaxPosition.x;
			rcWorkArea.top = pMinMax->ptMaxPosition.y;
			rcWorkArea.right = pMinMax->ptMaxPosition.x + pMinMax->ptMaxSize.x;
			rcWorkArea.bottom = pMinMax->ptMaxPosition.y + pMinMax->ptMaxSize.y;
			rcAspect;
			CopyRect(&rcAspect, &rcWorkArea);
			this->AspectMaxSizing(hWnd, WMSZ_BOTTOMRIGHT, rcAspect);
			pMinMax->ptMaxSize.x = rcAspect.right - rcAspect.left;
			pMinMax->ptMaxSize.y = rcAspect.bottom - rcAspect.top;
		}

		return true;
	}
	else
	{
		pMinMax->ptMaxTrackSize.x = GetSystemMetrics(SM_CXMAXTRACK);
		pMinMax->ptMaxTrackSize.y = GetSystemMetrics(SM_CYMAXTRACK);
		return true;
	}

	return false;
}

bool CAppWindow::OnWindowPositionChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (appStatus->m_bReady)
	{
		if (!pGx->IsFullscreen())
		{
			if (pGx->IsWentFullscreen())
			{
				this->SetWindowedMode(true);
				return true;
			}
		}
	}

	return false;
}

void CAppWindow::OnDisplayChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}

bool CAppWindow::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT& lresult)
{
	/*Dialogs*/
	shared_ptr<CDiagColour> pDiagColour;
	shared_ptr<CDiagKeyboard> pDiagKeyboard;
	shared_ptr<CDiagJoystick> pDiagJoystick;
	shared_ptr<CDiagEmulationSettingsTab> pDiagEmulationSettingsTab;
	shared_ptr<CDiagNewBlankDisk> pDiagNewBlankDisk;
	shared_ptr<CDiagAbout> pDiagAbout;
	HRESULT hr;
	CConfig* pTemporaryCfg;
	HRESULT hRet;
	INT_PTR r;
	int wmId, wmEvent;
	wmId = LOWORD(wParam); // Remember, these are...
	wmEvent = HIWORD(wParam); // ...different for Win32!

	//Parse the menu selections:
	switch (wmId)
	{
	case IDM_TAPE_INSERT:
		appCommand->SoundHalt();
		appCommand->InsertTapeDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_TAPE_PLAY:
		c64->TapePressPlay();
		lresult = 0;
		return true;
	case IDM_TAPE_STOP:
		c64->TapePressStop();
		lresult = 0;
		return true;
	case IDM_TAPE_REWIND:
		c64->TapePressRewind();
		lresult = 0;
		return true;
	case IDM_TAPE_LOADIMAGE:
		appCommand->SoundHalt();
		appCommand->LoadC64ImageDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_TAPE_LOADT64:
		appCommand->SoundHalt();
		appCommand->LoadT64Dialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_AUTOLOAD:
		appCommand->SoundHalt();
		appCommand->AutoLoadDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_HARDRESET:
		c64->HardReset(true);
		return 0;
	case IDM_FILE_SOFTRESET:
		c64->SoftReset(true);
		lresult = 0;
		return true;
	case IDM_CART_DETACHCART:
		appCommand->SoundHalt();
		c64->DetachCart();
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_CART_ATTACHCRT:
		appCommand->SoundHalt();
		appCommand->LoadCrtFileDialog(hWnd);
		appCommand->SoundResume();
		return 0;
	case IDM_CART_FREEZE:
		c64->CartFreeze(true);
		lresult = 0;
		return true;
	case IDM_CART_RESET:
		c64->CartReset(true);
		lresult = 0;
		return true;
	case IDM_FILE_MONITOR:
		if (appStatus->m_bWindowed)
		{
			appCommand->ShowDevelopment();
		}

		lresult = 0;
		return true;
	case IDM_FILE_PAUSE:
		appCommand->TogglePause();
		lresult = 0;
		return true;
	case IDM_SETTING_MUTESOUND:
		appCommand->ToggleSoundMute();
		lresult = 0;
		return true;
	case IDM_EXIT:
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		lresult = 0;
		return true;
	case IDM_HELP_ABOUT:
		appCommand->SoundHalt();
		try
		{
			pDiagAbout = shared_ptr<CDiagAbout>(new CDiagAbout());
			if (pDiagAbout != 0)
			{
				hr = pDiagAbout->Init(appCommand->GetVersionInfo());
				if (SUCCEEDED(hr))
				{
					pDiagAbout->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd);
				}
			}
		}
		catch (...)
		{
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_RESTOREDEFAULT:
		appCommand->SoundHalt();
		appCommand->RestoreDefaultSettings();
		if (SUCCEEDED(this->UpdateC64Window(true)))
		{
			this->Present();
		}

		G::DebugMessageBox(hWnd, TEXT("Default settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_LOADSETTINGS_RESTOREUSER:
		appCommand->SoundHalt();
		appCommand->RestoreUserSettings();
		if (SUCCEEDED(this->UpdateC64Window(true)))
		{
			this->Present();
		}

		G::DebugMessageBox(hWnd, TEXT("User settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_SAVE:
		appCommand->SoundHalt();
		appCommand->UpdateUserConfigFromSid();
		appCommand->SaveCurrentSetting();
		G::DebugMessageBox(hWnd, TEXT("Setting saved."), APPNAME, MB_OK | MB_ICONINFORMATION);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_EMULATION2:
		appCommand->SoundHalt();
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pDiagEmulationSettingsTab = shared_ptr<CDiagEmulationSettingsTab>(new CDiagEmulationSettingsTab());
			if (pDiagEmulationSettingsTab != 0)
			{
				hr = pDiagEmulationSettingsTab->Init(pGx, pTemporaryCfg);
				if (SUCCEEDED(hr))
				{
					pDiagEmulationSettingsTab->SetTabID(IDC_SETTINGTAB);
					hr = pDiagEmulationSettingsTab->SetPages(5, &m_tabPagesSetting[0]);
					if (SUCCEEDED(hr))
					{
						r = pDiagEmulationSettingsTab->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_SETTING2), hWnd);
						if (LOWORD(r) == IDOK)
						{
							appCommand->SetUserConfig(pDiagEmulationSettingsTab->NewCfg);
							appCommand->ApplyConfig(pDiagEmulationSettingsTab->NewCfg);
						}

						pDiagEmulationSettingsTab->FreePages();
					}
				}
				else
				{
					pDiagEmulationSettingsTab->DisplayError(hWnd, appCommand->GetAppName());
				}
			}
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_COLOUR:
		appCommand->SoundHalt();
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pDiagColour = shared_ptr<CDiagColour>(new CDiagColour());
			if (pDiagColour != 0)
			{
				hr = pDiagColour->Init(pTemporaryCfg);
				if (SUCCEEDED(hr))
				{
					r = pDiagColour->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_VICIICOLOURPALETTE), hWnd);
					if (LOWORD(r) == IDOK)
					{
						appCommand->SetUserConfig(pDiagColour->newCfg);
						appCommand->ApplyConfig(pDiagColour->newCfg);
					}
				}
				else
				{
					pDiagColour->DisplayError(hWnd, appCommand->GetAppName());
				}
			}
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_KEYBOARD:
		appCommand->SoundHalt();
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pDiagKeyboard = shared_ptr<CDiagKeyboard>(new CDiagKeyboard());
			if (pDiagKeyboard != 0)
			{
				hr = pDiagKeyboard->Init(dx, pTemporaryCfg);
				if (SUCCEEDED(hr))
				{
					pDiagKeyboard->SetTabID(IDC_KEYBOARDTAB);
					pDiagKeyboard->SetPages(4, &m_tabPagesKeyboard[0]);

					r = pDiagKeyboard->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_KEYBOARD), hWnd);
					if (LOWORD(r) == IDOK)
					{
						appCommand->SetUserConfig(pDiagKeyboard->newCfg);
						appCommand->ApplyConfig(pDiagKeyboard->newCfg);
					}

					pDiagKeyboard->FreePages();
				}
				else
				{
					pDiagKeyboard->DisplayError(hWnd, appCommand->GetAppName());
				}
			}
			dx->pKeyboard->Unacquire();
			dx->pKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_JOYSTICK:
		appCommand->SoundHalt();
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pDiagJoystick = shared_ptr<CDiagJoystick>(new CDiagJoystick(dx, pTemporaryCfg));
			if (pDiagJoystick != 0)
			{
				pDiagJoystick->Init();
				r = pDiagJoystick->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_JOYSTICK), hWnd);
				if (LOWORD(r) == IDOK)
				{
					appCommand->SetUserConfig(pDiagJoystick->newCfg);
					appCommand->ApplyConfig(pDiagJoystick->newCfg);
				}
			}
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_SWAPJOYSTICKS:
		appCommand->SoundHalt();
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pTemporaryCfg->m_bSwapJoysticks = !pTemporaryCfg->m_bSwapJoysticks;
			appCommand->SetUserConfig(*pTemporaryCfg);
			appCommand->ApplyConfig(*pTemporaryCfg);
			MessageBeep(MB_ICONASTERISK);
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTINGS_LOCKASPECTRATIO:
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();
			appCommand->GetUserConfig(*pTemporaryCfg);
			pTemporaryCfg->m_bWindowedLockAspectRatio = !pTemporaryCfg->m_bWindowedLockAspectRatio;
			appCommand->SetUserConfig(*pTemporaryCfg);
			appCommand->ApplyConfig(*pTemporaryCfg);
		}
		catch (...)
		{
		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		lresult = 0;
		return true;
	case IDM_SETTINGS_SIDSTEREO:
		pTemporaryCfg = nullptr;
		try
		{
			pTemporaryCfg = new CConfig();

			appCommand->GetUserConfig(*pTemporaryCfg);
			pTemporaryCfg->m_bSIDStereo = !pTemporaryCfg->m_bSIDStereo;
			appCommand->SetUserConfig(*pTemporaryCfg);
			appCommand->ApplyConfig(*pTemporaryCfg);
		}
		catch (...)
		{

		}

		if (pTemporaryCfg)
		{
			delete pTemporaryCfg;
			pTemporaryCfg = nullptr;
		}

		lresult = 0;
		return true;
	case IDM_SETTING_MAXSPEED:
		appCommand->SoundHalt();
		appCommand->ToggleMaxSpeed();
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_INSERT_EXISTINGDISK:
		appCommand->SoundHalt();
		appCommand->InsertDiskImageDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_D64:
		appCommand->SoundHalt();
		appCommand->SaveD64ImageDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_FDI:
		appCommand->SoundHalt();
		appCommand->SaveFDIImageDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_P64:
		appCommand->SoundHalt();
		appCommand->SaveP64ImageDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_SAVESTATE:
		appCommand->SoundHalt();
		appCommand->SaveC64StateDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_LOADSTATE:
		appCommand->SoundHalt();
		appCommand->LoadC64StateDialog(hWnd);
		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_INSERT_NEWBLANKDISK:
		appCommand->SoundHalt();
		try
		{
			pDiagNewBlankDisk = shared_ptr<CDiagNewBlankDisk>(new CDiagNewBlankDisk());
			if (pDiagNewBlankDisk != 0)
			{
				hr = pDiagNewBlankDisk->Init();
				if (SUCCEEDED(hr))
				{
					r = pDiagNewBlankDisk->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_NEWDISK), hWnd);
					if (LOWORD(r) == IDOK)
					{
						c64->InsertNewDiskImage(pDiagNewBlankDisk->diskname, pDiagNewBlankDisk->id1, pDiagNewBlankDisk->id2, pDiagNewBlankDisk->bAlignTracks, pDiagNewBlankDisk->numberOfTracks);
					}
				}
				else
					pDiagNewBlankDisk->DisplayError(hWnd, appCommand->GetAppName());
			}
		}
		catch (...)
		{
		}

		appCommand->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_REMOVEDISK:
		c64->RemoveDisk();
		lresult = 0;
		return true;
	case IDM_DISK_WRITEPROTECT_ON:
		c64->Set_DiskProtect(true);
		UpdateMenu();
		lresult = 0;
		return true;
	case IDM_DISK_WRITEPROTECT_OFF:
		c64->Set_DiskProtect(false);
		UpdateMenu();
		lresult = 0;
		return true;
	case IDM_DISK_RESETDRIVE:
		c64->DiskReset();
		lresult = 0;
		return true;
	case IDM_TOGGLEFULLSCREEN:
		appCommand->SoundHalt();
		if (appStatus->m_bActive && appStatus->m_bReady && !appStatus->m_bDebug)
		{
			hRet = ToggleFullScreen();
			if (FAILED(hRet))
			{
				DisplayError(hWnd, appCommand->GetAppName());
			}
		}

		lresult = 0;
		return true;
	}

	return false;
}

bool CAppWindow::OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT& lresult)
{
	if (ErrorLogger::HideWindow)
	{
		return false;
	}

	if (!appStatus->m_bActive)
	{
		return false;
	}

	//else if (appStatus->m_bWindowed)
	//{
	//	switch (wParam & 0xfff0)
	//	{
	//	case SC_SCREENSAVE:
	//	case SC_MONITORPOWER:
	//		lresult = 0;
	//		return true;
	//	}
	//}
	//else
	//{
	//	switch (wParam & 0xfff0)
	//	{
	//	case SC_MONITORPOWER:
	//	case SC_SCREENSAVE:
	//		lresult = 0;
	//		return true;
	//	}
	//}

	return false;
}

bool CAppWindow::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
	/*
	WM_CREATE message
	If an application processes this message, it should return zero to continue creation of the window. 
	If the application returns –1, the window is destroyed and the CreateWindowEx or CreateWindow function returns a NULL handle.
	*/
	lresult = 0;
	return true;
}

bool CAppWindow::OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
	HWND hWndDebuggerFrame;

	appCommand->SoundHalt();
	appStatus->m_bClosing = true;
	if (!this->m_pMDIDebugger.expired())
	{
		shared_ptr<CMDIDebuggerFrame> pwin = m_pMDIDebugger.lock();
		hWndDebuggerFrame = pwin->GetHwnd();
		if (IsWindow(hWndDebuggerFrame))
		{
			SendMessage(hWndDebuggerFrame, WM_CLOSE, 0, 0);
		}
	}

	if (pGx->IsFullscreen())
	{
		pGx->GoWindowed();
		//Fix to avoid strange crash if the default DestroyWindow is called straight away.
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		lresult = 0;
		return true;
	}
	else
	{
		appStatus->SaveWindowSetting(hWnd);
	}

	return false;
}

void CAppWindow::OnActivate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
bool bIsWindowActive;
bool bIsWindowMinimised;

	if (ErrorLogger::HideWindow)
	{
		return;
	}

	bIsWindowActive = !((BOOL)HIWORD(wParam)) && ((LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam) == WA_CLICKACTIVE));
	bIsWindowMinimised = !((BOOL)HIWORD(wParam));
	appStatus->m_bActive = bIsWindowMinimised;

	if (!appStatus->m_bWindowed && bIsWindowMinimised)
	{
		appCommand->SoundHalt();
	}

	if (appStatus->m_bReady)
	{
		c64->ResetKeyboard();
	}
}

bool CAppWindow::OnSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
	return false;
}

void CAppWindow::OnPaletteChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
}

bool CAppWindow::OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT& lresult)
{
	if (appStatus->m_bWindowed)
	{
		UpdateC64WindowWithObjects();
	}

	ValidateRect(hWnd, NULL);
	lresult = 0;
	return true;
}


LRESULT CAppWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HMENU hMenu  = 0;
LRESULT lr = 0;

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	switch (uMsg) 
	{
	case WM_CREATE:
		if (OnCreate(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
	case WM_ENTERMENULOOP:
		appCommand->SoundHalt();
		UpdateMenu();
		return 0;
	case WM_EXITMENULOOP:
		appCommand->SoundResume();
		return 0;
	case WM_ENTERSIZEMOVE:
		appCommand->SoundHalt();
		return 0;
	case WM_EXITSIZEMOVE:
		appCommand->SoundResume();
		return 0;
	case WM_ACTIVATE:
		OnActivate(hWnd, wParam, lParam);
		break;
	case WM_MOVE:
		if (OnMove(hWnd, wParam, lParam))
		{
			return 0;
		}

		break;
	case WM_SIZE:
		if (OnSize(hWnd, wParam, lParam))
		{
			return 0;
		}
		
		break;
	case WM_SIZING:
		if (OnSizing(hWnd, wParam, lParam))
		{
			return TRUE;
		}

		break;
	case WM_GETMINMAXINFO:
		if (OnGetMinMaxInfo(hWnd, wParam, lParam))
		{
			return 0;
		}
		
		break;
	case WM_WINDOWPOSCHANGED:
		if (OnWindowPositionChanged(hWnd, wParam, lParam))
		{
			return 0;
		}

		break;
	case WM_DISPLAYCHANGE:
		OnDisplayChange(hWnd, wParam, lParam);
		break;
	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			//const int dpi = HIWORD(wParam);
			//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}

		break;
	case WM_COMMAND:
		if (OnCommand(hWnd, wParam, lParam, lr))
		{
			return lr;
		}

		break;
	case WM_CLOSE:
		if (OnClose(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
	case WM_SYSCOMMAND:
		if (OnSysCommand(hWnd, wParam, lParam, lr))
		{
			return lr;
		}

		break;
	case WM_ERASEBKGND:
		return 1;
	case WM_QUERYNEWPALETTE:
		break;
	case WM_PALETTECHANGED:
		OnPaletteChanged(hWnd, wParam, lParam);
		break;
	case WM_NCPAINT:
		if (appStatus->m_bWindowed)
		{
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		else
		{
			return 0;
		}

		break;
	case WM_PAINT:
		if (OnPaint(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
    case WM_SETCURSOR:
		if (OnSetCursor(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
	case WM_DESTROY:
		appStatus->m_bReady = false;
		appCommand->FreeDirectX();
		PostQuitMessage(0);
		return 0;
	case WM_MONITOR_BREAK_CPU64:
		OnBreakCpu64(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPUDISK:
		OnBreakCpuDisk(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_VICRASTER:
		OnBreakVic(hWnd, uMsg, wParam, lParam);
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
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void CAppWindow::CloseWindow()
{	
	PostMessage(this->m_hWnd, WM_CLOSE, 0, 0);
}

void CAppWindow::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appCommand->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A C64 CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = appCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appCommand->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A disk CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = appCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appCommand->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A VIC raster compare breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = appCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::UpdateMenu()
{
HMENU hMenu;
	hMenu = GetMenu(m_hWnd);
	if (hMenu)
	{
		if (c64->Get_DiskProtect())
		{
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_ON, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_OFF, MF_BYCOMMAND | MF_UNCHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_ON, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_OFF, MF_BYCOMMAND | MF_CHECKED);
		}
	
		if (appStatus->m_bSwapJoysticks)
		{
			CheckMenuItem (hMenu, IDM_SETTING_SWAPJOYSTICKS, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTING_SWAPJOYSTICKS, MF_BYCOMMAND | MF_UNCHECKED);
		}

		if (appStatus->m_bWindowedLockAspectRatio)
		{
			CheckMenuItem (hMenu, IDM_SETTINGS_LOCKASPECTRATIO, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTINGS_LOCKASPECTRATIO, MF_BYCOMMAND | MF_UNCHECKED);
		}

		if (appStatus->m_bSIDStereo)
		{
			CheckMenuItem (hMenu, IDM_SETTINGS_SIDSTEREO, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTINGS_SIDSTEREO, MF_BYCOMMAND | MF_UNCHECKED);
		}

		if (appStatus->m_bMaxSpeed)
		{
			CheckMenuItem (hMenu, IDM_SETTING_MAXSPEED, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTING_MAXSPEED, MF_BYCOMMAND | MF_UNCHECKED);
		}

		if (appStatus->m_bPaused)
		{
			CheckMenuItem (hMenu, IDM_FILE_PAUSE, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_FILE_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);
		}

		if (appStatus->m_bSoundMute)
		{
			CheckMenuItem (hMenu, IDM_SETTING_MUTESOUND, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTING_MUTESOUND, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
}

void CAppWindow::SaveMainWindowSize()
{
WINDOWPLACEMENT wp;
	ZeroMemory(&wp, sizeof(wp));
	wp.length = sizeof(wp);
	if (GetWindowPlacement(m_hWnd, &wp))
	{
		CopyRect(&m_rcMainWindow, &wp.rcNormalPosition);
	}
}


HRESULT CAppWindow::ResetDirect3D()
{
	return SetWindowedMode(true);
}

void CAppWindow::RestoreWindowPosition()
{
	UINT flags = ErrorLogger::HideWindow ? SWP_HIDEWINDOW : SWP_SHOWWINDOW;
	SetWindowPos(m_hWnd, HWND_TOP, m_rcMainWindow.left, m_rcMainWindow.top, m_rcMainWindow.right - m_rcMainWindow.left, m_rcMainWindow.bottom - m_rcMainWindow.top, flags);
	G::EnsureWindowPosition(m_hWnd);
}

HRESULT CAppWindow::ToggleFullScreen()
{
	HRESULT hr = S_OK;
	hr = SetWindowedMode(!appStatus->m_bWindowed);

	if(appStatus->m_bWindowed)
	{
		SetThreadExecutionState(ES_CONTINUOUS);   
	}
	else
	{
		SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS); 
	}

	return hr;
}

void CAppWindow::SetWindowedStyle(bool bWindowed)
{
	HMENU hMt;
	LONG_PTR lp;

	if (bWindowed)
	{
		lp = GetWindowLongPtr(m_hWnd, GWL_STYLE);
		lp |= CAppWindow::StylesWindowed;
#pragma warning(disable:4244)
		SetWindowLongPtr(m_hWnd, GWL_STYLE, lp);
#pragma warning(default:4244)
		if (m_hMenuOld)
		{
			SetMenu(m_hWnd, m_hMenuOld);
			m_hMenuOld = NULL;
			PostMessage(m_hWnd, WM_KEYUP, VK_MENU, 0xc000001);
		}
	}
	else
	{
		hMt = GetMenu(m_hWnd);
		if (hMt)
		{
			SetMenu(m_hWnd, NULL);
			m_hMenuOld = hMt;
		}

		lp = GetWindowLongPtr(m_hWnd, GWL_STYLE);
		lp &= ~CAppWindow::StylesWindowed;
		lp |= CAppWindow::StylesNonWindowed;
#pragma warning(disable:4244)
		SetWindowLongPtr(m_hWnd, GWL_STYLE, lp);
#pragma warning(default:4244)
	}
}

HRESULT CAppWindow::SetWindowedMode(bool wantWindowed)
{
	this->appStatus->m_bReady = false;
	HWND hwndMainWindow = this->GetHwnd();
	bool usePointFilter = appStatus->m_blitFilter == HCFG::EMUWINDOWFILTER::EMUWINFILTER_POINT;	
	SetWindowedStyle(wantWindowed);

	if (!ErrorLogger::HideWindow)
	{
		if (wantWindowed)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
		else
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_None);
			SetCursor(NULL);
		}
	}

	HRESULT hr = pGx->SetMode(appStatus->m_fullscreenAdapterIsDefault, appStatus->m_fullscreenAdapterNumber, appStatus->m_fullscreenOutputNumber, hwndMainWindow, appStatus->m_fullscreenWidth, appStatus->m_fullscreenHeight, appStatus->m_fullscreenRefreshNumerator, appStatus->m_fullscreenRefreshDenominator, appStatus->m_fullscreenDxGiModeScanlineOrdering, appStatus->m_fullscreenDxGiModeScaling, wantWindowed, appStatus->m_borderSize, appStatus->m_bShowFloppyLed, appStatus->m_fullscreenStretch, usePointFilter, wantWindowed ? appStatus->m_syncModeWindowed : appStatus->m_syncModeFullscreen);
	if (SUCCEEDED(hr))
	{
		this->appStatus->m_bReady = true;
		if (wantWindowed)
		{
			RestoreWindowPosition();
		}
	}
	else
	{
		if (!ErrorLogger::HideWindow)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_Arrow);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}

	if (!pGx->IsFullscreen())
	{
		SetWindowedStyle(true);
	}

	return hr;
}

void CAppWindow::UpdateWindowTitle(const TCHAR *szTitle, DWORD emulationSpeed)
{
TCHAR szBuff[300];
	if (appStatus == NULL)
	{
		return;
	}

	if (appStatus->m_bShowSpeed && (long)emulationSpeed>=0)
	{
		_sntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, TEXT("%s at %d%%"), szTitle, emulationSpeed);
	}
	else
	{
		_sntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, TEXT("%s"), szTitle);
	}
	if (appStatus->m_bSoundMute)
	{
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Mute"));
	}

	if (appStatus->m_bMaxSpeed)
	{
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Max Speed"));
	}

	if (appStatus->m_bDebug)
	{
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Debug"));
	}

	if (appStatus->m_bPaused)
	{
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Paused"));
	}

	if (appStatus->m_bWindowed)
	{
		SetWindowText(m_hWnd, szBuff);
	}
}

struct tabpageitem CAppWindow::m_tabPagesKeyboard[4]=
{
	{
		IDD_KEYPAGE1,
		TEXT("Symbol")
	},
	{
		IDD_KEYPAGE2,
		TEXT("A - Z")
	},
	{
		IDD_KEYPAGE3,
		TEXT("0 - 9 F1 - F7")
	},
	{
		IDD_KEYPAGE4,
		TEXT("Joystick")
	}
};

struct tabpageitem CAppWindow::m_tabPagesSetting[5]=
{
	{
		IDD_SETTINGTABGENERAL,
		TEXT("General")
	},
	{
		IDD_SETTINGTABVIDEO,
		TEXT("Video")
	},
	{
		IDD_SETTINGTABAUDIO,
		TEXT("Audio")
	},
	{
		IDD_SETTINGTABDISK,
		TEXT("Floppy Disk")
	},
	{
		IDD_SETTINGTABCHIP,
		TEXT("Chip")
	}
};

HWND CAppWindow::ShowDevelopment()
{
	CDPI dpi;
	HWND hWnd = NULL;
	shared_ptr<CMDIDebuggerFrame> pwin;
	HRESULT hr;
	constexpr int X_GAP = 362;
	bool bCreated = false;
	bool ok = false;

	appCommand->SoundHalt();
	c64->SynchroniseDevicesWithVIC();
	appStatus->m_bRunning = false;
	appStatus->m_bDebug = true;
	RECT rcDesk;
	G::GetWorkArea(rcDesk);
	try
	{
		if (m_pMDIDebugger.expired() || m_pMDIDebugger.lock()->GetHwnd() == 0)
		{
			pwin = shared_ptr<CMDIDebuggerFrame>(new CMDIDebuggerFrame(this->c64, this->appCommand, this->appStatus, this->appStatus));
			if (pwin != NULL)
			{
				int x,y,w,h;
				x = dpi.ScaleX(X_GAP);
				y = 0;
				w = rcDesk.right - 2 * x;
				h = rcDesk.bottom - y;
				if (w<0 || h<0)
				{
					w = CW_USEDEFAULT;
					h = CW_USEDEFAULT;
				}

				POINT pos = {x,y};
				SIZE size= {w,h};
				hr = CConfig::LoadMDIWindowSetting(pos, size);
				if (SUCCEEDED(hr))
				{
					x = pos.x;
					y = pos.y;
					w = size.cx;
					h = size.cy;
				}

				hWnd = pwin->Create(m_hInst, 0, TEXT("C64 Monitor"), x, y, w, h, NULL);
				if (hWnd != 0)
				{
					pwin->keepAlive = pwin;
					bCreated = true;
					m_pMDIDebugger = pwin;
					G::EnsureWindowPosition(hWnd);
					ok = true;
				}
			}
		}
		else
		{
			pwin = m_pMDIDebugger.lock();
			if (pwin != nullptr)
			{
				hWnd = pwin->GetHwnd();
				ok = true;
			}
		}
	}
	catch (std::exception&)
	{
		ok = false;
	}

	if (ok)
	{
		::ShowWindow(hWnd, SW_SHOW);
		::SetForegroundWindow(hWnd);
		if (bCreated)
		{
			pwin->OpenNewCli();
			pwin->ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
			pwin->ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
		}

		this->UpdateWindowTitle(appCommand->GetAppTitle(), -1);
		this->UpdateC64WindowWithObjects();
	}
	else
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the debugger window."), appCommand->GetAppName(), MB_ICONWARNING);
		appCommand->Resume();
	}

	return hWnd;
}

// From old CEmuWindow child window
void CAppWindow::SetColours()
{
	if (c64 != nullptr && c64->IsInitOK())
	{
		c64->SetupColorTables();
	}
}

void CAppWindow::SetNotify(INotify* pINotify)
{
	m_pINotify = pINotify;
}

void CAppWindow::GetVicRasterPositionFromClientPosition(int x, int y, int& cycle, int& line)
{
	C64Display& c64display = pGx->c64display;
	int s = c64display.displayStart + DISPLAY_START;
	int st = c64display.displayFirstVicRaster;
	int sb = c64display.displayLastVicRaster;
	int wc = c64display.displayWidth;
	int hc = c64display.displayHeight;
	int wp = c64display.drcScreen.right - c64display.drcScreen.left;
	int hp = c64display.drcScreen.bottom - c64display.drcScreen.top;

	cycle = (s + ((x * wc) / wp)) & ~7;
	cycle = cycle / 8;
	cycle += 1;
	if (cycle < 1)
	{
		cycle = 1;
	}

	if (cycle > PAL_CLOCKS_PER_LINE)
	{
		cycle = PAL_CLOCKS_PER_LINE;
	}

	line = st + ((y * hc) / hp);
	line -= 1;

	if (line < 0)
	{
		line = 0;
	}

	if (line > PAL_MAX_LINE)
	{
		line = PAL_MAX_LINE;
	}
}

void CAppWindow::GetVicCycleRectFromClientPosition(int x, int y, RECT& rcVicCycle)
{
	int cycle;
	int line;
	GetVicRasterPositionFromClientPosition(x, y, cycle, line);
	SetRect(&rcVicCycle, (cycle - 1) * 8, line, (cycle) * 8, line + 1);
}

void CAppWindow::GetDisplayRectFromVicRect(const RECT rcVicCycle, RECT& rcDisplay)
{
	C64Display& c64display = pGx->c64display;
	int s = c64display.displayStart + DISPLAY_START;
	int st = c64display.displayFirstVicRaster;
	int sb = c64display.displayLastVicRaster;
	int wc = c64display.displayWidth;
	int hc = c64display.displayHeight;
	int wp = c64display.drcScreen.right - c64display.drcScreen.left;
	int hp = c64display.drcScreen.bottom - c64display.drcScreen.top;

	double left = ::ceil((double)(rcVicCycle.left - s) * (double)wp / (double)wc);
	double right = ::floor((double)(rcVicCycle.right - s) * (double)wp / (double)wc);
	rcDisplay.left = (LONG)left;
	rcDisplay.right = (LONG)right;
	if (right <= left)
	{
		right = left + 1;
	}

	double top = ::ceil((double)(rcVicCycle.top - st) * (double)hp / (double)hc);
	double bottom = ::floor((double)(rcVicCycle.bottom - st) * (double)hp / (double)hc);
	if (bottom <= top)
	{
		bottom = top + 1;
	}

	rcDisplay.top = (LONG)top;
	rcDisplay.bottom = (LONG)bottom;
	OffsetRect(&rcDisplay, c64display.drcScreen.left, c64display.drcScreen.top);
}


void CAppWindow::DisplayVicCursor(bool bEnabled)
{
	m_bDrawCycleCursor = bEnabled;
}

void CAppWindow::DisplayVicRasterBreakpoints(bool bEnabled)
{
	m_bDrawVicRasterBreakpoints = bEnabled;
}

void CAppWindow::SetVicCursorPos(int iCycle, int iLine)
{
	m_iVicCycleCursor = iCycle;
	m_iVicLineCursor = iLine;
}

void CAppWindow::GetVicCursorPos(int* piCycle, int* piLine)
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

void CAppWindow::OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ErrorLogger::HideWindow)
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
	}
}

void CAppWindow::OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ErrorLogger::HideWindow)
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
		}

		m_bDragMode = false;
	}
}

bool CAppWindow::OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (ErrorLogger::HideWindow)
	{
		return true;
	}

	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	appCommand->SetLastMousePosition(&x, &y);
	if (appStatus->m_bWindowed && m_bDrawCycleCursor)
	{
		if (m_bDragMode)
		{
			SetCursorAtClientPosition(x, y);
			UpdateC64WindowWithObjects();
			if (m_pINotify)
			{
				m_pINotify->VicCursorMove(m_iVicCycleCursor, m_iVicLineCursor);
			}
		}
	}

	return true;
}

void CAppWindow::SetCursorAtClientPosition(int x, int y)
{
	int cycle, line;
	GetVicRasterPositionFromClientPosition(x, y, cycle, line);
	this->SetVicCursorPos(cycle, line);
}

void CAppWindow::UpdateC64WindowWithObjects()
{
	if (ErrorLogger::HideWindow)
	{
		return;
	}

	if (appStatus->m_bWindowed && appStatus->m_bDebug && !appStatus->m_bRunning && m_bDrawVicRasterBreakpoints)
	{
		pGx->c64display.EnableVicCursorSprites(true);
		PrepareVicCursorsForDrawing();
	}

	HRESULT hr = this->UpdateC64Window(true);
	if (SUCCEEDED(hr))
	{
		this->Present();
	}

	pGx->c64display.EnableVicCursorSprites(false);
}

void CAppWindow::DrawCursorAtVicPosition(HDC hdc, int cycle, int line)
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
	{
		hInner = (HBRUSH)GetStockObject(WHITE_BRUSH);
	}

	if (!hOuter)
	{
		hOuter = (HBRUSH)GetStockObject(DKGRAY_BRUSH);
	}

	FillRect(hdc, &rcDisplay2, (HBRUSH)hOuter);
	FillRect(hdc, &rcDisplay, (HBRUSH)hInner);
}

void CAppWindow::PrepareVicCursorsForDrawing()
{
	pGx->c64display.ClearVicCursorSprites();
	if (m_bDrawCycleCursor)
	{
		// Prepare the user unconfirmed VIC breakpoint.
		pGx->c64display.InsertCursorSprite(VicCursorPosition(m_iVicCycleCursor, m_iVicLineCursor));
	}

	IEnumBreakpointItem* p = nullptr;
	try
	{
		// // Prepare all user confirmed VIC breakpoints.
		p = c64->GetMon()->BM_CreateEnumBreakpointItem();
		BreakpointItem v;
		while (p->GetNext(v))
		{
			if (v.bptype == DBGSYM::BreakpointType::VicRasterCompare)
			{
				pGx->c64display.InsertCursorSprite(VicCursorPosition(v.vic_cycle, v.vic_line));
			}
		}

		if (p)
		{
			delete p;
			p = nullptr;
		}
	}
	catch(std::bad_alloc)
	{
		if (p)
		{
			delete p;
			p = nullptr;
		}

		throw;
	}
}

HRESULT CAppWindow::RenderWindow()
{
	HRESULT hr;
	if (!pGx || !pGx->isInitOK)
	{
		return E_FAIL;
	}

	pGx->ClearFrame();
	hr = pGx->RenderFrame();
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
HRESULT CAppWindow::UpdateC64Window(bool refreshVicData)
{
	HRESULT hr = S_OK;

	if (ErrorLogger::HideWindow)
	{
		return S_FALSE;
	}

	if (refreshVicData)
	{
		if (c64 != nullptr && c64->IsInitOK())
		{
			hr = c64->UpdateBackBuffer();
		}
	}

	if (SUCCEEDED(hr))
	{
		//CAppWindow::RenderWindow Stretch-blits from the dx small surface to the dx backbuffer.
		hr = RenderWindow();
	}

	return hr;
}

HRESULT CAppWindow::Present()
{
	if (pGx == nullptr)
	{
		return E_FAIL;
	}

	return pGx->PresentFrame();
}

void CAppWindow::GetRequiredClientWindowSize(HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, int* w, int* h)
{
	C64WindowDimensions dims;
	dims.SetBorder(borderSize);
	*w = dims.Width;
	*h = dims.Height + pGx->c64display.GetToolBarHeight(bShowFloppyLed);
}
