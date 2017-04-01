#ifndef __ERRORMSG_H__
#define __ERRORMSG_H__

class ErrorMsg
{
public:
	ErrorMsg();
	~ErrorMsg();
	TCHAR errorText[300];
	HRESULT errorValue;

	bool OK();
	void ClearError();
	HRESULT SetError(HRESULT hRet, LPCTSTR szError, ...);
	HRESULT SetError(ErrorMsg& err);
	HRESULT SetErrorFromGetLastError();
	HRESULT SetErrorFromGetLastError(DWORD err, LPCTSTR szError);
	void DisplayError(HWND hWnd, const TCHAR title[]);
	const TCHAR *GetLastWindowsErrorString();
	static void ShowMessage(HWND hWnd, UINT uType, LPCTSTR szTitle, LPCTSTR szError, ...);
	static const LPCTSTR ERR_OUTOFMEMORY;

protected:
	const TCHAR *localLastWindowsErrorString;
};

#endif
