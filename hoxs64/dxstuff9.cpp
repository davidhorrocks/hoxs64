#define INITGUID
#include <windows.h>
#include <windowsx.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <dmusici.h>
#include <stdio.h>
#pragma warning(disable: 4005 4995)
#include <tchar.h>
#pragma warning(default: 4005 4995)
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 ) 
#include <dxdiag.h>
#include "math.h"
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "resource.h"

CDX9::CDX9()
{
int i;

	m_pD3D       = NULL; // Used to create the D3DDevice
	m_pd3dDevice = NULL; // Our rendering device
	m_pDiplaymodes = NULL;
	m_pSysMemSurface = NULL;
	for (int i=0; i < _countof(m_pSmallSurface); i++)
		m_pSmallSurface[i] = NULL;
	m_iIndexSmallSurface = 0;
	ZeroMemory(&m_sizeSmallSurface, sizeof(m_sizeSmallSurface));

	m_displayFirstVicRaster = 0;
	m_displayLastVicRaster = 0;
	m_displayXPos = 0;
	m_displayYPos = 0;
	m_displayWidth = 0;
	m_displayHeight = 0;
	m_displayStart = 0;

	pDirectDrawCreateEx=0;
	pDirectInputCreateEx=0;


	DIHinst = 0;
	pKeyboard = 0;
	pDI = 0;

	for (i=0; i<NUMJOYS; i++)
		joy[i] = 0;

	lpDirectSound=NULL;
	pPrimarySoundBuffer=NULL;
	pSecondarySoundBuffer=NULL;

	m_diplaymodes_count=0;
	BufferLockSize=0;

	m_iEraseCount = 0;

	m_psprLedMotor = NULL;
	m_psprLedDrive = NULL;
	m_psprLedWrite = NULL;

	m_ptxLedGreenOn = NULL;
	m_ptxLedGreenOff = NULL;
	m_ptxLedRedOn = NULL;
	m_ptxLedRedOff = NULL;
	m_ptxLedBlueOn = NULL;
	m_ptxLedBlueOff = NULL;

	m_soundResumeDelay = 0;
	m_soundVolumeDelay = 0;
}

CDX9::~CDX9()
{
	CleanupD3D();
}

void CDX9::CleanupD3D_Surfaces()
{
	ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS));

	m_iEraseCount = 0;
	if (m_appStatus)
	{
		m_appStatus->m_displayFormat = (DWORD)D3DFMT_UNKNOWN;
		m_appStatus->m_ScreenDepth = 0;
		m_appStatus->m_bUseCPUDoubler = false;
	}

	FreeTextures();

	FreeSmallSurface();
	FreeSysMemSurface();
}

void CDX9::FreeSmallSurface()
{
	for (int i=0; i < _countof(m_pSmallSurface); i++)
	{
		if (m_pSmallSurface[i])
			m_pSmallSurface[i]->Release();
		m_pSmallSurface[i] = NULL;
	}
}

void CDX9::FreeSysMemSurface()
{
	if (m_pSysMemSurface!=NULL)
	{		
		if (m_pSysMemSurface)
			m_pSysMemSurface->Release();
		m_pSysMemSurface = NULL;
	}
}

void CDX9::ClearSurfaces(D3DCOLOR colour)
{
	if (m_pd3dDevice)
	{
		m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, colour, 0.0, 0);

		if (m_pSysMemSurface!=NULL)
		{
			m_pd3dDevice->ColorFill(m_pSysMemSurface, NULL, colour);			
		}
		for (int i=0; i < _countof(m_pSmallSurface); i++)
		{
			if (m_pSmallSurface[i])
				m_pd3dDevice->ColorFill(m_pSmallSurface[i], NULL, colour);
		}
	}
}


HRESULT CDX9::CreateSmallSurface(int Width, int Height, D3DFORMAT Format)
{
HRESULT hr;
int i;
	FreeSmallSurface();
	FreeSysMemSurface();
	m_sizeSmallSurface.cx = Width;
	m_sizeSmallSurface.cy = Height;

#ifdef USESYSMEMSURFACE
	hr = m_pd3dDevice->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_SYSTEMMEM, &m_pSysMemSurface, NULL);
#else
	hr = S_OK;
#endif
	if (SUCCEEDED(hr))
	{
		for (i = 0; i < _countof(m_pSmallSurface); i++)
		{
			hr = m_pd3dDevice->CreateOffscreenPlainSurface(Width, Height, Format, D3DPOOL_DEFAULT, &m_pSmallSurface[i], NULL);
			if (FAILED(hr))
				break;
		}
	}
	if (FAILED(hr))
	{
		FreeSmallSurface();
		FreeSysMemSurface();
	}
	return hr;
}

void CDX9::ClearTargets(D3DCOLOR color)
{
	if (m_iEraseCount > 0)
	{
		m_pd3dDevice->Clear(m_iEraseCount, &m_drcEraseRects[0], D3DCLEAR_TARGET, color, 0.0, 0);
	}
}

HRESULT CDX9::UpdateBackbuffer(D3DTEXTUREFILTERTYPE filter)
{
HRESULT hr = E_FAIL;
	if (m_pd3dDevice)
	{
		IDirect3DSurface9 *pSmallSuface = m_pSmallSurface[m_iIndexSmallSurface];
		if (pSmallSuface != NULL)
		{
#ifdef USESYSMEMSURFACE
			if (m_pSysMemSurface != NULL)
			{
				m_pd3dDevice->UpdateSurface(m_pSysMemSurface, NULL, pSmallSuface, NULL);
			}
#endif
			LPDIRECT3DSURFACE9 pBackBuffer;
			if (D3D_OK == m_pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer))
			{
				hr = m_pd3dDevice->StretchRect(pSmallSuface, NULL, pBackBuffer, &m_rcTargetRect, filter);
				pBackBuffer->Release();
				pBackBuffer = NULL;
			}
		}
	}
	return hr;
}

IDirect3DSurface9 *CDX9::GetSmallSurface()
{
	IDirect3DSurface9 *surface = m_pSmallSurface[m_iIndexSmallSurface];
	if (surface)
	{
		surface->AddRef();
	}
	return surface;
}

IDirect3DSurface9 *CDX9::GetSysMemSurface()
{
	IDirect3DSurface9 *surface = m_pSysMemSurface;
	if (surface)
	{
		surface->AddRef();
	}
	return surface;
}

