#pragma once
#include "bits.h"
#include "cvirwindow.h"
#include "errormsg.h"

class CDiagNewBlankDisk : public CVirDialog, public ErrorMsg
{
public:
	CDiagNewBlankDisk() = default;
	virtual ~CDiagNewBlankDisk() = default;
	CDiagNewBlankDisk(const CDiagNewBlankDisk&) = default;
	CDiagNewBlankDisk(CDiagNewBlankDisk&&) = default;
	CDiagNewBlankDisk& operator=(const CDiagNewBlankDisk&) = default;
	CDiagNewBlankDisk& operator=(CDiagNewBlankDisk&&) = default;
	HRESULT Init();

	static const int DISKNAMELENGTH=16;
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	bit8 id1 = 0;
	bit8 id2 = 0;
	TCHAR diskname[DISKNAMELENGTH + 1] = {};
	bool bAlignTracks = false;
	int numberOfTracks = 35;
protected:
private:
};
