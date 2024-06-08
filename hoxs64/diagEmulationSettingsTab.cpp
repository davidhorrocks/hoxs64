#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <winuser.h>
#include "dx_version.h"
#include <assert.h>
#include "defines.h"
#include <commctrl.h>
#include "cvirwindow.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "cart.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "hconfig.h"
#include "appstatus.h"
#include "besttextwidth.h"
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

HRESULT CDiagEmulationSettingsTab::Init(Graphics *pGx, const CConfig *cfg)
{
	m_pGx = pGx;
	CurrentCfg = *cfg;
	NewCfg = *cfg;
	return S_OK;
}

void CDiagEmulationSettingsTab::VideoPageSizeComboBoxes()
{
	HWND hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	if (!hVideoPage)
	{
		return;
	}

	static int ctls[] = {IDC_CBO_ADAPTER, IDC_CBO_MODE, IDC_CBO_FORMAT, IDC_CBO_ADAPTER_REFRESH, IDC_CBO_FILTER, IDC_CBO_STRETCH, IDC_CBO_BORDER, IDC_CBO_FPS};
	G::AutoSetComboBoxHeight(hVideoPage, ctls, _countof(ctls), 0);
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
	LRESULT lr;
	HWND hWndCboAdapter;
	HWND hVideoPage;
	HMONITOR hMonitor;
	LRESULT currentSelection = -1;
	try
	{
		if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		{
			return;
		}

		hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
		if (!hVideoPage)
		{
			return;
		}

		hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
		if (hWndCboAdapter == NULL)
		{
			return;
		}

		SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_RESETCONTENT, 0, 0);
		m_AdapterArray.clear();
		m_vdxgiAdapters.clear();
		this->m_pGx->FillAdapters(m_vdxgiAdapters);
		CDisplayInfo2 displayInfoDeviceAuto;
		displayInfoDeviceAuto.isAuto = true;
		m_AdapterArray.push_back(displayInfoDeviceAuto);
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_INSERTSTRING, -1, (LPARAM) TEXT("Auto"));
		if (lr >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETITEMDATA, lr, (LPARAM) 0);
			currentSelection = lr;
		}

		HDC hdcControl = GetDC(hWndCboAdapter);
		BestTextWidthDC tw(hdcControl);
		tw.SetFont(this->defaultFont);
		for (i=0 ; i < m_vdxgiAdapters.size(); i++)
		{
			CDisplayInfo2 displayInfo;
			displayInfo.adapter = m_vdxgiAdapters[i];
			displayInfo.adapterOrdinal = i;
			if (SUCCEEDED(displayInfo.adapter->GetDesc1(&displayInfo.adapterDesc)))
			{
				UINT j = 0;
				Microsoft::WRL::ComPtr<IDXGIOutput> output;
				for (j = 0; displayInfo.adapter->EnumOutputs(j, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND; j++)
				{
					displayInfo.output = output;
					displayInfo.outputOrdinal = j;
					if (SUCCEEDED(displayInfo.output->GetDesc(&displayInfo.outputDesc)))
					{
						hMonitor = displayInfo.outputDesc.Monitor;
						if (hMonitor != 0)
						{
							displayInfo.monitor.cbSize = sizeof(MONITORINFOEX);
							if (GetMonitorInfo(hMonitor, &displayInfo.monitor))
							{
								if (SUCCEEDED(displayInfo.MakeName()))
								{
									m_AdapterArray.push_back(displayInfo);
									tw.GetWidthW(displayInfo.nameOfAdapter.c_str());
									lr = SendDlgItemMessageW(hVideoPage, IDC_CBO_ADAPTER, CB_INSERTSTRING, -1, (LPARAM)displayInfo.nameOfAdapter.c_str());
									if (lr >= 0)
									{
										SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETITEMDATA, lr, (LPARAM)(m_AdapterArray.size() - 1));
										if ((!NewCfg.m_fullscreenAdapterIsDefault))
										{
											if ((NewCfg.m_fullscreenAdapterNumber == i && NewCfg.m_fullscreenOutputNumber == j))
											{
												currentSelection = lr;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}

		if (tw.maxWidth > 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hVideoPage), 0);
		}

		if (currentSelection >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETCURSEL, currentSelection, 0);
		}
		else
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_SETCURSEL, 0, 0);
		}

		if (hdcControl)
		{
			ReleaseDC(hWndCboAdapter, hdcControl);
			hdcControl = NULL;
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

int CDiagEmulationSettingsTab::fnFindModeFormat(const CDisplayFormatInfo& mode1, const CDisplayFormatInfo& mode2)
{
	if (mode1.isAuto != mode2.isAuto)
	{
		return 1;
	}

	if (mode1.format == mode2.format)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int CDiagEmulationSettingsTab::fnCompareMode(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2)
{
	if (mode1.isAuto && !mode2.isAuto)
	{
		return -1;
	}
	else if (!mode1.isAuto && mode2.isAuto)
	{
		return 1;
	}

	if (mode1.width > mode2.width)
	{
		return 1;
	}
	else if (mode1.width < mode2.width)
	{
		return -1;
	}

	if (mode1.height > mode2.height)
	{
		return 1;
	}
	else if (mode1.height < mode2.height)
	{
		return -1;
	}

	if (mode1.scanlineOrdering > mode2.scanlineOrdering)
	{
		return 1;
	}
	else if (mode1.scanlineOrdering < mode2.scanlineOrdering)
	{
		return -1;
	}

	if (mode1.scaling > mode2.scaling)
	{
		return 1;
	}
	else if (mode1.scaling < mode2.scaling)
	{
		return -1;
	}

	return 0;
}

bool CDiagEmulationSettingsTab::fnForListCompareMode(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2)
{
	return fnCompareMode(mode1, mode2) < 0;
}

int CDiagEmulationSettingsTab::fnCompareFormat(const CDisplayFormatInfo &mode1, const CDisplayFormatInfo &mode2)
{
	if (mode1.isAuto && !mode2.isAuto)
	{
		return -1;
	}
	else if (!mode1.isAuto && mode2.isAuto)
	{
		return 1;
	}

	if (GraphicsHelper::GetBitsPerPixel(mode1.format) > GraphicsHelper::GetBitsPerPixel(mode2.format))
	{
		return 1;
	}
	else if (GraphicsHelper::GetBitsPerPixel(mode1.format) < GraphicsHelper::GetBitsPerPixel(mode2.format))
	{
		return -1;
	}

	if (mode1.format > mode2.format)
	{
		return 1;
	}
	else if (mode1.format < mode2.format)
	{
		return -1;
	}

	return 0;
}

bool CDiagEmulationSettingsTab::fnForListCompareFormat(const CDisplayFormatInfo& mode1, const CDisplayFormatInfo& mode2)
{
	return fnCompareFormat(mode1, mode2) < 0;
}

void CDiagEmulationSettingsTab::FillModes()
{
HWND hVideoPage, hWndCboAdapter;
LRESULT lr;
LRESULT index;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr < 0)
	{
		FillModes2(0);
		return;
	}
	else if (lr == 0)
	{
		FillModes2(0);
		return;
	}

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr < 0)
	{
		FillModes2(0);
		return;
	}

	if ((size_t)lr >= m_AdapterArray.size())
	{
		FillModes2(0);
		return;
	} 

	FillModes2((int)lr);
}

void CDiagEmulationSettingsTab::FillModes2(int displayInfoIndex)
{
	HWND hVideoPage, hWndCboMode;
	LRESULT currentSelection = -1;
	LRESULT lr;
	HRESULT hr;
	try
	{
		if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		{
			return;
		}

		hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
		hWndCboMode = GetDlgItem(hVideoPage, IDC_CBO_MODE);
		if (hWndCboMode == NULL)
		{
			return;
		}

		if (displayInfoIndex < 0 || (size_t)displayInfoIndex >= m_AdapterArray.size())
		{
			return;
		}

		CDisplayInfo2& currentAdapter = m_AdapterArray[displayInfoIndex];

		SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_RESETCONTENT, 0, 0);

		m_ModeList.clear();
		m_ModeCompleteList.clear();

		if (!currentAdapter.isAuto)
		{
			std::vector<DXGI_MODE_DESC> modeDescArray;
			for (UINT iFormatToTest = 0; iFormatToTest < _countof(GraphicsHelper::Formats); iFormatToTest++)
			{
				UINT iModeCount = 0;
				modeDescArray.clear();
				hr = currentAdapter.output->GetDisplayModeList(GraphicsHelper::Formats[iFormatToTest], DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &iModeCount, NULL);
				if (SUCCEEDED(hr))
				{
					if (iModeCount > 0)
					{
						modeDescArray.resize(iModeCount);
						hr = currentAdapter.output->GetDisplayModeList(GraphicsHelper::Formats[iFormatToTest], DXGI_ENUM_MODES_INTERLACED | DXGI_ENUM_MODES_SCALING, &iModeCount, modeDescArray.data());
						if (SUCCEEDED(hr))
						{
							for (std::vector<DXGI_MODE_DESC>::const_iterator iter = modeDescArray.cbegin(); iter != modeDescArray.cend(); iter++)
							{
								const DXGI_MODE_DESC& modeDesc = *iter;
								if (GraphicsHelper::IsAcceptableMode(modeDesc.Width, modeDesc.Height, modeDesc.Format))
								{
									m_ModeCompleteList.push_back(modeDesc);
								}
							}
						}
					}
				}
			}
		}

		for (auto iter = m_ModeCompleteList.begin(); iter != m_ModeCompleteList.end(); iter++)
		{
			DXGI_MODE_DESC& modeDesc = *iter;
			CDisplayModeInfo mode;
			mode.isAuto = false;
			mode.height = modeDesc.Height;
			mode.width = modeDesc.Width;
			mode.scaling = modeDesc.Scaling;
			mode.scanlineOrdering = modeDesc.ScanlineOrdering;
			mode.MakeName();
			CDisplayModeInfo_WxH_is_equal mode_equal_WxH(mode);
			if (std::find_if(m_ModeList.cbegin(), m_ModeList.cend(), mode_equal_WxH) != m_ModeList.cend())
			{
				continue;
			}

			m_ModeList.push_back(mode);
		}

		// Sort list
		CDisplayModeInfo_WxH_is_less mode_is_less;
		m_ModeList.sort(mode_is_less);

		// Insert Auto option
		CDisplayModeInfo modeInfoAuto;
		modeInfoAuto.isAuto = true;
		modeInfoAuto.MakeName();
		m_ModeList.push_front(modeInfoAuto);

		currentSelection = 0;
		// Fill the modes dropdrown list.
		if (m_ModeList.size() > 0)
		{
			LRESULT widthAndHeightOnlySelection = -1;
			bool foundExact = false;
			bool foundWidthAndHeightOnly = false;
			DXGI_MODE_SCALING currentScaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_STRETCHED;
			DXGI_MODE_SCANLINE_ORDER currentScanLineOrder = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST;
			for (auto iter = m_ModeList.begin(); iter != m_ModeList.end(); iter++)
			{
				CDisplayModeInfo& mode = *iter;
				lr = SendDlgItemMessageW(hVideoPage, IDC_CBO_MODE, CB_INSERTSTRING, -1, (LPARAM)mode.name.c_str());
				if (lr >= 0)
				{
					SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETITEMDATA, lr, (LPARAM)&mode);
					if (mode.isAuto)
					{
						if (NewCfg.m_fullscreenWidth == 0 || currentAdapter.isAuto)
						{
							if (!foundExact)
							{
								currentSelection = lr;
								foundExact = true;
							}
						}
					}
					else
					{
						if (NewCfg.m_fullscreenWidth == mode.width && NewCfg.m_fullscreenHeight == mode.height && NewCfg.m_fullscreenDxGiModeScaling == mode.scaling && NewCfg.m_fullscreenDxGiModeScanlineOrdering == mode.scanlineOrdering)
						{
							if (!foundExact)
							{
								currentSelection = lr;
								foundExact = true;
							}
						}
					}

					if (NewCfg.m_fullscreenWidth == mode.width && NewCfg.m_fullscreenHeight == mode.height)
					{
						if (mode.scaling <= currentScaling && mode.scanlineOrdering <= currentScanLineOrder)
						{
							widthAndHeightOnlySelection = lr;
							foundWidthAndHeightOnly = true;
						}
					}
				}
			}

			if (!foundExact && foundWidthAndHeightOnly)
			{
				currentSelection = widthAndHeightOnlySelection;
			}
		}

		if (currentSelection >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETCURSEL, currentSelection, 0);
		}
		else
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_SETCURSEL, 0, 0);
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

void CDiagEmulationSettingsTab::FillFormats()
{
	HWND hVideoPage, hWndCboAdapter, hWndCboMode;
	LRESULT lr;
	int displayInfoIndex;
	LRESULT index;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();

	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	hWndCboMode = GetDlgItem(hVideoPage, IDC_CBO_MODE);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		//clear formats list
		FillFormats2(NULL);
		return;
	}
	else if (lr == 0)
	{
		displayInfoIndex = 0;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr < 0)
		{
			//clear formats list
			FillFormats2(NULL);
			return;
		}

		displayInfoIndex = (int)lr;
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		//clear formats list
		FillFormats2(NULL);
		return;
	}
	else if (lr == 0)
	{
		FillFormats2(NULL);
		return;
	}

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr == NULL)
	{
		FillFormats2(NULL);
		return;
	} 

	const CDisplayModeInfo *pmode = reinterpret_cast<CDisplayModeInfo *>(lr);
	FillFormats2(pmode);
}

