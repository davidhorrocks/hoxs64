#pragma once
#include "cvirwindow.h"
#include "errormsg.h"

class CDiagNewBlankDisk : public CVirDialog, public ErrorMsg
{
public:
	CDiagNewBlankDisk();
	virtual ~CDiagNewBlankDisk();
	HRESULT Init();

	static const int DISKNAMELENGTH=16;
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	
	bit8 id1,id2;
	TCHAR diskname[DISKNAMELENGTH+1];
	bool bAlignTracks;
	int numberOfTracks;

protected:
private:
};