void CDX9::CleanupD3D_Devices()
{
	CleanupD3D_Surfaces();

	if (m_pDiplaymodes)
	{
		LocalFree(m_pDiplaymodes);
		m_pDiplaymodes = NULL;
	}

	if( m_pd3dDevice != NULL) 
        m_pd3dDevice->Release();
	m_pd3dDevice = NULL;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
// Desc: Releases all previously initialized objects
//-----------------------------------------------------------------------------
void CDX9::CleanupD3D()
{
	CleanupD3D_Devices();

    if( m_pD3D != NULL)
        m_pD3D->Release();
	m_pD3D = NULL;
}

HRESULT CDX9::Init(CConfig *cfg, CAppStatus *appStatus, bit32 vicColorTable[])
{
	int i;
	m_cfg = cfg;
	m_appStatus = appStatus;


	for (i = 0; i < 256; i++)
	{
		m_paletteEntry[i].peRed = (BYTE) (((i >> 5) & 0x07) * 255 / 7);
		m_paletteEntry[i].peGreen = (BYTE) (((i >> 2) & 0x07) * 255 / 7);
		m_paletteEntry[i].peBlue = (BYTE) (((i >> 0) & 0x03) * 255 / 3);
		m_paletteEntry[i].peFlags = (BYTE) 1;
	}
	for (i = 0; i < 16; i++)
	{
		m_paletteEntry[i].peRed = (BYTE) ((vicColorTable[i] & 0x00ff0000) >> 16);
		m_paletteEntry[i].peGreen = (BYTE) ((vicColorTable[i] & 0x0000ff00) >> 8);
		m_paletteEntry[i].peBlue = (BYTE) ((vicColorTable[i] & 0x000000ff));
		m_paletteEntry[i].peFlags = (BYTE) 1;
	}

	return S_OK;
}

BOOL CDX9::DXUTGetMonitorInfo(HMONITOR hMonitor, LPMONITORINFO lpMonitorInfo)
{
	G::InitLateBindLibraryCalls();
    if( G::s_pFnGetMonitorInfo ) 
		return G::s_pFnGetMonitorInfo(hMonitor, lpMonitorInfo);

    RECT rcWork;
    if ((hMonitor == DXUT_PRIMARY_MONITOR) && lpMonitorInfo && (lpMonitorInfo->cbSize >= sizeof(MONITORINFO)) && SystemParametersInfoA(SPI_GETWORKAREA, 0, &rcWork, 0))
    {
        lpMonitorInfo->rcMonitor.left = 0;
        lpMonitorInfo->rcMonitor.top  = 0;
        lpMonitorInfo->rcMonitor.right  = GetSystemMetrics(SM_CXSCREEN);
        lpMonitorInfo->rcMonitor.bottom = GetSystemMetrics(SM_CYSCREEN);
        lpMonitorInfo->rcWork = rcWork;
        lpMonitorInfo->dwFlags = MONITORINFOF_PRIMARY;
        return TRUE;
    }
    return FALSE;
}

HRESULT CDX9::Reset()
{
HRESULT hr;
	if (!m_pD3D)
		return E_FAIL;
	if (!m_pd3dDevice)
		return E_FAIL;

	CleanupD3D_Surfaces();

	hr = m_pd3dDevice->Reset(&m_d3dpp);
	if (FAILED(hr))
		return hr;

	D3DDISPLAYMODE currentDisplayMode;
	hr = m_pd3dDevice->GetDisplayMode(0, &currentDisplayMode);
	if (FAILED(hr))
		return hr;
	
	hr = SetRenderStyle(m_d3dpp.Windowed, m_bDoubleSizedWindow, m_borderSize, m_bShowFloppyLed, m_bUseBlitStretch, m_stretch, m_filter, currentDisplayMode);
	if(FAILED(hr))
	{
		return hr;
	}
	hr = LoadTextures(currentDisplayMode.Format);
	if(FAILED(hr))
	{
		return hr;
	}

	return S_OK;
}

HRESULT CDX9::GetAdapterFromWindow(IDirect3D9 *pD3D, HWND hWndDevice, UINT& adapterNumber)
{
UINT iNumberOfAdapters = pD3D->GetAdapterCount();
	adapterNumber = D3DADAPTER_DEFAULT;
	if (G::s_pFnMonitorFromWindow != NULL)
	{
		HMONITOR hMonitorInUse = G::s_pFnMonitorFromWindow(hWndDevice, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFOEX miMonitorInUse;
		MONITORINFOEX miMonitorToCheck;
		ZeroMemory(&miMonitorInUse, sizeof(miMonitorInUse));
		miMonitorInUse.cbSize = sizeof(miMonitorInUse);
		if (DXUTGetMonitorInfo(hMonitorInUse, &miMonitorInUse))
		{
			for (UINT i = 0; i < iNumberOfAdapters; i++)
			{
				HMONITOR hMonitorAdapterToCheck = pD3D->GetAdapterMonitor(i);
				if (hMonitorAdapterToCheck == hMonitorInUse)
				{
					adapterNumber = i;
					return S_OK;
				}
				ZeroMemory(&miMonitorToCheck, sizeof(miMonitorToCheck));
				miMonitorToCheck.cbSize = sizeof(miMonitorToCheck);
				if (DXUTGetMonitorInfo(hMonitorAdapterToCheck, &miMonitorToCheck))
				{
					if (_tcsncicmp(miMonitorToCheck.szDevice,  miMonitorInUse.szDevice, _countof(miMonitorInUse.szDevice)) == 0)
					{
						adapterNumber = i;
						return S_OK;
					}
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT CDX9::InitD3D(HWND hWndDevice, HWND hWndFocus, BOOL bWindowedMode, BOOL bDoubleSizedWindow, HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, HCFG::FULLSCREENSYNCMODE syncMode, DWORD adapterNumber, GUID fullscreenAdapterId, const D3DDISPLAYMODE &displayMode)
{
D3DPRESENT_PARAMETERS d3dpp;
D3DDISPLAYMODE chooseDisplayMode;
HRESULT hr;
UINT iNumberOfAdapters;
GUID empty;

	ZeroMemory(&empty, sizeof(empty));

	// Create the D3D object, which is needed to create the D3DDevice.
	if (m_pD3D == NULL)
	{
		if( NULL == ( m_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
			return E_FAIL;
	}

	iNumberOfAdapters = m_pD3D->GetAdapterCount();
	if (iNumberOfAdapters == 0)
		return E_FAIL;

	if (adapterNumber < 0 || adapterNumber >= iNumberOfAdapters)
		adapterNumber = D3DADAPTER_DEFAULT;

	if (!bWindowedMode && fullscreenAdapterId == empty || adapterNumber < 0 || adapterNumber >= iNumberOfAdapters)
	{
		UINT autoSelectAdapter = D3DADAPTER_DEFAULT;
		hr = GetAdapterFromWindow(m_pD3D, hWndDevice, autoSelectAdapter);
		if (SUCCEEDED(hr))
			adapterNumber = autoSelectAdapter;
	}

	if (bWindowedMode)
	{
		// Set up the structure used to create the D3DDevice. Most parameters are
		// zeroed out. We set Windowed to TRUE, since we want to do D3D in a
		// window, and then set the SwapEffect to "discard", which is the most
		// efficient method of presenting the back buffer to the display.  And 
		// we request a back buffer format that matches the current desktop display 
		// format.
		ZeroMemory( &d3dpp, sizeof(d3dpp) );
		d3dpp.Windowed = TRUE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;//D3DSWAPEFFECT_COPY;//D3DSWAPEFFECT_FLIP;//D3DSWAPEFFECT_DISCARD;
		d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		d3dpp.hDeviceWindow = hWndDevice;
		d3dpp.Flags = 0;D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		if (syncMode == HCFG::FSSM_VBL)
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		// Create the Direct3D device. Here we are using the default adapter (most
		// systems only have one, unless they have multiple graphics hardware cards
		// installed) and requesting the HAL (which is saying we want the hardware
		// device rather than a software one). Software vertex processing is 
		// specified since we know it will work on all cards. On cards that support 
		// hardware vertex processing, though, we would see a big performance gain 
		// by specifying hardware vertex processing.
		if (m_pd3dDevice != NULL)
		{
			if (FAILED(m_pd3dDevice->Reset(&d3dpp)))
			{
				CleanupD3D_Devices();
			}
		}
		if (m_pd3dDevice == NULL)
		{
			if (FAILED(hr = m_pD3D->CreateDevice(adapterNumber, D3DDEVTYPE_HAL, hWndFocus, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
			{
				if (FAILED(m_pD3D->CreateDevice(adapterNumber, D3DDEVTYPE_REF, hWndFocus, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
					return E_FAIL;
			}
		}
		if(FAILED(m_pD3D->GetAdapterDisplayMode(adapterNumber, &chooseDisplayMode)))
		{
			return E_FAIL;
		}
	}
	else
	{
		if (!CanDisplayManualMode(adapterNumber, displayMode, chooseDisplayMode))
			if (!CanDisplayCurrentMode(adapterNumber, chooseDisplayMode, adapterNumber))
				if (!CanDisplayOtherMode(adapterNumber, chooseDisplayMode))
					return E_FAIL;

		HMONITOR hMonitorAdapter = m_pD3D->GetAdapterMonitor(adapterNumber);
		MONITORINFO mi;
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		if (this->DXUTGetMonitorInfo(hMonitorAdapter, &mi))
		{
			SetWindowPos(hWndFocus, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top , mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_SHOWWINDOW);
		}
		else
		{
			SetWindowPos(hWndFocus, HWND_TOPMOST, 0, 0, chooseDisplayMode.Width, chooseDisplayMode.Height, SWP_SHOWWINDOW);
		}

		ZeroMemory( &d3dpp, sizeof(d3dpp) );
		d3dpp.Windowed = FALSE;
		d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		d3dpp.Flags = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		d3dpp.BackBufferCount = 1;
		d3dpp.BackBufferHeight = chooseDisplayMode.Height;
		d3dpp.BackBufferWidth = chooseDisplayMode.Width;
		d3dpp.BackBufferFormat = chooseDisplayMode.Format;
		d3dpp.FullScreen_RefreshRateInHz = chooseDisplayMode.RefreshRate;
		if (syncMode == HCFG::FSSM_VBL)
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
		else
			d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

		if (m_pd3dDevice != NULL)
		{
			if (FAILED(m_pd3dDevice->Reset(&d3dpp)))
			{
				CleanupD3D_Devices();
			}
		}
		if (m_pd3dDevice == NULL)
		{
			if( FAILED( m_pD3D->CreateDevice(adapterNumber, D3DDEVTYPE_HAL, hWndFocus, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice ) ) )
			{
				return E_FAIL;
			}
		}
	}

	hr = SetRenderStyle(bWindowedMode, bDoubleSizedWindow, borderSize, bShowFloppyLed, bUseBlitStretch, stretch, filter, chooseDisplayMode);
	if (FAILED(hr))
	{
		return E_FAIL;
	}

	hr = LoadTextures(chooseDisplayMode.Format);
	if(FAILED(hr))
	{
		return hr;
	}

	m_borderSize = borderSize;
	m_bShowFloppyLed = bShowFloppyLed;
	m_bDoubleSizedWindow = bDoubleSizedWindow;
	m_bUseBlitStretch = bUseBlitStretch;
	m_filter = filter;
	m_stretch = stretch;
	m_d3dpp = d3dpp;

    return S_OK;
}

bool CDX9::CanDisplayManualMode(DWORD adapterNumber, const D3DDISPLAYMODE &displayMode, D3DDISPLAYMODE &chooseDisplayMode)
{
HRESULT hr;
D3DDISPLAYMODE currentDisplayMode , mode;
	//Manually select display mode.


	if(FAILED(m_pD3D->GetAdapterDisplayMode(adapterNumber, &currentDisplayMode)))
	{
		return false;
	}

	mode = displayMode;
	if (mode.Width != 0 && mode.Height !=0)
	{
		if (mode.Format == D3DFMT_UNKNOWN)
			mode.Format = currentDisplayMode.Format;
		if (IsAcceptableMode(mode))
		{
			hr = m_pD3D->CheckDeviceType(adapterNumber, D3DDEVTYPE_HAL, mode.Format, mode.Format, FALSE);
			if (SUCCEEDED(hr))
			{
				chooseDisplayMode = mode;
				return true;
			}
		}
	}
	return false;
}

bool CDX9::CanDisplayCurrentMode(DWORD adapterNumber, D3DDISPLAYMODE &chooseDisplayMode, DWORD &chooseAdapterNumber)
{
D3DDISPLAYMODE currentDisplayMode, displayMode;
HRESULT hr;
HMONITOR hMonitor = NULL;
bool r = false;
	if(FAILED(m_pD3D->GetAdapterDisplayMode(adapterNumber, &currentDisplayMode)))
	{
		return false;
	}

	hMonitor = m_pD3D->GetAdapterMonitor(adapterNumber);
	if (hMonitor == NULL)
	{
		return false;
	}

	MONITORINFO mi;
	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	displayMode = currentDisplayMode;
	if (CDX9::DXUTGetMonitorInfo(hMonitor, &mi))
	{
		displayMode.Height = labs(mi.rcMonitor.bottom - mi.rcMonitor.top);
		displayMode.Width = labs(mi.rcMonitor.right - mi.rcMonitor.left);
		if (IsAcceptableMode(displayMode))
		{
			hr = m_pD3D->CheckDeviceType(adapterNumber, D3DDEVTYPE_HAL, displayMode.Format, displayMode.Format, FALSE);
			if (SUCCEEDED(hr))
			{
				chooseDisplayMode = displayMode;
				chooseAdapterNumber = adapterNumber;
				return true;
			}
		}
	}
	else
	{
		if (CDX9::DXUTGetMonitorInfo(DXUT_PRIMARY_MONITOR, &mi))
		{
			displayMode.Height = labs(mi.rcMonitor.bottom - mi.rcMonitor.top);
			displayMode.Width = labs(mi.rcMonitor.right - mi.rcMonitor.left);							
			hr = m_pD3D->CheckDeviceType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, displayMode.Format, FALSE);
			if (SUCCEEDED(hr))
			{
				chooseDisplayMode = displayMode;
				chooseAdapterNumber = D3DADAPTER_DEFAULT;
				return true;
			}
		}
	}
	
	return false;
}

bool CDX9::CanDisplayOtherMode(DWORD adapterNumber, D3DDISPLAYMODE &chooseDisplayMode)
{
HRESULT hr;
D3DDISPLAYMODE displayMode;
	hr = ChooseDisplayMode(adapterNumber, &displayMode);
	if (SUCCEEDED(hr))
	{
		chooseDisplayMode = displayMode;
		chooseDisplayMode.RefreshRate = 0;
		return true;
	}
	return false;
}

bool CDX9::IsAcceptableMode(const D3DDISPLAYMODE &displayMode)
{
	//return (displayMode.Width >= 400 && displayMode.Height >= 300 && displayMode.Format != D3DFMT_A8P8 && displayMode.Format != D3DFMT_P8 && GetBitsPerPixel(displayMode.Format) > 0);
	return (displayMode.Width >= 320 && displayMode.Height >= 200 && GetBitsPerPixel(displayMode.Format) >= 16);
}

bool CDX9::CanMode1X(const D3DDISPLAYMODE &displayMode, const C64WindowDimensions &dims, BOOL bShowFloppyLed)
{
	if (displayMode.Width < (UINT)dims.Width*1 || displayMode.Height < (UINT)(dims.Height*1 + GetToolBarHeight(bShowFloppyLed)))
		return false;
	else
		return true;
}

bool CDX9::CanMode2X(const D3DDISPLAYMODE &displayMode, const C64WindowDimensions &dims, BOOL bShowFloppyLed)
{
	if (displayMode.Width < (UINT)dims.Width*2 || displayMode.Height < (UINT)(dims.Height*2 + GetToolBarHeight(bShowFloppyLed)))
		return false;
	else
		return true;
}

UINT CDX9::GetDisplayResolutionText(const D3DDISPLAYMODE &displayMode, LPTSTR buffer, UINT charBufferLen)
{
TCHAR sMode[20];

	UINT r = _sntprintf_s(sMode, _countof(sMode), _TRUNCATE, TEXT("%d x %d"), (int)displayMode.Width, (int)displayMode.Height);
	if (charBufferLen == 0)
		charBufferLen = r + 1;
	else if ((r + 1) > charBufferLen)
		charBufferLen = r + 1;

	if (buffer!=NULL)
		_tcscpy_s(buffer, charBufferLen, &sMode[0]);
	return charBufferLen;
}

UINT CDX9::GetDisplayFormatText(const D3DDISPLAYMODE &displayMode, LPTSTR buffer, UINT charBufferLen)
{
TCHAR sMode[20];

	switch(displayMode.Format)
	{
		case D3DFMT_A8R8G8B8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp A8R8G8B8"));
			break;
		case D3DFMT_X8R8G8B8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp X8R8G8B8"));
			break;
		case D3DFMT_A8B8G8R8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp A8B8G8R8"));
			break;
		case D3DFMT_X8B8G8R8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp X8B8G8R8"));
			break;
		case D3DFMT_A2B10G10R10:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp A2B10G10R10"));
			break;
		case D3DFMT_A2R10G10B10:
			_tcscpy_s(sMode, _countof(sMode), TEXT("32bpp A2R10G10B10"));
			break;
		case D3DFMT_R8G8B8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("24bpp R8G8B8"));
			break;
		case D3DFMT_X1R5G5B5:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp X1R5G5B5"));
			break;
		case D3DFMT_A1R5G5B5:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp A1R5G5B5"));
			break;
		case D3DFMT_R5G6B5:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp R5G6B5"));
			break;
		case D3DFMT_X4R4G4B4:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp X4R4G4B4"));
			break;
		case D3DFMT_A4R4G4B4:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp A4R4G4B4"));
			break;
		case D3DFMT_A8R3G3B2:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp A8R3G3B2"));
			break;
		case D3DFMT_A8P8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("16bpp A8P8"));
			break;
		case D3DFMT_R3G3B2:
			_tcscpy_s(sMode, _countof(sMode), TEXT("8bpp R3G3B2"));
			break;
		case D3DFMT_P8:
			_tcscpy_s(sMode, _countof(sMode), TEXT("8bpp P8"));
			break;
		default:
			_tcscpy_s(sMode, _countof(sMode), TEXT("?"));
			break;
	}
	UINT r = lstrlen(sMode);

	if (charBufferLen == 0)
		charBufferLen = r + 1;
	else if ((r + 1) > charBufferLen)
		charBufferLen = r + 1;

	if (buffer!=NULL)
		_tcscpy_s(buffer, charBufferLen, &sMode[0]);
	return charBufferLen;
}

HRESULT CDX9::SetRenderStyle(BOOL bWindowedMode, BOOL bDoubleSizedWindow, HCFG::EMUBORDERSIZE borderSize, BOOL bShowFloppyLed, BOOL bUseBlitStretch, HCFG::EMUWINDOWSTRETCH stretch, D3DTEXTUREFILTERTYPE filter, D3DDISPLAYMODE currentDisplayMode)
{
HRESULT hr;
C64WindowDimensions  dims;
	m_iEraseCount = 0;
	SetRect(&m_rcTargetRect, 0, 0, 0, 0);
	D3DRECT drcEraseRects[5];//Top,Bottom,Left,Right,optional toolbar
	ZeroMemory(&drcEraseRects[0], sizeof(D3DRECT) * _countof(drcEraseRects));
	FreeSmallSurface();
	FreeSysMemSurface();

	dims.SetBorder(borderSize);
	m_displayXPos = 0;
	m_displayYPos = 0;
	m_displayWidth = dims.Width;
	m_displayHeight = dims.Height;
	m_displayFirstVicRaster = dims.FirstRasterLine;
	m_displayLastVicRaster = dims.LastRasterLine;
	m_displayStart = dims.Start;

	if (bWindowedMode)
	{
		if (bDoubleSizedWindow)
		{
			SetRect(&m_rcTargetRect, 0, 0, dims.Width *2, dims.Height *2);
			if (bUseBlitStretch) // bWindowedMode && bDoubleSizedWindow && bUseBlitStretch
			{
				hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
				if (FAILED(hr))
					return hr;
				CheckFilterCap(true, filter);

				m_appStatus->m_bUseCPUDoubler = false;

			}
			else // bWindowedMode && bDoubleSizedWindow && !bUseBlitStretch
			{
				hr = CreateSmallSurface(dims.Width*2, dims.Height*2, currentDisplayMode.Format);
				if (FAILED(hr))
					return hr;

				m_appStatus->m_bUseCPUDoubler = true;
			}

			m_drcStatusBar.x1 = 0;
			m_drcStatusBar.y1 = dims.Height*2;
			m_drcStatusBar.x2 = dims.Width*2;
			m_drcStatusBar.y2 = dims.Height*2 + GetToolBarHeight(bShowFloppyLed);

		}
		else // bWindowedMode && !bDoubleSizedWindow
		{
			SetRect(&m_rcTargetRect, 0, 0, dims.Width, dims.Height);
			hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
			if (FAILED(hr))
				return hr;

			m_appStatus->m_bUseCPUDoubler = false;

			m_drcStatusBar.x1 = 0;
			m_drcStatusBar.y1 = dims.Height;
			m_drcStatusBar.x2 = dims.Width;
			m_drcStatusBar.y2 = dims.Height + GetToolBarHeight(bShowFloppyLed);
		}
		memcpy(&drcEraseRects[0], &m_drcStatusBar, sizeof(D3DRECT));
	}
	else //Fullscreen
	{
		if (currentDisplayMode.Width < 320 || currentDisplayMode.Height < 200)
			return E_FAIL;

		if (stretch == HCFG::EMUWINSTR_AUTO)
		{
			if (bUseBlitStretch)
			{
				stretch = HCFG::EMUWINSTR_ASPECTSTRETCH;
			}
			else
			{
				stretch = HCFG::EMUWINSTR_2X;
			}
		}

		if ((stretch == HCFG::EMUWINSTR_2X) && CanMode2X(currentDisplayMode, dims, bShowFloppyLed)) // Fullscreen with pixel doubling
		{			
			if (bUseBlitStretch) // DirectX Stretch
			{ 
				hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
				if (FAILED(hr))
					return hr;

				CheckFilterCap(true, filter);

				m_appStatus->m_bUseCPUDoubler = false;
			}
			else //CPU Stretch
			{ 
				hr = CreateSmallSurface(dims.Width*2, dims.Height*2, currentDisplayMode.Format);
				if (FAILED(hr))
					return hr;
				m_appStatus->m_bUseCPUDoubler = true;
			}
			CalcClearingRects(currentDisplayMode, dims, 2L, bShowFloppyLed, m_rcTargetRect, drcEraseRects, m_drcStatusBar);
		}
		else if (((stretch == HCFG::EMUWINSTR_1X) || (stretch == HCFG::EMUWINSTR_2X)) && CanMode1X(currentDisplayMode, dims, bShowFloppyLed)) // Fullscreen no pixel doubling
		{
			hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
			if (FAILED(hr))
				return hr;

			m_appStatus->m_bUseCPUDoubler = false;

			CalcClearingRects(currentDisplayMode, dims, 1L, bShowFloppyLed, m_rcTargetRect, drcEraseRects, m_drcStatusBar);
		}
		else if ((stretch == HCFG::EMUWINSTR_1X) || (stretch == HCFG::EMUWINSTR_2X)) // Fullscreen no pixel doubling cut fit
		{
			dims.SetBorder(currentDisplayMode.Width, currentDisplayMode.Height, GetToolBarHeight(bShowFloppyLed));
			m_displayFirstVicRaster = dims.FirstRasterLine;
			m_displayLastVicRaster = dims.LastRasterLine;
			m_displayWidth = dims.Width;
			m_displayHeight = dims.Height;
			m_displayStart = dims.Start;
			m_appStatus->m_bUseCPUDoubler = false;
		
			hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
			if (FAILED(hr))
				return hr;

			CalcClearingRects(currentDisplayMode, dims, 1L, bShowFloppyLed, m_rcTargetRect, drcEraseRects, m_drcStatusBar);
		}
		else // Aspect Stretch
		{
			hr = CreateSmallSurface(dims.Width, dims.Height, currentDisplayMode.Format);
			if (FAILED(hr))
				return hr;
			CheckFilterCap(currentDisplayMode.Width >= (UINT)dims.Width, filter);
			m_appStatus->m_bUseCPUDoubler = false;
			
			CalcStretchToFitClearingRects(currentDisplayMode, dims, bShowFloppyLed, m_rcTargetRect, drcEraseRects, m_drcStatusBar);
		}

	}
	SetClearingRects(&drcEraseRects[0], _countof(drcEraseRects));

	m_appStatus->m_displayFormat = (DWORD)currentDisplayMode.Format;
	m_appStatus->m_ScreenDepth = GetBitsPerPixel(currentDisplayMode.Format);


	m_vecPositionLedMotor.x = (FLOAT)m_drcStatusBar.x1;
	m_vecPositionLedMotor.y = (FLOAT)(m_drcStatusBar.y1 + 1);
	m_vecPositionLedMotor.z = 0;

	m_vecPositionLedDrive.x = (FLOAT)(16 + m_drcStatusBar.x1);
	m_vecPositionLedDrive.y = (FLOAT)( m_drcStatusBar.y1 + 1);
	m_vecPositionLedDrive.z = 0;

	m_vecPositionLedWrite.x = (FLOAT)(32 + m_drcStatusBar.x1);
	m_vecPositionLedWrite.y = (FLOAT)( m_drcStatusBar.y1 + 1);
	m_vecPositionLedWrite.z = 0;	

    // Device state would normally be set here
    // Turn off culling
    m_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

    // Turn off D3D lighting
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, FALSE );

    // Turn off the zbuffer
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE, FALSE );

	if (currentDisplayMode.Format == D3DFMT_P8)
	{
		if(FAILED(m_pd3dDevice->SetPaletteEntries(0, &m_paletteEntry[0])))
		{
			return E_FAIL;
		}
	}

	return S_OK;
}

void CDX9::CalcClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, const DWORD scale, BOOL bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar)
{
	int tHeight = GetToolBarHeight(bShowFloppyLed);

	rcTargetRect.top = ((dims.Height * scale + tHeight)) < mode.Height ? (mode.Height - (dims.Height * scale + tHeight)) / 2 : 0;
	rcTargetRect.bottom = (rcTargetRect.top + dims.Height *scale) < mode.Height ? rcTargetRect.top + dims.Height *scale : mode.Height;
	rcTargetRect.left = (dims.Width*scale) < mode.Width ? (mode.Width - dims.Width*scale) / 2 : 0;
	rcTargetRect.right = (rcTargetRect.left + dims.Width *scale) < mode.Width ? rcTargetRect.left + dims.Width *scale : mode.Width;

	//Top
	drcEraseRects[0].x1 = 0;
	drcEraseRects[0].y1 = 0;
	drcEraseRects[0].x2 = mode.Width;
	drcEraseRects[0].y2 = rcTargetRect.top;

	//Bottom
	drcEraseRects[1].x1 = 0;
	drcEraseRects[1].y1 = rcTargetRect.bottom;
	drcEraseRects[1].x2 = mode.Width;
	drcEraseRects[1].y2 = mode.Height;

	//Left
	drcEraseRects[2].x1 = 0;
	drcEraseRects[2].y1 = drcEraseRects[0].y2;
	drcEraseRects[2].x2 = rcTargetRect.left;
	drcEraseRects[2].y2 = drcEraseRects[1].y1;

	//Right
	drcEraseRects[3].x1 = rcTargetRect.right;
	drcEraseRects[3].y1 = drcEraseRects[0].y2;
	drcEraseRects[3].x2 = mode.Width;
	drcEraseRects[3].y2 = drcEraseRects[1].y1;

	
	if (rcTargetRect.bottom + tHeight > (int)mode.Height)
	{
		drcStatusBar.x1 = rcTargetRect.left;
		drcStatusBar.y1 = mode.Height - tHeight;
		drcStatusBar.x2 = rcTargetRect.right;
		drcStatusBar.y2 = mode.Height;
	}
	else
	{
		drcStatusBar.x1 = rcTargetRect.left;
		drcStatusBar.y1 = rcTargetRect.bottom;
		drcStatusBar.x2 = rcTargetRect.right;
		drcStatusBar.y2 = rcTargetRect.bottom + tHeight;
	}
}
void CDX9::SetClearingRects(D3DRECT rects[], int count)
{
	m_iEraseCount = 0;
	for (int i = 0; i<count ; i++)
	{
		if (rects[i].x1 < rects[i].x2 && rects[i].y1 < rects[i].y2) 
		{
			memcpy_s(&m_drcEraseRects[m_iEraseCount], sizeof(D3DRECT), &rects[i], sizeof(D3DRECT));
			m_iEraseCount++;
		}
	}
}
void CDX9::CalcStretchToFitClearingRects(const D3DDISPLAYMODE& mode, const C64WindowDimensions &dims, BOOL bShowFloppyLed, RECT& rcTargetRect, D3DRECT drcEraseRects[], D3DRECT& drcStatusBar)
{
	int tHeight = GetToolBarHeight(bShowFloppyLed);
	double c64ratio, screenratio;
	c64ratio = (double)dims.Width / (double)dims.Height;
	screenratio = (double)mode.Width / (double)(mode.Height - tHeight);
	if (c64ratio <= screenratio)
	{
		rcTargetRect.top = 0;
		rcTargetRect.bottom = mode.Height - tHeight;
		rcTargetRect.left = (mode.Width - ((DWORD)(c64ratio * ((double)(mode.Height - tHeight))))) / 2L;
		rcTargetRect.right = mode.Width - m_rcTargetRect.left;
	}
	else
	{
		rcTargetRect.top = (mode.Height - tHeight - ((DWORD)((1.0 / c64ratio) * ((double)(mode.Width))))) / 2L;
		rcTargetRect.bottom = mode.Height - tHeight - m_rcTargetRect.top;
		rcTargetRect.left = 0;
		rcTargetRect.right = mode.Width;
	}


	//Top
	drcEraseRects[0].x1 = 0;
	drcEraseRects[0].y1 = 0;
	drcEraseRects[0].x2 = mode.Width;
	drcEraseRects[0].y2 = rcTargetRect.top;

	//Bottom
	drcEraseRects[1].x1 = 0;
	drcEraseRects[1].y1 = rcTargetRect.bottom;
	drcEraseRects[1].x2 = mode.Width;
	drcEraseRects[1].y2 = mode.Height;

	//Left
	drcEraseRects[2].x1 = 0;
	drcEraseRects[2].y1 = drcEraseRects[0].y2;
	drcEraseRects[2].x2 = rcTargetRect.left;
	drcEraseRects[2].y2 = drcEraseRects[1].y1;

	//Right
	drcEraseRects[3].x1 = rcTargetRect.right;
	drcEraseRects[3].y1 = drcEraseRects[0].y2;
	drcEraseRects[3].x2 = mode.Width;
	drcEraseRects[3].y2 = drcEraseRects[1].y1;

	if (rcTargetRect.bottom + tHeight > (int)mode.Height)
	{
		drcStatusBar.x1 = rcTargetRect.left;
		drcStatusBar.y1 = mode.Height - tHeight;
		drcStatusBar.x2 = rcTargetRect.right;
		drcStatusBar.y2 = mode.Height;
	}
	else
	{
		drcStatusBar.x1 = rcTargetRect.left;
		drcStatusBar.y1 = rcTargetRect.bottom;
		drcStatusBar.x2 = rcTargetRect.right;
		drcStatusBar.y2 = rcTargetRect.bottom + tHeight;
	}
}

int CDX9::GetToolBarHeight(BOOL bShowFloppyLed)
{
	return bShowFloppyLed ? m_iToolbarHeight : 0;
}

void CDX9::CheckFilterCap(bool bIsMagnifying, D3DTEXTUREFILTERTYPE filter)
{
D3DCAPS9 caps;
DWORD MagOrMinFilterCapPoint;
DWORD MagOrMinFilterCapLinear;

	ZeroMemory(&caps, sizeof(caps));
	if (SUCCEEDED(m_pd3dDevice->GetDeviceCaps(&caps)))
	{
		if (bIsMagnifying)
		{
			MagOrMinFilterCapPoint = D3DPTFILTERCAPS_MAGFPOINT;
			MagOrMinFilterCapLinear = D3DPTFILTERCAPS_MAGFLINEAR;
		}
		else
		{
			MagOrMinFilterCapPoint = D3DPTFILTERCAPS_MINFPOINT;
			MagOrMinFilterCapLinear = D3DPTFILTERCAPS_MINFLINEAR;
		}

		switch(filter)
		{
		case D3DTEXF_NONE:
			m_appStatus->m_blitFilter = D3DTEXF_NONE;
			break;
		case D3DTEXF_POINT:
			if (caps.StretchRectFilterCaps & MagOrMinFilterCapPoint)
				m_appStatus->m_blitFilter = D3DTEXF_POINT;
			else
				m_appStatus->m_blitFilter = D3DTEXF_NONE;
			break;
		default:
			if (caps.StretchRectFilterCaps & MagOrMinFilterCapLinear)
				m_appStatus->m_blitFilter = D3DTEXF_LINEAR;
			else if (caps.StretchRectFilterCaps & MagOrMinFilterCapPoint)
				m_appStatus->m_blitFilter = D3DTEXF_POINT;
			else
				m_appStatus->m_blitFilter = D3DTEXF_NONE;
			break;
		}
	}
	else
	{
		m_appStatus->m_blitFilter = D3DTEXF_POINT;
	}
}

HRESULT CDX9::ChooseDisplayMode(DWORD adapterNumber, D3DDISPLAYMODE *pMode)
{
HRESULT hRet;
long i;

	ClearError();

	m_diplaymodes_count=0;

	hRet = ListDisplayMode(adapterNumber);
	if (FAILED(hRet))
		return hRet;

	if (m_diplaymodes_count<=0)
		return E_FAIL;

	while (1)
	{	
		/*
		i= CheckForMode(320,240, 32, true);
		if (i>=0)
		{
			*ScreenType = stRGB32_320X240;
			break;
		}
		break;
		*/
		i= CheckForMode(400,300, 8, false);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(400,300, 16, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(400,300, 32, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(800,600, 8, false);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(800,600, 16, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(800,600, 32, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(1024,768, 8, false);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(1024,768, 16, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(1024,768, 32, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,240, 8, false);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,240, 16, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,240, 32, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,200, 8, false);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,200, 16, true);
		if (i>=0)
		{
			break;
		}
		i= CheckForMode(320,200, 32, true);
		if (i>=0)
		{
			break;
		}
		break;
	}
	if (i>=0) 
	{
		memcpy_s(pMode, sizeof(D3DDISPLAYMODE), &m_pDiplaymodes[i], sizeof(D3DDISPLAYMODE));
		return S_OK;
	}
	else
		return E_FAIL;
}

long CDX9::CheckForMode(long width,long height, long depth, bool is_rgb)
{
long i;
	if (m_diplaymodes_count <= 0)
		return -1;
	for (i=0 ; i < m_diplaymodes_count ; i++)
	{
		if (m_pDiplaymodes[i].Width ==width  && m_pDiplaymodes[i].Height == height)
		{
			switch (m_pDiplaymodes[i].Format)
			{
			case D3DFMT_P8:
				if (depth == 8 && !is_rgb)
					return i;
				break;
			case D3DFMT_R5G6B5:
				if (depth == 16 && is_rgb)
					return i;
				break;
			case D3DFMT_X1R5G5B5:
				if (depth == 16 && is_rgb)
					return i;
				break;
			case D3DFMT_R8G8B8:
				if (depth == 24 && is_rgb)
					return i;
				break;
			case D3DFMT_X8R8G8B8:
				if (depth == 32 && is_rgb)
					return i;
				break;
			}
		}
	}
	return -1;
}

HRESULT CDX9::ListDisplayMode(DWORD adapterNumber)
{
D3DDISPLAYMODE mode;
HRESULT hr;
UINT i , j , k;
D3DFORMAT fmt;

	m_diplaymodes_count=0;
	if (m_pDiplaymodes != NULL)
	{
		LocalFree(m_pDiplaymodes);
		m_pDiplaymodes = NULL;
	}

	//fmt = (D3DFORMAT)(D3DFMT_R5G6B5 | D3DFMT_X1R5G5B5 | D3DFMT_R8G8B8 | D3DFMT_X8R8G8B8 | D3DFMT_P8);
	//No palette mode
	fmt = (D3DFORMAT)(D3DFMT_R5G6B5 | D3DFMT_X1R5G5B5 | D3DFMT_R8G8B8 | D3DFMT_X8R8G8B8);
	j = m_pD3D->GetAdapterModeCount(adapterNumber, fmt);

	m_pDiplaymodes = (D3DDISPLAYMODE *)LocalAlloc(LMEM_FIXED, sizeof(D3DDISPLAYMODE) * j);
	k = 0;
	for (i = 0; i < j; i++)
	{
		hr = m_pD3D->EnumAdapterModes(adapterNumber, fmt, i, &mode);
		if (FAILED(hr))
		{
			continue;
		}
		memcpy_s(&m_pDiplaymodes[k], sizeof(D3DDISPLAYMODE), &mode, sizeof(D3DDISPLAYMODE));
		k++;
	}
	m_diplaymodes_count = k;
	return S_OK;
}

DWORD CDX9::GetBitsPerPixel(D3DFORMAT fmt)
{
	switch (fmt)
	{
	case D3DFMT_A8R8G8B8:
	case D3DFMT_X8R8G8B8:
	case D3DFMT_A8B8G8R8:
	case D3DFMT_X8B8G8R8:
	case D3DFMT_A2B10G10R10:
	case D3DFMT_A2R10G10B10:
		return 32;
	case D3DFMT_R8G8B8:
		return 24;
	case D3DFMT_X1R5G5B5:
	case D3DFMT_A1R5G5B5:
	case D3DFMT_R5G6B5:
	case D3DFMT_X4R4G4B4:
	case D3DFMT_A4R4G4B4:
	case D3DFMT_A8R3G3B2:
	case D3DFMT_A8P8:
		return 16;
	case D3DFMT_R3G3B2:
	case D3DFMT_P8:
		return 8;
	default:
		return 0;
	}
}
DWORD CDX9::DDColorMatch(IDirect3DSurface9* pdds, COLORREF rgb)
{
    COLORREF                rgbT;
    HDC                     hdc;
    DWORD                   dw = CLR_INVALID;
    D3DSURFACE_DESC         ddsd;
    HRESULT                 hr;
	D3DLOCKED_RECT			rect;
	DWORD                   bpp;
    //
    //  Use GDI SetPixel to color match for us
    //

	dw = 0;
	hr = pdds->GetDesc(&ddsd);
	if (FAILED(hr))
		return 0;

    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == D3D_OK)
    {
        rgbT = GetPixel(hdc, 0, 0);     // Save current pixel value
        SetPixel(hdc, 0, 0, rgb);       // Set our value
        pdds->ReleaseDC(hdc);
    }
	
    //
    // Now lock the surface so we can read back the converted color
    //
	hr = pdds->LockRect(&rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK | D3DLOCK_READONLY);
	if (hr == D3D_OK)
	{
		dw = *(DWORD *) rect.pBits;                 // Get DWORD
		bpp = GetBitsPerPixel(ddsd.Format);
        if (bpp < 32)
            dw &= (1 << bpp) - 1;  // Mask it to bpp
		pdds->UnlockRect();
	}

    //
    //  Now put the color that was there back.
    //
    if (rgb != CLR_INVALID && pdds->GetDC(&hdc) == D3D_OK)
    {
        SetPixel(hdc, 0, 0, rgbT);
        pdds->ReleaseDC(hdc);
    }

    return dw;
}

DWORD CDX9::CheckDXVersion9()
{
TCHAR strDirectXVersion[10];
DWORD dxVersion;
HRESULT hr;

	hr = GetDXVersion(&dxVersion, strDirectXVersion, 10);
	if (dxVersion < 0x0900)
	{
		MessageBox(0L, TEXT("You need to install the full version of DirectX 9 or higher. Press OK to quit."), APPNAME, MB_ICONSTOP);
	}
	return dxVersion;
}

HRESULT CDX9::GetDXVersion( DWORD* pdwDirectXVersion, TCHAR* strDirectXVersion, int cchDirectXVersion )
{
    bool bGotDirectXVersion = false;

    // Init values to unknown
    if( pdwDirectXVersion )
        *pdwDirectXVersion = 0;
    if( strDirectXVersion && cchDirectXVersion > 0 )
        strDirectXVersion[0] = 0;

    DWORD dwDirectXVersionMajor = 0;
    DWORD dwDirectXVersionMinor = 0;
    TCHAR cDirectXVersionLetter = ' ';

    // First, try to use dxdiag's COM interface to get the DirectX version.
    // The only downside is this will only work on DirectX9 or later.
    if( SUCCEEDED( GetDirectXVersionViaDxDiag( &dwDirectXVersionMajor, &dwDirectXVersionMinor, &cDirectXVersionLetter ) ) )
        bGotDirectXVersion = true;

    if( !bGotDirectXVersion )
    {
        // Getting the DirectX version info from DxDiag failed,
        // so most likely we are on DirectX8.x or earlier
        if( SUCCEEDED( GetDirectXVersionViaFileVersions( &dwDirectXVersionMajor, &dwDirectXVersionMinor, &cDirectXVersionLetter ) ) )
            bGotDirectXVersion = true;
    }

    // If both techniques failed, then return E_FAIL
    if( !bGotDirectXVersion )
        return E_FAIL;

    // Set the output values to what we got and return
    cDirectXVersionLetter = (char)tolower(cDirectXVersionLetter);

    if( pdwDirectXVersion )
    {
        // If pdwDirectXVersion is non-NULL, then set it to something
        // like 0x00080102 which would represent DirectX8.1b
        DWORD dwDirectXVersion = dwDirectXVersionMajor;
        dwDirectXVersion <<= 8;
        dwDirectXVersion += dwDirectXVersionMinor;
        dwDirectXVersion <<= 8;
        if( cDirectXVersionLetter >= 'a' && cDirectXVersionLetter <= 'z' )
            dwDirectXVersion += (cDirectXVersionLetter - 'a') + 1;

        *pdwDirectXVersion = dwDirectXVersion;
    }

    if( strDirectXVersion && cchDirectXVersion > 0 )
    {
        // If strDirectXVersion is non-NULL, then set it to something
        // like "8.1b" which would represent DirectX8.1b
        if( cDirectXVersionLetter == ' ' )
            StringCchPrintf( strDirectXVersion, cchDirectXVersion, TEXT("%d.%d"), dwDirectXVersionMajor, dwDirectXVersionMinor );
        else
            StringCchPrintf( strDirectXVersion, cchDirectXVersion, TEXT("%d.%d%c"), dwDirectXVersionMajor, dwDirectXVersionMinor, cDirectXVersionLetter );
    }

   return S_OK;
}



//-----------------------------------------------------------------------------
// Name: GetDirectXVersionViaDxDiag()
// Desc: Tries to get the DirectX version from DxDiag's COM interface
//-----------------------------------------------------------------------------
HRESULT CDX9::GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor,
                                    DWORD* pdwDirectXVersionMinor,
                                    TCHAR* pcDirectXVersionLetter )
{
    HRESULT hr;
    bool bCleanupCOM = false;

    bool bSuccessGettingMajor = false;
    bool bSuccessGettingMinor = false;
    bool bSuccessGettingLetter = false;

    // Init COM.  COM may fail if its already been inited with a different
    // concurrency model.  And if it fails you shouldn't release it.
    hr = CoInitialize(NULL);
    bCleanupCOM = SUCCEEDED(hr);

    // Get an IDxDiagProvider
    bool bGotDirectXVersion = false;
    IDxDiagProvider* pDxDiagProvider = NULL;
    hr = CoCreateInstance( CLSID_DxDiagProvider,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           IID_IDxDiagProvider,
                           (LPVOID*) &pDxDiagProvider );
    if( SUCCEEDED(hr) )
    {
        // Fill out a DXDIAG_INIT_PARAMS struct
        DXDIAG_INIT_PARAMS dxDiagInitParam;
        ZeroMemory( &dxDiagInitParam, sizeof(DXDIAG_INIT_PARAMS) );
        dxDiagInitParam.dwSize                  = sizeof(DXDIAG_INIT_PARAMS);
        dxDiagInitParam.dwDxDiagHeaderVersion   = DXDIAG_DX9_SDK_VERSION;
        dxDiagInitParam.bAllowWHQLChecks        = false;
        dxDiagInitParam.pReserved               = NULL;

        // Init the m_pDxDiagProvider
        hr = pDxDiagProvider->Initialize( &dxDiagInitParam );
        if( SUCCEEDED(hr) )
        {
            IDxDiagContainer* pDxDiagRoot = NULL;
            IDxDiagContainer* pDxDiagSystemInfo = NULL;

            // Get the DxDiag root container
            hr = pDxDiagProvider->GetRootContainer( &pDxDiagRoot );
            if( SUCCEEDED(hr) )
            {
                // Get the object called DxDiag_SystemInfo
                hr = pDxDiagRoot->GetChildContainer( L"DxDiag_SystemInfo", &pDxDiagSystemInfo );
                if( SUCCEEDED(hr) )
                {
                    VARIANT var;
                    VariantInit( &var );

                    // Get the "dwDirectXVersionMajor" property
                    hr = pDxDiagSystemInfo->GetProp( L"dwDirectXVersionMajor", &var );
                    if( SUCCEEDED(hr) && var.vt == VT_UI4 )
                    {
                        if( pdwDirectXVersionMajor )
                            *pdwDirectXVersionMajor = var.ulVal;
                        bSuccessGettingMajor = true;
                    }
                    VariantClear( &var );

                    // Get the "dwDirectXVersionMinor" property
                    hr = pDxDiagSystemInfo->GetProp( L"dwDirectXVersionMinor", &var );
                    if( SUCCEEDED(hr) && var.vt == VT_UI4 )
                    {
                        if( pdwDirectXVersionMinor )
                            *pdwDirectXVersionMinor = var.ulVal;
                        bSuccessGettingMinor = true;
                    }
                    VariantClear( &var );

                    // Get the "szDirectXVersionLetter" property
                    hr = pDxDiagSystemInfo->GetProp( L"szDirectXVersionLetter", &var );
                    if( SUCCEEDED(hr) && var.vt == VT_BSTR && SysStringLen( var.bstrVal ) != 0 )
                    {
#ifdef UNICODE
                        *pcDirectXVersionLetter = var.bstrVal[0];
#else
                        char strDestination[10];
                        WideCharToMultiByte( CP_ACP, 0, var.bstrVal, -1, strDestination, 10*sizeof(CHAR), NULL, NULL );
                        if( pcDirectXVersionLetter )
                            *pcDirectXVersionLetter = strDestination[0];
#endif
                        bSuccessGettingLetter = true;
                    }
                    VariantClear( &var );

                    // If it all worked right, then mark it down
                    if( bSuccessGettingMajor && bSuccessGettingMinor && bSuccessGettingLetter )
                        bGotDirectXVersion = true;

                    pDxDiagSystemInfo->Release();
                }

                pDxDiagRoot->Release();
            }
        }

        pDxDiagProvider->Release();
    }

    if( bCleanupCOM )
        CoUninitialize();

    if( bGotDirectXVersion )
        return S_OK;
    else
        return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: GetDirectXVersionViaFileVersions()
// Desc: Tries to get the DirectX version by looking at DirectX file versions
//-----------------------------------------------------------------------------
HRESULT CDX9::GetDirectXVersionViaFileVersions( DWORD* pdwDirectXVersionMajor,
                                         DWORD* pdwDirectXVersionMinor,
                                         TCHAR* pcDirectXVersionLetter )
{
    ULARGE_INTEGER llFileVersion;
    TCHAR szPath[512];
    TCHAR szFile[512];
    BOOL bFound = false;

    if( GetSystemDirectory( szPath, MAX_PATH ) != 0 )
    {
        szPath[MAX_PATH-1]=0;

        // Switch off the ddraw version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\ddraw.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 2, 0, 95 ) ) >= 0 ) // Win9x version
            {
                // flle is >= DirectX1.0 version, so we must be at least DirectX1.0
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 1;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }

            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 3, 0, 1096 ) ) >= 0 ) // Win9x version
            {
                // flle is is >= DirectX2.0 version, so we must DirectX2.0 or DirectX2.0a (no redist change)
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 2;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }

            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 4, 0, 68 ) ) >= 0 ) // Win9x version
            {
                // flle is is >= DirectX3.0 version, so we must be at least DirectX3.0
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 3;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }
        }

        // Switch off the d3drg8x.dll version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\d3drg8x.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 4, 0, 70 ) ) >= 0 ) // Win9x version
            {
                // d3drg8x.dll is the DirectX3.0a version, so we must be DirectX3.0a or DirectX3.0b  (no redist change)
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 3;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT('a');
                bFound = true;
            }
        }

        // Switch off the ddraw version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\ddraw.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 5, 0, 155 ) ) >= 0 ) // Win9x version
            {
                // ddraw.dll is the DirectX5.0 version, so we must be DirectX5.0 or DirectX5.2 (no redist change)
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 5;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }

            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 0, 318 ) ) >= 0 ) // Win9x version
            {
                // ddraw.dll is the DirectX6.0 version, so we must be at least DirectX6.0
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 6;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }

            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 0, 436 ) ) >= 0 ) // Win9x version
            {
                // ddraw.dll is the DirectX6.1 version, so we must be at least DirectX6.1
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 6;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 1;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }
        }

        // Switch off the dplayx.dll version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\dplayx.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 6, 3, 518 ) ) >= 0 ) // Win9x version
            {
                // ddraw.dll is the DirectX6.1 version, so we must be at least DirectX6.1a
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 6;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 1;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT('a');
                bFound = true;
            }
        }

        // Switch off the ddraw version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\ddraw.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 7, 0, 700 ) ) >= 0 ) // Win9x version
            {
                // TODO: find win2k version

                // ddraw.dll is the DirectX7.0 version, so we must be at least DirectX7.0
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 7;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }
        }

        // Switch off the dinput version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\dinput.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 4, 7, 0, 716 ) ) >= 0 ) // Win9x version
            {
                // ddraw.dll is the DirectX7.0 version, so we must be at least DirectX7.0a
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 7;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT('a');
                bFound = true;
            }
        }

        // Switch off the ddraw version
        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\ddraw.dll") );
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 0, 400 ) ) >= 0) || // Win9x version
                (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2258, 400 ) ) >= 0) ) // Win2k/WinXP version
            {
                // ddraw.dll is the DirectX8.0 version, so we must be at least DirectX8.0 or DirectX8.0a (no redist change)
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 8;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }
        }

        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\d3d8.dll"));
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 1, 881 ) ) >= 0) || // Win9x version
                (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2600, 881 ) ) >= 0) ) // Win2k/WinXP version
            {
                // d3d8.dll is the DirectX8.1 version, so we must be at least DirectX8.1
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 8;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 1;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }

            if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 8, 1, 901 ) ) >= 0) || // Win9x version
                (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 1, 2600, 901 ) ) >= 0) ) // Win2k/WinXP version
            {
                // d3d8.dll is the DirectX8.1a version, so we must be at least DirectX8.1a
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 8;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 1;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT('a');
                bFound = true;
            }
        }

        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\mpg2splt.ax"));
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( CompareLargeInts( llFileVersion, MakeInt64( 6, 3, 1, 885 ) ) >= 0 ) // Win9x/Win2k/WinXP version
            {
                // quartz.dll is the DirectX8.1b version, so we must be at least DirectX8.1b
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 8;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 1;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT('b');
                bFound = true;
            }
        }

        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\dpnet.dll"));
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            if( (HIWORD(llFileVersion.HighPart) == 4 && CompareLargeInts( llFileVersion, MakeInt64( 4, 9, 0, 134 ) ) >= 0) || // Win9x version
                (HIWORD(llFileVersion.HighPart) == 5 && CompareLargeInts( llFileVersion, MakeInt64( 5, 2, 3677, 134 ) ) >= 0) ) // Win2k/WinXP version
            {
                // dpnet.dll is the DirectX8.2 version, so we must be at least DirectX8.2
                if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 8;
                if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 2;
                if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
                bFound = true;
            }
        }

        StringCchCopy( szFile, 512, szPath );
        StringCchCat( szFile, 512, TEXT("\\d3d9.dll"));
        if( SUCCEEDED( GetFileVersion( szFile, &llFileVersion ) ) )
        {
            // File exists, but be at least DirectX9
            if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 9;
            if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
            if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
            bFound = true;
        }
    }

    if( !bFound )
    {
        // No DirectX installed
        if( pdwDirectXVersionMajor ) *pdwDirectXVersionMajor = 0;
        if( pdwDirectXVersionMinor ) *pdwDirectXVersionMinor = 0;
        if( pcDirectXVersionLetter ) *pcDirectXVersionLetter = TEXT(' ');
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetFileVersion()
// Desc: Returns ULARGE_INTEGER with a file version of a file, or a failure code.
//-----------------------------------------------------------------------------
HRESULT CDX9::GetFileVersion( TCHAR* szPath, ULARGE_INTEGER* pllFileVersion )
{
    if( szPath == NULL || pllFileVersion == NULL )
        return E_INVALIDARG;

    DWORD dwHandle;
    UINT  cb;
    cb = GetFileVersionInfoSize( szPath, &dwHandle );
    if (cb > 0)
    {
        BYTE* pFileVersionBuffer = new BYTE[cb];
        if( pFileVersionBuffer == NULL )
            return E_OUTOFMEMORY;

        if (GetFileVersionInfo( szPath, 0, cb, pFileVersionBuffer))
        {
            VS_FIXEDFILEINFO* pVersion = NULL;
            if (VerQueryValue(pFileVersionBuffer, TEXT("\\"), (VOID**)&pVersion, &cb) &&
                pVersion != NULL)
            {
                pllFileVersion->HighPart = pVersion->dwFileVersionMS;
                pllFileVersion->LowPart  = pVersion->dwFileVersionLS;
                delete[] pFileVersionBuffer;
                return S_OK;
            }
        }

        delete[] pFileVersionBuffer;
    }

    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: MakeInt64()
// Desc: Returns a ULARGE_INTEGER where a<<48|b<<32|c<<16|d<<0
//-----------------------------------------------------------------------------
ULARGE_INTEGER CDX9::MakeInt64( WORD a, WORD b, WORD c, WORD d )
{
    ULARGE_INTEGER ull;
    ull.HighPart = MAKELONG(b,a);
    ull.LowPart = MAKELONG(d,c);
    return ull;
}




//-----------------------------------------------------------------------------
// Name: CompareLargeInts()
// Desc: Returns 1 if ullParam1 > ullParam2
//       Returns 0 if ullParam1 = ullParam2
//       Returns -1 if ullParam1 < ullParam2
//-----------------------------------------------------------------------------
int CDX9::CompareLargeInts( ULARGE_INTEGER ullParam1, ULARGE_INTEGER ullParam2 )
{
    if( ullParam1.HighPart > ullParam2.HighPart )
        return 1;
    if( ullParam1.HighPart < ullParam2.HighPart )
        return -1;

    if( ullParam1.LowPart > ullParam2.LowPart )
        return 1;
    if( ullParam1.LowPart < ullParam2.LowPart )
        return -1;

    return 0;
}

DWORD CDX9::ReduceBits(BYTE v, BYTE bits)
{
	 return ((DWORD)ceil((((double)v) / 255.0) * ((double)((1L << bits) - 1)))) & ((1L << bits) -1);
}

DWORD CDX9::ConvertColour(D3DFORMAT format, COLORREF rgb)
{
DWORD v;
	v = 0;
	switch (format)
	{
	case D3DFMT_A8R8G8B8:
		v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		v |= 0xff000000;
		break;
	case D3DFMT_A8B8G8R8:
		v = GetBValue(rgb) << 16 | GetGValue(rgb) << 8 | GetRValue(rgb);
		v |= 0xff000000;
		break;
	case D3DFMT_A2B10G10R10:
		v = GetBValue(rgb) << 20 | GetGValue(rgb) << 10 | GetRValue(rgb);
		v |= 0xc0000000;
		break;
	case D3DFMT_A2R10G10B10:
		v = GetRValue(rgb) << 20 | GetGValue(rgb) << 10 | GetBValue(rgb);
		v |= 0xc0000000;
	case D3DFMT_X8R8G8B8:
		v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		break;
	case D3DFMT_X8B8G8R8:
		v = GetBValue(rgb) << 16 | GetGValue(rgb) << 8 | GetRValue(rgb);
		break;
	case D3DFMT_R8G8B8:		
		v = GetRValue(rgb) << 16 | GetGValue(rgb) << 8 | GetBValue(rgb);
		break;
	case D3DFMT_A1R5G5B5:
		v = (ReduceBits(GetRValue(rgb), 5)) << 10 | (ReduceBits(GetGValue(rgb), 5)) << 8 | (ReduceBits(GetBValue(rgb), 5));
		v |= 0x800000;
		break;
	case D3DFMT_A4R4G4B4:
		v = (ReduceBits(GetRValue(rgb), 4)) << 8 | (ReduceBits(GetGValue(rgb), 4)) << 4 | (ReduceBits(GetBValue(rgb), 4));
		v |= 0xf000;
		break;
	case D3DFMT_A8R3G3B2:
		v = (ReduceBits(GetRValue(rgb), 3)) << 5 | (ReduceBits(GetGValue(rgb), 3)) << 2 | (ReduceBits(GetBValue(rgb), 2));
		v |= 0xff00;
		break;
	case D3DFMT_X1R5G5B5:
		v = (ReduceBits(GetRValue(rgb), 5)) << 10 | (ReduceBits(GetGValue(rgb), 5)) << 5 | (ReduceBits(GetBValue(rgb), 5));
		break;
	case D3DFMT_X4R4G4B4:
		v = (ReduceBits(GetRValue(rgb), 4)) << 8 | (ReduceBits(GetGValue(rgb), 4)) << 4 | (ReduceBits(GetBValue(rgb), 4));
		break;
	case D3DFMT_R5G6B5:
		v = (ReduceBits(GetRValue(rgb), 5)) << 11 | (ReduceBits(GetGValue(rgb), 6)) << 5 | (ReduceBits(GetBValue(rgb), 5));
		break;
	case D3DFMT_R3G3B2:
		v = (ReduceBits(GetRValue(rgb), 3)) << 5 | (ReduceBits(GetGValue(rgb), 3)) << 2 | (ReduceBits(GetBValue(rgb), 2));
		break;
	case D3DFMT_P8:
		v = 0;
		break;
	default:
		v = 0;
	}
	return v;
}


void CDX9::CloseDirectInput()
{
	if (DIHinst)
	{
		if (pDI) 
		{ 
			ReleaseJoy();

			if (pKeyboard) 
			{ 
				pKeyboard->Unacquire(); 
				pKeyboard->Release();
				pKeyboard = NULL; 
			} 
			pDI->Release();
			pDI = NULL; 
		} 

		FreeLibrary(DIHinst);
		DIHinst=0;
	}
}

void CDX9::CloseDirectSound()
{

	if (lpDirectSound)
	{
		if (pSecondarySoundBuffer)
		{
			pSecondarySoundBuffer->Stop();
			pSecondarySoundBuffer->Release();
			pSecondarySoundBuffer=NULL;
		}

		if (pPrimarySoundBuffer)
		{
			pPrimarySoundBuffer->Stop();
			pPrimarySoundBuffer->Release();
			pPrimarySoundBuffer=NULL;
		}
		lpDirectSound->Release();
		lpDirectSound=NULL;
	}
}


HRESULT CDX9::OpenDirectInput(HINSTANCE hInstance, HWND hWnd)
{
HRESULT hr;

	CloseDirectInput();
	DIHinst = LoadLibrary(TEXT("DINPUT.DLL"));
	if (DIHinst==0)
	{
		return SetError(E_FAIL, TEXT("Could not load DINPUT.DLL"));
	}

	pDirectInputCreateEx = (DIRECTINPUTCREATEEX)GetProcAddress( DIHinst, "DirectInputCreateEx");
	if (pDirectInputCreateEx==0)
	{
		CloseDirectInput();
		return SetError(E_FAIL, TEXT("Could find DirectInputCreateEx in DINPUT.DLL"));
	}
	//
	hr = pDirectInputCreateEx(hInstance, 0x700 , IID_IDirectInput7, (void**)&pDI, NULL);
	if FAILED(hr) 
		return SetError(hr, TEXT("Could not open direct input."));
	hr = pDI->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7, (void**)&pKeyboard, NULL); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return SetError(hr,  TEXT("Could not create direct input keyboard."));
	} 
	hr = pKeyboard->SetDataFormat(&c_dfDIKeyboard); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return hr; 
	} 
	hr = pKeyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY); 
	if FAILED(hr) 
	{ 
		CloseDirectInput(); 
		return hr; 
	} 

	return S_OK;

}

