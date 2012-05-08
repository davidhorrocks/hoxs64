#ifndef __DIAGBREAKPOINTVICRASTER_H__
#define __DIAGBREAKPOINTVICRASTER_H__


class CDiagBreakpointVicRaster : public CVirDialog , public ErrorMsg
{
public:

	class INotify
	{
	public:
		virtual void OnDestory(CDiagBreakpointVicRaster *p) = 0;
		virtual void OnAccept(CDiagBreakpointVicRaster *p) = 0;
		virtual void OnCancel(CDiagBreakpointVicRaster *p) = 0;
	};
	CDiagBreakpointVicRaster(INotify *p);
	virtual ~CDiagBreakpointVicRaster();

	int GetRasterLine();
	int GetRasterCycle();
private:
	INotify *m_pINotify;
	int m_iLine;
	int m_iCycle;
	void InitControls(HWND hWndDlg);
	bool SaveUI();
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CDiagBreakpointVicRaster> Sp_CDiagBreakpointVicRaster;

#endif