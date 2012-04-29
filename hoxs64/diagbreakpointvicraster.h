#ifndef __DIAGBREAKPOINTVICRASTER_H__
#define __DIAGBREAKPOINTVICRASTER_H__

class CDiagBreakpointVicRaster : public CVirDialog , public ErrorMsg
{
public:
	CDiagBreakpointVicRaster();
	virtual ~CDiagBreakpointVicRaster();

	int GetRasterLine();
	int GetRasterCycle();
private:
	int m_iLine;
	int m_iCycle;
	void InitControls(HWND hWndDlg);
	bool SaveUI();
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CDiagBreakpointVicRaster> Sp_CDiagBreakpointVicRaster;

#endif