HRESULT CDX9::OpenDirectSound(HWND hWnd, HCFG::EMUFPS fps)
{
HRESULT hr;
DSBUFFERDESC dsbdesc;
//LPVOID ptr1;
//DWORD size1;

	ClearError();
	CloseDirectSound();

	hr= DirectSoundCreate(NULL, &lpDirectSound, NULL);
	if (FAILED(hr))
        return hr;
 
    hr = lpDirectSound->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
	
	
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, &pPrimarySoundBuffer, NULL);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

    ZeroMemory(&m_wfx, sizeof(WAVEFORMATEX)); 
    m_wfx.wFormatTag = WAVE_FORMAT_PCM; 
    m_wfx.nChannels = 1; 
    m_wfx.nSamplesPerSec = SAMPLES_PER_SEC; 
    m_wfx.wBitsPerSample = 16; 
    m_wfx.nBlockAlign = m_wfx.wBitsPerSample / 8 * m_wfx.nChannels;
    m_wfx.nAvgBytesPerSec = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign;
 
	hr = pPrimarySoundBuffer->SetFormat(&m_wfx);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	pPrimarySoundBuffer->Release();
	pPrimarySoundBuffer = NULL;
	
	SoundBufferSize = m_wfx.nSamplesPerSec * m_wfx.nBlockAlign / 50  * 6;

	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS;// | DSBCAPS_CTRLVOLUME;
	dsbdesc.dwBufferBytes = SoundBufferSize; 

	dsbdesc.lpwfxFormat = &m_wfx;

	SoundBytesPerSecond = (m_wfx.nSamplesPerSec * m_wfx.nBlockAlign);

	//if (fps == HCFG::EMUFPS_50)
	//{
	//	SoundBytesPerFrame = (DWORD)ceil((double)(SoundBytesPerSecond) / (double)PAL50FRAMESPERSECOND);
	//}
	//else
	//{
	//	SoundBytesPerFrame = (DWORD)ceil((double)(SoundBytesPerSecond) / ((double)PALCLOCKSPERSECOND / ((double)PALLINESPERFRAME * (double)PALCLOCKSPERLINE)));
	//}
		
	//Hack to use a sound lock buffer big enough for either EMUFPS_50 or EMUFPS_50_12
	//SoundBytesPerFrame = (DWORD)ceil((double)(SoundBytesPerSecond) / (double)PAL50FRAMESPERSECOND);

	BufferLockSize = SoundBytesPerFrame;
	
	hr = lpDirectSound->CreateSoundBuffer(&dsbdesc, &pSecondarySoundBuffer, NULL);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	hr	= RestoreSoundBuffers();
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	hr = pSecondarySoundBuffer->SetCurrentPosition(0);
	if (FAILED(hr))
	{
		CloseDirectSound();
        return hr;
	}

	ClearSoundBuffer(0);
	return S_OK;
}

