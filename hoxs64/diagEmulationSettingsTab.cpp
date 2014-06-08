#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>
#include <assert.h>
#include "defines.h"
#include <commctrl.h>
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "carray.h"
#include "MList.h"
#include "diagemulationsettingstab.h"
#include "resource.h"


const TCHAR CDiagEmulationSettingsTab::szAuto[] = TEXT("Auto");
const TCHAR CDiagEmulationSettingsTab::szVideoFilter_1X[] = TEXT("1X");
const TCHAR CDiagEmulationSettingsTab::szVideoFilter_2X[] = TEXT("2X");
const TCHAR CDiagEmulationSettingsTab::szVideoFilter_StretchToFit[] = TEXT("Stretch to fit");
const TCHAR CDiagEmulationSettingsTab::szVideoFilter_StretchWithBorderClip[] = TEXT("Stretch with border clip");

struct StringAndInt
{
	LPTSTR name;
	int value;
};

CDiagEmulationSettingsTab::CDiagEmulationSettingsTab()
{
}

CDiagEmulationSettingsTab::~CDiagEmulationSettingsTab()
{
}

HRESULT CDiagEmulationSettingsTab::Init(CDX9 *dx, const CConfig *cfg)
{
	m_pDx = dx;
	CurrentCfg = *cfg;
	NewCfg = *cfg;
	return S_OK;
}

void CDiagEmulationSettingsTab::VideoPageSizeComboBoxes()
{
	HWND hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	if (!hVideoPage)
		return;
	static int ctls[] = {IDC_CBO_ADAPTER, IDC_CBO_MODE, IDC_CBO_FORMAT, IDC_CBO_FILTER, IDC_CBO_STRETCH, IDC_CBO_BORDER, IDC_CBO_FPS};
	G::AutoSetComboBoxHeight(hVideoPage, ctls, _countof(ctls), 0);

	SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETDROPPEDWIDTH, 130, 0);
}

void CDiagEmulationSettingsTab::DiskPageSizeComboBoxes()
{
	HWND hDiskPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)->GetHwnd();
	if (!hDiskPage)
		return;
	static int ctls[] = {IDC_CBO_TRACKZEROSENSOR};
	G::AutoSetComboBoxHeight(hDiskPage, ctls, _countof(ctls), 0);
}

void CDiagEmulationSettingsTab::FillDevices()
{
UINT i;
HRESULT hr;
LRESULT lr;
CDisplayInfo displayInfo;
unsigned int j;
HWND hWndCboAdapter;
HWND hVideoPage;
HMONITOR hMonitor;
GUID emptyGuid;
LRESULT currentSelection = -1;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	if (!hVideoPage)
		return;
	
	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_RESETCONTENT, 0, 0);
	m_AdapterArray.Clear();

	UINT iAdapterCount = m_pDx->m_pD3D->GetAdapterCount();

	hr = m_AdapterArray.Resize(iAdapterCount);
	if (FAILED(hr))
	{
		return;
	}

	ZeroMemory(&emptyGuid, sizeof(emptyGuid));

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_ADDSTRING, 0, (LPARAM) TEXT("Auto"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETITEMDATA, lr, (LPARAM) 0);
		if (NewCfg.m_fullscreenAdapterId == emptyGuid)
		{
			currentSelection = lr;
		}
	}

	for (i=0 ; i < iAdapterCount ; i++)
	{
		ZeroMemory(&displayInfo, sizeof(displayInfo));
		displayInfo.bRequireClean = true;

		displayInfo.adapterOrdinal = i;
		m_pDx->m_pD3D->GetAdapterIdentifier(i, 0, &displayInfo.adapter);
		hMonitor = m_pDx->m_pD3D->GetAdapterMonitor(i);
		if (hMonitor!=0)
		{
			displayInfo.monitor.cbSize = sizeof(MONITORINFOEX);
			m_pDx->DXUTGetMonitorInfo(hMonitor, &displayInfo.monitor);
		}
		if (SUCCEEDED(displayInfo.MakeName()))
		{
			//appends a shallow copy of displayInfo
			hr = m_AdapterArray.Append(displayInfo, &j);
			if (FAILED(hr))
			{
				return;
			}
			//m_AdapterArray now controls the life time of displayInfo allocated memory
			//prevent clean of local displayInfo. 
			displayInfo.bRequireClean = false;


			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_ADDSTRING, 0, (LPARAM) displayInfo.name);
			if (lr >= 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETITEMDATA, lr, (LPARAM) j);
				//if (NewCfg.m_fullscreenAdapterId == displayInfo.adapter.DeviceIdentifier)
				if ((NewCfg.m_fullscreenAdapterNumber == i) && (NewCfg.m_fullscreenAdapterId != emptyGuid))
				{
					currentSelection = lr;
				}
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETCURSEL, 0, 0);
}

int CDiagEmulationSettingsTab::fnFindMode(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2)
{
	if (mode1.mode.Height == mode2.mode.Height && mode1.mode.Width == mode2.mode.Width)
		return 0;
	else
		return 1;
}

int CDiagEmulationSettingsTab::fnFindModeFormat(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2)
{
	if (mode1.mode.Format == mode2.mode.Format)
		return 0;
	else
		return 1;
}

int CDiagEmulationSettingsTab::fnCompareMode(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2)
{
	if (mode1.mode.Width>mode2.mode.Width)
		return 1;
	else if (mode1.mode.Width<mode2.mode.Width)
		return -1;

	if (mode1.mode.Height>mode2.mode.Height)
		return 1;
	else if (mode1.mode.Height<mode2.mode.Height)
		return -1;

	return 0;
}

int CDiagEmulationSettingsTab::fnCompareFormat(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2)
{
	if (CDX9::GetBitsPerPixel(mode1.mode.Format) > CDX9::GetBitsPerPixel(mode2.mode.Format))
		return 1;
	else if (CDX9::GetBitsPerPixel(mode1.mode.Format)<CDX9::GetBitsPerPixel(mode2.mode.Format))
		return -1;

	return 0;
}


