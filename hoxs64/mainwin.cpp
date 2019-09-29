#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <winuser.h>
#include <commctrl.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
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

#include "emuwin.h"
#include "mainwin.h"
#include "resource.h"

const LPTSTR CAppWindow::lpszClassName = HOXS_MAIN_WND_CLASS;
const LPTSTR CAppWindow::lpszMenuName = APPMENUNAME;



CAppWindow::CAppWindow(CDX9 *dx, IAppCommand *pAppCommand, CAppStatus *appStatus, IC64 *c64)
{
	SetRect(&m_rcMainWindow, 0, 0, 0, 0);
	m_hMenuOld=NULL;
	m_hOldCursor = NULL;
	m_hWndStatusBar = 0;
	m_iStatusBarHeight = 0;

	this->dx = dx;
	this->appStatus = appStatus;
	this->c64 = c64;
	this->m_pAppCommand = pAppCommand;

	appStatus->m_bWindowed = TRUE;

	m_hCursorBusy = LoadCursor(0L, IDC_WAIT);
	if (!m_hCursorBusy)
		throw std::runtime_error("LoadCursor failed.");
	m_pWinEmuWin = shared_ptr<CEmuWindow>(new CEmuWindow(dx, appStatus, appStatus, c64));
	if (m_pWinEmuWin == 0)
		throw std::bad_alloc();
}

CAppWindow::~CAppWindow()
{
	int i =0;
}

HRESULT CAppWindow::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

    // Fill in window class structure with parameters that describe
    // the main window.
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize			= sizeof(WNDCLASSEX);
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
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

void CAppWindow::SetColours()
{
	m_pWinEmuWin->SetColours();
}

void CAppWindow::GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bDoubleSizedWindow, int *w, int *h)
{
C64WindowDimensions dims;

	dims.SetBorder(borderSize);

	if (bDoubleSizedWindow)
	{
		*w=dims.Width*2 + GetSystemMetrics(SM_CXSIZEFRAME)*2;
		*h=dims.Height*2 + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) +  CDX9::GetToolBarHeight(bShowFloppyLed) + m_iStatusBarHeight;
	}
	else
	{
		*w=dims.Width + GetSystemMetrics(SM_CXSIZEFRAME)*2;
		*h=dims.Height + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) +  CDX9::GetToolBarHeight(bShowFloppyLed) + m_iStatusBarHeight;
	}
}

void CAppWindow::GetMinimumWindowedSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, int *w, int *h)
{
	C64WindowDimensions dims;
	dims.SetBorder(borderSize);
	int width = dims.Width / 2 + GetSystemMetrics(SM_CXSIZEFRAME)*2;
	int height = dims.Height / 2 + GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) + CDX9::GetToolBarHeight(bShowFloppyLed) + m_iStatusBarHeight;

	if (w)
	{
		*w = width;
	}

	if (h)
	{
		*h = height;
	}
}

bool CAppWindow::CalcEmuWindowSize(RECT rcMainWindow, int *w, int *h)
{
	int padw = GetSystemMetrics(SM_CXSIZEFRAME)*2;
	int padh = GetSystemMetrics(SM_CYSIZEFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) + m_iStatusBarHeight;
	if (w)
	{
		*w = max(0, rcMainWindow.right - rcMainWindow.left - padw);
	}
	if (h)
	{
		*h = max(0, rcMainWindow.bottom - rcMainWindow.top - padh);
	}
	return true;
}

HWND CAppWindow::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	this->m_hInst = hInstance;
	HWND hWnd = CVirWindow::CreateVirWindow(0L, lpszClassName, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX  | WS_MINIMIZEBOX | WS_SIZEBOX, x, y, w, h, hWndParent, hMenu, hInstance);
	return hWnd;
}

