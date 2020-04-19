#pragma once
#include "cvirwindow.h"
#include "errormsg.h"

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
	virtual void WindowRelease() override;
	int GetRasterLine();
	int GetRasterCycle();

	shared_ptr<CDiagBreakpointVicRaster> keepAlive;
private:
	int m_iLine;
	int m_iCycle;
	bool m_bInOnVicCursorChange;
	bool m_bInOnTextChange;
	IAppCommand *m_pIAppCommand;
	IC64 *c64;

	void InitControls(HWND hWndDlg);
	bool SaveUI();
	bool TryGetCycle(int& v);
	bool TryGetLine(int& v);
	void SetVicCursor();
	void UpdateHexText(int textDecId, int textHexId);
	void UpdateDecText(int textHexId, int textDecId);
	virtual void OnVicCursorChange(void *sender, VicCursorMoveEventArgs& e);
	virtual BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

typedef shared_ptr<CDiagBreakpointVicRaster> Sp_CDiagBreakpointVicRaster;
typedef weak_ptr<CDiagBreakpointVicRaster> Wp_CDiagBreakpointVicRaster;