void CDiagEmulationSettingsTab::FillModes()
{
HWND hVideoPage, hWndCboAdapter;
LRESULT lr;
LRESULT index;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
		return;

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr < 0)
	{
		FillModes((UINT)-1);
		return;
	}
	else if (lr == 0)
	{
		FillModes(D3DADAPTER_DEFAULT);
		return;
	}

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr < 0 || lr >= MAXLONG)
	{
		FillModes((UINT)-1);
		return;
	} 

	if ((unsigned)lr >= m_AdapterArray.Count())
	{
		FillModes((UINT)-1);
		return;
	} 

	
	FillModes((UINT)m_AdapterArray[(ULONG)lr].adapterOrdinal);
}

void CDiagEmulationSettingsTab::FillModes(UINT adapterOrdinal)
{
HWND hVideoPage, hWndCboMode;
LRESULT currentSelection = -1;
CDisplayModeInfo  modeInfo;
UINT i;
HRESULT hr;
LRESULT lr;


	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboMode = GetDlgItem(hVideoPage, IDC_CBO_MODE);
	if (hWndCboMode == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_RESETCONTENT, 0, 0);
	m_ModeList.Clear();

	UINT iAdapterCount = m_pDx->m_pD3D->GetAdapterCount();	
	if (adapterOrdinal >= iAdapterCount)
		return;

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_ADDSTRING, 0, (LPARAM) szAuto);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETITEMDATA, lr, (LPARAM) 0);
		if (NewCfg.m_fullscreenWidth == 0)
		{
			currentSelection = lr;
		}
	}

	for (UINT iFormatToTest = 0; (signed int)CDX9::Formats[iFormatToTest] > 0 ; iFormatToTest++)
	{
		UINT iModeCount = m_pDx->m_pD3D->GetAdapterModeCount(adapterOrdinal, CDX9::Formats[iFormatToTest]);

		for (i=0 ; i < iModeCount ; i++)
		{
			ZeroMemory(&modeInfo, sizeof(modeInfo));
			modeInfo.bRequireClean = true;
			m_pDx->m_pD3D->EnumAdapterModes(adapterOrdinal, CDX9::Formats[iFormatToTest], i, &modeInfo.mode);
			if (m_pDx->IsAcceptableMode(modeInfo.mode))
			{
				if (m_ModeList.FindElement(CDiagEmulationSettingsTab::fnFindMode, modeInfo) != NULL)
					continue;
				if (SUCCEEDED(modeInfo.MakeName(m_pDx)))
				{
					//appends a shallow copy of modeInfo
					hr = m_ModeList.Append(modeInfo);
					if (FAILED(hr))
					{
						return;
					}
					//m_ModeList will clean modeInfo allocated memory
					//prevent clean of local modeInfo.
					modeInfo.bRequireClean = false;
				}
			}		
		}
	}

	m_ModeList.MergeSort(&fnCompareMode);
	for (CDisplayModeElement *p = m_ModeList.Head() ; p != NULL ; p=p->Next())
	{
		
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_ADDSTRING, 0, (LPARAM) p->m_data.name);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETITEMDATA, lr, (LPARAM) &p->m_data);
			if (NewCfg.m_fullscreenWidth == p->m_data.mode.Width && NewCfg.m_fullscreenHeight == p->m_data.mode.Height)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETCURSEL, 0, 0);
}


void CDiagEmulationSettingsTab::FillFormats()
{
HWND hVideoPage, hWndCboAdapter, hWndCboMode;
LRESULT lr;
UINT iAdapterOrdinal;
D3DDISPLAYMODE mode;
LRESULT index;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
		return;

	hWndCboMode = GetDlgItem(hVideoPage, IDC_CBO_MODE);
	if (hWndCboAdapter == NULL)
		return;

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr < 0 || lr >= MAXLONG)
	{
		//clear formats list
		ZeroMemory(&mode, sizeof(mode));
		FillFormats((UINT)-1, mode);
		return;
	}
	else if (lr == 0)
	{
		iAdapterOrdinal = D3DADAPTER_DEFAULT;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr < 0 || lr >= MAXLONG)
		{
			//clear formats list
			ZeroMemory(&mode, sizeof(mode));
			FillFormats((UINT)-1, mode);
			return;
		}
		iAdapterOrdinal  = m_AdapterArray[(ULONG)lr].adapterOrdinal;
	}
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
	if (lr < 0 || lr >= MAXLONG)
	{
		//clear formats list
		ZeroMemory(&mode, sizeof(mode));
		FillFormats((UINT)-1, mode);
		return;
	}
	else if (lr == 0)
	{
		ZeroMemory(&mode, sizeof(mode));
		FillFormats(iAdapterOrdinal, mode);
		return;
	}

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr == NULL)
	{
		ZeroMemory(&mode, sizeof(mode));
		FillFormats((UINT)-1, mode);
		return;
	} 

	FillFormats(iAdapterOrdinal, ((CDisplayModeInfo *)lr)->mode);
}


