#include <windows.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "errormsg.h"
#include "C64.h"
#include "edln.h"
#include "assembler.h"
#include "toolitemaddress.h"
#include "dchelper.h"
#include "resource.h"

#define WNDCLASS_DFREBARCHILD TEXT("Hoxs64DFRebarChild")

//CToolItemAddress::CToolItemAddress()
//{
//}
//
//HRESULT Init(CVirWindow *parent, HFONT hFont)
//{
//}
//
//HRESULT CToolItemAddress::RegisterClass(HINSTANCE hInstance)
//{
//WNDCLASSEX  wc;
//
//	ZeroMemory(&wc, sizeof(wc));
//	wc.cbSize        = sizeof(WNDCLASSEX);
//	wc.style         = CS_HREDRAW | CS_VREDRAW;
//	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
//	wc.cbClsExtra    = 0;
//	wc.cbWndExtra    = sizeof(CToolItemAddress *);
//	wc.hInstance     = hInstance;
//	wc.hIcon         = 0;
//	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
//	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
//	wc.lpszMenuName  = 0L;
//    wc.lpszClassName = WNDCLASS_DFREBARCHILD;
//	wc.hIconSm       = NULL;
//	if (RegisterClassEx(&wc)==0)
//		return E_FAIL;
//	return S_OK;	
//}
//
//
//HWND CToolItemAddress::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
//{
//}