void CDiagEmulationSettingsTab::FillFormats2(const CDisplayModeInfo* pModeSelected)
{
	HWND hVideoPage, hWndCboFormat;
	LRESULT currentSelection = -1;
	CDisplayModeInfo modeInfo;
	LRESULT lr;

	try
	{
		if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		{
			return;
		}

		hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
		hWndCboFormat = GetDlgItem(hVideoPage, IDC_CBO_FORMAT);
		if (hWndCboFormat == NULL)
		{
			return;
		}

		SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_RESETCONTENT, 0, 0);
		m_ModeFormatList.clear();
	
		if (pModeSelected != NULL && !pModeSelected->isAuto && pModeSelected->width != 0 && pModeSelected->height != 0)
		{
			CDisplayModeInfo_WxH_is_equal mode_equal_WxH(*pModeSelected);
			for (auto iter = m_ModeCompleteList.begin(); iter != m_ModeCompleteList.end(); iter++)
			{
				DXGI_MODE_DESC& modeDesc = *iter;
				if (pModeSelected->height != modeDesc.Height || pModeSelected->width != modeDesc.Width || pModeSelected->scaling != modeDesc.Scaling || pModeSelected->scanlineOrdering != modeDesc.ScanlineOrdering)
				{
					continue;
				}

				CDisplayFormatInfo format;
				format.format = modeDesc.Format;
				format.MakeName();
				CDisplayModeInfo_format_is_equal mode_equal_format(format);
				if (std::find_if(m_ModeFormatList.cbegin(), m_ModeFormatList.cend(), mode_equal_format) != m_ModeFormatList.cend())
				{
					continue;
				}

				m_ModeFormatList.push_back(format);
			}
		}

		CDisplayModeInfo_format_is_less mode_is_less;
		m_ModeFormatList.sort(mode_is_less);

		// Insert Auto option
		CDisplayFormatInfo formatInfoAuto;
		formatInfoAuto.isAuto = true;
		formatInfoAuto.MakeName();
		m_ModeFormatList.push_front(formatInfoAuto);
		currentSelection = 0;
		bool foundExact = false;
		for (auto iter = m_ModeFormatList.begin(); iter != m_ModeFormatList.end(); iter++)
		{
			CDisplayFormatInfo& format = *iter;
			lr = SendDlgItemMessageW(hVideoPage, IDC_CBO_FORMAT, CB_INSERTSTRING, -1, (LPARAM)iter->name.c_str());
			if (lr >= 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETITEMDATA, lr, (LPARAM)&format);
				if (format.isAuto)
				{
					if (NewCfg.m_fullscreenFormat == 0 || pModeSelected == NULL || pModeSelected->isAuto)
					{
						if (!foundExact)
						{
							currentSelection = lr;
							foundExact = true;
						}
					}
				}
				else
				{
					if (NewCfg.m_fullscreenFormat == format.format)
					{
						if (!foundExact)
						{
							currentSelection = lr;
							foundExact = true;
						}
					}
				}
			}
		}

		if (currentSelection >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETCURSEL, currentSelection, 0);
		}
		else
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_SETCURSEL, 0, 0);
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