void CDX9::ClearSoundBuffer(short value)
{
	ClearSoundBuffer(pSecondarySoundBuffer, value);
}

void CDX9::ClearSoundBuffer(LPDIRECTSOUNDBUFFER pSoundBuffer, short value)
{
LPVOID p1= NULL;
LPVOID p2= NULL;
DWORD i1 =0;
DWORD i2 =0;
	if (!pSoundBuffer)
		return;
	if (SUCCEEDED(pSoundBuffer->Lock(0, 0, &p1, &i1, &p2, &i2, DSBLOCK_ENTIREBUFFER)))
	{
		//if (p1!=0 && i1!=0)
		//	ZeroMemory(p1, i1);
		//if (p2!=0 && i2!=0)
		//	ZeroMemory(p2, i2);
		for (DWORD i = 0 ; i < i1/2 ; i++)
		{
			((WORD *)p1)[i] = value;
		}
		for (DWORD i = 0 ; i < i2/2 ; i++)
		{
			((WORD *)p2)[i] = value;
		}
		pSoundBuffer->Unlock(p1, i1, p2, i2);
	}
}

HRESULT CDX9::RestoreSoundBuffers()
{
HRESULT hr;
DWORD dwStatus;
int i;
	ClearError();
    if( NULL == pSecondarySoundBuffer)
        return E_POINTER;

    if( FAILED( hr = pSecondarySoundBuffer->GetStatus( &dwStatus ) ) )
        return hr;

    if( dwStatus & DSBSTATUS_BUFFERLOST )
    {
        // Since the app could have just been activated, then
        // DirectSound may not be giving us control yet, so 
        // the restoring the buffer may fail.  
        // If it does, sleep until DirectSound gives us control.
		i=0;
        do 
        {
            hr = pSecondarySoundBuffer->Restore();
            if( hr == DSERR_BUFFERLOST )
                Sleep( 10 );
			if (++i > 1000)
				return hr;
        }
        while( hr = pSecondarySoundBuffer->Restore() );
    }
	ClearSoundBuffer(0);
    return S_OK;
}

