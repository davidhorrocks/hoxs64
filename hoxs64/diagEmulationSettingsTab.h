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
};

typedef class CArrayElement<CDisplayInfo> CAdapterElement;
typedef class CArray<CDisplayInfo> CAdapterArray;


typedef class MListElement<CDisplayModeInfo> CDisplayModeElement;
typedef class MList<CDisplayModeInfo> CDisplayModeList;

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
protected:
	void SettingsOnLimitSpeedChange();
	void SettingsOnPixelDoublerChange();


private:
	void UpdatePage(int pageno, HWND hWndDlg);
	void UpdatePageGeneral(HWND hWndDlg);
	void UpdatePageVideo(HWND hWndDlg);
	void UpdatePageAudio(HWND hWndDlg);
	void UpdatePageDisk(HWND hWndDlg);

	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnPageEvent(CTabPageDialog *page, HWND hWndDlg,UINT uMsg,WPARAM wParam,LPARAM lParam);
	CDX9 *m_pDx;

	CAdapterArray m_AdapterArray;
	CDisplayModeList m_ModeList;
	CDisplayModeList m_ModeFormatList;
	void FillDevices();
	void FillModes();
	void FillModes(UINT adapterOrdinal);
	void FillFormats();
	void FillFormats(UINT adapterOrdinal, D3DDISPLAYMODE &mode);
	void FillSizes();
	void FillFilters();
	void FillBorder();
	void FillDiskTrackZero();
	void FillFps();
	void VideoPageSizeComboBoxes();
	void DiskPageSizeComboBoxes();

	static int fnFindMode(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2);
	static int fnFindModeFormat(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2);
	static int fnCompareMode(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2);
	static int fnCompareFormat(CDisplayModeInfo &mode1, CDisplayModeInfo &mode2);

	HRESULT ReadAdapterOrdinal(DWORD *adapterOrdinal, GUID *adapterId);
	HRESULT ReadAdapterMode(D3DDISPLAYMODE *pMode);
	HRESULT ReadAdapterFormat(D3DFORMAT *pFormat);
	HRESULT ReadAdapterStretch(HCFG::EMUWINDOWSTRETCH *pFormat);
	HRESULT ReadAdapterFilter(HCFG::EMUWINDOWFILTER *pFilter);
	void ReadTrackZeroSensor(HCFG::ETRACKZEROSENSORSTYLE *v);
	void ReadBorder(HCFG::EMUBORDERSIZE *pBorder);
	void ReadFps(HCFG::EMUFPS *pFps);

	static const int TABPAGE_GENERAL = 0;
	static const int TABPAGE_VIDEO = 1;
	static const int TABPAGE_AUDIO = 2;
	static const int TABPAGE_DISK = 3;
	static const int TABPAGE_CHIP = 4;

	static const TCHAR szAuto[];
	static const TCHAR szVideoFilter_1X[];
	static const TCHAR szVideoFilter_2X[];
	static const TCHAR szVideoFilter_StretchToFit[];
};


#endif