void CDiagEmulationSettingsTab::FillRefresh()
{
HWND hVideoPage, hWndCboAdapter, hWndCboMode;
LRESULT lr;
LRESULT index;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	hWndCboMode = GetDlgItem(hVideoPage, IDC_CBO_MODE);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	unsigned int iAdapterOrdinal = 0;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		FillRefresh2(NULL, NULL);
		return;
	}
	else if (lr == 0)
	{
		iAdapterOrdinal = 0;
	}
	else
	{
		index = lr;
		lr = SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
		if (lr == CB_ERR || lr < 0)
		{
			FillRefresh2(NULL, NULL);
			return;
		}

		iAdapterOrdinal  = m_AdapterArray[(ULONG)lr].adapterOrdinal;
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		FillRefresh2(NULL, NULL);
		return;
	}	

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr == NULL)
	{
		FillRefresh2(NULL, NULL);
		return;
	} 

	const CDisplayModeInfo* pmode = reinterpret_cast<CDisplayModeInfo*>(lr);

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_GETCURSEL, 0, 0);
	if (lr == CB_ERR || lr < 0)
	{
		FillRefresh2(pmode, NULL);
		return;
	}

	index = lr;
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FORMAT, CB_GETITEMDATA, index, 0);
	if (lr == CB_ERR || lr == NULL)
	{
		FillRefresh2(pmode, NULL);
		return;
	} 
	
	const CDisplayFormatInfo* pformat = reinterpret_cast<CDisplayFormatInfo*>(lr);
	FillRefresh2(pmode, pformat);
}

void CDiagEmulationSettingsTab::FillRefresh2(const CDisplayModeInfo* pModeSelected, const CDisplayFormatInfo* pFormatSelected)
{
	HWND hVideoPage, hWndCboRefresh;
	LRESULT currentSelection = -1;
	CDisplayModeInfo  modeInfo;
	LRESULT lr;

	try
	{
		if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
		{
			return;
		}

		hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
		hWndCboRefresh = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER_REFRESH);
		if (hWndCboRefresh == NULL)
		{
			return;
		}

		SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER_REFRESH, CB_RESETCONTENT, 0, 0);
		m_ModeRefreshList.clear();
		if (pModeSelected != NULL && !pModeSelected->isAuto && pFormatSelected != NULL && !pFormatSelected->isAuto)
		{
			for (auto iter = m_ModeCompleteList.begin(); iter != m_ModeCompleteList.end(); iter++)
			{
				DXGI_MODE_DESC& modeDesc = *iter;
				if (pModeSelected->height != modeDesc.Height || pModeSelected->width != modeDesc.Width || pModeSelected->scaling != modeDesc.Scaling || pModeSelected->scanlineOrdering != modeDesc.ScanlineOrdering)
				{
					continue;
				}

				if (pFormatSelected->format != modeDesc.Format)
				{
					continue;
				}

				CDisplayRefreshInfo refresh;
				refresh.refreshRateNumerator = modeDesc.RefreshRate.Numerator;
				refresh.refreshRateDenominator = modeDesc.RefreshRate.Denominator;
				refresh.MakeName();
				CDisplayModeInfo_refresh_is_equal refresh_equal(refresh);
				if (std::find_if(m_ModeRefreshList.cbegin(), m_ModeRefreshList.cend(), refresh_equal) != m_ModeRefreshList.cend())
				{
					continue;
				}

				m_ModeRefreshList.push_back(refresh);
			}
		}

		CDisplayModeInfo_refresh_is_less refresh_is_less;
		m_ModeRefreshList.sort(refresh_is_less);

		// Insert Auto option
		CDisplayRefreshInfo refreshInfoAuto;
		refreshInfoAuto.isAuto = true;
		refreshInfoAuto.MakeName();
		m_ModeRefreshList.push_front(refreshInfoAuto);
		currentSelection = 0;
		bool foundExact = false;

		for (auto iter = m_ModeRefreshList.begin(); iter != m_ModeRefreshList.end(); iter++)
		{
			CDisplayRefreshInfo& refresh = *iter;
			lr = SendDlgItemMessageW(hVideoPage, IDC_CBO_ADAPTER_REFRESH, CB_INSERTSTRING, -1, (LPARAM)iter->name.c_str());
			if (lr >= 0)
			{
				SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER_REFRESH, CB_SETITEMDATA, lr, (LPARAM) & *iter);
				if (refresh.isAuto)
				{
					if (NewCfg.m_fullscreenRefreshNumerator == 0 || NewCfg.m_fullscreenRefreshDenominator == 0 || pFormatSelected == NULL || pFormatSelected->isAuto)
					{
						if (!foundExact)
						{
							currentSelection = lr;
							foundExact = true;
						}
					}
				}
				else
				{
					if (NewCfg.m_fullscreenRefreshNumerator == refresh.refreshRateNumerator && NewCfg.m_fullscreenRefreshDenominator == refresh.refreshRateDenominator)
					{
						if (!foundExact)
						{
							currentSelection = lr;
							foundExact = true;
						}
					}
				}
			}
		}

		if (currentSelection >= 0)
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER_REFRESH, CB_SETCURSEL, currentSelection, 0);
		}
		else
		{
			SendDlgItemMessage(hVideoPage, IDC_CBO_ADAPTER_REFRESH, CB_SETCURSEL, 0, 0);
		}
	}
	catch (std::exception&)
	{
		return;
	}
}

