#pragma once
#include "cvirwindow.h"
#include "errormsg.h"
#include "graphics.h"

struct CDisplayInfo2
{
	CDisplayInfo2();
	bool isAuto;
	DXGI_ADAPTER_DESC1 adapterDesc;
	DXGI_OUTPUT_DESC outputDesc;
	UINT adapterOrdinal;
	UINT outputOrdinal;
	MONITORINFOEXW monitor;
	std::wstring nameOfAdapter;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<IDXGIOutput> output;
	HRESULT MakeName();
};

struct CDisplayModeInfo
{
	CDisplayModeInfo();
	bool isAuto;
	unsigned int width;
	unsigned int height;
	DXGI_MODE_SCANLINE_ORDER scanlineOrdering;
	DXGI_MODE_SCALING scaling;
	std::wstring name;
	HRESULT MakeName();
};

struct CDisplayFormatInfo
{
	CDisplayFormatInfo();
	bool isAuto;
	DXGI_FORMAT format;
	std::wstring name;
	HRESULT MakeName();
};
struct CDisplayRefreshInfo
{
	CDisplayRefreshInfo();
	bool isAuto;
	unsigned int refreshRateNumerator;
	unsigned int refreshRateDenominator;
	std::wstring name;
	HRESULT MakeName();
};
class CDiagEmulationSettingsTab : public CTabDialog, public ErrorMsg
{
public:
	CDiagEmulationSettingsTab();
	virtual ~CDiagEmulationSettingsTab();
	void loadconfig(const CConfig *);
	void saveconfig(CConfig *);

	HRESULT Init(Graphics* pGx, const CConfig *cfg);
	CConfig CurrentCfg;
	CConfig NewCfg;
	HFONT defaultFont;
protected:
	void SettingsOnLimitSpeedChange();
	void SettingsOnCiaChipChange();
	CDPI m_dpi;

private:
	void UpdatePage(int pageno, HWND hWndDlg);
	void UpdatePageGeneral(HWND hWndDlg);
	void UpdatePageVideo(HWND hWndDlg);
	void UpdatePageAudio(HWND hWndDlg);
	void UpdatePageDisk(HWND hWndDlg);
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnPageEvent(CTabPageDialog *page, HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);

	Graphics* m_pGx;
	std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> m_vdxgiAdapters;
	vector<CDisplayInfo2> m_AdapterArray;
	vector<DXGI_MODE_DESC> m_ModeCompleteList;
	list<CDisplayModeInfo> m_ModeList;
	list<CDisplayFormatInfo> m_ModeFormatList;
	list<CDisplayRefreshInfo> m_ModeRefreshList;
	
	void FillDevices();
	void FillModes();
	void FillModes2(int displayInfoIndex);
	void FillFormats();
	void FillFormats2(const CDisplayModeInfo * pModeSelected);
	void FillRefresh();
	void FillRefresh2(const CDisplayModeInfo* pModeSelected, const CDisplayFormatInfo* pFormatSelected);
	void FillSizes();
	void FillFilters();
	void FillSidCount();
	void FillSidAddress();
	void FillBorder();
	void FillDiskTrackZero();
	void FillFps();
	void VideoPageSizeComboBoxes();
	void DiskPageSizeComboBoxes();
	void InitFonts();
	void CloseFonts();

public:
	static int fnFindModeFormat(const CDisplayFormatInfo& mode1, const CDisplayFormatInfo& mode2);
	static int fnCompareMode(const CDisplayModeInfo& mode1, const CDisplayModeInfo& mode2);
	static bool fnForListCompareMode(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
	static int fnCompareFormat(const CDisplayFormatInfo& mode1, const CDisplayFormatInfo& mode2);
	static bool fnForListCompareFormat(const CDisplayFormatInfo& mode1, const CDisplayFormatInfo& mode2);
private:
	HRESULT ReadAdapterOrdinal(bool *pbIsDefault, unsigned int* pAdapterOrdinal, unsigned int* pOutputOrdinal);
	HRESULT ReadAdapterMode(CDisplayModeInfo **pMode);
	HRESULT ReadAdapterFormat(CDisplayFormatInfo** ppFormat);
	HRESULT ReadAdapterRefresh(UINT* pRefreshRateNumerator, UINT* pRefreshRateDenominator);
	HRESULT ReadAdapterStretch(HCFG::EMUWINDOWSTRETCH *pFormat);
	HRESULT ReadAdapterFilter(HCFG::EMUWINDOWFILTER *pFilter);
	void ReadTrackZeroSensor(HCFG::ETRACKZEROSENSORSTYLE *v);
	void ReadBorder(HCFG::EMUBORDERSIZE *pBorder);
	void ReadFps(HCFG::EMUFPS *pFps);
	bool ReadExtraSidCount(int *extraSidCount);
	bool ReadSidAddress2(bit16 *sidAddress);
	bool ReadSidAddress3(bit16 *sidAddress);
	bool ReadSidAddress4(bit16 *sidAddress);
	bool ReadSidAddress5(bit16 *sidAddress);
	bool ReadSidAddress6(bit16 *sidAddress);
	bool ReadSidAddress7(bit16 *sidAddress);
	bool ReadSidAddress8(bit16 *sidAddress);
	bool ReadComboItemDataInt(int page, int control_id, int *data);

	static const int TABPAGE_GENERAL = 0;
	static const int TABPAGE_VIDEO = 1;
	static const int TABPAGE_AUDIO = 2;
	static const int TABPAGE_DISK = 3;
	static const int TABPAGE_CHIP = 4;

	static const TCHAR szAuto[];
	static const TCHAR szVideoFilter_1X[];
	static const TCHAR szVideoFilter_2X[];
	static const TCHAR szVideoFilter_StretchToFit[];
	static const TCHAR szVideoFilter_StretchWithBorderClip[];

	class CDisplayModeInfo_WxH_is_equal
	{
	public:
		CDisplayModeInfo_WxH_is_equal(const CDisplayModeInfo& mode);
		bool operator()(const CDisplayModeInfo& mode) const;
	private:
		const CDisplayModeInfo &mode;
	};

	class CDisplayModeInfo_WxH_is_less
	{
	public:
		bool operator()(const CDisplayModeInfo& a, const CDisplayModeInfo& b) const;
	private:
	};

	class CDisplayModeInfo_format_is_equal
	{
	public:
		CDisplayModeInfo_format_is_equal(const CDisplayFormatInfo& format);
		bool operator()(const CDisplayFormatInfo& format) const;
	private:
		const CDisplayFormatInfo& format;
	};

	class CDisplayModeInfo_format_is_less
	{
	public:
		bool operator()(const CDisplayFormatInfo& a, const CDisplayFormatInfo& b) const;
	private:
	};

	class CDisplayModeInfo_refresh_is_equal
	{
	public:
		CDisplayModeInfo_refresh_is_equal(const CDisplayRefreshInfo& refresh);
		bool operator()(const CDisplayRefreshInfo& refresh) const;
	private:
		const CDisplayRefreshInfo& refresh;
	};

	class CDisplayModeInfo_refresh_is_less
	{
	public:
		bool operator()(const CDisplayRefreshInfo& a, const CDisplayRefreshInfo& b) const;
	private:
	};
};
