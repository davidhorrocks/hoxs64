#ifndef __DIAGABOUT_H__
#define __DIAGABOUT_H__

class CDiagAbout : public CVirDialog, public ErrorMsg
{
public:
	CDiagAbout();
	~CDiagAbout();

	HRESULT Init(VS_FIXEDFILEINFO *pVinfo);
private:
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	VS_FIXEDFILEINFO *m_pVinfo;
};
#endif
