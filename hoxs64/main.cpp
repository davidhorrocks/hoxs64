#include <windows.h>
#include <tchar.h>
#include <windowsx.h>
#include <commctrl.h>
#include <Winuser.h>
#include "dx_version.h"
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
#include "Wfs.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "cevent.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "besttextwidth.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
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
#include "mainwin.h"
#include "main.h"
#include "resource.h"

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	CApp* app = new CApp();
	int r = app->Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	delete app;
	return r;
}

CApp::CApp() noexcept(false)
{
	ZeroMemory(&m_Vinfo, sizeof(m_Vinfo));

	wsAppName = APPNAME;
	wsMainWndClsName = HOXS_MAIN_WND_CLASS;
	wsMonitorWndClsName = HOXS_MONITOR_WND_CLASS;
	wsMonitorDisassembleWndClsName = HOXS_MONITOR_DISS_WND_CLASS;

	isImGuiStarted = false;
	m_bStartFullScreen = false;
	m_bShowSpeed = true;
	m_bLimitSpeed = true;
	m_bSkipFrames = true;

	m_bActive = false;
	m_bReady = false;
	m_bRunning = false;
	m_bWindowed = true;
	m_bDebug = false;
	m_bPaused = false;
	m_bMaxSpeed = false;
	m_syncModeFullscreen = HCFG::FSSM_LINE;
	m_bAutoload = false;
	m_bClosing = false;
	m_bBusy = false;
	hCursorBusy = LoadCursor(0L, IDC_WAIT);
	hOldCursor = NULL;

	ZeroMemory(&m_StartupStickyKeys, sizeof(m_StartupStickyKeys));
	ZeroMemory(&m_StartupToggleKeys, sizeof(m_StartupToggleKeys));
	ZeroMemory(&m_StartupFilterKeys, sizeof(m_StartupFilterKeys));
	m_StartupStickyKeys.cbSize = sizeof(STICKYKEYS);
	m_StartupToggleKeys.cbSize = sizeof(TOGGLEKEYS);
	m_StartupFilterKeys.cbSize = sizeof(FILTERKEYS);

	lastMouseX = 0;
	lastMouseY = 0;
}

CApp::~CApp()
{
	Cleanup();
}

void* CApp::operator new(size_t i)
{
	return _mm_malloc(i, 16);
}

void CApp::operator delete(void* p)
{
	_mm_free(p);
}

void CApp::Cleanup() noexcept
{
	FreeDirectX();
	CloseImGuiContext();
}

void CApp::CloseAppWindow()
{
	this->m_bRunning = false;
	this->m_bClosing = true;
	this->m_pWinAppWindow->CloseWindow();
}

void CApp::CreateImGuiContext()
{
	if (isImGuiStarted)
	{
		return;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;
	//io.ConfigViewportsNoDefaultParent = true;
	//io.ConfigDockingAlwaysTabBar = true;
	//io.ConfigDockingTransparentPayload = true;
#if 0
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;     // FIXME-DPI: THIS CURRENTLY DOESN'T WORK AS EXPECTED. DON'T USE IN USER APP!
	io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI
#endif

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	//io.ConfigDocking

	// Setup Platform/Renderer bindings
	//ImGui_ImplWin32_Init(m_pWinAppWindow->GetHwnd());
	isImGuiStarted = true;
}

void CApp::CloseImGuiContext() noexcept
{
	try
	{
		if (isImGuiStarted)
		{
			isImGuiStarted = false;
			ImGui::DestroyContext();
		}
	}
	catch(...)
	{

	}
}


int CApp::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HRESULT hRet;
	ULARGE_INTEGER frequency, last_counter, new_counter, tSlice;
	ULARGE_INTEGER frequencyDoubler;
	DWORD emulationSpeed = FRAMEUPDATEFREQ * 100;
	ULARGE_INTEGER start_counter,end_counter;
	short captionUpdateCounter = 0;
	BOOL bRet;

#if (_WIN32_WINNT >= 0x0A00 /*_WIN32_WINNT_WIN10*/)
	Microsoft::WRL::Wrappers::RoInitializeWrapper initialize(RO_INIT_MULTITHREADED);
	if (FAILED(hRet))
	{
		G::InitFail(0, hRet, TEXT("Microsoft::WRL::Wrappers::RoInitializeWrapper failed"));
		return ShellExitInitFailed;
	}
#else
	hRet = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);
	if (FAILED(hRet))
	{
		G::InitFail(0, hRet, TEXT("CoInitializeEx failed"));
		return ShellExitInitFailed;
	}
