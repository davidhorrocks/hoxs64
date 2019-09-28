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
#include <iostream>
#include <malloc.h>
#include <memory.h>
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
#include "besttextwidth.h"
#include "errormsg.h"
#include "hexconv.h"
#include "C64.h"
#include "user_message.h"
#include "prgbrowse.h"
#include "diagcolour.h"
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
#include "wpccli.h"
#include "mdichildcli.h"
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

	m_bStartFullScreen = false;
	m_bShowSpeed = true;
	m_bLimitSpeed = true;
	m_bSkipFrames = true;
	
	m_bActive     = false;
	m_bReady      = false;
	m_bRunning    = false;
	m_bWindowed   = true;
	m_bDebug      = false;
	m_bPaused      = false;
	m_bMaxSpeed   = false;
	m_syncModeFullscreen = HCFG::FSSM_LINE;
	m_bAutoload = false;
	m_bWindowSizing = false;
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
short captionUpdateCounter = 0;
BOOL bRet;

	m_hInstance = hInstance;
	if (QueryPerformanceFrequency((PLARGE_INTEGER)&m_systemfrequency)==0)
	{
		G::InitFail(0,0,TEXT("The system does not support a high performance timer."));
		return ShellExitInitFailed;
	}
	else
	{
		if (m_systemfrequency.QuadPart < 60)
		{
			G::InitFail(0, 0, TEXT("The system does not support a high performance timer."));
			return ShellExitInitFailed;
		}
	}
	frequency.QuadPart = m_framefrequency.QuadPart;
	QueryPerformanceCounter((PLARGE_INTEGER)&last_counter);
	QueryPerformanceCounter((PLARGE_INTEGER)&end_counter);
	start_counter.QuadPart = end_counter.QuadPart - frequency.QuadPart * (ULONGLONG)FRAMEUPDATEFREQ;

	if (FAILED(RegisterWindowClasses(hInstance)))
	{
		return ShellExitInitFailed;
	}

	if (FAILED(InitInstance(nCmdShow, lpCmdLine)))
	{
		return ShellExitInitFailed;
	}
	

#if _WIN32_WINNT >= 0x400
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL,  ::LowLevelKeyboardProc, GetModuleHandle(NULL), 0 );
#else
	m_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD,  ::KeyboardProc, GetModuleHandle(NULL), 0 );