void CDiagEmulationSettingsTab::FillSizes()
{
HWND hVideoPage, hDiskPage, hWndCboSize;
LRESULT lr;
LRESULT currentSelection = -1;
bool bShowFloppyLed;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboSize = GetDlgItem(hVideoPage, IDC_CBO_STRETCH);
	if (hWndCboSize == NULL)
	{
		return;
	}

	HDC hdcControl = GetDC(hWndCboSize);
	BestTextWidthDC tw(hdcControl);
	tw.SetFont(this->defaultFont);
	SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_RESETCONTENT, 0, 0);
	tw.GetWidth(szAuto);
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

	tw.GetWidth(szVideoFilter_StretchToFit);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM)szVideoFilter_StretchToFit);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM)HCFG::EMUWINSTR_ASPECTSTRETCH);
		if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_ASPECTSTRETCH)
		{
			currentSelection = lr;
		}
	}

	tw.GetWidth(szVideoFilter_StretchWithBorderClip);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM)szVideoFilter_StretchWithBorderClip);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM)HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP);
		if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_ASPECTSTRETCHBORDERCLIP)
		{
			currentSelection = lr;
		}
	}

	tw.GetWidth(szVideoFilter_1X);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_ADDSTRING, 0, (LPARAM)szVideoFilter_1X);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETITEMDATA, lr, (LPARAM)HCFG::EMUWINSTR_1X);
		if (NewCfg.m_fullscreenStretch == HCFG::EMUWINSTR_1X)
		{
			currentSelection = lr;
		}
	}

	if (currentSelection >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETCURSEL, currentSelection, 0);
	}
	else
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETCURSEL, 0, 0);
	}

	if (tw.maxWidth > 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_STRETCH, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hVideoPage), 0);
	}

	if (hdcControl != NULL)
	{
		ReleaseDC(hWndCboSize, hdcControl);
		hdcControl = NULL;
	}
}

void CDiagEmulationSettingsTab::FillFilters()
{
	HWND hVideoPage, hDiskPage, hWndCboAdapter, hWndCboFilter;
	LRESULT lr;
	bool bShowFloppyLed;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboAdapter = GetDlgItem(hVideoPage, IDC_CBO_ADAPTER);
	if (hWndCboAdapter == NULL)
	{
		return;
	}

	hWndCboFilter = GetDlgItem(hVideoPage, IDC_CBO_FILTER);
	if (hWndCboFilter == NULL)
	{
		return;
	}

	LRESULT currentSelection = -1;
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
	
	if (GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK))
	{
		hDiskPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)->GetHwnd();
		bShowFloppyLed = IsDlgButtonChecked(hDiskPage, IDC_CHK_SHOWFLOPPYLED) != 0;
	}

	HCFG::EMUBORDERSIZE border;
	ReadBorder(&border);
	C64WindowDimensions dims;
	dims.SetBorder(border);

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_ADDSTRING, 0, (LPARAM) TEXT("Point"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETITEMDATA, lr, (LPARAM)HCFG:: EMUWINFILTER_POINT);
		if (NewCfg.m_blitFilter == HCFG::EMUWINFILTER_POINT)
		{
			currentSelection = lr;
		}
	}

	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_ADDSTRING, 0, (LPARAM) TEXT("Linear"));
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUWINFILTER_LINEAR);
		if (NewCfg.m_blitFilter == HCFG::EMUWINFILTER_LINEAR)
		{
			currentSelection = lr;
		}
	}

	if (currentSelection >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETCURSEL, currentSelection, 0);
	}
	else
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FILTER, CB_SETCURSEL, 0, 0);
	}
}

void CDiagEmulationSettingsTab::FillSidCount()
{
HWND hWndPage;
HWND hWndCbo;
LRESULT lr;
LRESULT currentSelection = -1;
const int MaxExtraSids = 7;
TCHAR str[10];
const int CBO_CONTROL_ID = IDC_CBO_EXTRA_SID;

	shared_ptr<CTabPageDialog> page = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP);
	if (!page)
	{
		return;
	}

	hWndPage = page->GetHwnd();
	hWndCbo = GetDlgItem(hWndPage, CBO_CONTROL_ID);
	if (hWndCbo == NULL)
	{
		return;
	}

	HDC hdcControl = GetDC(hWndCbo);
	BestTextWidthDC tw(hdcControl);
	tw.SetFont(this->defaultFont);

	SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_RESETCONTENT, 0, 0);	
	for (int sidExtraCount = 0; sidExtraCount <= MaxExtraSids; sidExtraCount++)
	{
		_sntprintf_s(str, _countof(str), _TRUNCATE, TEXT("%d"), sidExtraCount);
		tw.GetWidth(str);
		lr = SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_ADDSTRING, 0, (LPARAM) str);
		if (lr >= 0)
		{
			SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_SETITEMDATA, lr, (LPARAM) sidExtraCount);
			if (NewCfg.m_numberOfExtraSIDs == sidExtraCount)
			{
				currentSelection = lr;
			}
		}
	}

	if (currentSelection >= 0)
	{
		SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_SETCURSEL, currentSelection, 0);
	}
	else
	{
		SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_SETCURSEL, 0, 0);
	}

	if (tw.maxWidth > 0)
	{
		SendDlgItemMessage(hWndPage, CBO_CONTROL_ID, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hWndPage), 0);
	}

	if (hdcControl != NULL)
	{
		ReleaseDC(hWndCbo, hdcControl);
		hdcControl = NULL;
	}
}

void CDiagEmulationSettingsTab::FillSidAddress()
{
HWND hWndPage;
HWND hWndCbo;
LRESULT lr;
LRESULT currentSelection = -1;
const int MaxExtraSids = 7;
TCHAR str[10];
const int CBO_CONTROL_ID[] = { IDC_CBO_SID2_ADDRESS, IDC_CBO_SID3_ADDRESS, IDC_CBO_SID4_ADDRESS, IDC_CBO_SID5_ADDRESS, IDC_CBO_SID6_ADDRESS, IDC_CBO_SID7_ADDRESS, IDC_CBO_SID8_ADDRESS };
const int LBL_CONTROL_ID[] = { IDC_LBL_SID2_ADDRESS, IDC_LBL_SID3_ADDRESS, IDC_LBL_SID4_ADDRESS, IDC_LBL_SID5_ADDRESS, IDC_LBL_SID6_ADDRESS, IDC_LBL_SID7_ADDRESS, IDC_LBL_SID8_ADDRESS };
bit16* cfgaddress[]  = { &NewCfg.m_Sid2Address, &NewCfg.m_Sid3Address, &NewCfg.m_Sid4Address, &NewCfg.m_Sid5Address, &NewCfg.m_Sid6Address, &NewCfg.m_Sid7Address, &NewCfg.m_Sid8Address };

	shared_ptr<CTabPageDialog> page = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP);
	if (!page)
	{
		return;
	}

	hWndPage = page->GetHwnd();
	for (int k=0; k < _countof(CBO_CONTROL_ID); k++)
	{
		currentSelection = -1;
		hWndCbo = GetDlgItem(hWndPage, CBO_CONTROL_ID[k]);
		if (hWndCbo == NULL)
		{
			continue;
		}

		HDC hdcControl = GetDC(hWndCbo);
		BestTextWidthDC tw(hdcControl);
		tw.SetFont(this->defaultFont);

		SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_RESETCONTENT, 0, 0);	
		for (int sidAddress = 0xD420; sidAddress <= 0xDFE0; sidAddress += 0x20)
		{
			if (sidAddress == 0xD800)
			{
				sidAddress = 0xDE00;
			}			

			_sntprintf_s(str, _countof(str), _TRUNCATE, TEXT("%4X"), sidAddress);
			tw.GetWidth(str);
			lr = SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_ADDSTRING, 0, (LPARAM) str);
			if (lr >= 0)
			{
				SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_SETITEMDATA, lr, (LPARAM) sidAddress);
				if (*cfgaddress[k] == sidAddress)
				{
					currentSelection = lr;
				}
			}
		}

		if (currentSelection >= 0)
		{
			SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_SETCURSEL, currentSelection, 0);
		}
		else
		{
			SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_SETCURSEL, 0, 0);
		}

		if (tw.maxWidth > 0)
		{
			SendDlgItemMessage(hWndPage, CBO_CONTROL_ID[k], CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hWndPage), 0);
		}

		if (hdcControl != NULL)
		{
			ReleaseDC(hWndCbo, hdcControl);
			hdcControl = NULL;
		}
	}
}