void CDiagEmulationSettingsTab::FillFormats(UINT adapterOrdinal, D3DDISPLAYMODE &mode)
{
HWND hVideoPage, hWndCboFormat;
LRESULT currentSelection = -1;
CDisplayModeInfo  modeInfo;
UINT i;
HRESULT hr;
LRESULT lr;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboFormat = GetDlgItem(hVideoPage, IDC_CBO_FORMAT);
	if (hWndCboFormat == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_RESETCONTENT, 0, 0);
	m_ModeFormatList.Clear();

	UINT iAdapterCount = m_pDx->m_pD3D->GetAdapterCount();	
	if (adapterOrdinal >= iAdapterCount)
		return;

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_ADDSTRING, 0, (LPARAM) szAuto);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETITEMDATA, lr, (LPARAM) 0);
		if (NewCfg.m_fullscreenFormat == 0)
		{
			currentSelection = lr;
		}
	}
	
	if (mode.Height != 0 && mode.Width != 0)
	{
		for (UINT iFormatToTest = 0; (signed int)CDX9::Formats[iFormatToTest] > 0 ; iFormatToTest++)
		{
			UINT iModeCount = m_pDx->m_pD3D->GetAdapterModeCount(adapterOrdinal, CDX9::Formats[iFormatToTest]);
			for (i=0 ; i < iModeCount ; i++)
			{
				ZeroMemory(&modeInfo, sizeof(modeInfo));
				modeInfo.bRequireClean = true;
				m_pDx->m_pD3D->EnumAdapterModes(adapterOrdinal, CDX9::Formats[iFormatToTest], i, &modeInfo.mode);
				if (m_pDx->IsAcceptableMode(modeInfo.mode))
				{
					if (m_ModeFormatList.FindElement(CDiagEmulationSettingsTab::fnFindModeFormat, modeInfo) != NULL)
						continue;
					if (SUCCEEDED(modeInfo.MakeNameFormat(m_pDx)))
					{
						//appends a shallow copy of modeInfo
						hr = m_ModeFormatList.Append(modeInfo);
						if (FAILED(hr))
						{
							return;
						}
						//m_ModeFormatList will clean modeInfo allocated memory
						//prevent clean of local modeInfo.
						modeInfo.bRequireClean = false;
					}
				}		
			}
		}
	}

	m_ModeFormatList.MergeSort(&fnCompareFormat);
	for (CDisplayModeElement *p = m_ModeFormatList.Head(); p != NULL; p=p->Next())
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_ADDSTRING, 0, (LPARAM) p->m_data.name);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETITEMDATA, lr, (LPARAM) &p->m_data);
			if (NewCfg.m_fullscreenFormat == p->m_data.mode.Format)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETCURSEL, 0, 0);
}

void CDiagEmulationSettingsTab::FillSizes()
{
HWND hVideoPage, hDiskPage, hWndCboSize;
LRESULT lr;
D3DDISPLAYMODE *pMode;
LRESULT index;
LRESULT currentSelection = -1;
bool bAdd1X, bAdd2X, bAddStretch;
bool bShowFloppyLed;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboSize = GetDlgItem(hVideoPage, IDC_CBO_STRETCH);
	if (hWndCboSize == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_RESETCONTENT, 0, 0);

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szAuto);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_AUTO);
		if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_AUTO)
		{
			currentSelection = lr;
		}
	}

	if (GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK))
	{
		//THE floppy LEDs affect the screen dimensions.
		hDiskPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)->GetHwnd();
		bShowFloppyLed = IsDlgButtonChecked(hDiskPage, IDC_CHK_SHOWFLOPPYLED) != 0;
	}


	HCFG::EMUBORDERSIZE border;
	ReadBorder(&border);
	C64WindowDimensions dims;
	dims.SetBorder(border);

	bAdd1X = false;
	bAdd2X = false;
	bAddStretch = false;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
	if (lr < 0)
	{
		return;
	}
	else if (lr == 0)
	{
		bAdd1X = true;
		bAdd2X = true;
		if (IsDlgButtonChecked(hVideoPage, IDC_DOUBLER_BLIT))
			bAddStretch = true;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr == NULL)
		{
			return;
		} 

		pMode = &((CDisplayModeInfo *)lr)->mode;
		if (pMode)
		{
			if (m_pDx->IsAcceptableMode(*pMode))
			{
				bAdd1X = m_pDx->CanMode1X(*pMode, dims, bShowFloppyLed);
				bAdd2X = m_pDx->CanMode2X(*pMode, dims, bShowFloppyLed);
				if (IsDlgButtonChecked(hVideoPage, IDC_DOUBLER_BLIT))
					bAddStretch = true;
			}
		}
	}
	if (bAdd1X)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_1X);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_1X);
			if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_1X)
			{
				currentSelection = lr;
			}
		}
	}
	if (bAdd2X)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_2X);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_2X);
			if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_2X)
			{
				currentSelection = lr;
			}
		}
	}
	if (bAddStretch)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_StretchToFit);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_ASPECTSTRETCH);
			if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_ASPECTSTRETCH)
			{
				currentSelection = lr;
			}
		}
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_StretchWithBorderClip);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP);
			if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETCURSEL, 0, 0);
}

void CDiagEmulationSettingsTab::FillFilters()
{
HWND hVideoPage, hDiskPage, hWndCboAdapter, hWndCboFilter;
LRESULT lr;
UINT iAdapterOrdinal;
D3DDISPLAYMODE *pMode;
LRESULT index;
LRESULT currentSelection = -1;
bool bAddNone, bAddPoint, bAddLinear;
bool bShowFloppyLed;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
		return;

	hWndCboFilter = GetDlgItem(hVideoPage, IDC_CBO_FILTER);
	if (hWndCboFilter == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_RESETCONTENT, 0, 0);

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_ADDSTRING, 0, (LPARAM) szAuto);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINFILTER_AUTO);
		if (NewCfg.m_blitFilter == HCFG::EMUWINFILTER_AUTO)
		{
			currentSelection = lr;
		}
	}
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr < 0 || lr >= MAXLONG)
	{
		return;
	}
	else if (lr == 0)
	{
		iAdapterOrdinal = D3DADAPTER_DEFAULT;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr < 0 || lr >= MAXLONG)
		{
			return;
		}
		iAdapterOrdinal  = m_AdapterArray[(ULONG)lr].adapterOrdinal;
	}
	D3DCAPS9 caps;
	ZeroMemory(&caps, sizeof(caps));
	if (FAILED(m_pDx->m_pD3D->GetDeviceCaps(iAdapterOrdinal, D3DDEVTYPE_HAL, &caps)))
		return ;
	
	
	if (GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK))
	{
		hDiskPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)->GetHwnd();
		bShowFloppyLed = IsDlgButtonChecked(hDiskPage, IDC_CHK_SHOWFLOPPYLED) != 0;
	}

	HCFG::EMUBORDERSIZE border;
	ReadBorder(&border);
	C64WindowDimensions dims;
	dims.SetBorder(border);

	bAddNone = true;
	bAddPoint = false;
	bAddLinear = false;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
	if (lr < 0 || lr >= MAXLONG)
	{
		return;
	}
	else if (lr == 0)
	{
		bAddPoint = true;
		bAddLinear = true;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr == NULL)
		{
			return;
		} 

		pMode = &((CDisplayModeInfo *)lr)->mode;
		if (pMode)
		{
			if (m_pDx->IsAcceptableMode(*pMode))
			{
				if (m_pDx->CanMode1X(*pMode, dims, bShowFloppyLed))
				{
					if (caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFPOINT)
						bAddPoint = true;
					if (caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MAGFLINEAR)
						bAddLinear = true;
				}
				else
				{
					if (caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFPOINT)
						bAddPoint = true;
					if (caps.StretchRectFilterCaps & D3DPTFILTERCAPS_MINFLINEAR)
						bAddLinear = true;
				}
			}
		}
	}
	if (bAddPoint)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_ADDSTRING, 0, (LPARAM) TEXT("Point"));
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETITEMDATA, lr, (LPARAM)HCFG:: EMUWINFILTER_POINT);
			if (NewCfg.m_blitFilter == HCFG::EMUWINFILTER_POINT)
			{
				currentSelection = lr;
			}
		}
	}
	if (bAddLinear)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_ADDSTRING, 0, (LPARAM) TEXT("Linear"));
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINFILTER_LINEAR);
			if (NewCfg.m_blitFilter == HCFG::EMUWINFILTER_LINEAR)
			{
				currentSelection = lr;
			}
		}
	}
	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETCURSEL, 0, 0);
}


