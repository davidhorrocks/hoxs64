#ifndef __DIAGBREAKPOINTVICRASTER_H__
#define __DIAGBREAKPOINTVICRASTER_H__

class CDiagBreakpointVicRaster_EventSink_OnVicCursorChange : protected EventSink<VicCursorMoveEventArgs>
{
private:
	virtual int Sink(void *sender, VicCursorMoveEventArgs& e)
	{
		OnVicCursorChange(sender, e);
		return 0;
	}
	virtual void OnVicCursorChange(void *sender, VicCursorMoveEventArgs& e)=0;
};

class CDiagBreakpointVicRaster_EventSink : 
	protected CDiagBreakpointVicRaster_EventSink_OnVicCursorChange
{
};

class CDiagBreakpointVicRaster : public CVirDialog, CDiagBreakpointVicRaster_EventSink , public ErrorMsg
{
public:
	CDiagBreakpointVicRaster(IAppCommand *pIAppCommand, IC64 *c64);
	virtual ~CDiagBreakpointVicRaster();

	int GetRasterLine();
	int GetRasterCycle();
private:
	int m_iLine;
	int m_iCycle;
	bool m_bInOnVicCursorChange;
	IAppCommand *m_pIAppCommand;
	IC64 *c64;

	void InitControls(HWND hWndDlg);
	bool SaveUI();
	bool TryGetCycle(int& v);
	bool TryGetLine(int& v);
	void SetVicCursor();
	virtual void OnVicCursorChange(void *sender, VicCursorMoveEventArgs& e);
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CDiagBreakpointVicRaster> Sp_CDiagBreakpointVicRaster;
typedef weak_ptr<CDiagBreakpointVicRaster> Wp_CDiagBreakpointVicRaster;

#endif