void CDiagEmulationSettingsTab::FillBorder()
{
HWND hVideoPage, hWndCboBorder;
LRESULT lr;
LRESULT currentSelection = -1;

	if (!GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO))
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboBorder = GetDlgItem(hVideoPage, IDC_CBO_BORDER);
	if (hWndCboBorder == NULL)
	{
		return;
	}

	HDC hdcControl = GetDC(hWndCboBorder);
	BestTextWidthDC tw(hdcControl);
	tw.SetFont(this->defaultFont);

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
		tw.GetWidth(lst[i].Text);
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
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETCURSEL, currentSelection, 0);
	}
	else
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETCURSEL, 0, 0);
	}

	if (tw.maxWidth > 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_BORDER, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hVideoPage), 0);
	}

	if (hdcControl != NULL)
	{
		ReleaseDC(hWndCboBorder, hdcControl);
		hdcControl = NULL;
	}
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
	{
		return;
	}

	hVideoPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)->GetHwnd();
	hWndCboFps = GetDlgItem(hVideoPage, IDC_CBO_FPS);
	if (hWndCboFps == NULL)
	{
		return;
	}

	LPCTSTR ptxt;
	HDC hdcControl = GetDC(hWndCboFps);
	BestTextWidthDC tw(hdcControl);
	tw.SetFont(this->defaultFont);
	SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_RESETCONTENT, 0, 0);	

	ptxt = TEXT("50Hz (Performance)");
	tw.GetWidth(ptxt);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) ptxt);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUFPS_50);
		if (NewCfg.m_fps == HCFG::EMUFPS_50)
		{
			currentSelection = lr;
		}
	}

	ptxt = TEXT("50.12Hz (Large FIR)");
	tw.GetWidth(ptxt);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) ptxt);
	if (lr >= 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETITEMDATA, lr, (LPARAM) HCFG::EMUFPS_50_12);
		if (NewCfg.m_fps == HCFG::EMUFPS_50_12)
		{
			currentSelection = lr;
		}
	}

#ifdef ALLOW_EMUFPS_50_12_MULTI
	ptxt = TEXT("50.12Hz (Multistage FIR)");
	tw.GetWidth(ptxt);
	lr = SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_ADDSTRING, 0, (LPARAM) ptxt);
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
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETCURSEL, currentSelection, 0);
	}
	else
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETCURSEL, 0, 0);
	}

	if (tw.maxWidth > 0)
	{
		SendDlgItemMessage(hVideoPage, IDC_CBO_FPS, CB_SETDROPPEDWIDTH, tw.GetSuggestedDlgComboBoxWidth(hVideoPage), 0);
	}

	if (hdcControl != NULL)
	{
		ReleaseDC(hWndCboFps, hdcControl);
		hdcControl = NULL;
	}
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

void CDiagEmulationSettingsTab::SettingsOnCiaChipChange()
{
HWND hTabPage;
bool bIsNewCia;
shared_ptr<CTabPageDialog> pPage;
	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP)) != NULL)
	{
		hTabPage = pPage->GetHwnd();
		bIsNewCia = (IsDlgButtonChecked(hTabPage, IDC_RAD_CIA6526A) != BST_UNCHECKED);
		if (bIsNewCia)
		{
			CheckDlgButton(hTabPage, IDC_CHK_TIMERBBUG, BST_UNCHECKED);
		}
		else
		{
			CheckDlgButton(hTabPage, IDC_CHK_TIMERBBUG, BST_CHECKED);
		}
	}
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterOrdinal(bool* pbIsDefault, unsigned int* pAdapterOrdinal, unsigned int* pOutputOrdinal)
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
				if (pbIsDefault != NULL)
				{
					*pbIsDefault = true;
				}

				if (pAdapterOrdinal != NULL)
				{
					*pAdapterOrdinal = 0;
				}

				if (pOutputOrdinal != NULL)
				{
					*pOutputOrdinal = 0;
				}

				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_ADAPTER, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR && lr >= 0 && (size_t)lr < m_AdapterArray.size())
				{
					const CDisplayInfo2& displayInfo = m_AdapterArray[(size_t)lr];
					if (pAdapterOrdinal != NULL)
					{
						*pAdapterOrdinal = displayInfo.adapterOrdinal;
					}

					if (pOutputOrdinal != NULL)
					{
						*pOutputOrdinal = displayInfo.outputOrdinal;
					}

					if (pbIsDefault != NULL)
					{
						*pbIsDefault = false;
					}

					return S_OK;
				}
			}
		}
	}

	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterMode(CDisplayModeInfo **ppMode)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_MODE, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, IDC_CBO_MODE, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR && lr != 0)
			{
				if (ppMode != NULL)
				{
					*ppMode  = reinterpret_cast<CDisplayModeInfo *>(lr);
					return S_OK;
				}
			}
		}
	}

	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterFormat(CDisplayFormatInfo** ppFormat)
{
	HWND hWnd;
	shared_ptr<CTabPageDialog> pPage;
	LRESULT lr;
	LRESULT index;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_FORMAT, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, IDC_CBO_FORMAT, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR && lr != 0)
			{
				if (ppFormat != NULL)
				{
					*ppFormat = reinterpret_cast<CDisplayFormatInfo*>(lr);
					return S_OK;
				}
			}
		}
	}

	return E_FAIL;
}

