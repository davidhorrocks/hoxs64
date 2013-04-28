#ifndef __ERRORMSG_H__
#define __ERRORMSG_H__

class ErrorMsg
{
public:
	ErrorMsg();
	bool OK();
	void ClearError();
	HRESULT SetError(HRESULT hRet, LPCTSTR szError, ...);
	HRESULT SetError(ErrorMsg& err);
	HRESULT SetErrorFromGetLastError(HRESULT hRet);
	HRESULT SetErrorFromGetLastError(HRESULT hRet, LPCTSTR szError);
	TCHAR errorText[300];
	HRESULT errorValue;
	void DisplayError(HWND hWnd, const TCHAR title[]);
	static void ShowMessage(HWND hWnd, UINT uType, LPCTSTR szTitle, LPCTSTR szError, ...);

	static const LPCTSTR ERR_OUTOFMEMORY;
};

#endif
