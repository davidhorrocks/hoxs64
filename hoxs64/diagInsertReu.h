#pragma once
#include <string.h>
#include "bits.h"
#include "cvirwindow.h"
#include "errormsg.h"

class CDiagInsertReu : public CVirDialog, public ErrorMsg
{
public:

	CDiagInsertReu() = default;
	virtual ~CDiagInsertReu() = default;
	CDiagInsertReu(const CDiagInsertReu&) = default;
	CDiagInsertReu(CDiagInsertReu&&) = default;
	CDiagInsertReu& operator=(const CDiagInsertReu&) = default;
	CDiagInsertReu& operator=(CDiagInsertReu&&) = default;
	void Init(bool reu_use_image_file, std::wstring reu_image_filename, unsigned int reu_extraAddressBits);

	static const int DISKNAMELENGTH = 16;
	BOOL DialogProc(HWND hWndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	bool m_reu_use_image_file = false;
	std::wstring m_reu_image_filename;
	unsigned int m_reu_extraAddressBits = 0;
	bool m_ok = false;
protected:
private:
	void BrowseReuFile(HWND hWnd);
	void UpdateReuSize(HWND hWnd, unsigned int extraAddressBits);
	void UpdateReuUseCustomFile(HWND hWnd, bool use_image_file);
	void ReadFileName(HWND hWnd);
	void ReadReuSize(HWND hWnd);
	void ReadReuUseCustomFile(HWND hWnd);
};