HRESULT CDiagEmulationSettingsTab::ReadAdapterRefresh(UINT *pRefreshRateNumerator, UINT* pRefreshRateDenominator)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;
LRESULT lr;
LRESULT index;

	if (pRefreshRateNumerator != NULL)
	{
		*pRefreshRateNumerator = 0;
	}

	if (pRefreshRateDenominator != NULL)
	{
		*pRefreshRateDenominator = 0;
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, IDC_CBO_ADAPTER_REFRESH, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			if (lr == 0)
			{
				return S_OK;
			}
			else
			{
				index = lr;
				lr = SendDlgItemMessage(hWnd, IDC_CBO_ADAPTER_REFRESH, CB_GETITEMDATA, index, 0);
				if (lr != CB_ERR && lr != 0)
				{
					CDisplayRefreshInfo* pRefreshRate = reinterpret_cast<CDisplayRefreshInfo *>(lr);
					if (!pRefreshRate->isAuto)
					{
						if (pRefreshRateNumerator != NULL)
						{
							*pRefreshRateNumerator = pRefreshRate->refreshRateNumerator;
						}

						if (pRefreshRateDenominator != NULL)
						{
							*pRefreshRateDenominator = pRefreshRate->refreshRateDenominator;
						}
					}

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
					{
						*pStretch = (HCFG::EMUWINDOWSTRETCH)lr;
					}

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
					{
						*pFilter = (HCFG::EMUWINDOWFILTER)lr;
					}

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

bool CDiagEmulationSettingsTab::ReadComboItemDataInt(int page, int control_id, int *data)
{
	HWND hWnd;
	shared_ptr<CTabPageDialog> pPage;
	LRESULT lr;
	LRESULT index;

	*data = 0;
	pPage = GetPage(page);
	if (pPage)
	{
		hWnd = pPage->GetHwnd();
		lr = SendDlgItemMessage(hWnd, control_id, CB_GETCURSEL, 0, 0);
		if (lr != CB_ERR && lr >= 0)
		{
			index = lr;
			lr = SendDlgItemMessage(hWnd, control_id, CB_GETITEMDATA, index, 0);
			if (lr != CB_ERR)
			{
				*data  = (int)lr;
				return true;
			}
		}
	}

	return false;
}

bool CDiagEmulationSettingsTab::ReadExtraSidCount(int *extraSidCount)
{
	return ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_EXTRA_SID, extraSidCount);
}

bool CDiagEmulationSettingsTab::ReadSidAddress2(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID2_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress3(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID3_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress4(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID4_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress5(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID5_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress6(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID6_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress7(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID7_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
	}
}

bool CDiagEmulationSettingsTab::ReadSidAddress8(bit16 *sidAddress)
{
	int v;
	if (ReadComboItemDataInt(CDiagEmulationSettingsTab::TABPAGE_CHIP, IDC_CBO_SID8_ADDRESS, &v))
	{
		*sidAddress = (bit16)v;
		return true;
	}
	else
	{
		*sidAddress=0;
		return false;
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
		{
			CheckDlgButton(hWnd, IDC_SHOWSPEED, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_SHOWSPEED, BST_UNCHECKED);
		}

		if (cfg->m_bLimitSpeed)
		{
			CheckDlgButton(hWnd, IDC_LIMITSPEED, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_LIMITSPEED, BST_UNCHECKED);
		}

		if (cfg->m_bCPUFriendly)
		{
			CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_FRIENDLY);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_HOSTCPU_FRIENDLY, IDC_HOSTCPU_AGGRESSIVE, IDC_HOSTCPU_AGGRESSIVE);
		}

		if (cfg->m_bD1541_Thread_Enable)
		{
			CheckDlgButton(hWnd, IDC_DISKONSEPARATETHREAD, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_DISKONSEPARATETHREAD, BST_UNCHECKED);
		}

		if (cfg->m_bAllowOpposingJoystick)
		{
			CheckDlgButton(hWnd, IDC_ALLOWOPPOSINGJOYSTICK, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_ALLOWOPPOSINGJOYSTICK, BST_UNCHECKED);
		}

		if (cfg->m_bEnableImGuiWindowed)
		{
			CheckDlgButton(hWnd, IDC_IMGUI_WINDOWEDMODE_ENABLE, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_IMGUI_WINDOWEDMODE_ENABLE, BST_UNCHECKED);
		}

		if (cfg->m_bSaveWindowPositionOnExit)
		{
			CheckDlgButton(hWnd, IDC_SAVE_WINDOW_POSITION_ON_EXIT, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_SAVE_WINDOW_POSITION_ON_EXIT, BST_UNCHECKED);
		}
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_VIDEO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bSkipFrames)
		{
			CheckDlgButton(hWnd, IDC_SKIPFRAMES, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_SKIPFRAMES, BST_UNCHECKED);
		}

		if (cfg->m_syncModeFullscreen == HCFG::FSSM_VBL)
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_FSC, IDC_LINESYNC_DOUBLERATE_FSC, IDC_VBLSYNC_FSC);
		}
		else if (cfg->m_syncModeFullscreen == HCFG::FSSM_LINE)
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_FSC, IDC_LINESYNC_DOUBLERATE_FSC, IDC_LINESYNC_FSC);
		}
		else if (cfg->m_syncModeFullscreen == HCFG::FSSM_FRAME_DOUBLER)
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_FSC, IDC_LINESYNC_DOUBLERATE_FSC, IDC_LINESYNC_DOUBLERATE_FSC);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_FSC, IDC_LINESYNC_DOUBLERATE_FSC, IDC_LINESYNC_DOUBLERATE_FSC);
		}

		if (cfg->m_syncModeWindowed == HCFG::FSSM_VBL)
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_WND, IDC_LINESYNC_WND, IDC_VBLSYNC_WND);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_VBLSYNC_WND, IDC_LINESYNC_WND, IDC_LINESYNC_WND);
		}
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_AUDIO)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bSID_Emulation_Enable)
		{
			CheckDlgButton(hWnd, IDC_SID_EMULATION, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_SID_EMULATION, BST_UNCHECKED);
		}

		if (cfg->m_bSIDResampleMode)
		{
			CheckRadioButton(hWnd, IDC_SID_RESAMPLE, IDC_SID_DOWNSAMPLE, IDC_SID_RESAMPLE);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_SID_RESAMPLE, IDC_SID_DOWNSAMPLE, IDC_SID_DOWNSAMPLE);
		}

		if (cfg->m_bSIDStereo)
		{
			CheckRadioButton(hWnd, IDC_SID_STEREO, IDC_SID_MONO, IDC_SID_STEREO);
		}
		else
		{
			CheckRadioButton(hWnd, IDC_SID_STEREO, IDC_SID_MONO, IDC_SID_MONO);
		}

		if (cfg->m_bAudioClockSync)
		{
			CheckDlgButton(hWnd, IDC_AUDIOCLOCKSYNC, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_AUDIOCLOCKSYNC, BST_UNCHECKED);
		}

		if (cfg->m_bSidDigiBoost)
		{
			CheckDlgButton(hWnd, IDC_SIDDIGIBOOT, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_SIDDIGIBOOT, BST_UNCHECKED);
		}
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bD1541_Emulation_Enable)
		{
			CheckDlgButton(hWnd, IDC_1541_EMULATION, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_1541_EMULATION, BST_UNCHECKED);
		}

		if (cfg->m_bShowFloppyLed)
		{
			CheckDlgButton(hWnd, IDC_CHK_SHOWFLOPPYLED, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_CHK_SHOWFLOPPYLED, BST_UNCHECKED);
		}
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (cfg->m_bTimerBbug)
		{
			CheckDlgButton(hWnd, IDC_CHK_TIMERBBUG, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_CHK_TIMERBBUG, BST_UNCHECKED);
		}

		if (cfg->m_CIAMode == HCFG::CM_CIA6526)
		{
			CheckDlgButton(hWnd, IDC_RAD_CIA6526, BST_CHECKED);
		}
		else if (cfg->m_CIAMode == HCFG::CM_CIA6526A)
		{
			CheckDlgButton(hWnd, IDC_RAD_CIA6526A, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_RAD_CIA6526A, BST_CHECKED);
		}

		if (cfg->m_reu_insertCartridge)
		{
			CheckDlgButton(hWnd, IDC_CHK_REU_INSERT, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_CHK_REU_INSERT, BST_UNCHECKED);
		}

		if (cfg->m_reu_extraAddressBits == 0)
		{
			CheckDlgButton(hWnd, IDC_RAD_REU_512K, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hWnd, IDC_RAD_REU_16M, BST_CHECKED);
		}
	}

	FillFps();
	FillBorder();
	FillDevices();
	FillModes();
	FillFormats();
	FillRefresh();
	FillSizes();
	FillFilters();
	FillDiskTrackZero();
	FillSidCount();
	FillSidAddress();
	SettingsOnLimitSpeedChange();
}

