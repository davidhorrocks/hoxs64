#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Winuser.h>
//include <Winable.h>
//include <shlwapi.h>
#include "dx_version.h"
#include <d3d9.h>
#include <D3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <memory.h>
#include <vector>
#include <list>
#include <assert.h>
#include "servicerelease.h"
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "register.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "hexconv.h"
#include "filter.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "cia6526.h"
#include "cia1.h"
#include "cia2.h"
#include "vic6569.h"
#include "cia1.h"
#include "cia2.h"
#include "sid.h"
#include "tap.h"

#include "d64.h"
#include "d1541.h"
#include "via6522.h"
#include "via1.h"
#include "via2.h"
#include "t64.h"
#include "tap.h"
#include "diskinterface.h"
#include "sidfile.h"
#include "C64.h"
#include "c64file.h"

#include "user_message.h"
#include "monitor.h"

#include "prgbrowse.h"
#include "diagkeyboard.h"
#include "diagjoystick.h"
#include "diagemulationsettingstab.h"
#include "diagnewblankdisk.h"
#include "diagabout.h"
#include "diagfilesaved64.h"

#include "cmdarg.h"

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

#include "emuwin.h"
#include "mainwin.h"

#include "main.h"
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#if _WIN32_WINNT >= 0x400
LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );
#else
LRESULT CALLBACK KeyboardProc( int nCode, WPARAM wParam, LPARAM lParam );
#endif

BOOL CenterWindow (HWND, HWND);

CApp app;


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	return app.Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

CApp::CApp()
{
	ZeroMemory(&m_Vinfo, sizeof(m_Vinfo));
	_tcsncpy_s(m_szAppName, _countof(m_szAppName), APPNAME, _TRUNCATE);
	_tcsncpy_s(m_szMainWndClsName, _countof(m_szMainWndClsName), HOXS_MAIN_WND_CLASS, _TRUNCATE);
	_tcsncpy_s(m_szMonitorWndClsName, _countof(m_szMonitorWndClsName), HOXS_MONITOR_WND_CLASS, _TRUNCATE);
	_tcsncpy_s(m_szMonitorDisassembleWndClsName, _countof(m_szMonitorDisassembleWndClsName), HOXS_MONITOR_DISS_WND_CLASS, _TRUNCATE);

	m_bShowSpeed = TRUE;
	m_bLimitSpeed = TRUE;
	m_bSkipFrames = TRUE;
	
	m_bActive     = FALSE;
	m_bReady      = FALSE;
	m_bRunning    = FALSE;
	m_bWindowed   = TRUE;
	m_bDebug      = FALSE;
	m_bPaused      = FALSE;
	m_bMaxSpeed   = FALSE;
	m_syncMode = HCFG::FSSM_LINE;
	m_bAutoload = FALSE;
	m_bFixWindowSize = FALSE;
	m_bWindowSizing = FALSE;
	m_bClosing = false;
	m_bBusy = false;
	hCursorBusy = LoadCursor(0L, IDC_WAIT);
	hOldCursor = NULL;
	m_hKeyboardHook = NULL;

	ZeroMemory(&m_StartupStickyKeys, sizeof(m_StartupStickyKeys));
	ZeroMemory(&m_StartupToggleKeys, sizeof(m_StartupToggleKeys));
	ZeroMemory(&m_StartupFilterKeys, sizeof(m_StartupFilterKeys));
	m_StartupStickyKeys.cbSize = sizeof(STICKYKEYS);
	m_StartupToggleKeys.cbSize = sizeof(TOGGLEKEYS);
	m_StartupFilterKeys.cbSize = sizeof(FILTERKEYS);
}

CApp::~CApp()
{
	FreeDirectX();
}


int CApp::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
MSG msg;
HRESULT hRet;
ULARGE_INTEGER frequency, last_counter, new_counter, tSlice;
DWORD emulationSpeed = FRAMEUPDATEFREQ * 100;
ULARGE_INTEGER start_counter,end_counter;
WORD frameCount = 1;
BOOL bRet;
HWND hWndMainEmulationFrame = 0;
HWND hWndDebuggerMdiClient = 0;	

	CCommandArgArray *pArgs = NULL;
	CParseCommandArg::ParseCommandLine(lpCmdLine, &pArgs);

	if (QueryPerformanceFrequency((PLARGE_INTEGER)&m_systemfrequency)==0)
	{
		G::InitFail(0,0,TEXT("The system does not support a high performance timer."));
		return FALSE;
	}
	else
	{
		if (m_systemfrequency.QuadPart < 100)
		{
			G::InitFail(0,0,TEXT("The system does not support a high performance timer."));
			return FALSE;
		}
	}

	if (!hPrevInstance) 
	{
		// Perform instance Initialisation:
		if (S_OK != InitApplication(hInstance)) 
		{
			return (FALSE);
		}
	}

	// Perform application Initialisation:
	if (S_OK != InitInstance(nCmdShow, pArgs)) 
	{
		return (FALSE);
	}

	if (pArgs)
	{
		delete pArgs;
		pArgs = NULL;
	}

	hWndMainEmulationFrame = appWindow.GetHwnd();

    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);
    SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_StartupToggleKeys, 0);
    SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &m_StartupFilterKeys, 0);

	if (G::IsDwmApiOk() && m_bDisableDwmFullscreen)
	{
		if (SUCCEEDED(G::s_pFnDwmIsCompositionEnabled(&m_oldDwm)))
			m_bGotOldDwm = true;
	}