void CDiagEmulationSettingsTab::FillBorder()
{
HWND hVideoPage, hWndCboBorder;
LRESULT lr;
LRESULT currentSelection = -1;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboBorder = GetDlgItem(hVideoPage, IDC_CBO_BORDER);
	if (hWndCboBorder == NULL)
		return;

	ComboTextAndValue lst[] = 
	{
		{
			TEXT("Full"), HCFG::EMUBORDER_FULL
		},
		{
			TEXT("TV"), HCFG::EMUBORDER_TV
		},
		{
			TEXT("Small"), HCFG::EMUBORDER_SMALL
		},
		{
			TEXT("No Side"), HCFG::EMUBORDER_NOSIDE
		},
		{
			TEXT("No Upper/Lower"), HCFG::EMUBORDER_NOTOP
		},
		{
			TEXT("None"), HCFG::EMUBORDER_NOBORDER
		},
	};
	SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_RESETCONTENT, 0, 0);

	
	for (int i = 0; i<_countof(lst); i++)
	{
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_ADDSTRING, 0, (LPARAM) lst[i].Text);
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETITEMDATA, lr, (LPARAM) lst[i].Value);
			if (NewCfg.m_borderSize == (HCFG::EMUBORDERSIZE)lst[i].Value)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETCURSEL, 0, 0);
}

void CDiagEmulationSettingsTab::FillDiskTrackZero()
{
HWND hDiskPage, hWndCboTrackZero;
LRESULT lr;
LRESULT currentSelection = -1;
StringAndInt lst[4] = {
	{TEXT("Pull high"), (int)HCFG::TZSSPullHigh}, 
	{TEXT("Pull low"), (int)HCFG::TZSSPullLow}, 
	{TEXT("Positive high (1541C compatible)"), (int)HCFG::TZSSPositiveHigh},
	{TEXT("Positive low"), (int)HCFG::TZSSPositiveLow}
};
	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK))
		return;
	hDiskPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)->GetHwnd();

	hWndCboTrackZero = GetDlgItem(hDiskPage, IDC_CBO_TRACKZEROSENSOR);
	if (hWndCboTrackZero == NULL)
		return;

	SendDlgItemMessage(hDiskPage, IDC_CBO_TRACKZEROSENSOR, CB_RESETCONTENT, 0, 0);
	
	for (int i=0; i<_countof(lst); i++)
	{
		lr = SendDlgItemMessage(hDiskPage, IDC_CBO_TRACKZEROSENSOR, CB_ADDSTRING, 0, (LPARAM) lst[i].name);
		if (lr >= 0)
		{
			SendDlgItemMessage(hDiskPage, IDC_CBO_TRACKZEROSENSOR, CB_SETITEMDATA, lr, (LPARAM) lst[i].value);
			if (NewCfg.m_TrackZeroSensorStyle == lst[i].value)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
		SendDlgItemMessage(hDiskPage, IDC_CBO_TRACKZEROSENSOR, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hDiskPage, IDC_CBO_TRACKZEROSENSOR, CB_SETCURSEL, 0, 0);
}

void CDiagEmulationSettingsTab::FillFps()
{
HWND hVideoPage, hWndCboFps;
LRESULT lr;
LRESULT currentSelection = -1;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		return;
	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboFps = GetDlgItem(hVideoPage, IDC_CBO_FPS);
	if (hWndCboFps == NULL)
		return;

	SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_RESETCONTENT, 0, 0);

	
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) TEXT("50Hz (Performance)"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUFPS_50);
		if (NewCfg.m_fps == HCFG::EMUFPS_50)
		{
			currentSelection = lr;
		}
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) TEXT("50.12Hz (Large FIR)"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUFPS_50_12);
		if (NewCfg.m_fps == HCFG::EMUFPS_50_12)
		{
			currentSelection = lr;
		}
	}

#ifdef ALLOW_EMUFPS_50_12_MULTI
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) TEXT("50.12Hz (Multistage FIR)"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUFPS_50_12_MULTI);
		if (NewCfg.m_fps == HCFG::EMUFPS_50_12_MULTI)
		{
			currentSelection = lr;
		}
	}
#endif
	if (currentSelection >= 0)
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETCURSEL, currentSelection, 0);
	else
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETCURSEL, 0, 0);
}