void CDiagEmulationSettingsTab::saveconfig(CConfig *cfg)
{
HWND hWnd;
shared_ptr<CTabPageDialog> pPage;

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_GENERAL)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_HOSTCPU_FRIENDLY))
		{
			cfg->m_bCPUFriendly=true;
		}
		else
		{
			cfg->m_bCPUFriendly=false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SHOWSPEED))
		{
			cfg->m_bShowSpeed = true;
		}
		else
		{
			cfg->m_bShowSpeed = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_LIMITSPEED))
		{
			cfg->m_bLimitSpeed = true;
		}
		else
		{
			cfg->m_bLimitSpeed = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_DISKONSEPARATETHREAD))
		{
			cfg->m_bD1541_Thread_Enable = true;
		}
		else
		{
			cfg->m_bD1541_Thread_Enable = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_ALLOWOPPOSINGJOYSTICK))
		{
			cfg->m_bAllowOpposingJoystick = true;
		}
		else
		{
			cfg->m_bAllowOpposingJoystick = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_IMGUI_WINDOWEDMODE_ENABLE))
		{
			cfg->m_bEnableImGuiWindowed = true;
		}
		else
		{
			cfg->m_bEnableImGuiWindowed = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SAVE_WINDOW_POSITION_ON_EXIT))
		{
			cfg->m_bSaveWindowPositionOnExit = true;
		}
		else
		{
			cfg->m_bSaveWindowPositionOnExit = false;
		}		
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

		if (IsDlgButtonChecked(hWnd, IDC_VBLSYNC_FSC))
		{
			cfg->m_syncModeFullscreen = HCFG::FSSM_VBL;
		}
		else if (IsDlgButtonChecked(hWnd, IDC_LINESYNC_FSC))
		{
			cfg->m_syncModeFullscreen = HCFG::FSSM_LINE;
		}
		else if (IsDlgButtonChecked(hWnd, IDC_LINESYNC_DOUBLERATE_FSC))
		{
			cfg->m_syncModeFullscreen = HCFG::FSSM_FRAME_DOUBLER;
		}		
		else
		{
			cfg->m_syncModeFullscreen = HCFG::FSSM_LINE;
		}		

		if (IsDlgButtonChecked(hWnd, IDC_VBLSYNC_WND))
		{
			cfg->m_syncModeWindowed = HCFG::FSSM_VBL;
		}
		else
		{
			cfg->m_syncModeWindowed = HCFG::FSSM_LINE;
		}

		unsigned int adapterOrdinal = 0;
		unsigned int outputOrdinal = 0;
		bool isDefaultAdapter = 0;
		if (SUCCEEDED(ReadAdapterOrdinal(&isDefaultAdapter, &adapterOrdinal, &outputOrdinal)))
		{
			cfg->m_fullscreenAdapterIsDefault = isDefaultAdapter;
			cfg->m_fullscreenAdapterNumber = adapterOrdinal;
			cfg->m_fullscreenOutputNumber = outputOrdinal;
		}

		CDisplayModeInfo* pMode;
		if (SUCCEEDED(ReadAdapterMode(&pMode)))
		{
			if (pMode->isAuto)
			{
				cfg->m_fullscreenWidth = 0;
				cfg->m_fullscreenHeight = 0;
				cfg->m_fullscreenDxGiModeScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
				cfg->m_fullscreenDxGiModeScaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
			}
			else
			{
				cfg->m_fullscreenWidth = pMode->width;
				cfg->m_fullscreenHeight = pMode->height;
				cfg->m_fullscreenDxGiModeScanlineOrdering = pMode->scanlineOrdering;
				cfg->m_fullscreenDxGiModeScaling = pMode->scaling;
			}
		}

		CDisplayFormatInfo *pFormat;
		if (SUCCEEDED(ReadAdapterFormat(&pFormat)))
		{
			cfg->m_fullscreenFormat = pFormat->format;
		}

		UINT adapterRefreshNumerator;
		UINT adapterRefreshDenominator;
		if (SUCCEEDED(ReadAdapterRefresh(&adapterRefreshNumerator, &adapterRefreshDenominator)))
		{
			cfg->m_fullscreenRefreshNumerator = adapterRefreshNumerator;
			cfg->m_fullscreenRefreshDenominator = adapterRefreshDenominator;
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
		{
			cfg->m_bAudioClockSync=true;
		}
		else
		{
			cfg->m_bAudioClockSync=false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SIDDIGIBOOT))
		{
			cfg->m_bSidDigiBoost=true;
		}
		else
		{
			cfg->m_bSidDigiBoost=false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SID_EMULATION))
		{
			cfg->m_bSID_Emulation_Enable = true;
		}
		else
		{
			cfg->m_bSID_Emulation_Enable = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SID_RESAMPLE))
		{
			cfg->m_bSIDResampleMode = true;
		}
		else
		{
			cfg->m_bSIDResampleMode = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_SID_STEREO))
		{
			cfg->m_bSIDStereo = true;
		}
		else
		{
			cfg->m_bSIDStereo = false;
		}		
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_DISK)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_1541_EMULATION))
		{
			cfg->m_bD1541_Emulation_Enable=true;
		}
		else
		{
			cfg->m_bD1541_Emulation_Enable=false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_CHK_SHOWFLOPPYLED))
		{
			cfg->m_bShowFloppyLed=true;
		}
		else
		{
			cfg->m_bShowFloppyLed=false;
		}

		ReadTrackZeroSensor(&cfg->m_TrackZeroSensorStyle);
	}

	if ((pPage = GetPage(CDiagEmulationSettingsTab::TABPAGE_CHIP)) != NULL)
	{
		hWnd = pPage->GetHwnd();
		if (IsDlgButtonChecked(hWnd, IDC_RAD_CIA6526))
		{
			cfg->m_CIAMode = HCFG::CM_CIA6526; 
		}
		else if (IsDlgButtonChecked(hWnd, IDC_RAD_CIA6526A))
		{
			cfg->m_CIAMode = HCFG::CM_CIA6526A; 
		}
		else
		{
			cfg->m_CIAMode = HCFG::CM_CIA6526A; 
		}

		if (IsDlgButtonChecked(hWnd, IDC_CHK_TIMERBBUG))
		{
			cfg->m_bTimerBbug = true;
		}
		else
		{
			cfg->m_bTimerBbug = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_CHK_REU_INSERT))
		{
			cfg->m_reu_insertCartridge = true;
		}
		else
		{
			cfg->m_reu_insertCartridge = false;
		}

		if (IsDlgButtonChecked(hWnd, IDC_RAD_REU_512K))
		{
			cfg->m_reu_extraAddressBits = 0;
		}
		else if (IsDlgButtonChecked(hWnd, IDC_RAD_REU_16M))
		{
			cfg->m_reu_extraAddressBits = CartReu1750::MaxExtraBits;
		}
		else
		{
			cfg->m_reu_extraAddressBits = 0;
		}

		int v;
		cfg->m_numberOfExtraSIDs = 0;
		if (this->ReadExtraSidCount(&v))
		{
			cfg->m_numberOfExtraSIDs = v;
		}

		bit16 sidAddress;
		cfg->m_Sid2Address = 0;
		cfg->m_Sid3Address = 0;
		cfg->m_Sid4Address = 0;
		cfg->m_Sid5Address = 0;
		cfg->m_Sid6Address = 0;
		cfg->m_Sid7Address = 0;
		cfg->m_Sid8Address = 0;
		if (this->ReadSidAddress2(&sidAddress))
		{
			cfg->m_Sid2Address = sidAddress;
		}

		if (this->ReadSidAddress3(&sidAddress))
		{
			cfg->m_Sid3Address = sidAddress;
		}

		if (this->ReadSidAddress4(&sidAddress))
		{
			cfg->m_Sid4Address = sidAddress;
		}

		if (this->ReadSidAddress5(&sidAddress))
		{
			cfg->m_Sid5Address = sidAddress;
		}

		if (this->ReadSidAddress6(&sidAddress))
		{
			cfg->m_Sid6Address = sidAddress;
		}

		if (this->ReadSidAddress7(&sidAddress))
		{
			cfg->m_Sid7Address = sidAddress;
		}

		if (this->ReadSidAddress8(&sidAddress))
		{
			cfg->m_Sid8Address = sidAddress;
		}
	}

	cfg->m_bMaxSpeed = false;
}