void CAppWindow::AspectSizing(HWND hWnd, int edge, RECT &rect)
{
int w,h;
int x, y;
int minw, minh;
RECT rcMain;

	minw = 0x80;
	minh = 0x80;
	HCFG::EMUBORDERSIZE borderSize = this->appStatus->m_borderSize;
	C64WindowDimensions dims;
	dims.SetBorder(borderSize);
	int window_ratio_x = dims.Width;
	int window_ratio_y = dims.Height;	
	if (GetWindowRect(hWnd, &rcMain))
	{
		CalcEmuWindowSize(rcMain, &w, &h);

		h = h - CDX9::GetToolBarHeight(this->appStatus->m_bShowFloppyLed);
		y = rcMain.top;
		x = rcMain.left;
		int window_adjust_x = rcMain.right - rcMain.left - w;
		int window_adjust_y = rcMain.bottom - rcMain.top - h;
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
}

bool CAppWindow::OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
int w, h;
int x, y;
int mainWidth;
int mainHeight;
int clientWidth;
int clientHeight;
int emuWinWidth;
int emuWinHeight;
RECT rcClient;

	if (G::IsHideWindow)
	{
		return 0;
	}

    // Check to see if we are losing our window...
	w = (int)(DWORD)LOWORD(lParam);
	h = (int)(DWORD)HIWORD(lParam);
    if (SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam)
	{
		if (appStatus != NULL)
		{
			appStatus->SoundHalt();
			appStatus->m_bActive = false;
		}
	}
    else
	{
		if (appStatus != NULL && appStatus->m_bWindowed)
		{
			if (appStatus->m_bReady && !appStatus->m_bClosing)
			{
				//appStatus->m_bReady will be false whilst we are transiting between windowed mode and fullscreen mode.
				appStatus->m_bActive = true;
				if (wParam == SIZE_RESTORED)
				{
					SaveMainWindowSize();
				}

				if (GetClientRect(hWnd, &rcClient))
				{
					y = rcClient.top;
					x = rcClient.left;
					clientWidth = max(0, rcClient.right - rcClient.left) - x;
					clientHeight = max(0, rcClient.bottom - rcClient.top) - y - m_iStatusBarHeight;
					if (clientWidth > 0 && clientHeight > 0)
					{
						bool isCustomSizeBlit = true;
						UINT showWindowFlags;
						if (G::IsHideWindow)
						{
							showWindowFlags = SWP_NOZORDER | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOREDRAW;
						}
						else
						{
							showWindowFlags = SWP_NOZORDER | SWP_NOMOVE;
						}

						if (SetWindowPos(m_pWinEmuWin->GetHwnd(), HWND_NOTOPMOST, 0, 0, clientWidth, clientHeight, showWindowFlags))
						{
							CConfig tCfg;
							appStatus->GetUserConfig(tCfg);
							if (!tCfg.m_bUseBlitStretch)
							{
								this->GetRequiredMainWindowSize(tCfg.m_borderSize, tCfg.m_bShowFloppyLed, tCfg.m_bDoubleSizedWindow, &mainWidth, &mainHeight);
								RECT rcMain = {0, 0, mainWidth, mainHeight};
								this->CalcEmuWindowSize(rcMain, &emuWinWidth, &emuWinHeight);
								if (emuWinWidth == w && emuWinHeight == h)
								{
									isCustomSizeBlit = false;
								}
							}

							tCfg.m_bWindowedCustomSize = isCustomSizeBlit;
							appStatus->SetUserConfig(tCfg);

							//Hack to propagate m_bWindowedCustomSize. TODO Fix function ApplyConfig() to handle this efficiently.								
							appStatus->m_bWindowedCustomSize = isCustomSizeBlit;
							if (dx != NULL)
							{
								dx->m_bWindowedCustomSize = isCustomSizeBlit;
								dx->Reset();
							}
						}
					}
				}
			}
			//if (m_hWndStatusBar != NULL)				
			//	SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
		}
	}

	return 0;	 
}

bool CAppWindow::OnSizing(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (lParam != NULL)
	{
		if (appStatus != NULL && appStatus->m_bWindowed && appStatus->m_bWindowedLockAspectRatio)
		{
			if (appStatus->m_bReady && !appStatus->m_bClosing)
			{
				AspectSizing(hWnd, (int)wParam, *((LPRECT)lParam));
				return true;
			}
		}
	}

	return false;
}