void CDiagEmulationSettingsTab::SettingsOnLimitSpeedChange()
{
HWND hWndCtrl;
BOOL bOn;
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_GENERAL)) != NULL)
	{
		hWnd = pPage->GetHwnd();

		bOn = (IsDlgButtonChecked(hWnd, IDC_LIMITSPEED) != BST_UNCHECKED) ? TRUE : FALSE;
		hWndCtrl = GetDlgItem(hWnd, IDC_AUDIOCLOCKSYNC);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, bOn);
		hWndCtrl = GetDlgItem(hWnd, IDC_HOSTCPU_FRIENDLY);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, bOn);
		hWndCtrl = GetDlgItem(hWnd, IDC_HOSTCPU_AGGRESSIVE);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, bOn);
		hWndCtrl = GetDlgItem(hWnd, IDC_SETTINGS_HOSTCPUUSAGE);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, bOn);
		if (bOn)
		{
			if (NewCfg.m_bCPUFriendly)
				CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_FRIENDLY);
			else
				CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_AGGRESSIVE);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_AGGRESSIVE);
			CheckDlgButton(hWnd, IDC_AUDIOCLOCKSYNC, BST_UNCHECKED);
		}
	}
}

void CDiagEmulationSettingsTab::SettingsOnPixelDoublerChange()
{
HWND hWndCtrl, hVideoPage;
bool bIsCpuDoubler;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hVideoPage = pPage->GetHwnd();
		bIsCpuDoubler = (IsDlgButtonChecked(hVideoPage, IDC_DOUBLER_CPU) != BST_UNCHECKED);

		hWndCtrl = GetDlgItem(hVideoPage, IDC_LBL_FILTER);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, !bIsCpuDoubler);

		hWndCtrl = GetDlgItem(hVideoPage, IDC_CBO_FILTER);
		if (hWndCtrl)
			EnableWindow(hWndCtrl, !bIsCpuDoubler);


		if (bIsCpuDoubler)
		{
			//Filters are not supported by the CPU pixel doubler.
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_GETCOUNT, 0, 0);
			if(lr != CB_ERR && lr >= 0 && lr > 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETCURSEL, 0, 0);
			}
		}


		if (bIsCpuDoubler)
		{
			//Remove 'stretch to fit' if there.
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_FINDSTRING, 0, (LPARAM) szVideoFilter_StretchToFit);
			if (lr != CB_ERR && lr >= 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_DELETESTRING, lr, 0);				
			}
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_FINDSTRING, 0, (LPARAM) szVideoFilter_StretchWithBorderClip);
			if (lr != CB_ERR && lr >= 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_DELETESTRING, lr, 0);				
			}
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_GETCURSEL, 0, 0);
			if (lr == CB_ERR)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETCURSEL, 0, 0);
			}

		}
		else
		{
			//Add 'stretch to fit' if not there.
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_FINDSTRING, 0, (LPARAM) szVideoFilter_StretchToFit);
			if (lr == CB_ERR || lr < 0)
			{
				lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_StretchToFit);
				if (lr >= 0)
				{
					SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_ASPECTSTRETCH);
				}
			}
			lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_FINDSTRING, 0, (LPARAM) szVideoFilter_StretchWithBorderClip);
			if (lr == CB_ERR || lr < 0)
			{
				lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM) szVideoFilter_StretchWithBorderClip);
				if (lr >= 0)
				{
					SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP);
				}
			}
		}
	}
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterOrdinal(DWORD *adapterOrdinal, GUID *adapterId)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				if (adapterOrdinal!=NULL)
					*adapterOrdinal = D3DADAPTER_DEFAULT;
				if (adapterId!=NULL)
					ZeroMemory(adapterId, sizeof(GUID));
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR && lr >= 0 && lr < MAXLONG)
				{
					if (adapterOrdinal!=NULL)
						*adapterOrdinal  = m_AdapterArray[(ULONG)lr].adapterOrdinal;
					if (adapterId!=NULL)
						*adapterId  = m_AdapterArray[(ULONG)lr].adapter.DeviceIdentifier;
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterMode(D3DDISPLAYMODE *pMode)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	ZeroMemory(pMode, sizeof(D3DDISPLAYMODE));
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR && lr != NULL)
				{
					if (pMode != NULL)
						*pMode  = ((CDisplayModeInfo *)lr)->mode;
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterFormat(D3DFORMAT *pFormat)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	*pFormat = D3DFMT_UNKNOWN;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_FORMAT, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_FORMAT, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR && lr != NULL)
				{
					if (pFormat != NULL)
						*pFormat  = ((CDisplayModeInfo *)lr)->mode.Format;
					return S_OK;
				}
			}
		}
	}
	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterStretch(HCFG::EMUWINDOWSTRETCH *pStretch)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;


	*pStretch = HCFG::EMUWINSTR_AUTO;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_STRETCH, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				*pStretch = HCFG::EMUWINSTR_AUTO;
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_STRETCH, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR)
				{
					if (pStretch != NULL)
						*pStretch  = (HCFG::EMUWINDOWSTRETCH)lr;
					return S_OK;
				}
			}

		}
	}
	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterFilter(HCFG::EMUWINDOWFILTER *pFilter)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	*pFilter = HCFG::EMUWINFILTER_AUTO;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_FILTER, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				*pFilter = HCFG::EMUWINFILTER_AUTO;
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_FILTER, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR)
				{
					if (pFilter != NULL)
						*pFilter  = (HCFG::EMUWINDOWFILTER)lr;
					return S_OK;
				}
			}

		}
	}
	return E_FAIL;
}


void CDiagEmulationSettingsTab::ReadBorder(HCFG::EMUBORDERSIZE *pBorder)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	*pBorder = HCFG::EMUBORDER_FULL;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_BORDER, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, IDC_CBO_BORDER, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR)
			{
				*pBorder  = (HCFG::EMUBORDERSIZE)lr;
			}
		}
	}
	switch (*pBorder)
	{
	case HCFG::EMUBORDER_FULL:
	case HCFG::EMUBORDER_TV:
	case HCFG::EMUBORDER_SMALL:
	case HCFG::EMUBORDER_NOSIDE:
	case HCFG::EMUBORDER_NOTOP:
	case HCFG::EMUBORDER_NOBORDER:
		break;
	default:
		*pBorder = HCFG::EMUBORDER_FULL;
	}
}

void CDiagEmulationSettingsTab::ReadTrackZeroSensor(HCFG::ETRACKZEROSENSORSTYLE *v)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	*v = HCFG::TZSSPullHigh;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_TRACKZEROSENSOR, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, IDC_CBO_TRACKZEROSENSOR, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR)
			{
				*v  = (HCFG::ETRACKZEROSENSORSTYLE)lr;
			}
		}
	}
}