#if _WIN32_WINNT >= 0x400
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,  ::LowLevelKeyboardProc, GetModuleHandle(NULL), 0 );
#else
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD,  ::KeyboardProc, GetModuleHandle(NULL), 0 );
#endif
	AllowAccessibilityShortcutKeys(false);

	if (dx.pSecondarySoundBuffer)
	{
		dx.RestoreSoundBuffers();
		SoundHalt();
		dx.pSecondarySoundBuffer->SetCurrentPosition(0);
	}

	m_bRunning=1;
	m_bPaused=0;
    //-------------------------------------------------------------------------
    //                          The Message Pump
    //-------------------------------------------------------------------------

	frequency.QuadPart = m_framefrequency.QuadPart;
	QueryPerformanceCounter((PLARGE_INTEGER)&last_counter);
	QueryPerformanceCounter((PLARGE_INTEGER)&end_counter);
	start_counter.QuadPart = end_counter.QuadPart - frequency.QuadPart * (ULONGLONG)FRAMEUPDATEFREQ;
	m_bInitDone = true;
	while (true)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			bRet = GetMessage(&msg, NULL, 0, 0 );
			if (bRet==-1 || bRet==0)
				goto finish;

			if (appWindow.m_pMDIDebugger != NULL)
			{
				HWND defwindow = GetActiveWindow();
				if (defwindow == NULL || 
					(
						defwindow != appWindow.GetHwnd() 
						&& defwindow != appWindow.m_pMDIDebugger->GetHwnd() 
						&& defwindow != appWindow.m_pMDIDebugger->m_debugCpuC64.GetHwnd()
						&& defwindow != appWindow.m_pMDIDebugger->m_debugCpuDisk.GetHwnd()
					))
				{
					defwindow = appWindow.m_pMDIDebugger->GetHwnd();
				}
				hWndDebuggerMdiClient = appWindow.m_pMDIDebugger->Get_MDIClientWindow();
				if (
					(!appWindow.m_pMDIDebugger->IsWinDlgModelessBreakpointVicRaster() || !IsDialogMessage(appWindow.m_pMDIDebugger->m_pdlgModelessBreakpointVicRaster->GetHwnd(), &msg))
					&& (!IsWindow(hWndDebuggerMdiClient) || !TranslateMDISysAccel(hWndDebuggerMdiClient, &msg)) 
					&& !TranslateAccelerator(defwindow, app.m_hAccelTable, &msg))
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg);
				}
				continue;
			}
			if (!TranslateAccelerator(appWindow.GetHwnd(), app.m_hAccelTable, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}
		}
		frequency.QuadPart = m_framefrequency.QuadPart;

		if (m_bActive && m_bReady && m_bRunning && !m_bPaused)
		{
			SoundResume();
			frameCount--;
			if (frameCount==0)
			{
				frameCount = FRAMEUPDATEFREQ;
				QueryPerformanceCounter((PLARGE_INTEGER)&end_counter);
				emulationSpeed =(ULONG) (((ULONGLONG)100) * (frequency.QuadPart * (ULONGLONG)FRAMEUPDATEFREQ) / (end_counter.QuadPart - start_counter.QuadPart));
				appWindow.UpdateWindowTitle(m_szTitle, emulationSpeed);
				QueryPerformanceCounter((PLARGE_INTEGER)&start_counter);
				c64.PreventClockOverflow();
			}
			QueryPerformanceCounter((PLARGE_INTEGER)&new_counter);				
			tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
			if ((LONGLONG)tSlice.QuadPart <= 0) // prevent timer overflow. A large negative "tSlice" would cause a long delay before "tSlice"s value would surpass the value of "frequency".
				last_counter.QuadPart = new_counter.QuadPart;
			if (m_bLimitSpeed)
			{
				if (m_bAudioClockSync && m_bSID_Emulation_Enable)
				{
					if (m_audioSpeedStatus == HCFG::AUDIO_QUICK)
					{
						last_counter.QuadPart = last_counter.QuadPart - frequency.QuadPart/4;
						tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
					}
					else if (m_audioSpeedStatus == HCFG::AUDIO_SLOW)
					{
						last_counter.QuadPart = last_counter.QuadPart + frequency.QuadPart/4;
						tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
					}
					m_audioSpeedStatus = HCFG::AUDIO_OK;
				}
				if ((LONGLONG)tSlice.QuadPart < (LONGLONG)frequency.QuadPart)
				{
					while ((LONGLONG)tSlice.QuadPart < (LONGLONG)frequency.QuadPart)
					{
						if (m_bCPUFriendly)
						{
							if (frequency.QuadPart > tSlice.QuadPart)
								Sleep(1);
						}
						QueryPerformanceCounter((PLARGE_INTEGER)&new_counter);				
						tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
					}				
				}
				else
				{
					//If we get here is means we took a very large time slice.
					if (tSlice.QuadPart > 4*frequency.QuadPart)
					{
						tSlice.QuadPart = 4*frequency.QuadPart;
					}
				}
				last_counter.QuadPart = new_counter.QuadPart-(tSlice.QuadPart-frequency.QuadPart);
			}
			else
			{
				//No limit speed
				if (m_bSkipFrames)
				{
					//No limit speed and skip frames.
					if ((LONGLONG)tSlice.QuadPart < (LONGLONG)frequency.QuadPart)
					{
						//We were quick so skip this frame.
						m_fskip = 1;
					}
					else
					{
						//We were slow so show this frame.
						last_counter.QuadPart = new_counter.QuadPart;
						m_fskip = -1;
					}
				}
				else
				{
					//No limit speed but want every frame
					last_counter.QuadPart = new_counter.QuadPart;
					m_fskip = -1;
				}
			}
			
			//
			//Execute one frame.
			if (!m_bDebug)
				c64.ExecuteFrame();
			else
				c64.ExecuteDebugFrame();

			bool bDrawThisFrame = (m_fskip < 0) || m_bDebug;

			if (bDrawThisFrame)
			{
				appWindow.emuWin.UpdateC64Window();

			}				
			//Handle frame skip
			if (m_bSkipFrames && bDrawThisFrame)
				m_fskip = 1;
			else if (m_fskip>=0)
				--m_fskip;
		}
		else
		{
			//Handle paused or non ready states in a CPU friendly manner.
			SoundHalt();
			if (m_bActive && !m_bReady && !m_bPaused && !m_bClosing)
			{
				hRet = E_FAIL;

				hRet = dx.m_pd3dDevice->TestCooperativeLevel();
				if (hRet == D3DERR_DEVICENOTRESET)
				{
					if (SUCCEEDED(dx.Reset()))
					{
						appWindow.SetColours();
						m_bReady = true;
					}
					else
					{
						if (SUCCEEDED(hRet = appWindow.ResetDirect3D()))
							m_bReady = true;
					}					
				}
				else if (hRet == D3DERR_DEVICELOST)
				{
					G::WaitMessageTimeout(100);
				}
				else if (hRet == D3D_OK)
				{
					m_bReady = true;
				}
			}
			else
			{
				if (m_bPaused && m_bWindowed && !m_bClosing)
				{
					appWindow.UpdateWindowTitle(m_szTitle, -1);
				}
				if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
					WaitMessage();
			}
		}
	}
