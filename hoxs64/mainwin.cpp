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
#include <vector>
#include <list>
#include <assert.h>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "user_message.h"
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
#include "c64file.h"
#include "monitor.h"
#include "prgbrowse.h"
#include "diagkeyboard.h"
#include "diagjoystick.h"
#include "diagemulationsettingstab.h"
#include "diagnewblankdisk.h"
#include "diagabout.h"
#include "diagfilesaved64.h"

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
#include "mdidebuggerframe.h"

#include "monitor.h"
#include "emuwin.h"
#include "mainwin.h"
#include "resource.h"

const LPTSTR CAppWindow::lpszClassName = HOXS_MAIN_WND_CLASS;
const LPTSTR CAppWindow::lpszMenuName = APPMENUNAME;



CAppWindow::CAppWindow(CDX9 *dx, IAppCommand *pAppCommand, CConfig *cfg, CAppStatus *appStatus, C64 *c64)
{
	m_hOldCursor = NULL;
	m_hWndStatusBar = 0;
	m_iStatusBarHeight = 0;

	this->dx = dx;
	this->cfg = cfg;
	this->appStatus = appStatus;
	this->c64 = c64;
	this->m_pAppCommand = pAppCommand;

	appStatus->m_bWindowed = TRUE;

	m_hCursorBusy = LoadCursor(0L, IDC_WAIT);
	if (!m_hCursorBusy)
		throw std::runtime_error("LoadCursor failed.");
	m_pWinEmuWin = shared_ptr<CEmuWindow>(new CEmuWindow(dx, cfg, appStatus, c64));
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

void CAppWindow::SetMainWindowSize(bool bDoubleSizedWindow)
{
int w,h;

	GetRequiredMainWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
	SetWindowPos(m_hWnd,0,0,0,w,h, SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOOWNERZORDER|SWP_NOZORDER);
}

void CAppWindow::GetRequiredMainWindowSize(HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, bool bDoubleSizedWindow, int *w, int *h)
{
C64WindowDimensions dims;

	dims.SetBorder(borderSize);

	if (bDoubleSizedWindow)
	{
		*w=dims.Width*2 + GetSystemMetrics(SM_CXFIXEDFRAME)*2;
		*h=dims.Height*2 + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) +  CDX9::GetToolBarHeight(bShowFloppyLed) + m_iStatusBarHeight;
	}
	else
	{
		*w=dims.Width + GetSystemMetrics(SM_CXFIXEDFRAME)*2;
		*h=dims.Height + GetSystemMetrics(SM_CYFIXEDFRAME)*2 + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYCAPTION) +  CDX9::GetToolBarHeight(bShowFloppyLed) + m_iStatusBarHeight;
	}
}

HWND CAppWindow::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	this->m_hInst = hInstance;
	HWND hWnd = CVirWindow::CreateVirWindow(0L, lpszClassName, title, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, x, y, w, h, hWndParent, hMenu, hInstance);
	return hWnd;
}