void CDiagEmulationSettingsTab::ReadFps(HCFG::EMUFPS *pFps)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	*pFps = HCFG::EMUFPS_50;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_FPS, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, IDC_CBO_FPS, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR)
			{
				*pFps  = (HCFG::EMUFPS)lr;
			}
		}
	}
	switch (*pFps)
	{
	case HCFG::EMUFPS_50:
	case HCFG::EMUFPS_50_12:
	case HCFG::EMUFPS_50_12_MULTI:
		break;
	default:
		*pFps = HCFG::EMUFPS_50;
	}
}

void CDiagEmulationSettingsTab::loadconfig(const CConfig *cfg)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_GENERAL)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bShowSpeed)
			CheckDlgButton(hWnd, IDC_SHOWSPEED, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_SHOWSPEED, BST_UNCHECKED);
		if (cfg->m_bLimitSpeed)
			CheckDlgButton(hWnd, IDC_LIMITSPEED, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_LIMITSPEED, BST_UNCHECKED);
		if (cfg->m_bCPUFriendly)
			CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_FRIENDLY);
		else
			CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_AGGRESSIVE);

		if (cfg->m_bD1541_Thread_Enable)
			CheckDlgButton(hWnd, IDC_DISKONSEPARATETHREAD, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_DISKONSEPARATETHREAD, BST_UNCHECKED);

		if (cfg->m_bAllowOpposingJoystick)
			CheckDlgButton(hWnd, IDC_ALLOWOPPOSINGJOYSTICK, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_ALLOWOPPOSINGJOYSTICK, BST_UNCHECKED);

		if (cfg->m_bDisableDwmFullscreen)
			CheckDlgButton(hWnd, IDC_DISABLE_DWM_FULLSCREEN, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_DISABLE_DWM_FULLSCREEN, BST_UNCHECKED);
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bSkipFrames)
			CheckDlgButton(hWnd, IDC_SKIPFRAMES, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_SKIPFRAMES, BST_UNCHECKED);
		if (cfg->m_syncMode == HCFG::FSSM_VBL)
			CheckRadioButton(hWnd, IDC_VBLSYNC, IDC_LINESYNC, IDC_VBLSYNC);
		else
			CheckRadioButton(hWnd, IDC_VBLSYNC, IDC_LINESYNC, IDC_LINESYNC);
		if (cfg->m_bUseBlitStretch)
			CheckRadioButton(hWnd, IDC_DOUBLER_BLIT, IDC_DOUBLER_CPU, IDC_DOUBLER_BLIT);
		else
			CheckRadioButton(hWnd, IDC_DOUBLER_BLIT, IDC_DOUBLER_CPU, IDC_DOUBLER_CPU);	
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_AUDIO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bSID_Emulation_Enable)
			CheckDlgButton(hWnd, IDC_SID_EMULATION, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_SID_EMULATION, BST_UNCHECKED);
		if (cfg->m_bSIDResampleMode)
			CheckRadioButton(hWnd, IDC_SID_RESAMPLE, IDC_SID_DOWNSAMPLE, IDC_SID_RESAMPLE);
		else
			CheckRadioButton(hWnd, IDC_SID_RESAMPLE, IDC_SID_DOWNSAMPLE, IDC_SID_DOWNSAMPLE);
		if (cfg->m_bAudioClockSync)
			CheckDlgButton(hWnd, IDC_AUDIOCLOCKSYNC, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_AUDIOCLOCKSYNC, BST_UNCHECKED);

		if (cfg->m_bSidDigiBoost)
			CheckDlgButton(hWnd, IDC_SIDDIGIBOOT, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_SIDDIGIBOOT, BST_UNCHECKED);
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bD1541_Emulation_Enable)
			CheckDlgButton(hWnd, IDC_1541_EMULATION, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_1541_EMULATION, BST_UNCHECKED);

		if (cfg->m_bShowFloppyLed)
			CheckDlgButton(hWnd, IDC_CHK_SHOWFLOPPYLED, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_CHK_SHOWFLOPPYLED, BST_UNCHECKED);
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bTimerBbug)
			CheckDlgButton(hWnd, IDC_CHK_TIMERBBUG, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_CHK_TIMERBBUG, BST_UNCHECKED);

		if (cfg->m_CIAMode == HCFG::CM_CIA6526)
			CheckDlgButton(hWnd, IDC_RAD_CIA6526, BST_CHECKED);
		else if (cfg->m_CIAMode == HCFG::CM_CIA6526A)
			CheckDlgButton(hWnd, IDC_RAD_CIA6526A, BST_CHECKED);
		else
			CheckDlgButton(hWnd, IDC_RAD_CIA6526A, BST_CHECKED);
	}
}