#endif
	
	ImGui_ImplWin32_EnableDpiAwareness();

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
	frequencyDoubler.QuadPart = m_framefrequencyDoubler.QuadPart;
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
	
	AllowAccessibilityShortcutKeys(false);
	
	m_bReady = true;
	m_bRunning=1;
	m_bPaused=0;
	m_bInitDone = true;
	bool bExecuteFrameDone = false;
	int framesSkipped = 0;
	G::EnsureWindowPosition(m_pWinAppWindow->GetHwnd());

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
					(!TranslateAccelerator(hWndDefaultAccelerator, this->m_hAccelTable, &msg)))
				{
					TranslateMessage(&msg); 
					DispatchMessage(&msg);
				}
				continue;
			}
			if (!TranslateAccelerator(m_pWinAppWindow->GetHwnd(), this->m_hAccelTable, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}
		}
		frequency.QuadPart = m_framefrequency.QuadPart;
		frequencyDoubler.QuadPart = m_framefrequencyDoubler.QuadPart;

		if (((m_bActive && m_bReady) || ErrorLogger::HideWindow) && m_bRunning && !m_bPaused)
		{
			if (!bExecuteFrameDone)
			{
				SoundResume();
				if (captionUpdateCounter<=0)
				{
					captionUpdateCounter = FRAMEUPDATEFREQ;
					QueryPerformanceCounter((PLARGE_INTEGER)&end_counter);
					emulationSpeed =(ULONG) (((ULONGLONG)100) * (frequency.QuadPart * (ULONGLONG)FRAMEUPDATEFREQ) / (end_counter.QuadPart - start_counter.QuadPart));
					m_pWinAppWindow->UpdateWindowTitle(wsTitle.c_str(), emulationSpeed);
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
							last_counter.QuadPart = last_counter.QuadPart - frequency.QuadPart/8;
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
								if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD))
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
					this->CloseAppWindow();
				}
			}

			captionUpdateCounter--;
			bool bDrawThisFrame = ((m_fskip < 0) || m_bIsDebugCart || m_bDebug);
			if (bDrawThisFrame)
			{
				hRet = m_pWinAppWindow->UpdateC64Window(true);
				if (SUCCEEDED(hRet))
				{
					if (!m_bIsDebugCart)
					{
						if (SUCCEEDED(hRet))
						{
							hRet = gx.PresentFrame();
							if (SUCCEEDED(hRet))
							{
								if (this->m_syncModeFullscreen == HCFG::FSSM_FRAME_DOUBLER && !this->m_bWindowed)
								{
									ULARGE_INTEGER last_present_counter;
									QueryPerformanceCounter((PLARGE_INTEGER)&last_present_counter);
									tSlice.QuadPart = 0;
									hRet = m_pWinAppWindow->UpdateC64Window(false);
									if (SUCCEEDED(hRet))
									{
										if (((LONGLONG)tSlice.QuadPart < (LONGLONG)frequency.QuadPart))
										{
											while ((LONGLONG)tSlice.QuadPart < (LONGLONG)frequencyDoubler.QuadPart)
											{
												QueryPerformanceCounter((PLARGE_INTEGER)&new_counter);
												tSlice.QuadPart = new_counter.QuadPart - last_present_counter.QuadPart;
											}

											gx.PresentFrame();
										}
									}
								}
							}
						}
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
			if (!m_bClosing && m_bActive && !m_bReady && !ErrorLogger::HideWindow)
			{
				hRet = E_FAIL;				
				if (gx.isInitOK)
				{
					hRet = gx.TestPresent();
					if (hRet == S_OK)
					{
						m_bReady = true;
					}
					else if (hRet == DXGI_ERROR_DEVICE_REMOVED || hRet == DXGI_ERROR_DEVICE_RESET)
					{
						if (SUCCEEDED(hRet = m_pWinAppWindow->ResetDirect3D()))
						{
							m_bReady = true;
						}

						G::WaitMessageTimeout(1000);
					}
					else 
					{
						CloseAppWindow();
					}
				}
				else
				{
					G::WaitMessageTimeout(1000);
					if (SUCCEEDED(hRet = m_pWinAppWindow->ResetDirect3D()))
					{
						m_bReady = true;
					}
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
		G::DebugMessageBox(0L, TEXT("Failed to register main window class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}

	hr = RegisterKeyPressWindow(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register GetKeyPressClass class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}

	hr = RegisterVicColorWindow(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register VicColorClass class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}

	hr = CMDIDebuggerFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CMDIDebuggerFrame class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WpcCli::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WpcCli class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CMDIChildCli::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CMDIChildCli class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CToolItemAddress::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register RegisterClass class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyFrame::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyFrame class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyChild class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyEditChild::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyEditChild class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = CDisassemblyReg::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register CDisassemblyReg class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WPanel::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WPanel class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}
	hr = WpcBreakpoint::RegisterClass(hInstance);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Failed to register WpcBreakpoint class."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		return hr;
	}

	return S_OK;
}

const wchar_t *CApp::GetAppTitle()
{
	return wsTitle.c_str(); 
}

const wchar_t* CApp::GetAppName()
{
	return wsAppName.c_str(); 
}

VS_FIXEDFILEINFO *CApp::GetVersionInfo()
{
	return &m_Vinfo;
}

HRESULT CApp::InitInstance(int nCmdShow, PWSTR lpCmdLine)
{
	BOOL br;
	HRESULT hr;
	DWORD lr;
	HWND hWndMain;
	TCHAR t[60];
	CConfig *thisCfg = this;
	CAppStatus *thisAppStatus = this;
	IC64Event *thisC64Event = this;
	IAppCommand* thisAppCommand = this;
	int minWidth;
	int minHeight;
	int mainWinWidth;
	int mainWinHeight;
	int mainWinFixedWidth;
	int mainWinFixedHeight;
	WCHAR maxpathbuffer[MAX_PATH + 1];

	ClearError();
	lr = GetModuleFileName(NULL, maxpathbuffer, _countof(maxpathbuffer) - 1);
	if (lr==0) 
	{
		return E_FAIL;
	}

	wsAppFullPath = maxpathbuffer;
	
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
			ErrorLogger::HideWindow = true;
		}

		ErrorLogger::HideMessageBox = true;
	}
	else if (caStartFullscreen)
	{
		this->m_bStartFullScreen = true;
	}

	std::wstring wsroot;
	std::wstring wsfilename;
	Wfs::SplitRootPath(wsAppFullPath, wsroot, wsAppDirectory, wsfilename);
	wsAppDirectory = wsroot + wsAppDirectory;
	wsAppConfigPath.clear();
	Wfs::Path_Append(wsAppConfigPath, wsAppDirectory);
	Wfs::Path_Append(wsAppConfigPath, L"hoxs64.ini");
	if (LoadString(m_hInstance, IDS_APP_TITLE, maxpathbuffer, _countof(maxpathbuffer) - 1))
	{
		wsTitle = maxpathbuffer;
	}

	if (LoadString(m_hInstance, IDS_MONITOR_TITLE, maxpathbuffer, _countof(maxpathbuffer) - 1))
	{
		wsMonitorTitle = maxpathbuffer;
	}

	if (SUCCEEDED(G::GetVersion_Res(wsAppFullPath.c_str(), &m_Vinfo)))
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
		wsTitle.append(t);
	}

	//Set up some late bound OS DLL calls
	G::InitRandomSeed();

	//Initialise common control library.
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC   = ICC_COOL_CLASSES|ICC_BAR_CLASSES|ICC_LISTVIEW_CLASSES|ICC_USEREX_CLASSES;
	br = ::InitCommonControlsEx(&icex);
	if (br == FALSE)
	{
		G::DebugMessageBox(0L, TEXT("InitCommonControlsEx() failed."), wsAppName.c_str(), MB_ICONWARNING);
		return E_FAIL;
	}

	hr = dx.Init(thisAppStatus);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Initialisation failed for DirectX helper."), wsAppName.c_str(), MB_ICONWARNING);
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
	m_hAccelTable = LoadAccelerators (m_hInstance, wsAppName.c_str());
	try
	{
		m_pWinAppWindow = std::make_shared<CAppWindow>(&gx, &dx, static_cast<IAppCommand *>(this), static_cast<CAppStatus *>(this), &c64);
	}
	catch (...)
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the application window."), wsAppName.c_str(), MB_ICONWARNING);
		return E_FAIL;
	}

	m_pWinAppWindow->SetNotify(this);	
	m_pWinAppWindow->GetMinimumWindowedSize(m_borderSize, m_bShowFloppyLed, &minWidth, &minHeight);
	POINT winpos = {0,0};
	m_pWinAppWindow->GetRequiredMainWindowSize(m_borderSize, m_bShowFloppyLed, &mainWinFixedWidth, &mainWinFixedHeight);

	//Look for saved x,y position and custom window size.
	hr = mainCfg.LoadWindowSetting(winpos, mainWinWidth, mainWinHeight);
	if (FAILED(hr))
	{
		mainWinWidth = mainWinFixedWidth;
		mainWinHeight = mainWinFixedHeight;
		winpos = GetCenteredPos(mainWinWidth, mainWinHeight);
	}

	mainWinWidth = std::max(minWidth, mainWinWidth);
	mainWinHeight = std::max(minHeight, mainWinHeight);
	//Apply custom window size flag.
	hWndMain = m_pWinAppWindow->Create(m_hInstance, NULL, wsTitle.c_str(), winpos.x, winpos.y, mainWinWidth, mainWinHeight, NULL);
	if (!hWndMain)
	{
		G::DebugMessageBox(0L, TEXT("Unable to create the application window."), wsAppName.c_str(), MB_ICONWARNING);
		return E_FAIL;
	}

	m_bWindowed = true;
	if (!ErrorLogger::HideWindow)
	{
		G::EnsureWindowPosition(hWndMain);
	}

	m_pWinAppWindow->SaveMainWindowSize();

	CreateImGuiContext();

	// Initialise Direct 3D
	hr = gx.Initialize(&c64, thisAppCommand, thisAppStatus);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Initialisation failed for Direct 3D 11.1."), wsAppName.c_str(), MB_ICONWARNING);
		DestroyWindow(hWndMain);
		return E_FAIL;
	}
	
	// Initialise Direct Input
	hr = dx.OpenDirectInput(m_hInstance, hWndMain);
	if (FAILED(hr))
	{
		G::DebugMessageBox(0L, TEXT("Initialisation failed for Direct Input."), wsAppName.c_str(), MB_ICONEXCLAMATION);
		DestroyWindow(hWndMain);
		return hr;
	}
	
	m_bSoundOK = false;
	if (caSoundOff == NULL)
	{
		hr = dx.OpenDirectSound(hWndMain, m_fps);
		if (FAILED(hr))
		{
			G::DebugMessageBox(hWndMain, TEXT("Direct Sound has failed to initialise with the primary sound driver. Sound will be unavailable."), wsAppName.c_str(), MB_ICONWARNING);
		}
		else
		{
			m_bSoundOK = true;
		}
	}

	//Initialise the c64 emulation.
	if (S_OK != c64.Init(thisAppCommand, thisAppStatus, thisC64Event, &gx, &dx, wsAppDirectory.c_str()))
	{
		c64.DisplayError(0, wsAppName.c_str());
		DestroyWindow(hWndMain);
		return E_FAIL;
	}

	// Call ApplyConfig a second time to push settings to the C64.
	ApplyConfig(mainCfg);

	hr = m_pWinAppWindow->SetWindowedMode(!m_bStartFullScreen);
	if (FAILED(hr))
	{
		m_pWinAppWindow->DisplayError(m_pWinAppWindow->GetHwnd(), wsAppName.c_str());
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
	if (!ErrorLogger::HideWindow)
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
	r.x = (rcWorkArea.right - rcWorkArea.left - w) / 2;
	if (r.x < 0)
	{
		r.x = 0;
	}

	r.y = (rcWorkArea.bottom - rcWorkArea.top - h) / 2;
	if (r.y < 0)
	{
		r.y = 0;
	}

	return r;
}