void CDX9::SoundHalt(short value)
{
	//TEST
	//m_soundResumeDelay = SOUNDRESUMEDELAY;
	m_soundResumeDelay = 0;
	m_soundVolumeDelay = SOUNDVOLUMEDELAYZERO;
	if (pSecondarySoundBuffer)
	{
		//TEST
		//pSecondarySoundBuffer->SetVolume(DSBVOLUME_MIN);
		ClearSoundBuffer(value);
		pSecondarySoundBuffer->Stop();
	}
}

void CDX9::SoundResume()
{
DWORD SoundStatus;
LONG vol;
//HRESULT hr;
	if (pSecondarySoundBuffer)
	{
		//TEST
		if (m_soundVolumeDelay > 0)
		{
			--m_soundVolumeDelay;
			if (m_soundVolumeDelay<SOUNDVOLUMEDELAYZERO)
				vol = (LONG) ceil ((double)(DSBVOLUME_MIN - DSBVOLUME_MAX) * ((double)m_soundVolumeDelay / (double)(SOUNDVOLUMEDELAY-1)));
			else
				vol = DSBVOLUME_MIN;
			//hr = pSecondarySoundBuffer->SetVolume(vol);
		}
		if (m_soundResumeDelay > 0)
		{
			--m_soundResumeDelay;
		}
		else
		{
			m_soundResumeDelay = SOUNDRESUMEDELAY;
			if (S_OK == pSecondarySoundBuffer->GetStatus(&SoundStatus))
			{
				if (SoundStatus & DSBSTATUS_BUFFERLOST)
				{
					if (S_OK == pSecondarySoundBuffer->Restore())
					{
						pSecondarySoundBuffer->Play(0,0,DSBPLAY_LOOPING);
					}
				}
				else if ((SoundStatus & DSBSTATUS_PLAYING) == 0)
				{
					pSecondarySoundBuffer->Play(0,0,DSBPLAY_LOOPING);
					//pSecondarySoundBuffer->SetVolume(DSBVOLUME_MAX);
				}
			}
		}
	}
}