finish:
	AllowAccessibilityShortcutKeys(true);
	if (m_hKeyboardHook)
	{
		UnhookWindowsHookEx(m_hKeyboardHook );
		m_hKeyboardHook = NULL;
	}
	return (int)(msg.wParam);

   lpCmdLine; // This will prevent 'unused formal parameter' warnings
}

HRESULT CApp::RegisterKeyPressWindow(HINSTANCE hInstance)
{
	WNDCLASS  wc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=sizeof(LPVOID *);
	wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hIcon=NULL;
	wc.hInstance=hInstance;
	wc.lpfnWndProc=::GetKeyPressWindowProc;
	wc.lpszClassName=TEXT("GetKeyPressClass");//GetStringRes(IDS_WINCLASS_KEYPRESS);
	wc.lpszMenuName=NULL;
	wc.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	if (RegisterClass(&wc)==0)
		return E_FAIL;
	return S_OK;
}

HRESULT CApp::InitApplication(HINSTANCE hInstance)
{
HRESULT hr;
	m_hInstance=hInstance;

	hr = CAppWindow::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register main window class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = CEmuWindow::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register emulation window class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = RegisterKeyPressWindow(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register GetKeyPressClass class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = CMDIDebuggerFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register CMDIDebuggerFrame class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CToolItemAddress::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register RegisterClass class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register CDisassemblyFrame class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register CDisassemblyChild class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyEditChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register CDisassemblyEditChild class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyReg::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register CDisassemblyReg class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WPanel::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register WPanel class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WpcBreakpoint::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Failed to register WpcBreakpoint class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	return S_OK;
}

LPTSTR CApp::GetAppTitle()
{
	return m_szTitle; 
}

LPTSTR CApp::GetAppName()
{
	return m_szAppName; 
}

LPTSTR CApp::GetMonitorTitle()
{
	return m_szMonitorTitle;
}

VS_FIXEDFILEINFO *CApp::GetVersionInfo()
{
	return &m_Vinfo;
}

HRESULT CApp::InitInstance(int nCmdShow, const CCommandArgArray *pArgs)
{
int w,h;
BOOL br;
HRESULT hr;
DWORD lr;
HWND hWndMain;
TCHAR t[60];
CConfig *thisCfg = static_cast<CConfig *>(this);
CAppStatus *thisAppStatus = static_cast<CAppStatus *>(this);
IC64Event *thisC64Event = static_cast<IC64Event *>(this);
TCHAR drive[_MAX_DRIVE];
TCHAR dir[_MAX_DIR];
TCHAR fname[_MAX_FNAME];
TCHAR ext[_MAX_EXT];

	ClearError();
	lr = GetModuleFileName(NULL, m_szAppFullPath, _countof(m_szAppFullPath));
	if (lr==0) 
	{
		G::ShowLastError(NULL);
		return E_FAIL;
	}

	//Save application directory path name
	if (_tcscpy_s(m_szAppDirectory, _countof(m_szAppDirectory), m_szAppFullPath) != 0)
		m_szAppDirectory[0] = 0;

	if (_tsplitpath_s(m_szAppDirectory, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext))==0)
	{
		m_szAppDirectory[0] = 0;
		if (_tmakepath_s(m_szAppDirectory, _countof(m_szAppDirectory), drive, dir, 0, 0)!=0)
			m_szAppDirectory[0] = 0;
	}
	else
		m_szAppDirectory[0] = 0;

	m_szAppConfigPath[0] = 0;
	if (_tmakepath_s(m_szAppConfigPath, _countof(m_szAppConfigPath), 0, m_szAppDirectory, TEXT("hoxs64"), TEXT("ini"))!=0)
		m_szAppConfigPath[0]=0;

	//Make a window title based on the version information.
	m_szTitle[0]='\0';
	m_szMonitorTitle[0]='\0';
	LoadString(m_hInstance, IDS_APP_TITLE, m_szTitle, MAX_STR_LEN);
	LoadString(m_hInstance, IDS_MONITOR_TITLE, m_szMonitorTitle, MAX_STR_LEN);

	if (SUCCEEDED(G::GetVersion_Res(m_szAppFullPath, &m_Vinfo)))
	{

#if defined(SERVICERELEASE) && (SERVICERELEASE > 0)
		_sntprintf_s(t, _countof(t), _TRUNCATE, TEXT("    V %d.%d.%d.%d SR %d")
			, (int)(m_Vinfo.dwProductVersionMS>>16 & 0xFF)
			, (int)(m_Vinfo.dwProductVersionMS & 0xFF)
			, (int)(m_Vinfo.dwProductVersionLS>>16 & 0xFF)
			, (int)(m_Vinfo.dwProductVersionLS & 0xFF)
			, (int)SERVICERELEASE);
#else
		_sntprintf_s(t, _countof(t), _TRUNCATE, TEXT("    V %d.%d.%d.%d")
			, (int)(m_Vinfo.dwProductVersionMS>>16 & 0xFF)
			, (int)(m_Vinfo.dwProductVersionMS & 0xFF)
			, (int)(m_Vinfo.dwProductVersionLS>>16 & 0xFF)
			, (int)(m_Vinfo.dwProductVersionLS & 0xFF));
#endif
		_tcsncat_s(m_szTitle, _countof(m_szTitle), t, _TRUNCATE);

	}

	//Set up some late bound OS DLL calls
	G::InitLateBindLibraryCalls();

	if (CDX9::CheckDXVersion9() < 0x0900)
		return (E_FAIL);

	//Initialise common control library.
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_LISTVIEW_CLASSES|ICC_USEREX_CLASSES;
	br = ::InitCommonControlsEx(&icex);
	if (br == FALSE)
	{
		MessageBox(0L, TEXT("InitCommonControlsEx() failed."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	//Load the users settings.
	hr = mainCfg.LoadCurrentSetting();
	if (FAILED(hr))
	{
		mainCfg.LoadDefaultSetting();
	}
	//Apply the users settings.
	ApplyConfig(mainCfg);


	m_hAccelTable = LoadAccelerators (m_hInstance, m_szAppName);

	hr = appWindow.Init(&dx, this, this, this, &c64);
	if (FAILED(hr))
	{
		return SetError(appWindow);
	}

	appWindow.GetRequiredMainWindowSize(m_borderSize, m_bShowFloppyLed, m_bDoubleSizedWindow, &w, &h);

	POINT winpos = {0,0};
	hr = mainCfg.LoadWindowSetting(winpos);
	if (FAILED(hr))
	{
		RECT rcWorkArea;
		br = SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rcWorkArea, 0);
		if (!br) 
		{
			rcWorkArea.left = rcWorkArea.top = 0;
			rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
			rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
		}
		winpos.x = max(0, (rcWorkArea.right - rcWorkArea.left - w) / 2);
		winpos.y = max(0, (rcWorkArea.bottom - rcWorkArea.top - h) / 2);
	}

	hWndMain = appWindow.Create(m_hInstance, NULL, m_szTitle, winpos.x, winpos.y, w, h, NULL);
	if (!hWndMain)
	{
		MessageBox(0L, TEXT("Unable to create the application window."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	G::EnsureWindowPosition(hWndMain);

	//Initialise directx
	hr = dx.Init(thisCfg, thisAppStatus, VIC6569::vic_color_array);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Initialisation failed for direct 3D."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	hr = dx.OpenDirectInput(m_hInstance, hWndMain);
	if (FAILED(hr))
	{
		MessageBox(0L, TEXT("Initialisation failed for direct input."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = dx.OpenDirectSound(hWndMain, m_fps);
	if (FAILED(hr))
	{
		m_bSoundOK = FALSE;
		MessageBox(hWndMain, TEXT("Direct Sound has failed to initialise with the primary sound driver. Sound will be unavailable."), m_szAppName, MB_ICONWARNING);
	}
	else
	{
		m_bSoundOK = TRUE;
	}

	//Initialise the c64 emulation.
	if (S_OK != c64.Init(thisCfg, thisAppStatus, thisC64Event, &dx, m_szAppDirectory)) 
	{
		c64.DisplayError(0, m_szAppName);
		return E_FAIL;
	}

	//Make the application start in windowed mode.
	m_bWindowed = TRUE;
	hr = appWindow.SetWindowedMode(m_bWindowed, m_bDoubleSizedWindow, m_bUseBlitStretch);
	if (S_OK != hr) 
	{
		appWindow.DisplayError(appWindow.GetHwnd(), m_szAppName);
		return E_FAIL;
	}

	//Initialise joysticks
	dx.InitJoys(hWndMain, m_joy1config, m_joy2config);

	//Reset the C64
	c64.Reset(0);

	//Process command line arguments.
	if (pArgs)
	{
		int directoryIndex = -1; //default to a LOAD"*",8,1 for disk files
		TCHAR *fileName = NULL;
		CommandArg *caAutoLoad = CParseCommandArg::FindOption(pArgs, TEXT("-AutoLoad"));
		CommandArg *caQuickLoad = CParseCommandArg::FindOption(pArgs, TEXT("-QuickLoad"));
		CommandArg *caAlignD64Tracks= CParseCommandArg::FindOption(pArgs, TEXT("-AlignD64Tracks"));
		if (caAutoLoad)
		{
			if (caAutoLoad->ParamCount >= 1)
			{
				if (caAutoLoad->ParamCount >= 2)
				{
					directoryIndex = _tstoi(&caAutoLoad->pParam[1][0]);
					if (directoryIndex < 0)
						directoryIndex = -1;
					else if	(directoryIndex > C64Directory::D64MAXDIRECTORYITEMCOUNT)
						directoryIndex = C64Directory::D64MAXDIRECTORYITEMCOUNT;
				}
				c64.AutoLoad(caAutoLoad->pParam[0], directoryIndex, true, NULL, caQuickLoad != NULL, caAlignD64Tracks != NULL);
			}
		}
	}

	m_bReady = TRUE;
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain);
	return (S_OK);
}

void CApp::FreeDirectX(){
	m_bReady = FALSE;
	dx.CloseDirectSound();
	dx.CloseDirectInput();
	dx.CleanupD3D();
}

void CApp::RestoreDefaultSettings()
{
	mainCfg.LoadDefaultSetting();
	ApplyConfig(mainCfg);
}

void CApp::RestoreUserSettings()
{
	mainCfg.LoadCurrentSetting();
	ApplyConfig(mainCfg);
}

void CApp::SaveCurrentSetting()
{
	mainCfg.SaveCurrentSetting();
}

void CApp::ToggleMaxSpeed()
{
	CConfig tCfg = *(static_cast<CConfig *>(this));
	tCfg.ToggleMaxSpeed();
	ApplyConfig(tCfg);
}

void CApp::InsertTape(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
BOOL b;

	initfilename[0] = 0;
	
	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a raw tape file"),
		initfilename,
		MAX_PATH,
		TEXT("Raw tapes (*.tap)\0*.tap\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = GetOpenFileName((LPOPENFILENAME)&of);
	if (b)
	{
		HRESULT hr = c64.LoadTAPFile(initfilename);
		if (FAILED(hr))
			c64.DisplayError(hWnd, TEXT("Insert Tape"));
	}
}

void CApp::AutoLoad(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
BOOL b;
HRESULT hr;
CPRGBrowse prgBrowse;

	initfilename[0]=0;

	hr = prgBrowse.Init(c64.ram.mCharGen);
	if (FAILED(hr))
		return ; 

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a C64 program file"),
		initfilename,
		MAX_PATH,
		TEXT("C64 file (*.fdi;*.d64;*.g64;*.t64;*.tap;*.p00;*.prg;*.sid)\0*.fdi;*.d64;*.g64;*.t64;*.tap;*.p00;*.prg;*.sid\0\0"),
		NULL,
		0);
	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, CPRGBrowse::ALL);
	if (b)
	{
		HRESULT hr = c64.AutoLoad(initfilename, prgBrowse.SelectedDirectoryIndex, false, prgBrowse.SelectedC64FileName, prgBrowse.SelectedQuickLoadDiskFile, prgBrowse.SelectedAlignD64Tracks);
		if (FAILED(hr))
			c64.DisplayError(hWnd, TEXT("Auto Load"));
		else if (hr == APPERR_BAD_CRC)
			c64.DisplayError(hWnd, TEXT("Auto Load"));

		//The call to c64.AutoLoad can enable disk emulation
		mainCfg.m_bD1541_Emulation_Enable = m_bD1541_Emulation_Enable;
	}
}

void CApp::LoadC64Image(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
BOOL b;
bit16 start, loadSize;
HRESULT hr;

	initfilename[0] = 0;

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a C64 image file"),
		initfilename,
		MAX_PATH,
		TEXT("Image file (*.p00;*.prg)\0*.p00;*.prg\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = GetOpenFileName((LPOPENFILENAME)&of);
	if (b)
	{
		hr = c64.LoadImageFile(initfilename, &start, &loadSize);
		if (SUCCEEDED(hr))
			ShowMessage(hWnd, MB_ICONINFORMATION, TEXT("Load Image") , TEXT("START=%d"), (int)start);
		else
			c64.DisplayError(hWnd, TEXT("Load Image"));

	}
}

void CApp::LoadT64(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH+1];
BOOL b;
bit16 start, loadSize;
HRESULT hr;
CPRGBrowse prgBrowse;
int i;
	
	initfilename[0] = 0;

	hr = prgBrowse.Init(c64.ram.mCharGen);
	if (FAILED(hr))
		return ; 

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a T64 file"),
		initfilename,
		MAX_PATH,
		TEXT("Image file (*.t64)\0*.t64\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, CPRGBrowse::T64);
	if (b)
	{
		i = (prgBrowse.SelectedDirectoryIndex < 0) ? 0 : prgBrowse.SelectedDirectoryIndex;
		hr = c64.LoadT64ImageFile(initfilename, i, &start, &loadSize);
		if (SUCCEEDED(hr))
			ShowMessage(hWnd, MB_ICONINFORMATION, TEXT("Load T64"), TEXT("START=%d"), (int)start);
		else
			c64.DisplayError(hWnd, TEXT("Load T64"));

	}
}


void CApp::InsertDiskImage(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH+1];
BOOL b;
HRESULT hr;
CPRGBrowse prgBrowse;

	initfilename[0] = 0;

	TCHAR title[] = TEXT("Insert Disk Image");

	hr = prgBrowse.Init(c64.ram.mCharGen);
	if (FAILED(hr))
		return ; 

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a disk image file"),
		initfilename,
		MAX_PATH,
		TEXT("Disk image file (*.d64;*.g64;*.fdi)\0*.d64;*.g64;*.fdi\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	prgBrowse.DisableQuickLoad = true;
	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, (CPRGBrowse::filetype)(CPRGBrowse::D64 | CPRGBrowse::G64 | CPRGBrowse::FDI) );
	if (b)
	{
		SetBusy(true);
		hr = c64.InsertDiskImageFile(initfilename, prgBrowse.SelectedAlignD64Tracks);
		SetBusy(false);
		if (FAILED(hr))
			c64.DisplayError(hWnd, title);
		else if (hr == APPERR_BAD_CRC)
			c64.DisplayError(hWnd, title);
			
	}
}

void CApp::SaveD64Image(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
TCHAR filename[MAX_PATH];
BOOL b;
HRESULT hr;
CDiagFileSaveD64 childDialog;

	TCHAR title[] = TEXT("Save D64 Image");
	if (c64.diskdrive.m_diskLoaded==0)
	{
		MessageBox(hWnd, TEXT("No disk has been inserted"), title, MB_OK | MB_ICONEXCLAMATION); 
		return;
	}

	hr = childDialog.Init(c64.diskdrive.m_d64TrackCount);
	if (FAILED(hr))
		return ; 

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	else
		of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWnd;
	of.lpstrFilter = TEXT("Disk image file (*.d64)\0*.d64\0All files (*.*)\0*.*\0\0");
	of.nFilterIndex =1;
	initfilename[0]=0;
	of.lpstrDefExt=TEXT("D64");
	of.lpstrFile = initfilename;
	of.nMaxFile = MAX_PATH;
	of.lpstrFileTitle = filename;
	of.nMaxFileTitle = MAX_PATH;
	of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	of.lpstrTitle = TEXT("Save a D64 disk image file");
	b = childDialog.Open(m_hInstance, (LPOPENFILENAME)&of);
	if (b)
	{
		SetBusy(true);
		hr = c64.SaveD64ToFile(initfilename, childDialog.SelectedNumberOfTracks);
		SetBusy(false);
		if (SUCCEEDED(hr))
		{
			if (hr==0)
			{
				MessageBox(hWnd,
				TEXT("Disk saved."), 
				title, 
				MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(hWnd,
				TEXT("A save was made but this disk cannot be properly saved to a D64 due a non-standard disk format. FDI can handle all disk formats. Please re-save as an FDI file."), 
				title, 
				MB_OK | MB_ICONWARNING);
			}
		}
		else
		{
			c64.DisplayError(hWnd, title);
		}
	}
}

void CApp::SaveFDIImage(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH+1];
TCHAR filename[MAX_PATH+1];
BOOL b;
HRESULT hr;

	TCHAR title[] = TEXT("Save FDI Image");
	if (c64.diskdrive.m_diskLoaded==0)
	{
		MessageBox(hWnd, TEXT("No disk has been inserted"), title, MB_OK | MB_ICONEXCLAMATION); 
		return;
	}

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	else
		of.lStructSize = sizeof(OPENFILENAME);
	of.hwndOwner = hWnd;
	of.lpstrFilter = TEXT("Disk image file (*.fdi)\0*.fdi\0All files (*.*)\0*.*\0\0");
	of.nFilterIndex = 1;
	initfilename[0] = 0;
	of.lpstrDefExt=TEXT("FDI");
	of.lpstrFile = initfilename;
	of.nMaxFile = MAX_PATH-1;
	of.lpstrFileTitle = filename;
	of.nMaxFileTitle = MAX_PATH-1;
	of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	of.lpstrTitle = TEXT("Save an FDI disk image file");
	b = GetSaveFileName((LPOPENFILENAME)&of);
	if (b)
	{
		SetBusy(true);
		hr =  c64.SaveFDIToFile(initfilename);
		SetBusy(false);
		if (SUCCEEDED(hr))
		{
			MessageBox(hWnd,
			TEXT("Disk saved."), 
			title, 
			MB_OK | MB_ICONINFORMATION);
		}
		else
		{
			c64.DisplayError(hWnd, title);
		}
	}
}

void CApp::SoundHalt()
{
	if (m_bSoundOK)
		dx.SoundHalt(c64.sid.m_last_dxsample);
	c64.sid.SoundHalt(c64.sid.m_last_dxsample);
}

void CApp::SoundResume()
{
	if ((m_bSID_Emulation_Enable && m_bMaxSpeed==0) && m_bSoundOK && m_bRunning)
		dx.SoundResume();
}

void CApp::GetUserConfig(CConfig& cfg)
{
	cfg = this->mainCfg;
}

void CApp::SetUserConfig(const CConfig& newcfg)
{
	this->mainCfg = newcfg;
}

void CApp::ApplyConfig(const CConfig& newcfg)
{	
HWND hWnd;
CConfig &cfg= *(static_cast<CConfig *>(this));
CAppStatus &status= *(static_cast<CAppStatus *>(this));
HRESULT hRet;
bool bNeedDisplayReset = false;
bool bNeedSoundFilterInit = false;

	hWnd = appWindow.GetHwnd();

	c64.cia1.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);
	c64.cia2.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);

	if (newcfg.m_bSID_Emulation_Enable!=FALSE && cfg.m_bSID_Emulation_Enable==FALSE)
		c64.sid.CurrentClock = c64.cpu.CurrentClock;

	if (newcfg.m_bD1541_Emulation_Enable!=FALSE && cfg.m_bD1541_Emulation_Enable==FALSE)
		c64.diskdrive.CurrentPALClock = c64.cpu.CurrentClock;

	if (newcfg.m_bUseBlitStretch != cfg.m_bUseBlitStretch 
		|| newcfg.m_bDoubleSizedWindow != cfg.m_bDoubleSizedWindow 
		|| newcfg.m_blitFilter != cfg.m_blitFilter
		|| newcfg.m_borderSize != cfg.m_borderSize
		|| newcfg.m_bShowFloppyLed != cfg.m_bShowFloppyLed
		|| newcfg.m_syncMode != cfg.m_syncMode
		|| newcfg.m_fullscreenAdapterNumber != cfg.m_fullscreenAdapterNumber)
	{
		bNeedDisplayReset = true;
	}

	if (newcfg.m_fps != cfg.m_fps
		|| newcfg.m_bSIDResampleMode != cfg.m_bSIDResampleMode)
	{
		bNeedSoundFilterInit = true;
	}

	cfg = newcfg;

	if (bNeedDisplayReset)
	{
		if (hWnd)
		{
			if (IsWindow(hWnd))
				hRet = appWindow.SetWindowedMode(m_bWindowed, newcfg.m_bDoubleSizedWindow, newcfg.m_bUseBlitStretch);
		}
	}

	ULARGE_INTEGER frequency;
	frequency.QuadPart = 1;
	if (QueryPerformanceFrequency((PLARGE_INTEGER)&frequency)!=0)
	{
		if (cfg.m_fps == HCFG::EMUFPS_50)
			status.m_framefrequency.QuadPart = frequency.QuadPart / PAL50FRAMESPERSECOND;
		else 
			status.m_framefrequency.QuadPart = (ULONGLONG)(((double)frequency.QuadPart / ((double)PALCLOCKSPERSECOND/((double)PALLINESPERFRAME * (double)PALCLOCKSPERLINE))));

	}
	CopyMemory(&c64.cia1.c64KeyMap[0], &newcfg.m_KeyMap[0], sizeof(c64.cia1.c64KeyMap));

	status.m_fskip = -1;
	
	m_bUseKeymap = FALSE;

	dx.InitJoys(hWnd, cfg.m_joy1config, cfg.m_joy2config);

	if (status.m_bSoundOK)
	{
		c64.sid.UpdateSoundBufferLockSize(cfg.m_fps);
		if (bNeedSoundFilterInit)
			c64.sid.InitResamplingFilters(cfg.m_fps);
	}

	if (hWnd)
		appWindow.UpdateWindowTitle(m_szTitle, -1);
}

void CApp::SetBusy(bool bBusy)
{
	if (bBusy)
	{
		this->m_bBusy = true;
		UpdateWindow(appWindow.GetHwnd());
		if (hOldCursor == NULL)
			hOldCursor = SetCursor(hCursorBusy);
	}
	else
	{
		this->m_bBusy = false;
		if (hOldCursor)
			SetCursor(hOldCursor);
		hOldCursor = NULL;
	}
}

void CApp::BreakExecuteCpu64()
{
HWND hWnd;
	hWnd = appWindow.GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = TRUE;
		m_bRunning = FALSE;
		PostMessage(hWnd, WM_MONITOR_BREAK_CPU64, 0, 0);
	}
}

void CApp::BreakExecuteCpuDisk()
{
HWND hWnd;
	hWnd = appWindow.GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = TRUE;
		m_bRunning = FALSE;
		PostMessage(hWnd, WM_MONITOR_BREAK_CPUDISK, 0, 0);
	}
}

void CApp::BreakVicRasterCompare()
{
HWND hWnd;
	hWnd = appWindow.GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = TRUE;
		m_bRunning = FALSE;
		PostMessage(hWnd, WM_MONITOR_BREAK_VICRASTER, 0, 0);
	}
}

void CApp::BreakpointChanged()
{
	BreakpointChangedEventArgs e;
	this->EsBreakpointChanged.Raise(NULL, e);	
}

void CApp::DiskMotorLed(bool bOn)
{
	m_bDiskLedMotor = bOn;
}

void CApp::DiskDriveLed(bool bOn)
{
	m_bDiskLedDrive = bOn;
}

void CApp::DiskWriteLed(bool bOn)
{
	m_bDiskLedWrite = bOn;
}

void CApp::ShowErrorBox(LPCTSTR title, LPCTSTR message)
{
	HWND hWnd = appWindow.GetHwnd();
	MessageBox(hWnd, message, title, MB_OK | MB_ICONEXCLAMATION);
}

void CApp::Resume()
{
HWND hWnd;
EventArgs e;
	m_bDebug = FALSE;
	m_bBreak = FALSE;
	m_bRunning = TRUE;
	m_bPaused = FALSE;
	if (!m_bClosing)
	{
		hWnd = appWindow.GetHwnd();
		if (hWnd)
			SetForegroundWindow(hWnd);
		SoundResume();
		EsResume.Raise(this, e);
	}
}

void CApp::Trace()
{
HWND hWnd;
EventArgs e;
	hWnd = appWindow.GetHwnd();
	m_bDebug = TRUE;
	m_bBreak = FALSE;
	m_bRunning = TRUE;
	m_bPaused = FALSE;
	appWindow.UpdateWindowTitle(m_szTitle, -1);
	if (hWnd)
		SetForegroundWindow(hWnd);
	SoundResume();
	EsTrace.Raise(this, e);
}

void CApp::TraceFrame()
{
EventArgs e;
	m_fskip = -1;
	c64.ExecuteDebugFrame();
	EsTraceFrame.Raise(this, e);
}

void CApp::ExecuteC64Clock()
{
EventArgs e;
	m_fskip = -1;
	c64.ExecuteC64Clock();
	EsExecuteC64Clock.Raise(this, e);
}

void CApp::ExecuteC64Instruction()
{
EventArgs e;
	m_fskip = -1;
	c64.ExecuteC64Instruction();
	EsExecuteC64Instruction.Raise(this, e);
}

void CApp::ExecuteDiskClock()
{
EventArgs e;
	m_fskip = -1;
	c64.ExecuteDiskClock();
	EsExecuteDiskClock.Raise(this, e);
}

void CApp::ExecuteDiskInstruction()
{
EventArgs e;
	m_fskip = -1;
	c64.ExecuteDiskInstruction();
	EsExecuteDiskInstruction.Raise(this, e);
}

void CApp::UpdateApplication()
{
EventArgs e;
	appWindow.emuWin.UpdateC64Window();
	EsUpdateApplication.Raise(this, e);
}

bool CApp::IsRunning()
{
	return m_bRunning!=FALSE;
}

void CApp::SoundOff()
{
	SoundHalt();
}

void CApp::SoundOn()
{
	SoundResume();
}

void CApp::ShowCpuDisassembly(int cpuid, DBGSYM::SetDisassemblyAddress::DisassemblyPCUpdateMode pcmode, bit16 address)
{
HWND hWndMdiDebugger = NULL;
	if (!appWindow.m_pMDIDebugger)
		hWndMdiDebugger = appWindow.ShowDevelopment();
	if (!appWindow.m_pMDIDebugger)
		return;

	if (cpuid == CPUID_MAIN)
		appWindow.m_pMDIDebugger->ShowDebugCpuC64(pcmode, address);
	else if (cpuid == CPUID_DISK)
		appWindow.m_pMDIDebugger->ShowDebugCpuDisk(pcmode, address);
}

void CApp::TogglePause()
{
	MessageBeep(MB_ICONASTERISK);
	m_bPaused = !m_bPaused;
	appWindow.UpdateWindowTitle(m_szTitle, -1);
}

void CApp::ToggleSoundMute()
{
	m_bSoundMute= !m_bSoundMute;
	if (m_bSoundMute)
		c64.sid.MasterVolume = 0.0;
	else
		c64.sid.MasterVolume = 1.0;
	appWindow.UpdateWindowTitle(m_szTitle, -1);
}

HWND CApp::ShowDevelopment()
{
EventArgs e;
HWND hWnd = NULL;

	hWnd = appWindow.ShowDevelopment();
	if (hWnd)
	{
		EsShowDevelopment.Raise(this, e);
	}
	return hWnd;
}


#if _WIN32_WINNT >= 0x400
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return app.LowLevelKeyboardProc(nCode, wParam, lParam); 
}
LRESULT CApp::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || nCode != HC_ACTION )  // do not process message 
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam); 
 
    bool bEatKeystroke = false;
    switch (wParam) 
    {
        case WM_KEYDOWN:  
        case WM_KEYUP:    
        {
			bool bWinKey;
			KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;
			bWinKey = ((p->vkCode == VK_LWIN) || (p->vkCode == VK_RWIN));
            bEatKeystroke = (!m_bWindowed && m_bActive && bWinKey);
            break;
        }
    }
 
    if( bEatKeystroke )
        return 1;
    else
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam );
}
#else
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return app.KeyboardProc(nCode, wParam, lParam); 
}
LRESULT CApp::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || nCode != HC_ACTION )  // do not process message 
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam); 
 
    bool bEatKeystroke = false;
    switch (wParam) 
    {
        case WM_KEYDOWN:  
        case WM_KEYUP:    
        {
			bool bWinKey;
			bWinKey = ((wParam == VK_LWIN) || (wParam == VK_RWIN));
            bEatKeystroke = (!m_bWindowed && m_bActive && bWinKey);
            break;
        }
    }
 
    if( bEatKeystroke )
        return 1;
    else
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam );
}
#endif