bool CAppWindow::OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
/*Dialogs*/
shared_ptr<CDiagColour> pDiagColour;
shared_ptr<CDiagKeyboard> pDiagKeyboard;
shared_ptr<CDiagJoystick> pDiagJoystick;
shared_ptr<CDiagEmulationSettingsTab> pDiagEmulationSettingsTab;
shared_ptr<CDiagNewBlankDisk> pDiagNewBlankDisk;
shared_ptr<CDiagAbout> pDiagAbout;
HRESULT hr;
CConfig tCfg;
HRESULT hRet;
INT_PTR r;
int wmId, wmEvent;
wmId    = LOWORD(wParam); // Remember, these are...
wmEvent = HIWORD(wParam); // ...different for Win32!
	//Parse the menu selections:
	switch (wmId) 
	{
	case IDM_TAPE_INSERT:
		appStatus->SoundHalt();
		appStatus->InsertTape(hWnd);
		appStatus->SoundResume();
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
		appStatus->SoundHalt();
		appStatus->LoadC64Image(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_TAPE_LOADT64:
		appStatus->SoundHalt();
		appStatus->LoadT64(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_AUTOLOAD:
		appStatus->SoundHalt();
		appStatus->AutoLoad(hWnd);
		appStatus->SoundResume();
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
		appStatus->SoundHalt();
		c64->DetachCart();
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_CART_ATTACHCRT:
		appStatus->SoundHalt();
		appStatus->LoadCrtFile(hWnd);
		appStatus->SoundResume();
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
			m_pAppCommand->ShowDevelopment();
		}

		lresult = 0;
		return true;
	case IDM_FILE_PAUSE:
		appStatus->TogglePause();
		lresult = 0;
		return true;
	case IDM_SETTING_MUTESOUND:
		appStatus->ToggleSoundMute();			
		lresult = 0;
		return true;
	case IDM_EXIT:
		PostMessage(hWnd, WM_CLOSE, 0, 0);
		lresult = 0;
		return true;
	case IDM_HELP_ABOUT:
		appStatus->SoundHalt();
		try
		{
			pDiagAbout = shared_ptr<CDiagAbout>(new CDiagAbout());
			if (pDiagAbout!=0)
			{
				hr = pDiagAbout->Init(appStatus->GetVersionInfo());
				if (SUCCEEDED(hr))
				{
					pDiagAbout->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd);
				}
			}
		}
		catch(...)
		{
		}

		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_RESTOREDEFAULT:
		appStatus->SoundHalt();
		appStatus->RestoreDefaultSettings();
		if (SUCCEEDED(this->m_pWinEmuWin->UpdateC64Window()))
		{
			this->m_pWinEmuWin->Present(0);
		}

		G::DebugMessageBox(hWnd, TEXT("Default settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION); 
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_LOADSETTINGS_RESTOREUSER:
		appStatus->SoundHalt();
		appStatus->RestoreUserSettings();
		if (SUCCEEDED(this->m_pWinEmuWin->UpdateC64Window()))
		{
			this->m_pWinEmuWin->Present(0);
		}
		G::DebugMessageBox(hWnd, TEXT("User settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION); 
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_SAVE:
		appStatus->SoundHalt();
		appStatus->UpdateUserConfigFromSid();
		appStatus->SaveCurrentSetting();
		G::DebugMessageBox(hWnd, TEXT("Setting saved."), APPNAME, MB_OK | MB_ICONINFORMATION); 
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_EMULATION2:
		appStatus->SoundHalt();
		try
		{
			appStatus->GetUserConfig(tCfg);
			pDiagEmulationSettingsTab = shared_ptr<CDiagEmulationSettingsTab>(new CDiagEmulationSettingsTab());
			if (pDiagEmulationSettingsTab!=0)
			{
				hr = pDiagEmulationSettingsTab->Init(dx, &tCfg);
				if (SUCCEEDED(hr))
				{
					pDiagEmulationSettingsTab->SetTabID(IDC_SETTINGTAB);
					hr = pDiagEmulationSettingsTab->SetPages(5, &m_tabPagesSetting[0]);
					if (SUCCEEDED(hr))
					{
						r = pDiagEmulationSettingsTab->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_SETTING2), hWnd);
						if (LOWORD(r) == IDOK)
						{
							tCfg = pDiagEmulationSettingsTab->NewCfg;
							appStatus->SetUserConfig(tCfg);
							appStatus->ApplyConfig(tCfg);
						}
						pDiagEmulationSettingsTab->FreePages();
					}
				}
				else
					pDiagEmulationSettingsTab->DisplayError(hWnd, appStatus->GetAppName());
			}
		}
		catch(...)
		{
		}
		appStatus->SoundResume();

		lresult = 0;
		return true;
	case IDM_SETTING_COLOUR:
		appStatus->SoundHalt();
		try
		{
			appStatus->GetUserConfig(tCfg);
			pDiagColour = shared_ptr<CDiagColour>(new CDiagColour());
			if (pDiagColour!=0)
			{
				hr = pDiagColour->Init(&tCfg);
				if (SUCCEEDED(hr))
				{			 
					r = pDiagColour->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_VICIICOLOURPALETTE),hWnd);
					if (LOWORD(r) == IDOK)
					{
						tCfg = pDiagColour->newCfg;
						appStatus->SetUserConfig(tCfg);
						appStatus->ApplyConfig(tCfg);
					}
				}
				else
				{
					pDiagColour->DisplayError(hWnd, appStatus->GetAppName());
				}
			}
		}
		catch(...)
		{
		}

		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_KEYBOARD:
		appStatus->SoundHalt();
		try
		{
			appStatus->GetUserConfig(tCfg);
			pDiagKeyboard = shared_ptr<CDiagKeyboard>(new CDiagKeyboard());
			if (pDiagKeyboard!=0)
			{
				hr = pDiagKeyboard->Init(dx, &tCfg);
				if (SUCCEEDED(hr))
				{
					pDiagKeyboard->SetTabID(IDC_KEYBOARDTAB);
					pDiagKeyboard->SetPages(4, &m_tabPagesKeyboard[0]);
			 
					r = pDiagKeyboard->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_KEYBOARD),hWnd);
					if (LOWORD(r) == IDOK)
					{
						tCfg = pDiagKeyboard->newCfg;
						appStatus->SetUserConfig(tCfg);
						appStatus->ApplyConfig(tCfg);
					}

					pDiagKeyboard->FreePages();
				}
				else
				{
					pDiagKeyboard->DisplayError(hWnd, appStatus->GetAppName());
				}
			}
			dx->pKeyboard->Unacquire();
			dx->pKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY); 
		}
		catch(...)
		{
		}

		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_JOYSTICK:
		appStatus->SoundHalt();
		try
		{
			appStatus->GetUserConfig(tCfg);
			pDiagJoystick = shared_ptr<CDiagJoystick>(new CDiagJoystick(dx, &tCfg));
			if (pDiagJoystick != 0)
			{
				pDiagJoystick->Init();
				r = pDiagJoystick->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_JOYSTICK), hWnd);
				if (LOWORD(r) == IDOK)
				{
					tCfg = pDiagJoystick->newCfg;
					appStatus->SetUserConfig(tCfg);
					appStatus->ApplyConfig(tCfg);
				}
			}
		}
		catch(...)
		{
		}

		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_DOUBLESIZEDWINDOW:
		appStatus->SoundHalt();
		appStatus->GetUserConfig(tCfg);
		tCfg.m_bDoubleSizedWindow = !tCfg.m_bDoubleSizedWindow;
		tCfg.m_bWindowedCustomSize = false;
		appStatus->SetUserConfig(tCfg);
		appStatus->ApplyConfig(tCfg);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTING_SWAPJOYSTICKS:
		appStatus->SoundHalt();
		appStatus->GetUserConfig(tCfg);
		tCfg.m_bSwapJoysticks = !tCfg.m_bSwapJoysticks;
		appStatus->SetUserConfig(tCfg);
		appStatus->ApplyConfig(tCfg);
		MessageBeep(MB_ICONASTERISK);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_SETTINGS_LOCKASPECTRATIO:
		appStatus->GetUserConfig(tCfg);
		tCfg.m_bWindowedLockAspectRatio = !tCfg.m_bWindowedLockAspectRatio;
		appStatus->SetUserConfig(tCfg);
		appStatus->ApplyConfig(tCfg);
		lresult = 0;
		return true;
	case IDM_SETTINGS_SIDSTEREO:
		appStatus->GetUserConfig(tCfg);
		tCfg.m_bSIDStereo = !tCfg.m_bSIDStereo;
		appStatus->SetUserConfig(tCfg);
		appStatus->ApplyConfig(tCfg);
		lresult = 0;
		return true;
	case IDM_SETTING_MAXSPEED:
		appStatus->SoundHalt();
		appStatus->ToggleMaxSpeed();
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_INSERT_EXISTINGDISK:
		appStatus->SoundHalt();
		appStatus->InsertDiskImage(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_D64:
		appStatus->SoundHalt();
		appStatus->SaveD64Image(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_FDI:
		appStatus->SoundHalt();
		appStatus->SaveFDIImage(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_SAVEDISK_P64:
		appStatus->SoundHalt();
		appStatus->SaveP64Image(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_SAVESTATE:
		appStatus->SoundHalt();
		appStatus->SaveC64State(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_FILE_LOADSTATE:
		appStatus->SoundHalt();
		appStatus->LoadC64State(hWnd);
		appStatus->SoundResume();
		lresult = 0;
		return true;
	case IDM_DISK_INSERT_NEWBLANKDISK:
		appStatus->SoundHalt();
		try
		{
			pDiagNewBlankDisk = shared_ptr<CDiagNewBlankDisk>(new CDiagNewBlankDisk());
			if (pDiagNewBlankDisk!=0)
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
					pDiagNewBlankDisk->DisplayError(hWnd, appStatus->GetAppName());
			}
		}
		catch(...)
		{
		}

		appStatus->SoundResume();
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
		appStatus->SoundHalt();
        if (appStatus->m_bActive && appStatus->m_bReady && !appStatus->m_bDebug)
        {
			//appStatus->m_bPaused = false;
			appStatus->m_bReady = false;
			hRet = ToggleFullScreen();
			if (FAILED(hRet))
			{
				DisplayError(hWnd, appStatus->GetAppName());
			}
        }

		lresult = 0;
		return true;
	}

	return false;
}

bool CAppWindow::OnSysCommand(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
	if (G::IsHideWindow)
	{
		return false;
	}

	if (!appStatus->m_bActive)
	{
		return false;
	}
	else if (appStatus->m_bWindowed)
	{
		switch (wParam & 0xfff0)
		{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				lresult =  0;
				return true;
			case SC_MOVE:
			case SC_SIZE:
			case SC_MINIMIZE:
			case SC_KEYMENU:
			case SC_MOUSEMENU:
				appStatus->SoundHalt();
				break;
		}
	}
	else
	{
		switch (wParam & 0xfff0)
		{
        case SC_MAXIMIZE:
        case SC_MONITORPOWER:
		case SC_SCREENSAVE:
		case SC_TASKLIST:
		case SC_DEFAULT:
			lresult =  0;
			return true;
        case SC_MOVE:
        case SC_SIZE:
		case SC_MINIMIZE:
		case SC_KEYMENU:
		case SC_MOUSEMENU:
			appStatus->SoundHalt();
			lresult =  0;
			return true;
		}
	}

	return false;
}

bool CAppWindow::OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
bool bOK;
int w, h;

	bOK = false;
	m_pWinEmuWin->GetRequiredWindowSize(appStatus->m_borderSize, appStatus->m_bShowFloppyLed, appStatus->m_bDoubleSizedWindow, &w, &h);
	if (m_pWinEmuWin->Create(m_hInst, m_hWnd, NULL, 0, 0, w, h, (HMENU) LongToPtr(IDC_MAIN_WINEMULATION)) == 0)
	{
		lresult = -1;
	}
	else
	{
		lresult = 0;
	}

	return true;

	/*
	const int iStatusParts = 1;
	RECT rcStatusBar;
	HLOCAL hloc;
	hloc = LocalAlloc(LHND, sizeof(int) * iStatusParts);
	if (hloc != 0)
	{

		int *pParts = (int *)LocalLock(hloc);
		if (pParts != 0)
		{
			pParts[0] = 20;
			m_hWndStatusBar = CreateWindowEx(0, STATUSCLASSNAME, NULL, CCS_BOTTOM | WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, m_hWnd, (HMENU) IDC_MAIN_WINSTATUSBAR, m_hInstance, NULL);
			if (m_hWndStatusBar != 0)
			{
				SendMessage(m_hWndStatusBar, SB_SETPARTS, (WPARAM)iStatusParts, (LPARFAM)pParts);
				bOK = true;				
			}
			LocalUnlock(pParts);
		}
		LocalFree(hloc);
	}
	if (m_hWndStatusBar)
	{
		GetWindowRect(m_hWndStatusBar, &rcStatusBar);
		m_iStatusBarHeight = rcStatusBar.bottom - rcStatusBar.top;
		if (m_iStatusBarHeight < 0)
			m_iStatusBarHeight = 0;
	}
	*/		
}

bool CAppWindow::OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
HWND hWndDebuggerFrame;

	appStatus->SoundHalt();
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

	if (!appStatus->m_bWindowed)
	{
		if (SUCCEEDED(ToggleFullScreen()))
		{
			//Fix to avoid strange crash if the default DestroyWindow is called straight away.
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			lresult = 0;
			return true;
		}
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

	if (G::IsHideWindow)
	{
		return;
	}

	bIsWindowActive = !((BOOL)HIWORD(wParam)) && ((LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam) == WA_CLICKACTIVE));
	bIsWindowMinimised = !((BOOL)HIWORD(wParam));
	appStatus->m_bActive = bIsWindowMinimised;

	if (!appStatus->m_bWindowed && bIsWindowMinimised)
	{
		appStatus->SoundHalt();
		if (G::IsDwmApiOk() && appStatus->m_bDisableDwmFullscreen)
		{
			if (!appStatus->m_bWindowed)
			{
				BOOL bIsDwmOn;
				if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
				{
					G::s_pFnDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
				}
			}
		}
	}

	if (appStatus->m_bReady)
	{
		c64->ResetKeyboard();
	}
}