HRESULT CDX9::CreateDeviceJoy(int joyindex, REFGUID refguid)
{
HRESULT hr;

	joyok[joyindex] = FALSE;
	if (joy[joyindex])
	{
		joy[joyindex]->Unacquire();
		joy[joyindex]->Release();
	}
	joy[joyindex] = NULL;
	hr = pDI->CreateDeviceEx(refguid, IID_IDirectInputDevice7, (LPVOID *)&joy[joyindex], NULL);
	return hr;
}


HRESULT CDX9::GetPropJoy(int joyindex, REFGUID rguid, LPDIPROPHEADER pph)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->GetProperty(rguid, pph);
}

HRESULT CDX9::SetPropJoy(int joyindex, REFGUID rguid, LPCDIPROPHEADER pph)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->SetProperty(rguid, pph);
}

HRESULT CDX9::GetDeviceState(int joyindex, LPVOID pData)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->GetDeviceState(sizeof(DIJOYSTATE), pData);
}

LPDIRECTINPUTDEVICE7 CDX9::GetJoy(int joyindex)
{
	return joy[joyindex];
}


void CDX9::ReleaseJoy()
{
int i;
	for (i=0 ; i<3 ; i++)
	{
		if (joy[i])
		{
			joy[i]->Unacquire();
			joy[i]->Release();
			joy[i]=NULL;
			joyok[i]=FALSE;
		}
	}
}


