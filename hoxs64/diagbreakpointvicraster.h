#ifndef __DIAGBREAKPOINTVICRASTER_H__
#define __DIAGBREAKPOINTVICRASTER_H__


class CDiagBreakpointVicRaster : public CVirDialog , public ErrorMsg
{
public:
	CDiagBreakpointVicRaster(IMonitorCommand *pIMonitorCommand, C64 *c64);
	virtual ~CDiagBreakpointVicRaster();

	int GetRasterLine();
	int GetRasterCycle();
private:
	int m_iLine;
	int m_iCycle;

	IMonitorCommand *m_pIMonitorCommand;
	C64 *c64;

	void InitControls(HWND hWndDlg);
	bool SaveUI();
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CDiagBreakpointVicRaster> Sp_CDiagBreakpointVicRaster;
typedef weak_ptr<CDiagBreakpointVicRaster> Wp_CDiagBreakpointVicRaster;

#endif