void CApp::FreeDirectX() noexcept
{
	m_bReady = false;
	dx.CloseDirectSound();
	dx.CloseDirectInput();
	gx.Cleanup();
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

void CApp::LoadCrtFileDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::InsertTapeDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::AutoLoadDialog(HWND hWnd)
{
OPENFILENAME of;
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
		hr = c64.AutoLoad(initfilename, prgBrowse.SelectedDirectoryIndex, false, prgBrowse.SelectedC64FileName, prgBrowse.SelectedQuickLoadDiskFile, prgBrowse.SelectedAlignD64Tracks);
		if (hr != S_OK)
		{
			c64.DisplayError(hWnd, TEXT("Auto Load"));
		}
		//The call to c64.AutoLoad can enable disk emulation
		mainCfg.m_bD1541_Emulation_Enable = m_bD1541_Emulation_Enable;
	}
}

void CApp::LoadC64ImageDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::LoadT64Dialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::InsertDiskImageDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::SaveD64ImageDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::SaveFDIImageDialog(HWND hWnd)
{
OPENFILENAME of;
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

void CApp::SaveP64ImageDialog(HWND hWnd)
{
OPENFILENAME of;
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
	of.lStructSize = sizeof(OPENFILENAME);
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

void CApp::SaveC64StateDialog(HWND hWnd)
{
OPENFILENAME of;
TCHAR initfilename[MAX_PATH+1];
TCHAR filename[MAX_PATH+1];
BOOL b;
HRESULT hr;

	TCHAR title[] = TEXT("Save State Image");

	ZeroMemory(&of, sizeof(of));
	of.lStructSize = sizeof(OPENFILENAME);
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

void CApp::LoadC64StateDialog(HWND hWnd)
{
OPENFILENAME of;
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
	if (c64.isInitOK)
	{
		this->c64.sid.SetSidChipAddressMap(numberOfExtraSidChips, addressOfSecondSID, addressOfThirdSID, addressOfFourthSID, addressOfFifthSID, addressOfSixthSID, addressOfSeventhSID, addressOfEighthSID);
	}
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
CConfig &cfg= *this;
bool bNeedDisplayReset = false;
bool bNeedDisplayStyleUpdate = false;
bool bNeedSoundFilterInit = false;
bool bPaletteChanged = false;
unsigned int i;

	if (m_pWinAppWindow!=0 && m_pWinAppWindow->GetHwnd() != 0 && IsWindow(m_pWinAppWindow->GetHwnd()))
	{
		hWnd = m_pWinAppWindow->GetHwnd();
	}

	if (c64.isInitOK)
	{
		c64.cia1.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);
		c64.cia2.SetMode(newcfg.m_CIAMode, newcfg.m_bTimerBbug);
		if (newcfg.m_bSID_Emulation_Enable != false && this->m_bSID_Emulation_Enable == false)
		{
			c64.sid.CurrentClock = c64.cpu.CurrentClock;
		}

		if (newcfg.m_bD1541_Emulation_Enable != false && this->m_bD1541_Emulation_Enable == false)
		{
			c64.diskdrive.CurrentPALClock = c64.cpu.CurrentClock;
		}
	}

	if (newcfg.m_fullscreenAdapterIsDefault != this->m_fullscreenAdapterIsDefault
		|| newcfg.m_fullscreenAdapterNumber != this->m_fullscreenAdapterNumber
		|| newcfg.m_fullscreenOutputNumber != this->m_fullscreenOutputNumber
		|| newcfg.m_fullscreenWidth != this->m_fullscreenWidth
		|| newcfg.m_fullscreenHeight != this->m_fullscreenHeight
		|| newcfg.m_fullscreenFormat != this->m_fullscreenFormat
		|| newcfg.m_fullscreenRefreshNumerator != this->m_fullscreenRefreshNumerator
		|| newcfg.m_fullscreenRefreshDenominator != this->m_fullscreenRefreshDenominator
		|| newcfg.m_syncModeFullscreen != this->m_syncModeFullscreen
		|| newcfg.m_syncModeWindowed != this->m_syncModeWindowed)
	{
		bNeedDisplayReset = true;
	}

	if (newcfg.m_bWindowedLockAspectRatio && !this->m_bWindowedLockAspectRatio
		|| newcfg.m_borderSize != this->m_borderSize
		|| newcfg.m_bShowFloppyLed != this->m_bShowFloppyLed)
	{
		bNeedDisplayStyleUpdate = true;
	}
	

	bool keepWindowPlacement = true;
	if (newcfg.m_bWindowedLockAspectRatio && !this->m_bWindowedLockAspectRatio
		|| newcfg.m_syncModeFullscreen != this->m_syncModeFullscreen
		|| newcfg.m_syncModeWindowed != this->m_syncModeWindowed
		|| newcfg.m_fullscreenAdapterNumber != this->m_fullscreenAdapterNumber)
	{
		keepWindowPlacement = false;
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

	this->m_fskip = -1;
	if (hWnd != 0 && gx.isInitOK)
	{
		if (bPaletteChanged)
		{
			if (m_pWinAppWindow)
			{
				m_pWinAppWindow->SetColours();
			}
		}

		this->gx.SetVsyncMode(this->m_bWindowed ? this->m_syncModeWindowed : this->m_syncModeFullscreen);
		this->gx.c64display.SetPixelFilter(m_blitFilter == HCFG::EMUWINDOWFILTER::EMUWINFILTER_POINT);
		if (bNeedDisplayReset)
		{
			if (m_pWinAppWindow)
			{
				m_pWinAppWindow->SetWindowedMode(m_bWindowed);
			}
		}
		else if (bNeedDisplayStyleUpdate)
		{
			this->gx.c64display.SetRenderStyle(0, 0, this->m_bWindowed, m_borderSize, m_bShowFloppyLed, m_fullscreenStretch, m_blitFilter == HCFG::EMUWINDOWFILTER::EMUWINFILTER_POINT);
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

		this->m_framefrequencyDoubler.QuadPart = this->m_framefrequency.QuadPart / 2;
	}

	if (hWnd)
	{
		dx.InitJoys(hWnd, this->m_joy1config, this->m_joy2config);
	}

	this->SetSidChipAddressMap(newcfg.m_numberOfExtraSIDs, newcfg.m_Sid2Address, newcfg.m_Sid3Address, newcfg.m_Sid4Address, newcfg.m_Sid5Address, newcfg.m_Sid6Address, newcfg.m_Sid7Address, newcfg.m_Sid8Address);	
	if (c64.isInitOK)
	{
		c64.UpdateKeyMap();
		if (this->m_bSoundOK)
		{
			c64.sid.UpdateSoundBufferLockSize(this->m_fps);
			if (bNeedSoundFilterInit)
			{
				c64.sid.InitResamplingFilters(this->m_fps);
			}
		}
	}

	if (hWnd)
	{
		m_pWinAppWindow->UpdateWindowTitle(wsTitle.c_str(), -1);
	}
}

void CApp::SetBusyApp(bool isBusy)
{
	if (isBusy)
	{
		this->SoundOff();
	}

	this->SetBusy(isBusy);

	if (!isBusy)
	{
		this->SoundOn();
	}
}

void CApp::SetBusy(bool isBusy)
{
	if (isBusy)
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
	m_pWinAppWindow->UpdateWindowTitle(wsTitle.c_str(), -1);
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
	if (m_pWinAppWindow == nullptr)
	{
		return;
	}

	m_pWinAppWindow->UpdateC64WindowWithObjects();
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
	if (m_pWinAppWindow == nullptr)
	{
		return;
	}

	m_pWinAppWindow->DisplayVicCursor(bEnabled);
}

void CApp::DisplayVicRasterBreakpoints(bool bEnabled)
{
	if (m_pWinAppWindow == nullptr)
	{
		return;
	}

	m_pWinAppWindow->DisplayVicRasterBreakpoints(bEnabled);
}

void CApp::SetVicCursorPos(int iCycle, int iLine)
{
	if (m_pWinAppWindow == nullptr)
	{
		return;
	}

	m_pWinAppWindow->SetVicCursorPos(iCycle, iLine);
}

void CApp::GetVicCursorPos(int *piCycle, int *piLine)
{
	if (m_pWinAppWindow == nullptr)
	{
		return;
	}

	m_pWinAppWindow->GetVicCursorPos(piCycle, piLine);
}

void CApp::SetRadixHexadecimal()
{
	this->c64.GetMon()->Set_Radix(DBGSYM::MonitorOption::Hex);
}

void CApp::SetRadixDecimal()
{
	this->c64.GetMon()->Set_Radix(DBGSYM::MonitorOption::Dec);
}

void CApp::GetLastMousePosition(int* x, int* y)
{
	*x = lastMouseX;
	*y = lastMouseY;
}

void CApp::SetLastMousePosition(const int* x, const int* y)
{
	lastMouseX = *x;
	lastMouseY = *y;
}

void CApp::PostCloseMainWindow()
{
	::PostMessage(this->GetMainFrameWindow(), WM_CLOSE, 0, 0);
}

void CApp::PostToggleFullscreen()
{
	::PostMessage(this->GetMainFrameWindow(), WM_COMMAND, MAKEWPARAM(IDM_TOGGLEFULLSCREEN, 0), 0);
}

bool CApp::PostAutoLoadFile(const wchar_t *pszFilename, int directoryIndex, bool quickload)
{
	if (pszFilename != nullptr)
	{
		if (std::wcslen(pszFilename) > 0)
		{
			HRESULT hr = this->c64.AutoLoad(pszFilename, directoryIndex, false, nullptr, quickload, false);
			if (SUCCEEDED(hr))
			{
				return true;
			}
		}
	}

	return false;
}

bool CApp::InsertDiskImageFromFile(const wchar_t* pszFilename)
{
	if (pszFilename != nullptr)
	{
		HRESULT hr = c64.InsertDiskImageFile(pszFilename, false, false);
		if (SUCCEEDED(hr))
		{
			return true;
		}
	}

	return false;
}

bool CApp::InsertTapeImageFromFile(const wchar_t* pszFilename)
{
	if (pszFilename != nullptr)
	{
		HRESULT hr = c64.tape64.InsertTAPFile(pszFilename);
		if (SUCCEEDED(hr))
		{
			return true;
		}
	}

	return false;
}

void CApp::EnableC64Input(bool enabled)
{
	c64.cia1.EnableInput(enabled);
}

void CApp::TogglePause()
{
	m_bPaused = !m_bPaused;
	m_pWinAppWindow->UpdateWindowTitle(wsTitle.c_str(), -1);
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

	m_pWinAppWindow->UpdateWindowTitle(wsTitle.c_str(), -1);
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