HRESULT CDX9::EnumDevices(DWORD dwDevType, 	LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	return pDI->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}


HRESULT CDX9::EnumObjectsJoy(int joyindex, LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef,DWORD dwFlags)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->EnumObjects(lpCallback, pvRef, dwFlags);
}

HRESULT CDX9::SetDataFormatJoy(int joyindex, LPCDIDATAFORMAT lpdf)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->SetDataFormat(lpdf);
}

HRESULT CDX9::AcquireJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Acquire();
}

HRESULT CDX9::UnacquireJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Unacquire();
}

HRESULT CDX9::PollJoy(int joyindex)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->Poll();
}

HRESULT CDX9::SetCooperativeLevelJoy(int joyindex, HWND hWnd, DWORD dwFlags)
{
	if (joy[joyindex] == NULL)
		return E_POINTER;
	return joy[joyindex]->SetCooperativeLevel(hWnd, dwFlags);
}


HRESULT CDX9::InitJoys(HWND hWnd, struct joyconfig &joy1config,struct joyconfig &joy2config)
{
HRESULT hr;
DIPROPRANGE diprg; 

	ClearError();
	ReleaseJoy();
	if (hWnd==0)
		return E_FAIL;
	
	joy1config.xMax= +1000;
	joy1config.xMin= -1000;
	joy1config.yMax= +1000;
	joy1config.yMin= -1000;
	joy2config.xMax= +1000;
	joy2config.xMin= -1000;
	joy2config.yMax= +1000;
	joy2config.yMin= -1000;
	
	if (joy1config.bValid && joy1config.bEnabled)
	{
		UnacquireJoy(JOY1);

		hr = CreateDeviceJoy(JOY1, joy1config.joystickID);
		if (SUCCEEDED(hr))
		{
			hr = SetDataFormatJoy(JOY1, &c_dfDIJoystick);
			if (SUCCEEDED(hr))
			{
				hr = SetCooperativeLevelJoy(JOY1, hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
				if (SUCCEEDED(hr))
				{
					diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
					diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
					diprg.diph.dwHow        = DIPH_BYOFFSET; 
					diprg.diph.dwObj        = joy1config.dwOfs_X; // Specify the enumerated axis
					diprg.lMin              = joy1config.xMin; 
					diprg.lMax              = joy1config.xMax; 
					hr = SetPropJoy(JOY1, DIPROP_RANGE, &diprg.diph);
					if (FAILED(hr))
					{
						ZeroMemory(&diprg, sizeof(diprg));
						diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
						diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
						diprg.diph.dwHow        = DIPH_BYOFFSET; 
						diprg.diph.dwObj        = joy1config.dwOfs_X; // Specify the enumerated axis
						hr = GetPropJoy(JOY1, DIPROP_RANGE, &diprg.diph);
						if (SUCCEEDED(hr))
						{
							joy1config.xMin = diprg.lMin; 
							joy1config.xMax = diprg.lMax; 
						}
					}
					if (FAILED(hr))
						SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
					if (SUCCEEDED(hr))
					{
						diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
						diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
						diprg.diph.dwHow        = DIPH_BYOFFSET; 
						diprg.diph.dwObj        = joy1config.dwOfs_Y; // Specify the enumerated axis
						diprg.lMin              = joy1config.yMin; 
						diprg.lMax              = joy1config.yMax; 
						hr = SetPropJoy(JOY1, DIPROP_RANGE, &diprg.diph);
						if (FAILED(hr))
						{
							ZeroMemory(&diprg, sizeof(diprg));
							diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
							diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
							diprg.diph.dwHow        = DIPH_BYOFFSET; 
							diprg.diph.dwObj        = joy1config.dwOfs_Y; // Specify the enumerated axis
							hr = GetPropJoy(JOY1, DIPROP_RANGE, &diprg.diph);
							if (SUCCEEDED(hr))
							{
								joy1config.yMin = diprg.lMin; 
								joy1config.yMax = diprg.lMax; 
							}
						}
						if (FAILED(hr))
							SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
						if (SUCCEEDED(hr))
						{
							joy1config.xLeft = joy1config.xMin + 60L * (joy1config.xMax - joy1config.xMin)/200L;
							joy1config.xRight = joy1config.xMax - 60L * (joy1config.xMax - joy1config.xMin)/200L;
							joy1config.yUp = joy1config.yMin + 60L * (joy1config.yMax - joy1config.yMin)/200L;
							joy1config.yDown = joy1config.yMax - 60L * (joy1config.yMax - joy1config.yMin)/200L;
							joyok[JOY1] = TRUE;
						}
					}
				}
			}
		}
	}

	if (joy2config.bValid && joy2config.bEnabled)
	{
		UnacquireJoy(JOY2);

		hr = CreateDeviceJoy(JOY2, joy2config.joystickID);
		if (SUCCEEDED(hr))
		{
			hr = SetDataFormatJoy(JOY2, &c_dfDIJoystick);
			if (SUCCEEDED(hr))
			{
				hr = SetCooperativeLevelJoy(JOY2, hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
				if (SUCCEEDED(hr))
				{
					diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
					diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
					diprg.diph.dwHow        = DIPH_BYOFFSET; 
					diprg.diph.dwObj        = joy2config.dwOfs_X; // Specify the enumerated axis
					diprg.lMin              = joy2config.xMin; 
					diprg.lMax              = joy2config.xMax; 
					hr = SetPropJoy(JOY2, DIPROP_RANGE, &diprg.diph);
					if (FAILED(hr))
					{
						ZeroMemory(&diprg, sizeof(diprg));
						diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
						diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
						diprg.diph.dwHow        = DIPH_BYOFFSET; 
						diprg.diph.dwObj        = joy2config.dwOfs_X; // Specify the enumerated axis
						hr = GetPropJoy(JOY2, DIPROP_RANGE, &diprg.diph);
						if (SUCCEEDED(hr))
						{
							joy2config.xMin = diprg.lMin; 
							joy2config.xMax = diprg.lMax; 
						}
					}
					if (FAILED(hr))
						SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
					if (SUCCEEDED(hr))
					{
						diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
						diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
						diprg.diph.dwHow        = DIPH_BYOFFSET; 
						diprg.diph.dwObj        = joy2config.dwOfs_Y; // Specify the enumerated axis
						diprg.lMin              = joy2config.yMin; 
						diprg.lMax              = joy2config.yMax; 
						hr = SetPropJoy(JOY2, DIPROP_RANGE, &diprg.diph);
						if (FAILED(hr))
						{
							ZeroMemory(&diprg, sizeof(diprg));
							diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
							diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
							diprg.diph.dwHow        = DIPH_BYOFFSET; 
							diprg.diph.dwObj        = joy2config.dwOfs_Y; // Specify the enumerated axis
							hr = GetPropJoy(JOY2, DIPROP_RANGE, &diprg.diph);
							if (SUCCEEDED(hr))
							{
								joy2config.yMin = diprg.lMin; 
								joy2config.yMax = diprg.lMax; 
							}
						}
						if (FAILED(hr))
							SetError(hr, TEXT("GetProperty DIPROP_RANGE failed."));
						if (SUCCEEDED(hr))
						{
							joy2config.xLeft = joy2config.xMin + 60L * (joy2config.xMax - joy2config.xMin)/200L;
							joy2config.xRight = joy2config.xMax - 60L * (joy2config.xMax - joy2config.xMin)/200L;
							joy2config.yUp = joy2config.yMin + 60L * (joy2config.yMax - joy2config.yMin)/200L;
							joy2config.yDown = joy2config.yMax - 60L * (joy2config.yMax - joy2config.yMin)/200L;
							joyok[JOY2] = TRUE;
						}
					}
				}
			}
		}
	}

	if ((joy1config.bValid && joy1config.bEnabled && !joyok[JOY1]) || (joy2config.bValid && joy2config.bEnabled && !joyok[JOY2]))
		return E_FAIL;
	else
		return S_OK;
}



HRESULT CDX9::LoadTextures(D3DFORMAT format)
{
HRESULT hr;
PALETTEENTRY *pPal = NULL;

	if (format == D3DFMT_P8)
	{
		pPal = &m_paletteEntry[0];
	}

	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDGREENON), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedGreenOn)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDGREENOFF), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedGreenOff)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDREDON), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedRedOn)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDREDOFF), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedRedOff)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDBLUEON), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedBlueOn)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateTextureFromResourceEx(m_pd3dDevice, GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LEDBLUEOFF), 8, 8, 1, 0, format, D3DPOOL_DEFAULT , D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DCOLOR_ARGB(0xff, 0, 0, 0), NULL, pPal, &m_ptxLedBlueOff)))
	{
		return hr;
	}

	if (FAILED(hr = D3DXCreateSprite(m_pd3dDevice,  &m_psprLedMotor)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateSprite(m_pd3dDevice,  &m_psprLedDrive)))
	{
		return hr;
	}
	if (FAILED(hr = D3DXCreateSprite(m_pd3dDevice,  &m_psprLedWrite)))
	{
		return hr;
	}


	return D3D_OK;
}