bool CAppWindow::OnGetMinMaxInfo(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
MINMAXINFO *pMinMax;
	if (appStatus != NULL)
	{
		if (appStatus->m_bWindowed)
		{
			int w;
			int h;
			GetMinimumWindowedSize(appStatus->m_borderSize, appStatus->m_bShowFloppyLed, &w, &h);
			pMinMax = (MINMAXINFO *)lParam;
			pMinMax->ptMinTrackSize.x = w;
			pMinMax->ptMinTrackSize.y = h;
			return true;
		}
	}

	return false;
}

bool CAppWindow::OnSetCursor(HWND hWnd, WPARAM wParam, LPARAM lParam, LRESULT &lresult)
{
    if (appStatus->m_bActive && !appStatus->m_bWindowed)
    {
        SetCursor(NULL);
        lresult = TRUE;
		return true;
    }
	else if (appStatus->m_bBusy)
	{
		SetCursor(m_hCursorBusy);
		lresult = TRUE;
		return true;
	}
	else
	{
		return false;
	}
}

void CAppWindow::OnDisplayChange(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (G::IsHideWindow)
	{
		return;
	}

	if (appStatus->m_bWindowed)
	{
		appStatus->m_bReady = false;
	}
}

void CAppWindow::OnPaletteChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (G::IsHideWindow)
	{
		return;
	}

	if (appStatus->m_bWindowed)
	{
		if (hWnd != (HWND) wParam)
		{
			SetColours();
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
}

LRESULT CAppWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HMENU hMenu  = 0;
LRESULT lr = 0;

	switch (uMsg) 
	{
	case WM_CREATE:
		if (OnCreate(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
	case WM_ENTERMENULOOP:
		appStatus->SoundHalt();
		UpdateMenu();
		return 0;
	case WM_EXITMENULOOP:
		appStatus->SoundResume();
		return 0;
	case WM_ENTERSIZEMOVE:
		appStatus->SoundHalt();
		return 0;
	case WM_EXITSIZEMOVE:
		appStatus->SoundResume();
		return 0;
	case WM_ACTIVATE:
		OnActivate(hWnd, wParam, lParam);
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
	case WM_MOVE:
		return 0;
	case WM_GETMINMAXINFO:
		if (OnGetMinMaxInfo(hWnd, wParam, lParam))
		{
			return 0;
		}
		
		break;
	case WM_DISPLAYCHANGE:
		OnDisplayChange(hWnd, wParam, lParam);
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
    case WM_SETCURSOR:
		if (OnSetCursor(hWnd, wParam, wParam, lr))
		{
			return lr;
		}

		break;
	case WM_DESTROY:
		appStatus->m_bReady = false;
		appStatus->FreeDirectX();
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
	}

	return (DefWindowProc(hWnd, uMsg, wParam, lParam));
}

void CAppWindow::CloseWindow()
{	
	PostMessage(this->m_hWnd, WM_CLOSE, 0, 0);
}

void CAppWindow::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A C64 CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = m_pAppCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A disk CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = m_pAppCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	//G::DebugMessageBox(hWnd, TEXT("A VIC raster compare breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = m_pAppCommand->ShowDevelopment();
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
		if (appStatus->m_bDoubleSizedWindow)
		{
			CheckMenuItem (hMenu, IDM_SETTING_DOUBLESIZEDWINDOW, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTING_DOUBLESIZEDWINDOW, MF_BYCOMMAND | MF_UNCHECKED);
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
	return SetWindowedMode(true, true, false, 0, 0, appStatus->m_bUseBlitStretch);
}


HRESULT CAppWindow::ToggleFullScreen()
{
	HRESULT r = SetWindowedMode(!appStatus->m_bWindowed, appStatus->m_bDoubleSizedWindow, appStatus->m_bWindowedCustomSize, this->m_rcMainWindow.right - this->m_rcMainWindow.left, this->m_rcMainWindow.bottom - this->m_rcMainWindow.top, appStatus->m_bUseBlitStretch);

	if (G::IsWin98OrLater())
	{
		if(appStatus->m_bWindowed)
		{
			SetThreadExecutionState(ES_CONTINUOUS);   
		}
		else
		{
			SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS); 
		}
	}
	return r;
}

HRESULT CAppWindow::SetWindowedMode(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch)
{
HRESULT hr;

	ClearError();
	appStatus->m_bReady = false;
	appStatus->m_fskip = -1;
    if (appStatus->m_bWindowed)
	{
		SaveMainWindowSize();
		if (!bWindowed)
		{
			SetWindowedStyle(false);
		}
	}

	hr = SetCoopLevel(bWindowed, bDoubleSizedWindow, bWindowedCustomSize, width, height, bUseBlitStretch);
	if (FAILED(hr))
	{
		TCHAR errorTextSave[300];
		_tcscpy_s(errorTextSave, _countof(errorText), errorText);
		SetCoopLevel(TRUE, bDoubleSizedWindow, bWindowedCustomSize, width, height, bUseBlitStretch);
		_tcscpy_s(errorText, _countof(errorText), errorTextSave);
	}

	if (appStatus->m_bWindowed)
	{
		SetWindowedStyle(true);
		UINT showWindowFlags;
		if (G::IsHideWindow)
		{
			showWindowFlags = SWP_NOZORDER | SWP_NOSIZE | SWP_NOSENDCHANGING | SWP_NOREDRAW | SWP_NOOWNERZORDER;
		}
		else
		{
			showWindowFlags = SWP_NOZORDER | SWP_FRAMECHANGED;
			if (width <= 0 || height <= 0)
			{
				showWindowFlags = SWP_NOSIZE;
			}
		}

		SetWindowPos(m_hWnd, HWND_TOP, m_rcMainWindow.left, m_rcMainWindow.top, width, height, showWindowFlags);
		G::EnsureWindowPosition(m_hWnd);
		SaveMainWindowSize();
		if (appStatus->m_syncModeWindowed == HCFG::FSSM_VBL && !appStatus->m_bDebug)
		{
			appStatus->m_fskip = 1;
		}

		if (!G::IsHideWindow)
		{
			if (SUCCEEDED(this->m_pWinEmuWin->UpdateC64Window()))
			{
				this->m_pWinEmuWin->Present(0);
			}
		}
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
		}

		if (m_hWndStatusBar)
		{
			ShowWindow(m_hWndStatusBar, SW_SHOW);
		}
	}
	else
	{
		hMt=GetMenu(m_hWnd);
		if (hMt)
		{
			SetMenu(m_hWnd, NULL);
			m_hMenuOld = hMt;
		}

		if (m_hWndStatusBar)
		{
			ShowWindow(m_hWndStatusBar, SW_HIDE);
		}

		lp = GetWindowLongPtr(m_hWnd, GWL_STYLE);
		lp &= ~CAppWindow::StylesWindowed;
		lp |= CAppWindow::StylesNonWindowed;
		#pragma warning(disable:4244)
		SetWindowLongPtr(m_hWnd, GWL_STYLE, lp);
		#pragma warning(default:4244)
	}
}