void CDiagEmulationSettingsTab::saveconfig(CConfig *cfg)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_GENERAL)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_HOSTCPU_FRIENDLY))
			cfg->m_bCPUFriendly=true;
		else
			cfg->m_bCPUFriendly=false;
		if (IsDlgButtonChecked(hWnd, IDC_SHOWSPEED))
			cfg->m_bShowSpeed = true;
		else
		{
			cfg->m_bShowSpeed = false;
		}
		if (IsDlgButtonChecked(hWnd, IDC_LIMITSPEED))
			cfg->m_bLimitSpeed = true;
		else
			cfg->m_bLimitSpeed = false;

		if (IsDlgButtonChecked(hWnd, IDC_DISKONSEPARATETHREAD))
			cfg->m_bD1541_Thread_Enable = true;
		else
			cfg->m_bD1541_Thread_Enable = false;

		if (IsDlgButtonChecked(hWnd, IDC_ALLOWOPPOSINGJOYSTICK))
			cfg->m_bAllowOpposingJoystick = true;
		else
			cfg->m_bAllowOpposingJoystick = false;

		if (IsDlgButtonChecked(hWnd, IDC_DISABLE_DWM_FULLSCREEN))
			cfg->m_bDisableDwmFullscreen = true;
		else
			cfg->m_bDisableDwmFullscreen = false;
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_SKIPFRAMES))
		{
			cfg->m_bSkipFrames = true;
		}
		else
		{
			cfg->m_bSkipFrames = false;
		}
		if (IsDlgButtonChecked(hWnd, IDC_VBLSYNC))
			cfg->m_syncMode = HCFG::FSSM_VBL;
		else
			cfg->m_syncMode = HCFG::FSSM_LINE;
		if (IsDlgButtonChecked(hWnd, IDC_DOUBLER_BLIT))
			cfg->m_bUseBlitStretch = true;
		else
			cfg->m_bUseBlitStretch = false;

		GUID zeroGuid;
		DWORD adapterOrdinal;
		GUID adapterId;
		ZeroMemory(&adapterId, sizeof(GUID));
		ZeroMemory(&zeroGuid, sizeof(GUID));
		if (SUCCEEDED(ReadAdapterOrdinal(&adapterOrdinal, &adapterId)))
		{
			if (adapterId == zeroGuid)
			{
				cfg->m_fullscreenAdapterNumber = D3DADAPTER_DEFAULT;
				cfg->m_fullscreenAdapterId = zeroGuid;
			}
			else
			{
				cfg->m_fullscreenAdapterNumber = adapterOrdinal;
				cfg->m_fullscreenAdapterId = adapterId;
			}
		}
		D3DDISPLAYMODE adapterMode;
		if (SUCCEEDED(ReadAdapterMode(&adapterMode)))
		{
			cfg->m_fullscreenWidth = adapterMode.Width;
			cfg->m_fullscreenHeight = adapterMode.Height;
		}
		D3DFORMAT adapterFormat;
		if (SUCCEEDED(ReadAdapterFormat(&adapterFormat)))
		{
			cfg->m_fullscreenFormat = adapterFormat;
		}
		HCFG::EMUWINDOWSTRETCH adapterStretch;
		if (SUCCEEDED(ReadAdapterStretch(&adapterStretch)))
		{
			cfg->m_fullscreenStretch = adapterStretch;
		}
		HCFG::EMUWINDOWFILTER adapterFilter;
		if (SUCCEEDED(ReadAdapterFilter(&adapterFilter)))
		{
			cfg->m_blitFilter = adapterFilter;
		}

		HCFG::EMUBORDERSIZE border;
		ReadBorder(&border);
		cfg->m_borderSize = border;


		HCFG::EMUFPS fps;
		ReadFps(&fps);
		cfg->m_fps = fps;

	}
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_AUDIO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_AUDIOCLOCKSYNC))
			cfg->m_bAudioClockSync=true;
		else
			cfg->m_bAudioClockSync=false;
		if (IsDlgButtonChecked(hWnd, IDC_SIDDIGIBOOT))
			cfg->m_bSidDigiBoost=true;
		else
			cfg->m_bSidDigiBoost=false;
		if (IsDlgButtonChecked(hWnd, IDC_SID_EMULATION))
		{
			cfg->m_bSID_Emulation_Enable = true;
		}
		else
			cfg->m_bSID_Emulation_Enable = false;
		if (IsDlgButtonChecked(hWnd, IDC_SID_RESAMPLE))
			cfg->m_bSIDResampleMode = true;
		else
			cfg->m_bSIDResampleMode = false;
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_1541_EMULATION))
			cfg->m_bD1541_Emulation_Enable=true;
		else
			cfg->m_bD1541_Emulation_Enable=false;

		if (IsDlgButtonChecked(hWnd, IDC_CHK_SHOWFLOPPYLED))
			cfg->m_bShowFloppyLed=true;
		else
			cfg->m_bShowFloppyLed=false;

		ReadTrackZeroSensor(&cfg->m_TrackZeroSensorStyle);
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_RAD_CIA6526))
			cfg->m_CIAMode = HCFG::CM_CIA6526; 
		else if (IsDlgButtonChecked(hWnd, IDC_RAD_CIA6526A))
			cfg->m_CIAMode = HCFG::CM_CIA6526A; 
		else
			cfg->m_CIAMode = HCFG::CM_CIA6526A; 

		if (IsDlgButtonChecked(hWnd, IDC_CHK_TIMERBBUG))
			cfg->m_bTimerBbug = true;
		else
			cfg->m_bTimerBbug = false;
	}

	cfg->m_bMaxSpeed = false;
}



BOOL CDiagEmulationSettingsTab::DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LPNMHDR lpnmhdr;
BOOL br;
HRESULT hr;
	switch (uMsg) 
	{ 
	case WM_INITDIALOG:
		//G::ArrangeOKCancel(hWndDlg);
		br = OnTabbedDialogInit(hWndDlg);
		if (!br)
			return FALSE;
		hr = CreateAllPages();
		if (FAILED(hr))
			return FALSE;
		loadconfig(&CurrentCfg);
		FillFps();
		FillBorder();
		FillDevices();
		FillModes();
		FillFormats();
		FillSizes();
		FillFilters();
		FillDiskTrackZero();
		SettingsOnLimitSpeedChange();
		SettingsOnPixelDoublerChange();
		VideoPageSizeComboBoxes();
		DiskPageSizeComboBoxes();
		OnSelChanged(hWndDlg);
		return true;
	case WM_NOTIFY: 
		if (lParam == 0)
			return FALSE;
		lpnmhdr = (LPNMHDR) lParam;
		if (lpnmhdr->hwndFrom == m_hwndTab)
		{
			switch (lpnmhdr->code) 
			{ 
			case TCN_SELCHANGING: 
				return FALSE;
			case TCN_SELCHANGE: 
				OnSelChanged(hWndDlg);
				return TRUE;
			}
		} 
		break; 
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDOK: 
			saveconfig(&NewCfg);
			EndDialog(hWndDlg, wParam); 
			return TRUE; 
		case IDCANCEL: 
			EndDialog(hWndDlg, wParam); 
			return TRUE; 
		}
		break;
	}
	return FALSE; 
}