void CDX9::FreeTextures()
{
	if (m_psprLedMotor)
	{
		m_psprLedMotor->Release();
		m_psprLedMotor = NULL;
	}
	if (m_psprLedDrive)
	{
		m_psprLedDrive->Release();
		m_psprLedDrive = NULL;
	}
	if (m_psprLedWrite)
	{
		m_psprLedWrite->Release();
		m_psprLedWrite = NULL;
	}

	if (m_ptxLedGreenOn)
	{
		m_ptxLedGreenOn->Release();
		m_ptxLedGreenOn = NULL;
	}

	if (m_ptxLedGreenOff)
	{
		m_ptxLedGreenOff->Release();
		m_ptxLedGreenOff = NULL;
	}

	if (m_ptxLedRedOn)
	{
		m_ptxLedRedOn->Release();
		m_ptxLedRedOn = NULL;
	}

	if (m_ptxLedRedOff)
	{
		m_ptxLedRedOff->Release();
		m_ptxLedRedOff = NULL;
	}

	if (m_ptxLedBlueOn)
	{
		m_ptxLedBlueOn->Release();
		m_ptxLedBlueOn = NULL;
	}

	if (m_ptxLedBlueOff)
	{
		m_ptxLedBlueOff->Release();
		m_ptxLedBlueOff = NULL;
	}
}

D3DTEXTUREFILTERTYPE CDX9::GetDxFilterFromEmuFilter(HCFG::EMUWINDOWFILTER emuFilter)
{
	switch(emuFilter)
	{
	case HCFG::EMUWINFILTER_AUTO:
		return D3DTEXF_LINEAR;
	case HCFG::EMUWINFILTER_NONE:
		return D3DTEXF_NONE;
	case HCFG::EMUWINFILTER_POINT:
		return D3DTEXF_POINT;
	case HCFG::EMUWINFILTER_LINEAR:
		return D3DTEXF_LINEAR;
	default:
		return D3DTEXF_LINEAR;
	}
}

int CDX9::GetAdapterOrdinalFromGuid(const GUID &id)
{
int iAdapterOrdinal = D3DADAPTER_DEFAULT;
int i,j;
D3DADAPTER_IDENTIFIER9 adapterIdentifier;
	
	j = m_pD3D->GetAdapterCount();
	for (i=0;i<j;i++)
	{
		if (SUCCEEDED(m_pD3D->GetAdapterIdentifier(i, 0, &adapterIdentifier)))
		{
			if (adapterIdentifier.DeviceIdentifier == id)
			{
				iAdapterOrdinal = i;
				break;
			}
				
		}
	}
	return iAdapterOrdinal;
}

const D3DFORMAT CDX9::Formats[] = {
	D3DFMT_A8R8G8B8,
	D3DFMT_X8R8G8B8,
	D3DFMT_A8B8G8R8,
	D3DFMT_X8B8G8R8,
	D3DFMT_A2B10G10R10,
	D3DFMT_A2R10G10B10,
	D3DFMT_R8G8B8,
	D3DFMT_X1R5G5B5,
	D3DFMT_A1R5G5B5,
	D3DFMT_R5G6B5,
	D3DFMT_X4R4G4B4,
	D3DFMT_A4R4G4B4,
	D3DFMT_A8R3G3B2,
	D3DFMT_A8P8,
	D3DFMT_R3G3B2,
	D3DFMT_P8,
	(D3DFORMAT)-1
};