void CApp::AllowAccessibilityShortcutKeys( bool bAllowKeys )
{
    if( bAllowKeys )
    {
        // Restore StickyKeys/etc to original state and enable Windows key      
        STICKYKEYS sk = m_StartupStickyKeys;
        TOGGLEKEYS tk = m_StartupToggleKeys;
        FILTERKEYS fk = m_StartupFilterKeys;
        
        SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);
        SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_StartupToggleKeys, 0);
        SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &m_StartupFilterKeys, 0);
    }
    else
    {
        // Disable StickyKeys/etc shortcuts but if the accessibility feature is on, 
        // then leave the settings alone as its probably being usefully used
        STICKYKEYS skOff = m_StartupStickyKeys;
        if( (skOff.dwFlags & SKF_STICKYKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &skOff, 0);
        }
 
        TOGGLEKEYS tkOff = m_StartupToggleKeys;
        if( (tkOff.dwFlags & TKF_TOGGLEKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            tkOff.dwFlags &= ~TKF_HOTKEYACTIVE;
            tkOff.dwFlags &= ~TKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETTOGGLEKEYS, sizeof(TOGGLEKEYS), &tkOff, 0);
        }
 
        FILTERKEYS fkOff = m_StartupFilterKeys;
        if( (fkOff.dwFlags & FKF_FILTERKEYSON) == 0 )
        {
            // Disable the hotkey and the confirmation
            fkOff.dwFlags &= ~FKF_HOTKEYACTIVE;
            fkOff.dwFlags &= ~FKF_CONFIRMHOTKEY;
 
            SystemParametersInfo(SPI_SETFILTERKEYS, sizeof(FILTERKEYS), &fkOff, 0);
        }
    }
}