LRESULT CAppWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
int w, h;
MINMAXINFO *pMinMax;
bool bOK;
const int iStatusParts = 1;
RECT rcStatusBar;
HMENU hMenu  = 0;
int wmId, wmEvent;
HRESULT hRet;
INT_PTR r;
HRESULT hr;
CConfig tCfg;
bool bIsWindowActive;
bool bIsWindowMinimised;
HWND hWndDebuggerFrame;
LRESULT lr;
/*Dialogs*/
shared_ptr<CDiagKeyboard> pDiagKeyboard;
shared_ptr<CDiagJoystick> pDiagJoystick;
shared_ptr<CDiagEmulationSettingsTab> pDiagEmulationSettingsTab;
shared_ptr<CDiagNewBlankDisk> pDiagNewBlankDisk;
shared_ptr<CDiagAbout> pDiagAbout;

	switch (uMsg) 
	{
	case WM_CREATE:
		bOK = false;
		m_pWinEmuWin->GetRequiredWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, cfg->m_bDoubleSizedWindow, &w, &h);
		if (m_pWinEmuWin->Create(m_hInst, m_hWnd, NULL, 0, 0, w, h, (HMENU) LongToPtr(IDC_MAIN_WINEMULATION)) == 0)
			return -1;
		/*
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
		*/
		
		bOK = true;
		if (m_hWndStatusBar)
		{
			GetWindowRect(m_hWndStatusBar , &rcStatusBar);
			m_iStatusBarHeight = rcStatusBar.bottom - rcStatusBar.top;
			if (m_iStatusBarHeight < 0)
				m_iStatusBarHeight = 0;
			SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
		}
		return bOK ? 0 : -1;
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
		// Pause if minimized
		bIsWindowActive = !((BOOL)HIWORD(wParam)) && ((LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam) == WA_CLICKACTIVE));
		bIsWindowMinimised = !((BOOL)HIWORD(wParam));
		//appStatus->m_bActive = !((BOOL)HIWORD(wParam));
		//appStatus->m_bActive = (LOWORD(wParam) == WA_ACTIVE) || (LOWORD(wParam) == WA_CLICKACTIVE);
		appStatus->m_bActive = bIsWindowMinimised;

		if (!appStatus->m_bWindowed && bIsWindowMinimised)
		{
			appStatus->SoundHalt();
			if (G::IsDwmApiOk() && cfg->m_bDisableDwmFullscreen)
			{
				if (!appStatus->m_bWindowed)
				{
					BOOL bIsDwmOn;
					if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
					{
						if (appStatus->m_bGotOldDwm && appStatus->m_oldDwm && !bIsDwmOn)
							G::s_pFnDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
					}
				}
			}
		}

		if (appStatus->m_bReady)
		{
			c64->cia1.ResetKeyboard();
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_CHAR:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KEYDOWN:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_KEYUP:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
        // Check to see if we are losing our window...
        if (SIZE_MAXHIDE==wParam || SIZE_MINIMIZED==wParam)
		{
			appStatus->SoundHalt();
            appStatus->m_bActive = FALSE;
		}
        else
            appStatus->m_bActive = TRUE;

		if (appStatus->m_bActive && appStatus->m_bWindowed && appStatus->m_bReady)
		{
			SaveMainWindowSize();
		}

		if (appStatus->m_bWindowed)
		{
			if (m_hWndStatusBar != NULL)				
				SendMessage(m_hWndStatusBar, WM_SIZE, 0, 0);
		}
		appStatus->m_lastAudioVBLCatchUpCounter = 0;
		return 0;
	case WM_MOVE:
		// Retrieve the window position after a move
		if (appStatus->m_bActive && appStatus->m_bReady && appStatus->m_bWindowed)
		{
			SaveMainWindowSize();
			appStatus->m_lastAudioVBLCatchUpCounter = 0;
		}
		return 0;
	case WM_GETMINMAXINFO:
		// Fix the size of the window

		GetRequiredMainWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, cfg->m_bDoubleSizedWindow, &w, &h);
		if (appStatus->m_bWindowed && appStatus->m_bFixWindowSize)
		{
			pMinMax = (MINMAXINFO *)lParam;
			pMinMax->ptMinTrackSize.x = w;
			pMinMax->ptMinTrackSize.y = h;
			pMinMax->ptMaxTrackSize.x = w;
			pMinMax->ptMaxTrackSize.y = h;
		}
		return 0;
	case WM_DISPLAYCHANGE:
		if (appStatus->m_bWindowed)
		{
			appStatus->m_bReady=FALSE;
		}
		return (DefWindowProc(m_hWnd, uMsg, wParam, lParam));
	case WM_COMMAND:
		wmId    = LOWORD(wParam); // Remember, these are...
		wmEvent = HIWORD(wParam); // ...different for Win32!

		//Parse the menu selections:
		switch (wmId) 
		{
		case IDM_TAPE_INSERT:
			appStatus->SoundHalt();
			appStatus->InsertTape(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_TAPE_PLAY:
			c64->TapePressPlay();
			break;
		case IDM_TAPE_STOP:
			c64->TapePressStop();
			break;
		case IDM_TAPE_REWIND:
			c64->TapeRewind();
			break;
		case IDM_TAPE_LOADIMAGE:
			appStatus->SoundHalt();
			appStatus->LoadC64Image(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_TAPE_LOADT64:
			appStatus->SoundHalt();
			appStatus->LoadT64(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_FILE_AUTOLOAD:
			appStatus->SoundHalt();
			appStatus->AutoLoad(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_FILE_HARDRESET:
			c64->HardReset(true);
			break;
		case IDM_FILE_SOFTRESET:
			c64->SoftReset(true);
			break;
		case IDM_FILE_MONITOR:
			if (appStatus->m_bWindowed)
				m_pAppCommand->ShowDevelopment();
			break;
		case IDM_FILE_PAUSE:
			appStatus->TogglePause();
			break;
		case IDM_SETTING_MUTESOUND:
			appStatus->ToggleSoundMute();			
			break;
		case IDM_EXIT:
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			break;
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
			break;
		case IDM_SETTING_RESTOREDEFAULT:
			appStatus->SoundHalt();
			appStatus->RestoreDefaultSettings();
			this->m_pWinEmuWin->UpdateC64Window();
			MessageBox(hWnd, TEXT("Default settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION); 
			appStatus->SoundResume();
			break;
		case IDM_SETTING_LOADSETTINGS_RESTOREUSER:
			appStatus->SoundHalt();
			appStatus->RestoreUserSettings();
			this->m_pWinEmuWin->UpdateC64Window();
			MessageBox(hWnd, TEXT("User settings restored."), APPNAME, MB_OK | MB_ICONINFORMATION); 
			appStatus->SoundResume();
			break;
		case IDM_SETTING_SAVE:
			appStatus->SoundHalt();
			appStatus->SaveCurrentSetting();
			MessageBox(hWnd, TEXT("Setting saved."), APPNAME, MB_OK | MB_ICONINFORMATION); 
			appStatus->SoundResume();
			break;
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
			break;
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
						pDiagKeyboard->DisplayError(hWnd, appStatus->GetAppName());
				}
				dx->pKeyboard->Unacquire();
				dx->pKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY); 
			}
			catch(...)
			{
			}
			appStatus->SoundResume();
			break;
		case IDM_SETTING_JOYSTICK:
			appStatus->SoundHalt();
			try
			{
				appStatus->GetUserConfig(tCfg);
				pDiagJoystick = shared_ptr<CDiagJoystick>(new CDiagJoystick());
				if (pDiagJoystick!=0)
				{
					hr = pDiagJoystick->Init(dx, &tCfg);
					if (SUCCEEDED(hr))
					{
						r = pDiagJoystick->ShowDialog(m_hInst, MAKEINTRESOURCE(IDD_JOYSTICK), hWnd);
						if (LOWORD(r) == IDOK)
						{
							tCfg = pDiagJoystick->newCfg;
							appStatus->SetUserConfig(tCfg);
							appStatus->ApplyConfig(tCfg);
						}
					}
					else
						pDiagJoystick->DisplayError(hWnd, appStatus->GetAppName());
				}
			}
			catch(...)
			{
			}
			appStatus->SoundResume();
			break;
		case IDM_SETTING_DOUBLESIZEDWINDOW:
			appStatus->SoundHalt();
			appStatus->GetUserConfig(tCfg);
			tCfg.m_bDoubleSizedWindow = !tCfg.m_bDoubleSizedWindow;
			appStatus->SetUserConfig(tCfg);
			appStatus->ApplyConfig(tCfg);
			appStatus->SoundResume();
			break;
		case IDM_SETTING_SWAPJOYSTICKS:
			appStatus->SoundHalt();
			appStatus->GetUserConfig(tCfg);
			tCfg.m_bSwapJoysticks = !tCfg.m_bSwapJoysticks;
			appStatus->SetUserConfig(tCfg);
			appStatus->ApplyConfig(tCfg);
			MessageBeep(MB_ICONASTERISK);
			appStatus->SoundResume();
			break;
		case IDM_SETTING_MAXSPEED:
			appStatus->SoundHalt();
			appStatus->ToggleMaxSpeed();
			appStatus->SoundResume();
			break;
		case IDM_DISK_INSERT_EXISTINGDISK:
			appStatus->SoundHalt();
			appStatus->InsertDiskImage(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_DISK_SAVEDISK_D64:
			appStatus->SoundHalt();
			appStatus->SaveD64Image(hWnd);
			appStatus->SoundResume();
			break;
		case IDM_DISK_SAVEDISK_FDI:
			appStatus->SoundHalt();
			appStatus->SaveFDIImage(hWnd);
			appStatus->SoundResume();
			break;
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
			break;
		case IDM_DISK_REMOVEDISK:
			c64->RemoveDisk();
			break;
		case IDM_DISK_WRITEPROTECT_ON:     
			c64->diskdrive.D64_DiskProtect(TRUE);
			UpdateMenu();
			break;
		case IDM_DISK_WRITEPROTECT_OFF:        
			c64->diskdrive.D64_DiskProtect(FALSE);
			UpdateMenu();
			break;
		case IDM_DISK_RESETDRIVE:
			c64->diskdrive.Reset(c64->cpu.CurrentClock);
			break;
		case IDM_TOGGLEFULLSCREEN:
			appStatus->SoundHalt();
            if (appStatus->m_bActive && appStatus->m_bReady && !appStatus->m_bDebug)
            {
				appStatus->m_bPaused= FALSE;
				appStatus->m_bReady = FALSE;
				hRet = ToggleFullScreen();
				if (FAILED(hRet))
					DisplayError(hWnd, appStatus->GetAppName());
            }
			break;
		default:
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		break;
	case WM_CLOSE:
		appStatus->SoundHalt();
		appStatus->m_bClosing = true;
		if (!this->m_pMDIDebugger.expired())
		{
			shared_ptr<CMDIDebuggerFrame> pwin = m_pMDIDebugger.lock();
			hWndDebuggerFrame = pwin->GetHwnd();
			if (IsWindow(hWndDebuggerFrame))
			{
				lr = SendMessage(hWndDebuggerFrame, WM_CLOSE, 0, 0);
			}
		}
		if (!appStatus->m_bWindowed)
		{
			if (SUCCEEDED(ToggleFullScreen()))
			{
				//Fix to avoid strange crash if the default DestroyWindow is called straight away.
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				return 0;
			}
			else
				return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
		else
		{
			cfg->SaveWindowSetting(hWnd);
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
	case WM_SYSCOMMAND:
		if (!appStatus->m_bActive)
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		else if (appStatus->m_bWindowed)
		{
			switch (wParam & 0xfff0)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
				case SC_MOVE:
				case SC_SIZE:
				case SC_MINIMIZE:
				case SC_KEYMENU:
				case SC_MOUSEMENU:
					appStatus->SoundHalt();
					break;
			}
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
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
				return 0;
            case SC_MOVE:
            case SC_SIZE:
			case SC_MINIMIZE:
			case SC_KEYMENU:
			case SC_MOUSEMENU:
				appStatus->SoundHalt();
				return 0;

			}
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		}
	case WM_ERASEBKGND:
		return 1;
	case WM_QUERYNEWPALETTE:
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	case WM_PALETTECHANGED:
		if (appStatus->m_bWindowed)
		{
			if (hWnd != (HWND) wParam)
			{
				SetColours();
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	case WM_NCPAINT:
		if (appStatus->m_bWindowed)
			return (DefWindowProc(hWnd, uMsg, wParam, lParam));
		else
			return 0;
    case WM_SETCURSOR:
        if (appStatus->m_bActive && !appStatus->m_bWindowed)
        {
            SetCursor(NULL);
            return TRUE;
        }
		else
		{
			if (appStatus->m_bBusy)
			{
				SetCursor(m_hCursorBusy);
				return TRUE;
			}
			else
			{
				return (DefWindowProc(hWnd, uMsg, wParam, lParam));
			}
		}
		break;
	case WM_DESTROY:
		appStatus->m_bReady = FALSE;
		appStatus->FreeDirectX();
		if (appStatus->m_bInitDone)
			PostQuitMessage(0);
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	case WM_MONITOR_BREAK_CPU64:
		OnBreakCpu64(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_CPUDISK:
		OnBreakCpuDisk(hWnd, uMsg, wParam, lParam);
		return 0;
	case WM_MONITOR_BREAK_VICRASTER:
		OnBreakVic(hWnd, uMsg, wParam, lParam);
		return 0;
	default:
		return (DefWindowProc(hWnd, uMsg, wParam, lParam));
	}
	return (0);

}

void CAppWindow::OnBreakCpu64(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	MessageBox(hWnd, TEXT("A C64 CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = m_pAppCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakCpuDisk(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	MessageBox(hWnd, TEXT("A disk CPU execute breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
	HWND hWndMon = m_pAppCommand->ShowDevelopment();
	if (hWndMon)
	{
		SendMessage(hWndMon, uMsg, wParam, lParam);
	}
}

void CAppWindow::OnBreakVic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	appStatus->SoundHalt();
	MessageBox(hWnd, TEXT("A VIC raster compare breakpoint occurred."), TEXT("Monitor Breakpoint"), MB_ICONSTOP|MB_OK);
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
		if (c64->diskdrive.m_d64_protectOff==0)
		{
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_ON, MF_BYCOMMAND | MF_CHECKED);
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_OFF, MF_BYCOMMAND | MF_UNCHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_ON, MF_BYCOMMAND | MF_UNCHECKED);
			CheckMenuItem (hMenu, IDM_DISK_WRITEPROTECT_OFF, MF_BYCOMMAND | MF_CHECKED);
		}
		if (cfg->m_bDoubleSizedWindow)
		{
			CheckMenuItem (hMenu, IDM_SETTING_DOUBLESIZEDWINDOW, MF_BYCOMMAND | MF_CHECKED);
		}
		else
		{
			CheckMenuItem (hMenu, IDM_SETTING_DOUBLESIZEDWINDOW, MF_BYCOMMAND | MF_UNCHECKED);
		}

	
		if (cfg->m_bSwapJoysticks)
			CheckMenuItem (hMenu, IDM_SETTING_SWAPJOYSTICKS, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem (hMenu, IDM_SETTING_SWAPJOYSTICKS, MF_BYCOMMAND | MF_UNCHECKED);

		if (cfg->m_bMaxSpeed)
			CheckMenuItem (hMenu, IDM_SETTING_MAXSPEED, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem (hMenu, IDM_SETTING_MAXSPEED, MF_BYCOMMAND | MF_UNCHECKED);

		if (appStatus->m_bPaused)
			CheckMenuItem (hMenu, IDM_FILE_PAUSE, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem (hMenu, IDM_FILE_PAUSE, MF_BYCOMMAND | MF_UNCHECKED);

		if (appStatus->m_bSoundMute)
			CheckMenuItem (hMenu, IDM_SETTING_MUTESOUND, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem (hMenu, IDM_SETTING_MUTESOUND, MF_BYCOMMAND | MF_UNCHECKED);
	}
}


void CAppWindow::SaveMainWindowSize()
{
	GetWindowRect(m_hWnd, &m_rcMainWindow);
}


HRESULT CAppWindow::ResetDirect3D()
{
	return SetWindowedMode(appStatus->m_bWindowed, cfg->m_bDoubleSizedWindow, cfg->m_bUseBlitStretch);	
}


HRESULT CAppWindow::ToggleFullScreen()
{
	HRESULT r = SetWindowedMode(!appStatus->m_bWindowed, cfg->m_bDoubleSizedWindow, cfg->m_bUseBlitStretch);

	if (G::IsWin98OrLater())
	{
		if(appStatus->m_bWindowed)
			SetThreadExecutionState(ES_CONTINUOUS);   
		else
			SetThreadExecutionState(ES_DISPLAY_REQUIRED | ES_CONTINUOUS); 
	}
	return r;
}

HRESULT CAppWindow::SetWindowedMode(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch)
{
HRESULT hr;
static HMENU hMenuOld=NULL,hMt;
LONG_PTR lp;

	ClearError();
	appStatus->m_bFixWindowSize = FALSE;
	appStatus->m_bReady = FALSE;
	appStatus->m_fskip = -1;

    if (appStatus->m_bWindowed)
	{
        GetWindowRect(m_hWnd, &m_rcMainWindow);
		if (!bWindowed)
		{
			hMt=GetMenu(m_hWnd);
			if (hMt)
			{
				SetMenu(m_hWnd, NULL);
				hMenuOld = hMt;
			}
			if (m_hWndStatusBar)
				ShowWindow(m_hWndStatusBar, SW_HIDE);

			//TEST
			lp = GetWindowLongPtr(m_hWnd, GWL_STYLE);
			lp &= ~CAppWindow::StylesWindowed;
			lp |= CAppWindow::StylesNonWindowed;
			#pragma warning(disable:4244)
			SetWindowLongPtr(m_hWnd, GWL_STYLE, lp);
			#pragma warning(default:4244)
		}
	}

	hr = SetCoopLevel(bWindowed, bDoubleSizedWindow, bUseBlitStretch);
	if (FAILED(hr))
	{
		TCHAR errorTextSave[300];
		_tcscpy_s(errorTextSave, _countof(errorText), errorText);

		SetCoopLevel(TRUE, bDoubleSizedWindow, bUseBlitStretch);
		_tcscpy_s(errorText, _countof(errorText), errorTextSave);
	}

	if (appStatus->m_bWindowed)
	{
		lp = GetWindowLongPtr(m_hWnd, GWL_STYLE);
		lp |= CAppWindow::StylesWindowed;
		#pragma warning(disable:4244)
		SetWindowLongPtr(m_hWnd, GWL_STYLE, lp);
		#pragma warning(default:4244)
		if (hMenuOld)
		{
			SetMenu(m_hWnd, hMenuOld);
			hMenuOld = NULL;
		}

		if (m_hWndStatusBar)
			ShowWindow(m_hWndStatusBar, SW_SHOW);

		SetWindowPos(m_hWnd, HWND_TOP, m_rcMainWindow.left, m_rcMainWindow.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		G::EnsureWindowPosition(m_hWnd);
		SaveMainWindowSize();

		if (cfg->m_syncMode == HCFG::FSSM_VBL && !appStatus->m_bDebug)
			appStatus->m_fskip = 1;

		this->m_pWinEmuWin->UpdateC64Window();
	}
	appStatus->m_bFixWindowSize = TRUE;
	appStatus->m_lastAudioVBLCatchUpCounter = 0;
	return hr;
}


HRESULT CAppWindow::SetCoopLevel(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch)
{
HRESULT hRet;
	BOOL bIsDwmOn = FALSE;

	if (G::IsDwmApiOk() && cfg->m_bDisableDwmFullscreen)
	{
		if (!bWindowed)
		{			
			if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
				if (bIsDwmOn)
					G::s_pFnDwmEnableComposition(DWM_EC_DISABLECOMPOSITION);
		}
	}

	hRet = InitSurfaces(bWindowed, bDoubleSizedWindow, bUseBlitStretch);
	if (SUCCEEDED(hRet))
	{
		appStatus->m_bWindowed = bWindowed;
	}

	if (G::IsDwmApiOk() && cfg->m_bDisableDwmFullscreen)
	{
		if (bWindowed)
		{
			if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&bIsDwmOn)))
			{
				if (appStatus->m_bGotOldDwm && appStatus->m_oldDwm && !bIsDwmOn)
					G::s_pFnDwmEnableComposition(DWM_EC_ENABLECOMPOSITION);
			}
		}
	}

    return hRet;
}

HRESULT CAppWindow::InitSurfaces(bool bWindowed, bool bDoubleSizedWindow, bool bUseBlitStretch)
{
HRESULT		        hRet;
int w,h;
D3DDISPLAYMODE displayModeRequested;
D3DTEXTUREFILTERTYPE filter;

	ClearError();
	ZeroMemory(&displayModeRequested, sizeof(D3DDISPLAYMODE));

	filter = CDX9::GetDxFilterFromEmuFilter(cfg->m_blitFilter);


	if (bWindowed)
	{
		m_pWinEmuWin->GetRequiredWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
		SetWindowPos(m_pWinEmuWin->GetHwnd(), HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);

		GetRequiredMainWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE);
		
		hRet = dx->InitD3D(m_pWinEmuWin->GetHwnd(), m_hWnd, TRUE, bDoubleSizedWindow, cfg->m_borderSize, cfg->m_bShowFloppyLed, bUseBlitStretch, cfg->m_fullscreenStretch, filter, cfg->m_syncMode, cfg->m_fullscreenAdapterNumber, cfg->m_fullscreenAdapterId, displayModeRequested);
		if (FAILED(hRet))
		{
			return SetError(hRet, TEXT("InitD3D failed."));
		}

		GetRequiredMainWindowSize(cfg->m_borderSize, cfg->m_bShowFloppyLed, bDoubleSizedWindow, &w, &h);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, w, h, SWP_NOMOVE);

		appStatus->m_bWindowed = TRUE;

	}
	else
	{
		displayModeRequested.Format = (D3DFORMAT)cfg->m_fullscreenFormat;
		displayModeRequested.Height = cfg->m_fullscreenHeight;
		displayModeRequested.Width = cfg->m_fullscreenWidth;
		displayModeRequested.RefreshRate = cfg->m_fullscreenRefresh;

		hRet = dx->InitD3D(m_hWnd, m_hWnd, FALSE, TRUE, cfg->m_borderSize, cfg->m_bShowFloppyLed, bUseBlitStretch, cfg->m_fullscreenStretch, filter, cfg->m_syncMode, cfg->m_fullscreenAdapterNumber, cfg->m_fullscreenAdapterId, displayModeRequested);
		if (FAILED(hRet))
		{
			return SetError(hRet, TEXT("InitD3D failed."));
		}
		appStatus->m_bWindowed = FALSE;
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
	if (cfg==NULL || appStatus==NULL)
		return;
	if (cfg->m_bShowSpeed && (long)emulationSpeed>=0)
	{
		_sntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, TEXT("%s at %d%%"), szTitle, emulationSpeed);
	}
	else
	{
		_sntprintf_s(szBuff, _countof(szBuff), _TRUNCATE, TEXT("%s"), szTitle);
	}
	if (appStatus->m_bSoundMute)
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Mute"));
	if (cfg->m_bMaxSpeed)
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Max Speed"));
	if (appStatus->m_bDebug)
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Debug"));
	if (appStatus->m_bPaused)
		_tcscat_s(szBuff, _countof(szBuff), TEXT(" - Paused"));
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
HWND hWnd = NULL;
shared_ptr<CMDIDebuggerFrame> pwin;
HRESULT hr;
bool bCreated = false;
bool ok = false;

	appStatus->SoundHalt();
	c64->SynchroniseDevicesWithVIC();
	appStatus->m_bRunning = FALSE;
	appStatus->m_bDebug = TRUE;

	try
	{
		if (m_pMDIDebugger.expired())
		{
			pwin = shared_ptr<CMDIDebuggerFrame>(new CMDIDebuggerFrame(this->c64, this->m_pAppCommand, this->cfg, this->appStatus));
			if (pwin != NULL)
			{
				int x,y,w,h;
				x = CW_USEDEFAULT;
				y = CW_USEDEFAULT;
				w = CW_USEDEFAULT;
				h = CW_USEDEFAULT;
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
			pwin->ShowDebugCpuDisk(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
			pwin->ShowDebugCpuC64(DBGSYM::SetDisassemblyAddress::EnsurePCVisible, 0);
		}
		this->UpdateWindowTitle(appStatus->GetAppTitle(), -1);
		this->m_pWinEmuWin->UpdateC64Window();
	}
	else
	{
		MessageBox(0L, TEXT("Unable to create the debugger window."), appStatus->GetAppName(), MB_ICONWARNING);
		m_pAppCommand->Resume();
	}
	return hWnd;
}