HRESULT CAppWindow::SetCoopLevel(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch)
{
HRESULT hRet;
	BOOL bIsDwmOn = FALSE;

	if (G::IsHideWindow)
	{
		return S_FALSE;
	}

	if (G::IsDwmApiOk() && appStatus->m_bDisableDwmFullscreen)
	{
		if (!bWindowed)
		{			
			if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
			{
				if (bIsDwmOn)
				{
					G::s_pFnDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
				}
			}
		}
	}

	hRet = InitSurfaces(bWindowed, bDoubleSizedWindow, bWindowedCustomSize, width, height, bUseBlitStretch);
	if (SUCCEEDED(hRet))
	{
		appStatus->m_bWindowed = bWindowed;
	}

	if (G::IsDwmApiOk() && appStatus->m_bDisableDwmFullscreen)
	{
		if (bWindowed)
		{
			if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
			{
				G::s_pFnDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
			}
		}
	}

    return hRet;
}

HRESULT CAppWindow::InitSurfaces(bool bWindowed, bool bDoubleSizedWindow, bool bWindowedCustomSize, int width, int height, bool bUseBlitStretch)
{
HRESULT		        hRet;
int w,h;
D3DDISPLAYMODE displayModeRequested;
D3DTEXTUREFILTERTYPE filter;
RECT rc;
	ClearError();
	ZeroMemory(&displayModeRequested, sizeof(D3DDISPLAYMODE));
	filter = CDX9::GetDxFilterFromEmuFilter(appStatus->m_blitFilter);
	if (bWindowed)
	{
  		if (bWindowedCustomSize)
		{
			SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_rcMainWindow.left, m_rcMainWindow.top, width, height, SWP_NOMOVE);
			SetRect(&rc, 0, 0, width, height);
			CalcEmuWindowSize(rc, &w, &h);
			SetWindowPos(m_pWinEmuWin->GetHwnd(), HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
		}
		else
		{
			GetRequiredMainWindowSize(appStatus->m_borderSize, appStatus->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
			SetWindowPos(m_hWnd, HWND_NOTOPMOST, m_rcMainWindow.left, m_rcMainWindow.top, w, h,  SWP_NOMOVE);
			m_pWinEmuWin->GetRequiredWindowSize(appStatus->m_borderSize, appStatus->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
			SetWindowPos(m_pWinEmuWin->GetHwnd(), HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
		}
		
		hRet = dx->InitD3D(m_pWinEmuWin->GetHwnd(), m_hWnd, true, bDoubleSizedWindow, bWindowedCustomSize, appStatus->m_borderSize, appStatus->m_bShowFloppyLed, bUseBlitStretch, appStatus->m_fullscreenStretch, filter, appStatus->m_syncModeWindowed, appStatus->m_fullscreenAdapterNumber, appStatus->m_fullscreenAdapterId, displayModeRequested);
		if (FAILED(hRet))
		{
			appStatus->m_bWindowed = true;
			appStatus->m_bWindowedCustomSize = false;
			return SetError(hRet, TEXT("InitD3D failed."));
		}

		appStatus->m_bWindowed = true;		
	}
	else
	{
		displayModeRequested.Format = (D3DFORMAT)appStatus->m_fullscreenFormat;
		displayModeRequested.Height = appStatus->m_fullscreenHeight;
		displayModeRequested.Width = appStatus->m_fullscreenWidth;
		displayModeRequested.RefreshRate = appStatus->m_fullscreenRefresh;
		hRet = dx->InitD3D(m_hWnd, m_hWnd, false, true, bWindowedCustomSize, appStatus->m_borderSize, appStatus->m_bShowFloppyLed, bUseBlitStretch, appStatus->m_fullscreenStretch, filter, appStatus->m_syncModeFullscreen, appStatus->m_fullscreenAdapterNumber, appStatus->m_fullscreenAdapterId, displayModeRequested);
		if (FAILED(hRet))
		{
			appStatus->m_bWindowed = true;
			appStatus->m_bWindowedCustomSize = false;
			return SetError(hRet, TEXT("InitD3D failed."));
		}

		appStatus->m_bWindowed = false;
	}

	ClearSurfaces();
	SetColours();
	return S_OK;
}

void CAppWindow::ClearSurfaces()
{
	m_pWinEmuWin->ClearSurfaces();
}

void CAppWindow::UpdateWindowTitle(TCHAR *szTitle, DWORD emulationSpeed)
{
TCHAR szBuff[300];
	szBuff[0];
	if (appStatus==NULL || appStatus==NULL)
		return;
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

void CAppWindow::SetDriveMotorLed(bool bOn)
{
	if (m_hWndStatusBar)
	{
		if (bOn)
		{
			SendMessage(m_hWndStatusBar, SB_SETBKCOLOR, 0, (LPARAM) RGB(33, 250, 131));
		}
		else
		{
			SendMessage(m_hWndStatusBar, SB_SETBKCOLOR, 0, (LPARAM) RGB(1, 63, 29));
		}
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
const int X_GAP = 362;
bool bCreated = false;
bool ok = false;

	appStatus->SoundHalt();
	c64->SynchroniseDevicesWithVIC();
	appStatus->m_bRunning = false;
	appStatus->m_bDebug = true;

	RECT rcDesk;
	G::GetWorkArea(rcDesk);

	try
	{
		if (m_pMDIDebugger.expired() || m_pMDIDebugger.lock()->GetHwnd() == 0)
		{
			pwin = shared_ptr<CMDIDebuggerFrame>(new CMDIDebuggerFrame(this->c64, this->m_pAppCommand, this->appStatus, this->appStatus));
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
			hWnd = pwin->GetHwnd();
			ok = true;
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
		this->UpdateWindowTitle(appStatus->GetAppTitle(), -1);
		this->m_pWinEmuWin->UpdateC64WindowWithObjects();
	}
	else
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the debugger window."), appStatus->GetAppName(), MB_ICONWARNING);
		m_pAppCommand->Resume();
	}
	return hWnd;
}