void CDiagEmulationSettingsTab::InitFonts()
{
	defaultFont = CreateFont(
	8,
	0,
	0,
	0,
	FW_NORMAL,
	FALSE,
	FALSE,
	FALSE,
	ANSI_CHARSET,
	OUT_TT_ONLY_PRECIS,
	CLIP_DEFAULT_PRECIS,
	CLEARTYPE_QUALITY,
	FIXED_PITCH | FF_DONTCARE,
	TEXT("MS Shell Dlg"));
}

void CDiagEmulationSettingsTab::CloseFonts()
{
	if (defaultFont)
	{
		DeleteObject(defaultFont);
		defaultFont = NULL;
	}
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
		{
			return FALSE;
		}

		hr = CreateAllPages();
		if (FAILED(hr))
		{
			return FALSE;
		}
		InitFonts();
		loadconfig(&CurrentCfg);
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
	case WM_DESTROY:
		CloseFonts();
		return 0;
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
		{
			m_hwndDisplay = NULL;
		}

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
				FillRefresh();
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
				FillRefresh();
				FillSizes();
				FillFilters();
				VideoPageSizeComboBoxes();
				return TRUE;
			}
			break;
		case IDC_CBO_FORMAT:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				FillRefresh();
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
		case IDC_RAD_CIA6526:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SettingsOnCiaChipChange();
				return TRUE;
			}
			break;
		case IDC_RAD_CIA6526A:
			switch (HIWORD(wParam))
			{
			case BN_CLICKED:
				SettingsOnCiaChipChange();
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
	default:
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


CDiagEmulationSettingsTab::CDisplayModeInfo_WxH_is_equal::CDisplayModeInfo_WxH_is_equal(const CDisplayModeInfo& mode)
	: mode(mode)
{
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_WxH_is_equal::operator()(const CDisplayModeInfo& mode) const
{
	return CDiagEmulationSettingsTab::fnCompareMode(this->mode, mode) == 0;
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_WxH_is_less::operator()(const CDisplayModeInfo& a, const CDisplayModeInfo& b) const
{
	return CDiagEmulationSettingsTab::fnCompareMode(a, b) < 0;
}

CDiagEmulationSettingsTab::CDisplayModeInfo_format_is_equal::CDisplayModeInfo_format_is_equal(const CDisplayFormatInfo& format)
	: format(format)
{
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_format_is_equal::operator()(const CDisplayFormatInfo& format) const
{
	return this->format.format == format.format && this->format.isAuto == format.isAuto;
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_format_is_less::operator()(const CDisplayFormatInfo& a, const CDisplayFormatInfo& b) const
{
	return CDiagEmulationSettingsTab::fnCompareFormat(a, b) < 0;
}

CDiagEmulationSettingsTab::CDisplayModeInfo_refresh_is_equal::CDisplayModeInfo_refresh_is_equal(const CDisplayRefreshInfo& refresh)
	: refresh(refresh)
{
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_refresh_is_equal::operator()(const CDisplayRefreshInfo& refresh) const
{
	return this->refresh.refreshRateNumerator == refresh.refreshRateNumerator && this->refresh.refreshRateDenominator == refresh.refreshRateDenominator;
}

bool CDiagEmulationSettingsTab::CDisplayModeInfo_refresh_is_less::operator()(const CDisplayRefreshInfo& a, const CDisplayRefreshInfo& b) const
{
	double rateL = 0.0;
	double rateR = 0.0;
	if (a.refreshRateDenominator != 0)
	{
		rateL = a.refreshRateNumerator / a.refreshRateDenominator;
	}

	if (b.refreshRateDenominator != 0)
	{
		rateR = b.refreshRateNumerator / b.refreshRateDenominator;
	}

	return rateL < rateR;
}

CDisplayInfo2::CDisplayInfo2()
{
	isAuto = false;
	adapterOrdinal = 0;
	outputOrdinal = 0;
	ZeroMemory(&adapterDesc, sizeof(adapterDesc));
	ZeroMemory(&outputDesc, sizeof(outputDesc));
	ZeroMemory(&monitor, sizeof(monitor));
}

HRESULT CDisplayInfo2::MakeName()
{
	try
	{
		nameOfAdapter.clear();
		if (isAuto)
		{
			nameOfAdapter.append(TEXT("Auto"));
			return S_OK;
		}

		nameOfAdapter.append(adapterDesc.Description);
		nameOfAdapter.append(L" ");
		nameOfAdapter.append(monitor.szDevice);
	}
	catch (...)
	{
		return E_FAIL;
	}

	return S_OK;
}

CDisplayModeInfo::CDisplayModeInfo()
{
	isAuto = false;
	width = 0;
	height = 0;
	scanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
}

HRESULT CDisplayModeInfo::MakeName()
{
	HRESULT hr = S_OK;
	wchar_t *p = nullptr;
	try
	{
		name.clear();
		if (isAuto)
		{
			name.append(TEXT("Auto"));
			return S_OK;
		}

		int lenName = GraphicsHelper::GetDisplayResolutionText(width, height, NULL, 0);
		p = new wchar_t[lenName];
		if (p != NULL)
		{
			GraphicsHelper::GetDisplayResolutionText(width, height, p, lenName);
			name.append(p);
			if (scanlineOrdering == DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE)
			{
				name.append(TEXT(" p"));
			}
			else if (scanlineOrdering == DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST)
			{
				name.append(TEXT(" i (L)"));
			}
			else if (scanlineOrdering == DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST)
			{
				name.append(TEXT(" i (U)"));
			}

			if (scaling == DXGI_MODE_SCALING::DXGI_MODE_SCALING_CENTERED)
			{
				name.append(TEXT(" centered"));
			}
			else if (scaling == DXGI_MODE_SCALING::DXGI_MODE_SCALING_STRETCHED)
			{
				name.append(TEXT(" stretched"));
			}

			hr = S_OK;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	if (p != nullptr)
	{
		delete[] p;
		p = nullptr;
	}

	return hr;
}

CDisplayFormatInfo::CDisplayFormatInfo()
{
	isAuto = false;
	format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
}

HRESULT CDisplayFormatInfo::MakeName()
{
	HRESULT hr = S_OK;
	wchar_t* p = nullptr;
	try
	{
		name.clear();
		if (isAuto)
		{
			name.append(L"Auto");
			return S_OK;
		}

		int lenName = GraphicsHelper::GetDisplayFormatText(format, NULL, 0);
		p = new wchar_t[lenName];
		if (p != NULL)
		{
			GraphicsHelper::GetDisplayFormatText(format, p, lenName);
			name.append(p);
			hr = S_OK;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	if (p != nullptr)
	{
		delete[] p;
		p = nullptr;
	}

	return hr;
}

CDisplayRefreshInfo::CDisplayRefreshInfo()
{
	isAuto = false;
	refreshRateNumerator = 0;
	refreshRateDenominator = 0;
}

HRESULT CDisplayRefreshInfo::MakeName()
{
	HRESULT hr = S_OK;
	wchar_t* p = nullptr;
	try
	{
		name.clear();
		if (isAuto)
		{
			name.append(TEXT("Auto"));
			return S_OK;
		}

		int lenName = GraphicsHelper::GetDisplayRefreshText(refreshRateNumerator, refreshRateDenominator, NULL, 0);
		p = new wchar_t[lenName];
		if (p != NULL)
		{
			GraphicsHelper::GetDisplayRefreshText(refreshRateNumerator, refreshRateDenominator, p, lenName);
			name.append(p);
			hr = S_OK;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	catch (...)
	{
		hr = E_FAIL;
	}

	if (p != nullptr)
	{
		delete[] p;
		p = nullptr;
	}

	return hr;
}
