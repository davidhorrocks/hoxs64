#ifndef __DIAGEMULATION_SETTINGS_TAB_H__
#define __DIAGEMULATION_SETTINGS_TAB_H__

struct CDisplayInfo
{
	CDisplayInfo();
	~CDisplayInfo();
	D3DADAPTER_IDENTIFIER9 adapter;
	MONITORINFOEXW monitor;
	UINT adapterOrdinal;
	TCHAR *name;
	TCHAR *nameFormat;
	bool bRequireClean;
	HRESULT MakeName();
};

struct CDisplayModeInfo
{
	CDisplayModeInfo();
	~CDisplayModeInfo();
	D3DDISPLAYMODE mode;
	TCHAR *name;
	int lenName;
	bool bRequireClean;
	HRESULT MakeName(CDX9 *dx);
	HRESULT MakeNameFormat(CDX9 *dx);
	HRESULT MakeNameRefresh(CDX9 *dx);
};

class CDiagEmulationSettingsTab : public CTabDialog, public ErrorMsg
{
public:
	CDiagEmulationSettingsTab();
	virtual ~CDiagEmulationSettingsTab();
	void loadconfig(const CConfig *);
	void saveconfig(CConfig *);

	HRESULT Init(CDX9 *dx, const CConfig *cfg);
	CConfig CurrentCfg;
	CConfig NewCfg;
	HFONT defaultFont;
protected:
	void SettingsOnLimitSpeedChange();
	void SettingsOnPixelDoublerChange();
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

	CDX9 *m_pDx;
	vector<CDisplayInfo> m_AdapterArray;
	list<CDisplayModeInfo> m_ModeList;
	list<CDisplayModeInfo> m_ModeFormatList;
	list<CDisplayModeInfo> m_ModeRefreshList;

	void FillDevices();
	void FillModes();
	void FillModes(int adapterOrdinal);
	void FillFormats();
	void FillFormats(int adapterOrdinal, UINT width, UINT height);
	void FillRefresh();
	void FillRefresh(int adapterOrdinal, UINT width, UINT height, D3DFORMAT format);
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
	static int fnFindModeFormat(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
	static int fnCompareMode(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
	static bool fnForListCompareMode(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
	static int fnCompareFormat(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
	static bool fnForListCompareFormat(const CDisplayModeInfo &mode1, const CDisplayModeInfo &mode2);
private:
	HRESULT ReadAdapterOrdinal(DWORD *adapterOrdinal, GUID *adapterId);
	HRESULT ReadAdapterMode(D3DDISPLAYMODE *pMode);
	HRESULT ReadAdapterFormat(D3DFORMAT *pFormat);
	HRESULT ReadAdapterRefresh(UINT *pRefreshRate);
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
		CDisplayModeInfo_format_is_equal(D3DFORMAT format);
		bool operator()(const CDisplayModeInfo& mode) const;
	private:
		const D3DFORMAT format;
	};

	class CDisplayModeInfo_format_is_less
	{
	public:
		bool operator()(const CDisplayModeInfo& a, const CDisplayModeInfo& b) const;
	private:
	};

	class CDisplayModeInfo_refresh_is_equal
	{
	public:
		CDisplayModeInfo_refresh_is_equal(UINT refresh);
		bool operator()(const CDisplayModeInfo& mode) const;
	private:
		const UINT refresh;
	};

	class CDisplayModeInfo_refresh_is_less
	{
	public:
		bool operator()(const CDisplayModeInfo& a, const CDisplayModeInfo& b) const;
	private:
	};
};


#endif