BOOL CDiagEmulationSettingsTab::OnPageEvent(CTabPageDialog *page, HWND hWndDlg, UINT message, WPARAM wParam, LPARAM lParam) 
{ 
	switch (message) 
	{ 
	case WM_INITDIALOG:
		UpdatePage(page->m_pageindex, hWndDlg);
		return OnChildDialogInit(hWndDlg);
	case WM_DESTROY:
		if (hWndDlg == m_hwndDisplay)
			m_hwndDisplay = NULL;
		return TRUE;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) 
		{ 
		case IDC_CBO_ADAPTER:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillModes();
				FillFormats();
				FillSizes();
				FillFilters();
				VideoPageSizeComboBoxes();
				return TRUE;
			}
			break;
		case IDC_CBO_MODE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillFormats();
				FillSizes();
				FillFilters();
				VideoPageSizeComboBoxes();
				return TRUE;
			}
			break;
		case IDC_CBO_BORDER:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillSizes();
				VideoPageSizeComboBoxes();
				return TRUE;
			}
			break;
			break;
		case IDC_LIMITSPEED:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SettingsOnLimitSpeedChange();
				return TRUE;
			}
			break;
		case IDC_DOUBLER_BLIT:
		case IDC_DOUBLER_CPU:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SettingsOnPixelDoublerChange();
				return TRUE;
			}
			break;
		}
	} 
	return FALSE; 
}

void CDiagEmulationSettingsTab::UpdatePage(int pageno, HWND hWndDlg)
{
	switch (pageno)
	{
	case 0:
		UpdatePageGeneral(hWndDlg);
		break;
	case 1:
		UpdatePageVideo(hWndDlg);
		break;
	case 2:
		UpdatePageAudio(hWndDlg);
		break;
	case 3:
		UpdatePageDisk(hWndDlg);
		break;
	}
}

void CDiagEmulationSettingsTab::UpdatePageGeneral(HWND hWndDlg)
{
}

void CDiagEmulationSettingsTab::UpdatePageVideo(HWND hWndDlg)
{
}

void CDiagEmulationSettingsTab::UpdatePageAudio(HWND hWndDlg)
{
}

void CDiagEmulationSettingsTab::UpdatePageDisk(HWND hWndDlg)
{
}



CDisplayInfo::CDisplayInfo()
{
	bRequireClean = true;
	adapterOrdinal = 0;
	ZeroMemory(&adapter, sizeof(adapter));
	ZeroMemory(&monitor, sizeof(monitor));
	name = NULL;
}


CDisplayInfo::~CDisplayInfo()
{	
	if (bRequireClean)
	{
		if (name)
			GlobalFree(name);
		name = NULL;
	}
}

HRESULT CDisplayInfo::MakeName()
{
int lenName;

	if (name)
	{
		GlobalFree(name);
		name = NULL;
	}
	lenName = sizeof(adapter.Description) + lstrlen(TEXT(" ")) + sizeof(monitor.szDevice) + 1;
	name = (TCHAR *)GlobalAlloc(GPTR, lenName * sizeof(TCHAR));
	if (name == NULL)
		return E_OUTOFMEMORY;

#ifdef UNICODE

	int lenUcBuffer = 0;
	name[0] = L'\0';
	//if (SUCCEEDED(G::AnsiToUcRequiredBufferLength(adapter.Description, 0, lenUcBuffer)))
	if (SUCCEEDED(G::AnsiToUc(adapter.Description, NULL, 0, lenUcBuffer)))
	{
		WCHAR *pTempUcBuffer = (WCHAR *)malloc(lenUcBuffer * sizeof(TCHAR));
		if (pTempUcBuffer!=NULL)
		{
			if (SUCCEEDED(G::AnsiToUc(adapter.Description, pTempUcBuffer, 0)))
			{
				wcsncpy_s((WCHAR *)name, lenName, pTempUcBuffer, _TRUNCATE);
			}
			free(pTempUcBuffer);
			pTempUcBuffer = NULL;
		}
	}

	wcsncat_s((WCHAR *)name, lenName, L" ", 1);
	wcsncat_s((WCHAR *)name, lenName, monitor.szDevice, _TRUNCATE);
#else
	strncpy_s(name, lenName, adapter.Description, strlen(adapter.Description));
	int lenAnsiBuffer = 0;
	if (SUCCEEDED(G::UcToAnsiRequiredBufferLength(monitor.szDevice, 0, lenAnsiBuffer)))
	{
		char *pTempAnsiBuffer = (char *)malloc(lenAnsiBuffer);
		if (pTempAnsiBuffer!=NULL)
		{
			if (SUCCEEDED(G::UcToAnsi(monitor.szDevice,  pTempAnsiBuffer, 0)))
			{
				strncat_s(name, lenName, " ", 1);
				strncat_s(name, lenName, pTempAnsiBuffer, _TRUNCATE);
			}
			free(pTempAnsiBuffer);
			pTempAnsiBuffer = NULL;
		}
	}
#endif
	
	return S_OK;
}

CDisplayModeInfo::CDisplayModeInfo()
{
	bRequireClean = true;
	ZeroMemory(&mode, sizeof(mode));
	name = NULL;
}

CDisplayModeInfo::~CDisplayModeInfo()
{
	if (bRequireClean)
	{
		if (name)
			GlobalFree(name);
		name = NULL;
	}
}

HRESULT CDisplayModeInfo::MakeName(CDX9 *dx)
{
int lenName;
	if (name)
	{
		GlobalFree(name);
		name = NULL;
	}

	lenName = dx->GetDisplayResolutionText(mode, NULL, 0);
	name = (TCHAR *)GlobalAlloc(GPTR, lenName * sizeof(TCHAR));
	if (name == NULL)
		return E_OUTOFMEMORY;

	dx->GetDisplayResolutionText(mode, name, lenName);
	return S_OK;

}

HRESULT CDisplayModeInfo::MakeNameFormat(CDX9 *dx)
{
int lenName;
	if (name)
	{
		GlobalFree(name);
		name = NULL;
	}

	lenName = dx->GetDisplayFormatText(mode, NULL, 0);
	name = (TCHAR *)GlobalAlloc(GPTR, lenName * sizeof(TCHAR));
	if (name == NULL)
		return E_OUTOFMEMORY;

	dx->GetDisplayFormatText(mode, name, lenName);
	return S_OK;
}