#endif
	AllowAccessibilityShortcutKeys(false);
	
	m_bReady = true;
	m_bRunning=1;
	m_bPaused=0;
	m_bInitDone = true;
	bool bExecuteFrameDone = false;
	int framesSkipped = 0;
    //-------------------------------------------------------------------------
    //                          The Message Pump
    //-------------------------------------------------------------------------
	while (true)
	{
		bExecuteFrameDone = false;
msgloop:
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			bRet = GetMessage(&msg, NULL, 0, 0 );
			if (bRet==-1 || bRet==0)
			{
				goto finish;
			}
			if (!m_pWinAppWindow->m_pMDIDebugger.expired())
			{
				HWND hWndDebug = 0;
				HWND hWndDebugCpuC64 = 0;
				HWND hWndDebugCpuDisk = 0;
				HWND hWndDlgVicRaster = 0;
				shared_ptr<CMDIDebuggerFrame> pWinDebugger = m_pWinAppWindow->m_pMDIDebugger.lock();				
				hWndDebug =  pWinDebugger->GetHwnd();
				shared_ptr<CDisassemblyFrame> pWinDebugCpuC64 = pWinDebugger->m_pWinDebugCpuC64.lock();
				if (pWinDebugCpuC64)
				{
					hWndDebugCpuC64 = pWinDebugCpuC64->GetHwnd();
				}

				shared_ptr<CDisassemblyFrame> pWinDebugCpuDisk = pWinDebugger->m_pWinDebugCpuDisk.lock();
				if (pWinDebugCpuDisk)
				{
					hWndDebugCpuDisk = pWinDebugCpuDisk->GetHwnd();
				}

				Sp_CDiagBreakpointVicRaster m_pdlgModelessBreakpointVicRaster = pWinDebugger->m_pdlgModelessBreakpointVicRaster.lock();
				if (m_pdlgModelessBreakpointVicRaster!=0)
				{
					hWndDlgVicRaster = m_pdlgModelessBreakpointVicRaster->GetHwnd();
				}

				HWND hWndDefaultAccelerator = GetActiveWindow();
				if (hWndDefaultAccelerator == 0 || 
					(
						hWndDefaultAccelerator != m_pWinAppWindow->GetHwnd() 
						&& hWndDefaultAccelerator != hWndDebug 
						&& hWndDefaultAccelerator != hWndDebugCpuC64
						&& hWndDefaultAccelerator != hWndDebugCpuDisk
					))
				{
					hWndDefaultAccelerator =pWinDebugger->GetHwnd();
				}
				HWND hWndDebuggerMdiClient = pWinDebugger->Get_MDIClientWindow();					
				if (
					//(!IsWindow(hWndDlgVicRaster) || !IsDialogMessage(hWndDlgVicRaster, &msg)) &&
                    //(!IsWindow(hWndDebugCpuC64) || !IsDialogMessage(hWndDebugCpuC64, &msg)) &&                    
                    //(!IsWindow(hWndDebugCpuDisk) || !IsDialogMessage(hWndDebugCpuDisk, &msg)) &&
                    //(!IsWindow(hWndDebuggerMdiClient) || !IsDialogMessage(hWndDebuggerMdiClient, &msg)) &&
					(!IsWindow(hWndDebuggerMdiClient) || !TranslateMDISysAccel(hWndDebuggerMdiClient, &msg)) &&
					(!TranslateAccelerator(hWndDefaultAccelerator, app.m_hAccelTable, &msg)))
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg);
				}
				continue;
			}
			if (!TranslateAccelerator(m_pWinAppWindow->GetHwnd(), app.m_hAccelTable, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}
		}
		frequency.QuadPart = m_framefrequency.QuadPart;

		if (((m_bActive && m_bReady) || G::IsHideWindow) && m_bRunning && !m_bPaused)
		{
			if (!bExecuteFrameDone)
			{
				SoundResume();
				if (captionUpdateCounter<=0)
				{
					captionUpdateCounter = FRAMEUPDATEFREQ;
					QueryPerformanceCounter((PLARGE_INTEGER)&end_counter);
					emulationSpeed =(ULONG) (((ULONGLONG)100) * (frequency.QuadPart * (ULONGLONG)FRAMEUPDATEFREQ) / (end_counter.QuadPart - start_counter.QuadPart));
					m_pWinAppWindow->UpdateWindowTitle(m_szTitle, emulationSpeed);
					QueryPerformanceCounter((PLARGE_INTEGER)&start_counter);
					c64.PreventClockOverflow();
				}
				QueryPerformanceCounter((PLARGE_INTEGER)&new_counter);				
				tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
				if ((LONGLONG)tSlice.QuadPart <= 0)
				{
					last_counter.QuadPart = new_counter.QuadPart;
					tSlice.QuadPart = 0ULL;
				}

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
							last_counter.QuadPart = last_counter.QuadPart + frequency.QuadPart/16;
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
								{
									if (!G::WaitMessageTimeout(1))
									{
										goto msgloop;
									}
								}
							}
							else
							{
								if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
								{
									goto msgloop;
								}
							}

							QueryPerformanceCounter((PLARGE_INTEGER)&new_counter);				
							tSlice.QuadPart = new_counter.QuadPart - last_counter.QuadPart;
						}
					}
					else
					{
						//If we get here is means we took a very large time slice.
						if ((LONGLONG)tSlice.QuadPart >= (LONGLONG)(2)*(LONGLONG)frequency.QuadPart)
						{
							if ((LONGLONG)tSlice.QuadPart > (LONGLONG)(4)*(LONGLONG)frequency.QuadPart)
							{
								tSlice.QuadPart = (LONGLONG)(4)*(LONGLONG)frequency.QuadPart;
							}
							
							if (framesSkipped <= 2)
							{
								this->m_fskip = 0;
							}
						}
					}

					last_counter.QuadPart = new_counter.QuadPart - (tSlice.QuadPart-frequency.QuadPart);
				}
				else
				{
					//No limit speed
					if (m_bSkipFrames)
					{
						//No limit speed and skip frames.						
						if ((LONGLONG)tSlice.QuadPart < (LONGLONG)frequency.QuadPart)
						{
							//We were quick so see if we can skip frames.
							if (framesSkipped < frequency.QuadPart / tSlice.QuadPart)
							{
								m_fskip = 0;
							}
						}
						else
						{
							//We were slow so we might as well show this frame.
							last_counter.QuadPart = new_counter.QuadPart;
							m_fskip = -1;
						}
					}
					else
					{
						if ((LONGLONG)tSlice.QuadPart >= (LONGLONG)(2)*(LONGLONG)frequency.QuadPart)
						{
							if (framesSkipped <= 2)
							{
								this->m_fskip = 0;
							}
						}
					}

					last_counter.QuadPart = new_counter.QuadPart;
				}
			
				//Execute one frame.
				bExecuteFrameDone = true;
				int frameResult;
				if (!m_bDebug)
				{
					frameResult = c64.ExecuteFrame();
				}
				else
				{
					BreakpointResult breakpointResult;
					frameResult = 0;
					if (c64.ExecuteDebugFrame(CPUID_MAIN, breakpointResult))
					{
						if (breakpointResult.IsApplicationExitCode)
						{
							frameResult = breakpointResult.ExitCode;
						}
					}
				}

				if (frameResult)
				{
					this->c64.WriteOnceExitCode(ShellExitCycleLimit);
					this->m_bRunning = false;
					this->m_bClosing = true;
					this->m_pWinAppWindow->CloseWindow();
				}
			}

			captionUpdateCounter--;
			bool bDrawThisFrame = ((m_fskip < 0) || m_bIsDebugCart || m_bDebug);
			if (bDrawThisFrame)
			{
				hRet = m_pWinAppWindow->m_pWinEmuWin->UpdateC64Window();
				if (!m_bIsDebugCart)
				{
					if (SUCCEEDED(hRet))
					{
						dx.Present(D3DPRESENT_DONOTWAIT);
					}
				}

				framesSkipped = 0;
			}
			else
			{
				framesSkipped++;
#if defined(DEBUG) && DEBUG_AUDIO_CLOCK_SYNC != 0
				OutputDebugString(TEXT("fskip\n"));
#endif
			}

			//Handle frame skip
			if (m_bSkipFrames && framesSkipped == 0)
			{
				m_fskip = 0;
			}
			else if (m_fskip >= 0)
			{
				--m_fskip;
			}
		}
		else
		{
			//Handle paused or non ready states in a CPU friendly manner.
			SoundHalt();
			if (!m_bClosing && m_bActive && !m_bReady && !G::IsHideWindow)
			{
				hRet = E_FAIL;
				if (dx.m_pd3dDevice != NULL)
				{
					hRet = dx.m_pd3dDevice->TestCooperativeLevel();
					if (hRet == D3DERR_DEVICENOTRESET)
					{
						hRet = dx.Reset();
						if (SUCCEEDED(hRet))
						{
							m_pWinAppWindow->SetColours();
							m_bReady = true;
						}
						else
						{
							if (SUCCEEDED(hRet = m_pWinAppWindow->ResetDirect3D()))
							{
								m_bReady = true;
							}
							else
							{
								G::WaitMessageTimeout(1000);
							}
						}					
					}
					else if (hRet == D3DERR_DEVICELOST)
					{
						G::WaitMessageTimeout(1000);
					}
					else if (hRet == D3D_OK)
					{
						m_bReady = true;
					}
				}
				else
				{
					G::WaitMessageTimeout(1000);
				}
			}
			else
			{
				if (m_bReady)
				{
					UpdateEmulationDisplay();
				}

				if (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
				{
					WaitMessage();
				}
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

	HRESULT pngresult = c64.SavePng(NULL);
	if (c64.Get_EnableDebugCart())
	{		
		return c64.Get_ExitCode();
	}
	else
	{
		if (FAILED(pngresult))
		{
			return ShellExitPngFailed;
		}
		else
		{
			return 0;
		}
	}
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

HRESULT CApp::RegisterVicColorWindow(HINSTANCE hInstance)
{
	WNDCLASS  wc;
	wc.cbClsExtra=0;
	wc.cbWndExtra=sizeof(LPVOID *);
	wc.hbrBackground=(HBRUSH)(COLOR_BTNFACE+1);
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.hIcon=NULL;
	wc.hInstance=hInstance;
	wc.lpfnWndProc=::VicColorWindowProc;
	wc.lpszClassName=TEXT("VicColorClass");//GetStringRes(IDS_WINCLASS_KEYPRESS);
	wc.lpszMenuName=NULL;
	wc.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	if (RegisterClass(&wc)==0)
		return E_FAIL;
	return S_OK;
}

HRESULT CApp::RegisterWindowClasses(HINSTANCE hInstance)
{
HRESULT hr;
	hr = CAppWindow::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register main window class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = CEmuWindow::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register emulation window class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = RegisterKeyPressWindow(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register GetKeyPressClass class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = RegisterVicColorWindow(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register VicColorClass class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}

	hr = CMDIDebuggerFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CMDIDebuggerFrame class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WpcCli::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WpcCli class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CMDIChildCli::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CMDIChildCli class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CToolItemAddress::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register RegisterClass class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyFrame class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyChild class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyEditChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyEditChild class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyReg::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyReg class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WPanel::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WPanel class."), m_szAppName, MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WpcBreakpoint::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WpcBreakpoint class."), m_szAppName, MB_ICONEXCLAMATION);
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

HRESULT CApp::InitInstance(int nCmdShow, LPTSTR lpCmdLine)
{
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
int minWidth;
int minHeight;
int mainWinWidth;
int mainWinHeight;
int emuWinWidth;
int emuWinHeight;
int mainWinFixedWidth;
int mainWinFixedHeight;
int emuWinFixedWidth;
int emuWinFixedHeight;
RECT rcMain;

	ClearError();
	lr = GetModuleFileName(NULL, m_szAppFullPath, _countof(m_szAppFullPath));
	if (lr==0) 
	{
		return E_FAIL;
	}
	
	//Process command line arguments.
	CParseCommandArg cmdArgs(lpCmdLine);
	CommandArg *caAutoLoad = cmdArgs.FindOption(TEXT("-AutoLoad"));
	CommandArg *caQuickLoad = cmdArgs.FindOption(TEXT("-QuickLoad"));
	CommandArg *caAlignD64Tracks= cmdArgs.FindOption(TEXT("-AlignD64Tracks"));
	CommandArg *caStartFullscreen = cmdArgs.FindOption(TEXT("-Fullscreen"));
	CommandArg *caDebugCart = cmdArgs.FindOption(TEXT("-debugcart"));
	CommandArg *caLimitCycles = cmdArgs.FindOption(TEXT("-limitcycles"));
	CommandArg *caExitScreenShot = cmdArgs.FindOption(TEXT("-exitscreenshot"));
	CommandArg *caCiaNew = cmdArgs.FindOption(TEXT("-cia-new"));
	CommandArg *caCiaOld = cmdArgs.FindOption(TEXT("-cia-old"));
	CommandArg *caHltReset = cmdArgs.FindOption(TEXT("-hlt-reset"));
	CommandArg *caHltExit = cmdArgs.FindOption(TEXT("-hlt-exit"));
	CommandArg *caMountDisk = cmdArgs.FindOption(TEXT("-mountdisk"));
	CommandArg *caNoMessageBox = cmdArgs.FindOption(TEXT("-nomessagebox"));
	CommandArg *caRunFast = cmdArgs.FindOption(TEXT("-runfast"));
	CommandArg *caWindowHide = cmdArgs.FindOption(TEXT("-window-hide"));
	CommandArg *caDefaultSettings = cmdArgs.FindOption(TEXT("-defaultsettings"));
	CommandArg *caSoundOff= cmdArgs.FindOption(TEXT("-nosound"));
	CommandArg *caSystemWarm = cmdArgs.FindOption(TEXT("-system-warm"));
	CommandArg *caSystemCold = cmdArgs.FindOption(TEXT("-system-cold"));

	if (caNoMessageBox || caWindowHide)
	{
		if (caWindowHide)
		{
			nCmdShow = SW_HIDE;
			G::IsHideWindow = true;
		}

		G::IsHideMessageBox = true;
	}
	else if (caStartFullscreen)
	{
		this->m_bStartFullScreen = true;
	}

	//Save application directory path name
	if (_tcscpy_s(m_szAppDirectory, _countof(m_szAppDirectory), m_szAppFullPath) != 0)
	{
		m_szAppDirectory[0] = 0;
	}

	if (_tsplitpath_s(m_szAppDirectory, drive, _countof(drive), dir, _countof(dir), fname, _countof(fname), ext, _countof(ext))==0)
	{
		m_szAppDirectory[0] = 0;
		if (_tmakepath_s(m_szAppDirectory, _countof(m_szAppDirectory), drive, dir, 0, 0)!=0)
		{
			m_szAppDirectory[0] = 0;
		}
	}
	else
	{
		m_szAppDirectory[0] = 0;
	}

	m_szAppConfigPath[0] = 0;
	if (_tmakepath_s(m_szAppConfigPath, _countof(m_szAppConfigPath), 0, m_szAppDirectory, TEXT("hoxs64"), TEXT("ini"))!=0)
	{
		m_szAppConfigPath[0]=0;
	}

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
	G::InitRandomSeed();
	if (CDX9::CheckDXVersion9() < 0x0900)
	{
		return (E_FAIL);
	}

	//Initialise common control library.
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_LISTVIEW_CLASSES|ICC_USEREX_CLASSES;
	br = ::InitCommonControlsEx(&icex);
	if (br == FALSE)
	{
		G::DebugMessageBox(0L, TEXT("InitCommonControlsEx() failed."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	bool alignD64 = false;
	bool systemWarm = false;
	bool warmForTestbench = false;
	//Load the users settings.
	if (caDefaultSettings)
	{
		warmForTestbench = true;
		mainCfg.LoadDefaultSetting();		
	}
	else
	{
		hr = mainCfg.LoadCurrentSetting();
		if (FAILED(hr))
		{
			mainCfg.LoadDefaultSetting();
		}
	}

	if (caCiaNew != NULL)
	{
		mainCfg.SetCiaNewOldMode(true);
	}
	else if (caCiaOld != NULL)
	{
		mainCfg.SetCiaNewOldMode(false);
	}

	if (caAlignD64Tracks)
	{
		alignD64 = true;
	}

	if (caRunFast != NULL)
	{
		mainCfg.SetRunFast();
	}

	//Apply the users settings.
	ApplyConfig(mainCfg);
	m_hAccelTable = LoadAccelerators (m_hInstance, m_szAppName);
	try
	{
		m_pWinAppWindow = shared_ptr<CAppWindow>(new CAppWindow(&dx, this, this, &c64));
	}
	catch (...)
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the application window."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	m_pWinAppWindow->m_pWinEmuWin->SetNotify(this);
	m_pWinAppWindow->GetMinimumWindowedSize(m_borderSize, m_bShowFloppyLed, &minWidth, &minHeight);
	POINT winpos = {0,0};
	bool bWindowedCustomSize = false;
	m_pWinAppWindow->GetRequiredMainWindowSize(m_borderSize, m_bShowFloppyLed, m_bDoubleSizedWindow, &mainWinFixedWidth, &mainWinFixedHeight);
	rcMain.left = 0;
	rcMain.top = 0;
	rcMain.right = mainWinFixedWidth;
	rcMain.bottom = mainWinFixedHeight;
	m_pWinAppWindow->CalcEmuWindowSize(rcMain, &emuWinFixedWidth, &emuWinFixedHeight);
	mainWinWidth = mainWinFixedWidth;
	mainWinHeight = mainWinFixedHeight;

	//Look for saved x,y position and custom window size.
	hr = mainCfg.LoadWindowSetting(winpos, bWindowedCustomSize, mainWinWidth, mainWinHeight);
	if (FAILED(hr))
	{
		bWindowedCustomSize = false;
		mainWinWidth = mainWinFixedWidth;
		mainWinHeight = mainWinFixedHeight;
		winpos = GetCenteredPos(mainWinWidth, mainWinHeight);
	}
	else
	{
		if (bWindowedCustomSize)
		{
			mainWinWidth = max(minWidth, mainWinWidth);
			mainWinHeight = max(minHeight, mainWinHeight);
		}
	}

	if (!mainCfg.m_bUseBlitStretch && bWindowedCustomSize)
	{
		//Attempt to use CPU blit if a custom window size matches the set size for the current state of "Double Sized Window".
		RECT rcMain = {0, 0, mainWinWidth, mainWinHeight};
		m_pWinAppWindow->CalcEmuWindowSize(rcMain, &emuWinWidth, &emuWinHeight);
		if (emuWinFixedWidth == emuWinWidth && emuWinFixedHeight == emuWinHeight)
		{
			bWindowedCustomSize = false;
		}
	}

	//Apply custom window size flag.
	m_bWindowedCustomSize = bWindowedCustomSize;
	hWndMain = m_pWinAppWindow->Create(m_hInstance, NULL, m_szTitle, winpos.x, winpos.y, mainWinWidth, mainWinHeight, NULL);
	if (!hWndMain)
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the application window."), m_szAppName, MB_ICONWARNING);
		return E_FAIL;
	}

	m_bWindowed = true;
	if (!G::IsHideWindow)
	{
		G::EnsureWindowPosition(hWndMain);
	}

	//Initialise directx
	hr = dx.Init(thisAppStatus);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Initialisation failed for direct 3D."), m_szAppName, MB_ICONWARNING);
		DestroyWindow(hWndMain);
		return E_FAIL;
	}
	
	hr = dx.OpenDirectInput(m_hInstance, hWndMain);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Initialisation failed for direct input."), m_szAppName, MB_ICONEXCLAMATION);
		DestroyWindow(hWndMain);
		return hr;
	}
	
	m_bSoundOK = false;
	if (caSoundOff == NULL)
	{
		hr = dx.OpenDirectSound(hWndMain, m_fps);
		if (FAILED(hr))
		{
			G::DebugMessageBox(hWndMain, TEXT("Direct Sound has failed to initialise with the primary sound driver. Sound will be unavailable."), m_szAppName, MB_ICONWARNING);
		}
		else
		{
			m_bSoundOK = true;
		}
	}

	//Initialise the c64 emulation.
	if (S_OK != c64.Init(thisAppStatus, thisC64Event, &dx, m_szAppDirectory)) 
	{
		c64.DisplayError(0, m_szAppName);
		DestroyWindow(hWndMain);
		return E_FAIL;
	}

	hr = m_pWinAppWindow->SetWindowedMode(!m_bStartFullScreen, m_bDoubleSizedWindow, m_bWindowedCustomSize, mainWinWidth, mainWinHeight, m_bUseBlitStretch);
	if (FAILED(hr))
	{
		m_pWinAppWindow->DisplayError(m_pWinAppWindow->GetHwnd(), m_szAppName);
		DestroyWindow(hWndMain);
		return E_FAIL;
	}

	//Initialise joysticks
	dx.InitJoys(hWndMain, m_joy1config, m_joy2config);
	this->m_bIsDebugCart = caDebugCart != NULL;
	c64.Set_EnableDebugCart(this->m_bIsDebugCart);
	if (caLimitCycles != NULL)
	{
		if (caLimitCycles->ParamCount >= 1)
		{
			errno = 0;
			__int64 limitCycles = _tstoi64(caLimitCycles->pParam[0]);
			if (errno != ERANGE && errno != EINVAL)
			{
				c64.Set_LimitCycles((ICLK)(ICLKS)limitCycles);
			}
		}
	}

	if (caExitScreenShot != NULL)
	{
		if (caExitScreenShot->ParamCount >= 1)
		{			
			c64.Set_ExitScreenShot(caExitScreenShot->pParam[0]);
		}
		else
		{
			c64.Set_ExitScreenShot(TEXT(""));
		}
	}

	if (caHltReset != NULL)
	{
		c64.cpu.bHardResetOnHltInstruction = true;
	}

	if (caHltExit != NULL)
	{
		c64.cpu.bExitOnHltInstruction = true;
	}	

	if (caMountDisk != NULL)
	{
		if (caMountDisk->ParamCount >= 1)
		{			
			c64.InsertDiskImageFile(caMountDisk->pParam[0], alignD64, true);
		}
	}

	if (caSystemWarm)
	{
		systemWarm = true;
	}
	else if (caSystemCold)
	{
		systemWarm = false;
	}
	else if (warmForTestbench)
	{
		systemWarm = true;
	}

	//Reset the C64
	c64.Reset(0, true);
	if (systemWarm)
	{
		c64.Warm();
	}

	int directoryIndex = -1; //default to a LOAD"*",8,1 for disk files
	TCHAR *fileName = NULL;	
	if (caAutoLoad)
	{
		if (caAutoLoad->ParamCount >= 1)
		{
			bit8* pC64filename = NULL;
			bit8 c64filename[C64DISKFILENAMELENGTH];
			bool indexPrgsOnly = false;
			if (caAutoLoad->ParamCount >= 2)
			{
				TCHAR lead = caAutoLoad->pParam[1][0];
				if (lead == TEXT(':') && lstrlen(caAutoLoad->pParam[1]) > 1)
				{
					if (SUCCEEDED(C64::CopyC64FilenameFromString(&caAutoLoad->pParam[1][1], c64filename, C64DISKFILENAMELENGTH)))
					{
						pC64filename = c64filename;
					}
				}
				else
				{
					TCHAR *p = &caAutoLoad->pParam[1][0];
					if (*p == TEXT('#'))
					{
						indexPrgsOnly = true;
						p++;
					}
					else if (*p == TEXT('@'))
					{
						indexPrgsOnly = false;
						p++;
					}

					errno = 0;
					directoryIndex = _tstoi(p);
					if (directoryIndex < 0 || errno == EINVAL  || errno == ERANGE)
					{
						directoryIndex = -1;
					}
					else if	(directoryIndex > C64Directory::D64MAXDIRECTORYITEMCOUNT)
					{
						directoryIndex = C64Directory::D64MAXDIRECTORYITEMCOUNT;
					}
				}
			}
			
			c64.AutoLoad(caAutoLoad->pParam[0], directoryIndex, indexPrgsOnly, pC64filename, caQuickLoad != NULL, alignD64);
		}
	}

    SystemParametersInfo(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &m_StartupStickyKeys, 0);
    SystemParametersInfo(SPI_GETTOGGLEKEYS, sizeof(TOGGLEKEYS), &m_StartupToggleKeys, 0);
    SystemParametersInfo(SPI_GETFILTERKEYS, sizeof(FILTERKEYS), &m_StartupFilterKeys, 0);
	if (dx.pSecondarySoundBuffer)
	{
		dx.RestoreSoundBuffers();
		SoundHalt();
		dx.pSecondarySoundBuffer->SetCurrentPosition(0);
	}

	ShowWindow(hWndMain, nCmdShow);
	if (!G::IsHideWindow)
	{
		UpdateWindow(hWndMain);
	}
	return (S_OK);
}

POINT CApp::GetCenteredPos(int w, int h)
{
	POINT r = {0,0};
	RECT rcWorkArea;
	BOOL br = SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rcWorkArea, 0);
	if (!br) 
	{
		rcWorkArea.left = rcWorkArea.top = 0;
		rcWorkArea.right = GetSystemMetrics(SM_CXSCREEN);
		rcWorkArea.bottom = GetSystemMetrics(SM_CYSCREEN);
	}
	r.x = max(0, (rcWorkArea.right - rcWorkArea.left - w) / 2);
	r.y = max(0, (rcWorkArea.bottom - rcWorkArea.top - h) / 2);
	return r;
}

void CApp::FreeDirectX(){
	m_bReady = false;
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
	if (!m_bMaxSpeed)
	{
		SaveSpeedSettings();
		m_bSkipFrames = true;
		m_bLimitSpeed = false;
		m_bMaxSpeed = true;
	}
	else
	{
		RestoreSpeedSettings();
		m_bMaxSpeed = false;
	}
}

void CApp::LoadCrtFile(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
BOOL b;

	initfilename[0] = 0;
	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a cartridge CRT file"),
		initfilename,
		MAX_PATH,
		TEXT("Cartridge CRT (*.crt)\0*.crt\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = GetOpenFileName((LPOPENFILENAME)&of);
	if (b)
	{
		HRESULT hr = c64.LoadCrtFile(initfilename);
		if (SUCCEEDED(hr))
		{
			UpdateApplication();
			Reset();
		}
		else
		{
			c64.DisplayError(hWnd, TEXT("Attach Cartridge"));
		}
	}
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
		{
			c64.DisplayError(hWnd, TEXT("Insert Tape"));
		}
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
	{
		return ; 
	}

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a C64 program file"),
		initfilename,
		MAX_PATH,
		TEXT("C64 file (*.fdi;*p64;*.d64;*.g64;*.t64;*.tap;*.p00;*.prg;*.sid;*.crt;*.64s)\0*.fdi;*p64;*.d64;*.g64;*.t64;*.tap;*.p00;*.prg;*.sid;*.crt;*.64s\0\0"),
		NULL,
		0);
	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, CPRGBrowse::FileTypeFlag::ALL);
	if (b)
	{
		HRESULT hr = c64.AutoLoad(initfilename, prgBrowse.SelectedDirectoryIndex, false, prgBrowse.SelectedC64FileName, prgBrowse.SelectedQuickLoadDiskFile, prgBrowse.SelectedAlignD64Tracks);
		if (hr != S_OK)
		{
			c64.DisplayError(hWnd, TEXT("Auto Load"));
		}
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
		{
			MemoryChanged();
			ShowMessage(hWnd, MB_ICONINFORMATION, TEXT("Load Image") , TEXT("START=%d"), (int)start);
		}
		else
		{
			c64.DisplayError(hWnd, TEXT("Load Image"));
		}
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
	TCHAR title[] = TEXT("Load T64");
	hr = prgBrowse.Init(c64.ram.mCharGen);
	if (FAILED(hr))
	{
		prgBrowse.DisplayError(hWnd, title);
		return ; 
	}

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a T64 file"),
		initfilename,
		MAX_PATH,
		TEXT("Image file (*.t64)\0*.t64\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, CPRGBrowse::FileTypeFlag::T64);
	if (b)
	{
		i = (prgBrowse.SelectedDirectoryIndex < 0) ? 0 : prgBrowse.SelectedDirectoryIndex;
		hr = c64.LoadT64ImageFile(initfilename, i, &start, &loadSize);
		if (SUCCEEDED(hr))
		{
			MemoryChanged();
			ShowMessage(hWnd, MB_ICONINFORMATION, TEXT("Load T64"), TEXT("START=%d"), (int)start);
		}
		else
		{
			c64.DisplayError(hWnd, title);
		}
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
	{		
		prgBrowse.DisplayError(hWnd, title);
		return ; 
	}

	G::InitOfn(of, 
		hWnd, 
		TEXT("Choose a disk image file"),
		initfilename,
		MAX_PATH,
		TEXT("Disk image file (*.d64;*.g64;*.p64;*.fdi)\0*.d64;*.g64;*.p64;*.fdi\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	prgBrowse.DisableQuickLoad = true;
	b = prgBrowse.Open(m_hInstance, (LPOPENFILENAME)&of, (CPRGBrowse::FileTypeFlag::filetype)(CPRGBrowse::FileTypeFlag::D64 | CPRGBrowse::FileTypeFlag::G64 | CPRGBrowse::FileTypeFlag::FDI | CPRGBrowse::FileTypeFlag::P64) );
	if (b)
	{
		SetBusy(true);
		hr = c64.InsertDiskImageFile(initfilename, prgBrowse.SelectedAlignD64Tracks, false);
		SetBusy(false);
		if (FAILED(hr))
		{
			c64.DisplayError(hWnd, title);
		}
		else if (hr == APPERR_BAD_CRC)
		{
			c64.DisplayError(hWnd, title);
		}			
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
		G::DebugMessageBox(hWnd, TEXT("No disk has been inserted"), title, MB_OK | MB_ICONEXCLAMATION); 
		return;
	}

	hr = childDialog.Init(c64.diskdrive.m_d64TrackCount);
	if (FAILED(hr))
	{
		return ; 
	}

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
	{
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	}
	else
	{
		of.lStructSize = sizeof(OPENFILENAME);
	}

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
				G::DebugMessageBox(hWnd,
				TEXT("Disk saved."), 
				title, 
				MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				G::DebugMessageBox(hWnd,
				TEXT("A save was made but this disk cannot be properly saved to a D64. Please re-save as a P64 or FDI file to ensure a safe save."), 
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
		G::DebugMessageBox(hWnd, TEXT("No disk has been inserted"), title, MB_OK | MB_ICONEXCLAMATION); 
		return;
	}

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
	{
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	}
	else
	{
		of.lStructSize = sizeof(OPENFILENAME);
	}

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
			G::DebugMessageBox(hWnd,
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

void CApp::SaveP64Image(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH+1];
TCHAR filename[MAX_PATH+1];
BOOL b;
HRESULT hr;

	TCHAR title[] = TEXT("Save P64 Image");
	if (c64.diskdrive.m_diskLoaded==0)
	{
		G::DebugMessageBox(hWnd, TEXT("No disk has been inserted"), title, MB_OK | MB_ICONEXCLAMATION); 
		return;
	}

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
	{
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	}
	else
	{
		of.lStructSize = sizeof(OPENFILENAME);
	}

	of.hwndOwner = hWnd;
	of.lpstrFilter = TEXT("Disk image file (*.p64)\0*.p64\0All files (*.*)\0*.*\0\0");
	of.nFilterIndex = 1;
	initfilename[0] = 0;
	of.lpstrDefExt=TEXT("P64");
	of.lpstrFile = initfilename;
	of.nMaxFile = MAX_PATH-1;
	of.lpstrFileTitle = filename;
	of.nMaxFileTitle = MAX_PATH-1;
	of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	of.lpstrTitle = TEXT("Save a P64 disk image file");
	b = GetSaveFileName((LPOPENFILENAME)&of);
	if (b)
	{
		SetBusy(true);
		hr =  c64.SaveP64ToFile(initfilename);
		SetBusy(false);
		if (SUCCEEDED(hr))
		{
			G::DebugMessageBox(hWnd,
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

void CApp::SaveC64State(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH+1];
TCHAR filename[MAX_PATH+1];
BOOL b;
HRESULT hr;

	TCHAR title[] = TEXT("Save State Image");

	ZeroMemory(&of, sizeof(of));
	if (G::IsEnhancedWinVer())
	{
		of.lStructSize = sizeof(OPENFILENAME_500EX);
	}
	else
	{
		of.lStructSize = sizeof(OPENFILENAME);
	}

	of.hwndOwner = hWnd;
	of.lpstrFilter = TEXT("State Image (*.64s)\0*.64s\0All files (*.*)\0*.*\0\0");
	of.nFilterIndex = 1;
	initfilename[0] = 0;
	of.lpstrDefExt=TEXT("64s");
	of.lpstrFile = initfilename;
	of.nMaxFile = MAX_PATH-1;
	of.lpstrFileTitle = filename;
	of.nMaxFileTitle = MAX_PATH-1;
	of.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	of.lpstrTitle = TEXT("Save a state image file");
	b = GetSaveFileName((LPOPENFILENAME)&of);
	if (b)
	{
		SetBusy(true);
		hr =  c64.SaveC64StateToFile(initfilename);
		SetBusy(false);
		if (FAILED(hr))
		{
			c64.DisplayError(hWnd, title);
		}
	}
}

void CApp::LoadC64State(HWND hWnd)
{
OPENFILENAME_500EX of;
TCHAR initfilename[MAX_PATH];
BOOL b;
HRESULT hr;

	TCHAR title[] = TEXT("Load State Image");
	initfilename[0] = 0;
	G::InitOfn(of, 
		hWnd, 
		title,
		initfilename,
		MAX_PATH,
		TEXT("State Image file (*.64s)\0*.64s\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	b = GetOpenFileName((LPOPENFILENAME)&of);
	if (b)
	{
		SetBusy(true);
		hr = c64.LoadC64StateFromFile(initfilename);
		SetBusy(false);
		if (SUCCEEDED(hr))
		{
			UpdateApplication();
			Reset();
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
	{
		dx.SoundHalt(c64.sid.m_last_dxsample);
	}

	c64.sid.SoundHalt(c64.sid.m_last_dxsample);
}

void CApp::SoundResume()
{
	if ((m_bSID_Emulation_Enable && m_bMaxSpeed==0) && m_bSoundOK && m_bRunning)
	{
		dx.SoundResume();
	}
}

void CApp::GetUserConfig(CConfig& cfg)
{
	cfg = this->mainCfg;
	UpdateConfigFromSid(cfg);
}

void CApp::UpdateUserConfigFromSid()
{
	UpdateConfigFromSid(this->mainCfg);
}

void CApp::UpdateConfigFromSid(CConfig& cfg)
{
	cfg.m_numberOfExtraSIDs = c64.sid.NumberOfExtraSidChips;
	cfg.m_Sid2Address = c64.sid.AddressOfSecondSID;
	cfg.m_Sid3Address = c64.sid.AddressOfThirdSID;
	cfg.m_Sid4Address = c64.sid.AddressOfFourthSID;
	cfg.m_Sid5Address = c64.sid.AddressOfFifthSID;
	cfg.m_Sid6Address = c64.sid.AddressOfSixthSID;
	cfg.m_Sid7Address = c64.sid.AddressOfSeventhSID;
	cfg.m_Sid8Address = c64.sid.AddressOfEighthSID;
}

void CApp::SetSidChipAddressMap(int numberOfExtraSidChips, bit16 addressOfSecondSID, bit16 addressOfThirdSID, bit16 addressOfFourthSID, bit16 addressOfFifthSID, bit16 addressOfSixthSID, bit16 addressOfSeventhSID, bit16 addressOfEighthSID)
{
	this->m_numberOfExtraSIDs = numberOfExtraSidChips;
	this->m_Sid2Address = addressOfSecondSID;
	this->m_Sid3Address = addressOfThirdSID;
	this->m_Sid4Address = addressOfFourthSID;
	this->m_Sid5Address = addressOfFifthSID;
	this->m_Sid6Address = addressOfSixthSID;
	this->m_Sid7Address = addressOfSeventhSID;
	this->m_Sid8Address = addressOfEighthSID;
	this->c64.sid.SetSidChipAddressMap(numberOfExtraSidChips, addressOfSecondSID, addressOfThirdSID, addressOfFourthSID, addressOfFifthSID, addressOfSixthSID, addressOfSeventhSID, addressOfEighthSID);
}

void CApp::ResetSidChipAddressMap()
{
	this->c64.sid.SetSidChipAddressMap(m_numberOfExtraSIDs, m_Sid2Address, m_Sid3Address, m_Sid4Address, m_Sid5Address, m_Sid6Address, m_Sid7Address, m_Sid8Address);
}

void CApp::SetUserConfig(const CConfig& newcfg)
{
	this->mainCfg = newcfg;
}

void CApp::ApplyConfig(const CConfig& newcfg)
{	
HWND hWnd = 0;
CConfig &cfg= *(static_cast<CConfig *>(this));
bool bNeedDisplayReset = false;
bool bNeedSoundFilterInit = false;
bool bPaletteChanged = false;
unsigned int i;

	if (m_pWinAppWindow!=0 && m_pWinAppWindow->GetHwnd() != 0 && IsWindow(m_pWinAppWindow->GetHwnd()))
	{
		hWnd = m_pWinAppWindow->GetHwnd();
	}

	c64.cia1.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);
	c64.cia2.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);

	if (newcfg.m_bSID_Emulation_Enable!=false && this->m_bSID_Emulation_Enable==false)
	{
		c64.sid.CurrentClock = c64.cpu.CurrentClock;
	}

	if (newcfg.m_bD1541_Emulation_Enable!=false && this->m_bD1541_Emulation_Enable==false)
	{
		c64.diskdrive.CurrentPALClock = c64.cpu.CurrentClock;
	}

	if (newcfg.m_bUseBlitStretch != this->m_bUseBlitStretch 
		|| newcfg.m_bWindowedCustomSize != this->m_bWindowedCustomSize 
		|| newcfg.m_bDoubleSizedWindow != this->m_bDoubleSizedWindow 
		|| newcfg.m_blitFilter != this->m_blitFilter
		|| newcfg.m_borderSize != this->m_borderSize
		|| newcfg.m_bShowFloppyLed != this->m_bShowFloppyLed
		|| newcfg.m_syncModeFullscreen != this->m_syncModeFullscreen
		|| newcfg.m_syncModeWindowed != this->m_syncModeWindowed
		|| newcfg.m_fullscreenAdapterNumber != this->m_fullscreenAdapterNumber)
	{
		bNeedDisplayReset = true;
	}

	for (i = 0; i < VicIIPalette::NumColours; i++)
	{
		if (newcfg.m_colour_palette[i] != this->m_colour_palette[i])
		{
			bPaletteChanged = true;
		}
	}

	if (newcfg.m_fps != this->m_fps	|| newcfg.m_bSIDResampleMode != this->m_bSIDResampleMode)
	{
		bNeedSoundFilterInit = true;
	}

	// Use the new config.
	cfg = newcfg;

	if (bPaletteChanged)
	{
		 if (m_pWinAppWindow)
		 {
			 m_pWinAppWindow->SetColours();
		 }
	}

	bool bWindowedCustomSize = newcfg.m_bWindowedCustomSize;
	if (bNeedDisplayReset)
	{
		if (hWnd)
		{
			int w = 0;
			int h = 0;
			if (m_bWindowed && bWindowedCustomSize)
			{
				bWindowedCustomSize = true;
				RECT rc;
				if (GetWindowRect(hWnd, &rc))
				{
					int minw;
					int minh;
					m_pWinAppWindow->GetMinimumWindowedSize(cfg.m_borderSize, cfg.m_bShowFloppyLed, &minw, &minh);
					w = max(minw, rc.right - rc.left);
					h = max(minh, rc.bottom - rc.top);
				}
			}

			m_pWinAppWindow->SetWindowedMode(m_bWindowed, newcfg.m_bDoubleSizedWindow, bWindowedCustomSize, w, h, newcfg.m_bUseBlitStretch);
		}
	}

	ULARGE_INTEGER frequency;
	frequency.QuadPart = 1;
	if (QueryPerformanceFrequency((PLARGE_INTEGER)&frequency)!=0)
	{
		if (this->m_fps == HCFG::EMUFPS_50)
		{
			this->m_framefrequency.QuadPart = frequency.QuadPart / PAL50FRAMESPERSECOND;
		}
		else 
		{
			this->m_framefrequency.QuadPart = (ULONGLONG)(((double)frequency.QuadPart / ((double)PALCLOCKSPERSECOND/((double)PAL_LINES_PER_FRAME * (double)PAL_CLOCKS_PER_LINE))));
		}

	}

	CopyMemory(&c64.cia1.c64KeyMap[0], &newcfg.m_KeyMap[0], sizeof(c64.cia1.c64KeyMap));
	this->m_fskip = -1;	
	m_bUseKeymap = false;
	dx.InitJoys(hWnd, this->m_joy1config, this->m_joy2config);
	this->SetSidChipAddressMap(newcfg.m_numberOfExtraSIDs, newcfg.m_Sid2Address, newcfg.m_Sid3Address, newcfg.m_Sid4Address, newcfg.m_Sid5Address, newcfg.m_Sid6Address, newcfg.m_Sid7Address, newcfg.m_Sid8Address);
	if (this->m_bSoundOK)
	{
		c64.sid.UpdateSoundBufferLockSize(this->m_fps);
		if (bNeedSoundFilterInit)
		{
			c64.sid.InitResamplingFilters(this->m_fps);
		}
	}

	if (hWnd)
	{
		m_pWinAppWindow->UpdateWindowTitle(m_szTitle, -1);
	}
}

void CApp::SetBusy(bool bBusy)
{
	if (bBusy)
	{
		this->m_bBusy = true;
		UpdateWindow(m_pWinAppWindow->GetHwnd());
		if (hOldCursor == NULL)
		{
			hOldCursor = SetCursor(hCursorBusy);
		}
	}
	else
	{
		this->m_bBusy = false;
		if (hOldCursor)
		{
			SetCursor(hOldCursor);
		}

		hOldCursor = NULL;
	}
}

void CApp::BreakExecuteCpu64()
{
HWND hWnd;
	hWnd = m_pWinAppWindow->GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = true;
		m_bRunning = false;
		PostMessage(hWnd, WM_MONITOR_BREAK_CPU64, 0, 0);
	}
}

void CApp::BreakExecuteCpuDisk()
{
HWND hWnd;
	hWnd = m_pWinAppWindow->GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = true;
		m_bRunning = false;
		PostMessage(hWnd, WM_MONITOR_BREAK_CPUDISK, 0, 0);
	}
}

void CApp::BreakVicRasterCompare()
{
HWND hWnd;
	hWnd = m_pWinAppWindow->GetHwnd();
	if (hWnd != 0)
	{
		m_bBreak = true;
		m_bRunning = false;
		PostMessage(hWnd, WM_MONITOR_BREAK_VICRASTER, 0, 0);
	}
}

void CApp::BreakpointChanged()
{
	this->UpdateEmulationDisplay();
	BreakpointChangedEventArgs e;
	this->EsBreakpointChanged.Raise(NULL, e);	
}

void CApp::Reset()
{
EventArgs e;
	this->EsTraceFrame.Raise(NULL, e);
}

void CApp::MemoryChanged()
{
EventArgs e;
	this->EsMemoryChanged.Raise(NULL, e);
}

void CApp::RadixChanged(DBGSYM::MonitorOption::Radix radix)
{
RadixChangedEventArgs e(radix);
	this->EsRadixChanged.Raise(NULL, e);
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
	HWND hWnd = m_pWinAppWindow->GetHwnd();
	G::DebugMessageBox(hWnd, message, title, MB_OK | MB_ICONEXCLAMATION);
}

void CApp::WriteExitCode(int exitCode)
{
	this->c64.Set_ExitCode(exitCode);
	this->m_bClosing = true;
	this->m_bRunning = false;
	this->m_pWinAppWindow->CloseWindow();
}

void CApp::Resume()
{
HWND hWnd;
EventArgs e;
	c64.GetMon()->QuitCommands();
	m_bDebug = false;
	m_bBreak = false;
	m_bRunning = true;
	m_bPaused = false;
	if (!m_bClosing)
	{
		hWnd = m_pWinAppWindow->GetHwnd();
		if (hWnd)
		{
			SetForegroundWindow(hWnd);
		}

		SoundResume();
		EsResume.Raise(this, e);
	}
}

void CApp::ClearAllTemporaryBreakpoints()
{
	this->c64.ClearAllTemporaryBreakpoints();
}

void CApp::TraceStepOver(int cpuId)
{
const int MaxFramesToTryBeforeEnteringNormalTrace = 5;
EventArgs e;
BreakpointResult breakpointResult;
bool breakPoint = false;
int i;

	c64.GetMon()->QuitCommands();
	c64.ClearAllTemporaryBreakpoints();
	if (cpuId == CPUID_MAIN)
	{
		c64.cpu.SetStepOverBreakpoint();
	}
	else
	{
		c64.diskdrive.cpu.SetStepOverBreakpoint();
	}

	// Try a few frames to help cut down on screen flicker
	for (i = 0; i < MaxFramesToTryBeforeEnteringNormalTrace; i++)
	{
		m_fskip = -1;
		breakPoint = c64.ExecuteDebugFrame(cpuId, breakpointResult);
		if (breakPoint)
		{
			break;
		}
	}

	if (breakPoint)
	{
		EsTrace.Raise(this, e);
	}
	else
	{
		// If a breakpoint has not occurred after trying a few frames then 
		// enable user interaction of the emulator main window by 
		// tracing via the application message loop.
		TraceWithTemporaryBreakpoints(cpuId);
	}
}

void CApp::TraceStepOut(int cpuId, bool requireRtsRti)
{
const int MaxFramesToTryBeforeEnteringNormalTrace = 5;
EventArgs e;
BreakpointResult breakpointResult;
bool breakPoint = false;
int i;

	c64.GetMon()->QuitCommands();
	c64.ClearAllTemporaryBreakpoints();
	if (cpuId == CPUID_MAIN)
	{
		if (requireRtsRti)
		{
			c64.cpu.SetStepOutWithRtsRti();
		}
		else
		{
			c64.cpu.SetStepOutWithRtsRtiPlaTsx();
		}
	}
	else
	{
		if (requireRtsRti)
		{
			c64.diskdrive.cpu.SetStepOutWithRtsRti();
		}
		else
		{
			c64.diskdrive.cpu.SetStepOutWithRtsRtiPlaTsx();
		}
	}

	// Try a few frames to help cut down on screen flicker
	for (i = 0; i < MaxFramesToTryBeforeEnteringNormalTrace; i++)
	{
		m_fskip = -1;
		breakPoint = c64.ExecuteDebugFrame(cpuId, breakpointResult);
		if (breakPoint)
		{
			break;
		}
	}

	if (breakPoint)
	{
		EsTrace.Raise(this, e);
	}
	else
	{
		// If a breakpoint has not occurred after trying a few frames then 
		// enable user interaction of the emulator main window by 
		// tracing via the application message loop.
		TraceWithTemporaryBreakpoints(cpuId);
	}
}

void CApp::Trace(int cpuId)
{
	this->ClearAllTemporaryBreakpoints();
	this->TraceWithTemporaryBreakpoints(cpuId);
}

void CApp::TraceWithTemporaryBreakpoints(int cpuId)
{
HWND hWnd;
EventArgs e;
	c64.GetMon()->QuitCommands();
	hWnd = m_pWinAppWindow->GetHwnd();
	m_bDebug = true;
	m_bBreak = false;
	m_bRunning = true;
	m_bPaused = false;
	m_pWinAppWindow->UpdateWindowTitle(m_szTitle, -1);
	if (hWnd)
	{
		SetForegroundWindow(hWnd);
	}

	SoundResume();
	EsTrace.Raise(this, e);
}

void CApp::TraceFrame(int cpuId)
{
EventArgs e;
BreakpointResult breakpointResult;
	c64.GetMon()->QuitCommands();
	ClearAllTemporaryBreakpoints();
	m_fskip = -1;
	c64.ExecuteDebugFrame(cpuId, breakpointResult);
	EsTraceFrame.Raise(this, e);
}

void CApp::ExecuteC64Clock()
{
EventArgs e;
	c64.GetMon()->QuitCommands();
	m_fskip = -1;
	c64.ExecuteC64Clock();
	EsExecuteC64Clock.Raise(this, e);
}

void CApp::ExecuteC64Instruction()
{
EventArgs e;
	c64.GetMon()->QuitCommands();
	m_fskip = -1;
	c64.ExecuteC64Instruction();
	EsExecuteC64Instruction.Raise(this, e);
}

void CApp::ExecuteDiskClock()
{
EventArgs e;
	c64.GetMon()->QuitCommands();
	m_fskip = -1;
	c64.ExecuteDiskClock();
	EsExecuteDiskClock.Raise(this, e);
}

void CApp::ExecuteDiskInstruction()
{
EventArgs e;
	c64.GetMon()->QuitCommands();
	m_fskip = -1;
	c64.ExecuteDiskInstruction();
	EsExecuteDiskInstruction.Raise(this, e);
}

void CApp::UpdateApplication()
{
EventArgs e;
	UpdateEmulationDisplay();
	EsUpdateApplication.Raise(this, e);
}

void CApp::UpdateEmulationDisplay()
{
	if (m_pWinAppWindow == 0 || m_pWinAppWindow->m_pWinEmuWin == 0)
	{
		return;
	}

	m_pWinAppWindow->m_pWinEmuWin->UpdateC64WindowWithObjects();
}

bool CApp::IsRunning()
{
	return m_bRunning;
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
	if (m_pWinAppWindow->m_pMDIDebugger.expired())
	{
		hWndMdiDebugger = m_pWinAppWindow->ShowDevelopment();
	}

	if (m_pWinAppWindow->m_pMDIDebugger.expired())
	{
		return;
	}

	shared_ptr<CMDIDebuggerFrame> pWinDebugger = m_pWinAppWindow->m_pMDIDebugger.lock();
	if (cpuid == CPUID_MAIN)
	{
		pWinDebugger->ShowDebugCpuC64(pcmode, address);
	}
	else if (cpuid == CPUID_DISK)
	{
		pWinDebugger->ShowDebugCpuDisk(pcmode, address);
	}
}

HWND CApp::GetMainFrameWindow()
{
	if (m_pWinAppWindow == 0)
	{
		return NULL;
	}

	return this->m_pWinAppWindow->GetHwnd();
}

void CApp::DisplayVicCursor(bool bEnabled)
{
	if (m_pWinAppWindow == 0 || m_pWinAppWindow->m_pWinEmuWin == 0)
	{
		return;
	}

	m_pWinAppWindow->m_pWinEmuWin->DisplayVicCursor(bEnabled);
}

void CApp::DisplayVicRasterBreakpoints(bool bEnabled)
{
	if (m_pWinAppWindow == 0 || m_pWinAppWindow->m_pWinEmuWin == 0)
	{
		return;
	}

	m_pWinAppWindow->m_pWinEmuWin->DisplayVicRasterBreakpoints(bEnabled);
}

void CApp::SetVicCursorPos(int iCycle, int iLine)
{
	if (m_pWinAppWindow == 0 || m_pWinAppWindow->m_pWinEmuWin == 0)
	{
		return;
	}

	m_pWinAppWindow->m_pWinEmuWin->SetVicCursorPos(iCycle, iLine);
}

void CApp::GetVicCursorPos(int *piCycle, int *piLine)
{
	if (m_pWinAppWindow == 0 || m_pWinAppWindow->m_pWinEmuWin == 0)
	{
		return;
	}

	m_pWinAppWindow->m_pWinEmuWin->GetVicCursorPos(piCycle, piLine);
}

void CApp::SetRadixHexadecimal()
{
	this->c64.GetMon()->Set_Radix(DBGSYM::MonitorOption::Hex);
}

void CApp::SetRadixDecimal()
{
	this->c64.GetMon()->Set_Radix(DBGSYM::MonitorOption::Dec);
}

void CApp::TogglePause()
{
	m_bPaused = !m_bPaused;
	m_pWinAppWindow->UpdateWindowTitle(m_szTitle, -1);
}

void CApp::ToggleSoundMute()
{
	m_bSoundMute= !m_bSoundMute;
	if (m_bSoundMute)
	{
		c64.sid.MasterVolume = 0.0;
	}
	else
	{
		c64.sid.MasterVolume = 1.0;
	}

	m_pWinAppWindow->UpdateWindowTitle(m_szTitle, -1);
}

HWND CApp::ShowDevelopment()
{
EventArgs e;
HWND hWnd = NULL;

	hWnd = m_pWinAppWindow->ShowDevelopment();
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
	{
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam); 
	}
 
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
	{
        return 1;
	}
    else
	{
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam );
	}
}
#else
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	return app.KeyboardProc(nCode, wParam, lParam); 
}
LRESULT CApp::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0 || nCode != HC_ACTION )  // do not process message 
	{
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam); 
	}
 
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
	{
        return 1;
	}
    else
	{
        return CallNextHookEx(m_hKeyboardHook, nCode, wParam, lParam );
	}
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

void CApp::VicCursorMove(int cycle, int line)
{
	this->EsVicCursorMove.Raise(this, VicCursorMoveEventArgs(cycle, line));
}