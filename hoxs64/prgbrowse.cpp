#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <winuser.h>

#include "dx_version.h"
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dsound.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include "assert.h"

#include "defines.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "register.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "ram64.h"
#include "vic6569.h"
#include "t64.h"
#include "d64.h"
#include "sidfile.h"
#include "c64file.h"
#include "resource.h"
#include "prgbrowse.h"

CDirectoryArray arr;

UINT_PTR CALLBACK PRGBrowseDialogHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define WM_FILEINSPECTOR WM_USER
CPRGBrowse::CPRGBrowse()
{
	mhEvtQuit = 0;
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = S_OK;
	mhWndInspector = 0;
	mhEvtComplete = 0;
	mbSectionOK = false;
	mbDestroyCalled = false;

	m_hbrush = 0;
	charGen = 0;
	hParent=0;
	hListBox= 0;
	idListBox=0;
	hInstance = 0;
	SelectedListItem=-1;
	SelectedDirectoryIndex=-1;
	SelectedQuickLoadDiskFile = false;
	SelectedAlignD64Tracks = false;
	mbGapsDone = false;
	miLoadedListItemCount = 0;
	DisableQuickLoad = false;
}

CPRGBrowse::~CPRGBrowse()
{
	CleanUp();
}
void CPRGBrowse::CleanUp()
{
	if (mhEvtComplete)
	{
		CloseHandle(mhEvtComplete);
		mhEvtComplete=0;
	}
	if (mhEvtQuit)
	{
		CloseHandle(mhEvtQuit);
		mhEvtQuit=0;
	}
	if (m_hbrush)
	{
		DeleteObject(m_hbrush);
		m_hbrush = 0;
	}
	if (mbSectionOK)
	{
		mbSectionOK=false;
		DeleteCriticalSection(&mCrtStatus);
	}
}

HRESULT CPRGBrowse::Init(bit8 *charGen)
{
HRESULT hr;
RGBQUAD rgb;
	hr = c64file.Init();
	if (FAILED(hr))
	{
		CleanUp();
		return E_FAIL;
	}
	//mhEvtComplete = CreateEvent(NULL, TRUE, TRUE, TEXT("HOX64EVENT_COMPLETED"));
	mhEvtComplete = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (!mhEvtComplete)
	{
		CleanUp();
		return E_FAIL;
	}
	//mhEvtQuit = CreateEvent(NULL, TRUE, TRUE, TEXT("HOX64EVENT_QUIT"));
	mhEvtQuit = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (!mhEvtQuit)
	{
		CleanUp();
		return E_FAIL;
	}

	memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicBLUE], 4);
	m_hbrush = CreateSolidBrush(RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));
	if (m_hbrush==0)
	{
		CleanUp();
		return E_FAIL;
	}

	__try
	{
	InitializeCriticalSection(&mCrtStatus);
	}
	__except(GetExceptionCode() == STATUS_NO_MEMORY ? 
		EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		CleanUp();
		return E_FAIL;
	}
	mbSectionOK=true;
	if (!mbSectionOK)
	{
		CleanUp();
		return E_FAIL;
	}
	this->charGen = charGen;
	return S_OK;
}

BOOL CPRGBrowse::Open(HINSTANCE hInstance, OPENFILENAME *pOF, enum filetype filetypes)
{
BOOL r = 0;
	CPRGBrowse::hInstance = hInstance;
	pOF->hInstance = hInstance;
	pOF->Flags = pOF->Flags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_ENABLESIZING;
	pOF->lpTemplateName = MAKEINTRESOURCE(IDD_BROWSEPRG);
	pOF->lCustData = (LPARAM)this;
	pOF->lpfnHook = PRGBrowseDialogHookProc;
	mAllowTypes = filetypes;
	r = GetOpenFileName(pOF);
	return r;
}

void CPRGBrowse::CreateControls(HWND hDlg)
{
	hListBox = GetDlgItem(hDlg, IDC_CUSTOMPRGLIST);
	hCheckQuickLoad = GetDlgItem(hDlg, IDC_CHKQUICKLOAD);
	hCheckAlignD64Tracks = GetDlgItem(hDlg, IDC_CHKALIGND64TRACKS);
	SelectedListItem=-1;
	SelectedDirectoryIndex=-1;
}

void CPRGBrowse::CancelFileInspector()
{
	EnterCriticalSection(&mCrtStatus);
	mhWndInspector = 0;
	SetEvent(mhEvtQuit);
	LeaveCriticalSection(&mCrtStatus);
	WaitForSingleObject(mhEvtComplete, INFINITE);
}

HRESULT CPRGBrowse::BeginFileInspector(HWND hWndDlg, TCHAR *fileName)
{
DWORD threadId;
HANDLE hThread;

	EnterCriticalSection(&mCrtStatus);
	ResetEvent(mhEvtQuit);
	mhWndInspector = hWndDlg;
	memset(mptsFileName, 0, sizeof(mptsFileName));
	_tcscpy_s(mptsFileName, _countof(mptsFileName), fileName);
	LeaveCriticalSection(&mCrtStatus);
	hThread = CreateThread(NULL, 0, StartFileInspectorThread, this, CREATE_SUSPENDED, &threadId);
	if (hThread == 0)
		return E_FAIL;
	else
	{
		ResetEvent(mhEvtComplete);
		ResumeThread(hThread);
		return S_OK;
	}
}


DWORD WINAPI CPRGBrowse::StartFileInspectorThread(LPVOID lpParam)
{
	return ((CPRGBrowse *)lpParam)->StartFileInspector();
}

DWORD CPRGBrowse::StartFileInspector()
{
bool bTypeOK=false;
C64File::eC64FileType ftype;
HRESULT hr;

	InspectorStart();

	if (mAllowTypes != ALL)
	{
		hr = c64file.GetC64FileType(mptsFileName, ftype);
		if (FAILED(hr))
		{
			InspectorCompleteFail();
			return 0;
		}

		switch(ftype)
		{
		case C64File::ef_FDI:
			if (mAllowTypes & FDI)
				bTypeOK=true;
			break;
		case C64File::ef_G64:
			if (mAllowTypes & G64)
				bTypeOK=true;
			break;
		case C64File::ef_D64:
			if (mAllowTypes & D64)
				bTypeOK=true;
			break;
		case C64File::ef_PRG:
			if (mAllowTypes & PRG)
				bTypeOK=true;
			break;
		case C64File::ef_P00:
			if (mAllowTypes & P00)
				bTypeOK=true;
			break;
		case C64File::ef_T64:
			if (mAllowTypes & T64)
				bTypeOK=true;
			break;
		case C64File::ef_TAP:
			if (mAllowTypes & TAP)
				bTypeOK=true;
			break;
		case C64File::ef_SID:
			if (mAllowTypes & SID)
				bTypeOK=true;
			break;
		}
	}
	if (bTypeOK || mAllowTypes == ALL)
	{
		int i;
		hr = c64file.LoadDirectory(mptsFileName, MAXT64LIST, i, false, mhEvtQuit);
		if (FAILED(hr))
		{
			InspectorCompleteFail();
			return 0;
		}
	}
	InspectorCompleteOK();
	return 0;
}

void CPRGBrowse::InspectorStart()
{
	EnterCriticalSection(&mCrtStatus);
	c64file.ClearDirectory();
	mFileInspectorStatus = WORKING;
	mFileInspectorResult = E_FAIL; 
	ResetEvent(mhEvtComplete);
	if (mhWndInspector!=0 || mbDestroyCalled)
		PostMessage(mhWndInspector, WM_FILEINSPECTOR, 0, 0);
	LeaveCriticalSection(&mCrtStatus);
}

void CPRGBrowse::InspectorCompleteFail()
{
	EnterCriticalSection(&mCrtStatus);
	c64file.ClearDirectory();
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = E_FAIL; 
	SetEvent(mhEvtComplete);
	if (mhWndInspector!=0 || mbDestroyCalled)
		PostMessage(mhWndInspector, WM_FILEINSPECTOR, 0, 0);
	LeaveCriticalSection(&mCrtStatus);
}

void CPRGBrowse::InspectorCompleteOK()
{
	EnterCriticalSection(&mCrtStatus);
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = S_OK; 
	SetEvent(mhEvtComplete);
	if (mhWndInspector!=0 || mbDestroyCalled)
		PostMessage(mhWndInspector, WM_FILEINSPECTOR, 0, 0);
	LeaveCriticalSection(&mCrtStatus);
}

LRESULT CPRGBrowse::ChildDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
NMHDR *mh;
TCHAR fileName[MAX_PATH+1];
int len=0;
LPMEASUREITEMSTRUCT lpmis;
LPDRAWITEMSTRUCT lpdis;
BYTE tempC64String[24];
RGBQUAD rgb;
HBRUSH hBrushListBox;
HBRUSH hbrushOld;
HRESULT hr;
int i;

LPOFNOTIFY lpOfNotify;
CPRGBrowse::FIS fis;
WORD w,h;
RECT rcListBox;
RECT rcCheckBoxQuickLoad;
RECT rcCheckBoxAlignD64Tracks;
RECT rcDlg;
LRESULT lr;
//RECT rcParentDlg;
	switch (msg) 
	{ 
	case WM_INITDIALOG:
		hParent = GetParent(hDlg);
		mbGapsDone = false;
		CreateControls(hDlg);

		return TRUE;
	case WM_SHOWWINDOW:
		return FALSE;
	case WM_NOTIFY:
		mh = (NMHDR *)lParam;
		if (mh)
		{
			switch(mh->code)
			{
			case CDN_INITDONE:
				if (hDlg !=0 && hListBox!=0 && hCheckQuickLoad!=0 && hCheckAlignD64Tracks!=0)
				{
					if (GetWindowRect(hListBox, &rcListBox)!=0 && GetWindowRect(hCheckQuickLoad, &rcCheckBoxQuickLoad)!=0 && GetWindowRect(hCheckAlignD64Tracks, &rcCheckBoxAlignD64Tracks)!=0)
					{
						ScreenToClient(hDlg, (POINT*)&rcListBox.left);
						ScreenToClient(hDlg, (POINT*)&rcListBox.right);
						ScreenToClient(hDlg, (POINT*)&rcCheckBoxQuickLoad.left);
						ScreenToClient(hDlg, (POINT*)&rcCheckBoxQuickLoad.right);
						ScreenToClient(hDlg, (POINT*)&rcCheckBoxAlignD64Tracks.left);
						ScreenToClient(hDlg, (POINT*)&rcCheckBoxAlignD64Tracks.right);

						GetClientRect(hDlg, &rcDlg);
						mgapListBoxBottom = rcDlg.bottom - rcListBox.bottom;
						mgapCheckBoxQuickLoadBottom = rcDlg.bottom - rcCheckBoxQuickLoad.bottom;
						mgapCheckBoxAlignD64TracksBottom = rcDlg.bottom - rcCheckBoxAlignD64Tracks.bottom;
						mbGapsDone = true;							
						ReSize(hDlg, rcDlg.right- rcDlg.left, rcDlg.bottom - rcDlg.top);
					}
				}
				if (DisableQuickLoad)
					ShowWindow(hCheckQuickLoad, SW_HIDE);
				break;
			case CDN_SELCHANGE:
				CancelFileInspector();
				EnterCriticalSection(&mCrtStatus);
				c64file.ClearDirectory();
				SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
				LeaveCriticalSection(&mCrtStatus);
				lr = SendMessage(GetParent(hDlg), CDM_GETSPEC, MAX_PATH-1, (LPARAM)fileName);
				if (lr <= 1 || lr > MAXLONG)
					break;
				lr = SendMessage(GetParent(hDlg), CDM_GETFILEPATH, MAX_PATH-1, (LPARAM)fileName);
				if (lr <= 1 || lr > MAXLONG)
					break;
				len = (LONG)lr;
				if (len <= 1)
					break;
				if (len > MAX_PATH)
				{
					MessageBox(GetParent(hDlg), TEXT("Path too long."), APPNAME, MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				EnableWindow(hCheckQuickLoad, TRUE);
				EnableWindow(hCheckAlignD64Tracks, TRUE);
				hr = BeginFileInspector(hDlg, fileName);
				break;
			case CDN_FILEOK:
				CancelFileInspector();
				SelectedListItem = -1;
				SelectedDirectoryIndex = -1;
				SelectedC64FileNameLength = 0;
				SelectedQuickLoadDiskFile = (IsDlgButtonChecked(hDlg, IDC_CHKQUICKLOAD) != BST_UNCHECKED) ? true : false;
				SelectedAlignD64Tracks = (IsDlgButtonChecked(hDlg, IDC_CHKALIGND64TRACKS) != BST_UNCHECKED) ? true : false;
				memset(SelectedC64FileName, 0xA0, sizeof(SelectedC64FileName));
				lpOfNotify = (LPOFNOTIFY) lParam;
				lr = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
				if (lr == LB_ERR || lr <= 0 || lr > MAXLONG)
				{
					//FALSE indicates to accept the file
					return FALSE;
				}
				lr = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
				if (lr == LB_ERR || lr <= 0 || lr > MAXLONG)
				{
					return FALSE;
				}
				i = (LONG)lr;
				
				SelectedListItem = i;

				if (SelectedListItem >= HEADERLINES)
				{
					lr = SendMessage(hListBox, LB_GETITEMDATA, SelectedListItem, 0);
					if (lr != LB_ERR && lr >= 0 && lr < MAXLONG)
					{
						i = (LONG)lr;
						SelectedDirectoryIndex = c64file.GetOriginalDirectoryIndex(i);
						SelectedC64FileNameLength = c64file.GetDirectoryItemName(i, SelectedC64FileName, sizeof(SelectedC64FileName));
						if (SelectedC64FileNameLength > sizeof(SelectedC64FileName))
							SelectedC64FileNameLength = sizeof(SelectedC64FileName);
					}
				}
				return FALSE;
			}
		}
		break;
	case WM_CTLCOLORLISTBOX:
		return (LRESULT)m_hbrush;
	case WM_MEASUREITEM:
		if (wParam == IDC_CUSTOMPRGLIST)
		{
			lpmis = (LPMEASUREITEMSTRUCT) lParam;
			lpmis->itemHeight = 16;
			//lpmis->itemWidth = 8*16;
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_CUSTOMPRGLIST)
		{
			lpdis = (LPDRAWITEMSTRUCT) lParam;
			if (lpdis->itemID == -1)
				return TRUE;
			switch (lpdis->itemAction)
			{
			case ODA_SELECT:
			case ODA_DRAWENTIRE:
				int charLen;
				EnterCriticalSection(&mCrtStatus);
				bool bUsedShifedCharROMSet = false;
				if (mFileInspectorStatus == WORKING)
				{
					memset(tempC64String, 0x20, sizeof(tempC64String));
					if (lpdis->itemID == 0)
					{
						memcpy_s(tempC64String, sizeof(tempC64String), "WORKING..", 9);
					}
				}
				else if (lpdis->itemID == 0)
				{
					memset(tempC64String, 0x20, sizeof(tempC64String));
					charLen = c64file.GetC64Diskname(tempC64String, sizeof(tempC64String));
					if (charLen > sizeof(tempC64String))
						charLen = sizeof(tempC64String);
				}
				else if (lpdis->itemData >= (DWORD)miLoadedListItemCount)
				{
					charLen = sizeof(tempC64String);
					memset(tempC64String, 0x20, sizeof(tempC64String));
				}
				else
				{
					memset(tempC64String, 0x20, sizeof(tempC64String));
					charLen = c64file.GetDirectoryItemName((int)lpdis->itemData, tempC64String, sizeof(tempC64String));
					if (charLen > sizeof(tempC64String))
						charLen = sizeof(tempC64String);
					const bit8 *ftname = c64file.GetDirectoryItemTypeName((int)lpdis->itemData);
					if (ftname!=NULL)
					{
						if (sizeof(tempC64String) >= (charLen + sizeof(C64File::FTN_CLR) + 1))
						{
							memcpy_s(&tempC64String[sizeof(tempC64String) - sizeof(C64File::FTN_CLR)], sizeof(C64File::FTN_CLR), ftname, sizeof(C64File::FTN_CLR));
						}
					}
				}

				if (c64file.GetFileType()==C64File::ef_SID && charLen >0)
				{
					bUsedShifedCharROMSet = true;
					for (i=0; i<charLen; i++)
					{
						if (tempC64String[i] >= 'A' && tempC64String[i] <= 'Z')
							tempC64String[i]+=0x20;
						else if (tempC64String[i] >= 'a' && tempC64String[i] <= 'z')
							tempC64String[i]-=0x20;
					}
				}
				memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicBLUE], 4);
				hBrushListBox = CreateSolidBrush(RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));

				hbrushOld = (HBRUSH)SelectObject(lpdis->hDC, hBrushListBox);

				FillRect(lpdis->hDC, &lpdis->rcItem, hBrushListBox);

				DrawC64String(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top + 4, tempC64String, sizeof(tempC64String), bUsedShifedCharROMSet);

				if (lpdis->itemID == 0 && miLoadedListItemCount!=0)
				{
					memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicGREEN], 4);
					HPEN hPen = CreatePen(PS_SOLID, 2, RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));
					if (hPen)
					{
						HPEN hOldPen = (HPEN)SelectObject(lpdis->hDC, hPen);
						MoveToEx(lpdis->hDC, 0, lpdis->rcItem.bottom-3, NULL);
						LineTo(lpdis->hDC, lpdis->rcItem.right-1, lpdis->rcItem.bottom-3);
						SelectObject(lpdis->hDC, hOldPen);
						DeleteObject(hPen);
						hPen = NULL;
					}
				}
                if (lpdis->itemState & ODS_SELECTED) 
                { 
                    DrawFocusRect(lpdis->hDC, &lpdis->rcItem); 
                } 

				SelectObject(lpdis->hDC, hbrushOld);
				DeleteObject(hBrushListBox);
				
				LeaveCriticalSection(&mCrtStatus);
			}
		}
		return TRUE;
		break;
	case WM_COMMAND:
		EnterCriticalSection(&mCrtStatus);
		fis = mFileInspectorStatus;
		LeaveCriticalSection(&mCrtStatus);
		if (LOWORD(wParam) == IDCANCEL)
			CancelFileInspector();
		if (LOWORD(wParam) == IDC_CUSTOMPRGLIST)
		{
			if ((HIWORD(wParam) == LBN_DBLCLK) && (fis == COMPLETED))
			{
				lr = SendMessage(hListBox, LB_GETCURSEL, 0, 0);
				if (lr >= 0 && lr <= MAXLONG)
				{
					i = (LONG)lr;
					SelectedListItem = i;
					PostMessage(GetParent(hDlg), WM_COMMAND, MAKEWPARAM(IDOK, 1), (LPARAM)GetDlgItem(GetParent(hDlg), IDOK));
				}
			}
			return TRUE;
		}
		else
			return FALSE;
	case WM_FILEINSPECTOR:
		if (mbDestroyCalled)
			return TRUE;
		
		EnterCriticalSection(&mCrtStatus);
		if (mFileInspectorStatus == CPRGBrowse::COMPLETED)
		{
			PopulateList(hDlg);

			switch (c64file.GetFileType())
			{
			case C64File::ef_FDI:
				EnableWindow(hCheckQuickLoad, TRUE);
				EnableWindow(hCheckAlignD64Tracks, FALSE);
				break;
			case C64File::ef_G64:
				EnableWindow(hCheckQuickLoad, TRUE);
				EnableWindow(hCheckAlignD64Tracks, FALSE);
				break;
			case C64File::ef_D64:
				EnableWindow(hCheckQuickLoad, TRUE);
				EnableWindow(hCheckAlignD64Tracks, TRUE);
				//CheckDlgButton(hDlg, IDC_CHKQUICKLOAD, BST_UNCHECKED);
				break;
			default:
				//CheckDlgButton(hDlg, IDC_CHKQUICKLOAD, BST_CHECKED);
				EnableWindow(hCheckQuickLoad, FALSE);
				EnableWindow(hCheckAlignD64Tracks, FALSE);
				break;
			}
		}
		else if (mFileInspectorStatus == CPRGBrowse::WORKING)
		{
			SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT("Working..."));
		}
		LeaveCriticalSection(&mCrtStatus);
		return TRUE;
	case WM_SIZE:
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		ReSize(hDlg, w, h);
		return TRUE;
	case WM_DESTROY:
		EnterCriticalSection(&mCrtStatus);
		mbDestroyCalled = false;
		mhWndInspector = NULL;
		LeaveCriticalSection(&mCrtStatus);
		CancelFileInspector();
		return TRUE;
	default:
		return FALSE;

	}
	return FALSE;
}
void CPRGBrowse::ReSize(HWND hDlg, LONG w, LONG h)
{
RECT rcListBox;
RECT rcCheckBoxQuickLoad;
RECT rcCheckBoxAlignD64Tracks;
RECT rcDlg;
	if (hDlg !=0 && hListBox!=0 && hCheckQuickLoad!=0 && hCheckAlignD64Tracks!=0 && mbGapsDone)
	{
		if (GetWindowRect(hListBox, &rcListBox)!=0 && GetWindowRect(hCheckQuickLoad, &rcCheckBoxQuickLoad)!=0 && GetWindowRect(hCheckAlignD64Tracks, &rcCheckBoxAlignD64Tracks)!=0)
		{
			ScreenToClient(hDlg, (POINT*)&rcListBox.left);
			ScreenToClient(hDlg, (POINT*)&rcListBox.right);
			ScreenToClient(hDlg, (POINT*)&rcCheckBoxQuickLoad.left);
			ScreenToClient(hDlg, (POINT*)&rcCheckBoxQuickLoad.right);
			ScreenToClient(hDlg, (POINT*)&rcCheckBoxAlignD64Tracks.left);
			ScreenToClient(hDlg, (POINT*)&rcCheckBoxAlignD64Tracks.right);
			GetClientRect(hDlg, &rcDlg);
			w = rcDlg.right - rcDlg.left;
			h = rcDlg.bottom - rcDlg.top;
			if (w>0 && h >0)
			{
				InvalidateRect(hDlg, &rcCheckBoxQuickLoad, TRUE);
				InvalidateRect(hDlg, &rcCheckBoxAlignD64Tracks, TRUE);
				
				MoveWindow(hListBox, rcListBox.left, rcListBox.top, (rcListBox.right - rcListBox.left), h - mgapListBoxBottom - rcListBox.top, TRUE);
				MoveWindow(hCheckQuickLoad, rcListBox.left, h - mgapCheckBoxQuickLoadBottom - (rcCheckBoxQuickLoad.bottom-rcCheckBoxQuickLoad.top), (rcCheckBoxQuickLoad.right - rcCheckBoxQuickLoad.left), (rcCheckBoxQuickLoad.bottom - rcCheckBoxQuickLoad.top), TRUE);
				MoveWindow(hCheckAlignD64Tracks, rcListBox.left, h - mgapCheckBoxAlignD64TracksBottom - (rcCheckBoxAlignD64Tracks.bottom-rcCheckBoxAlignD64Tracks.top), (rcCheckBoxAlignD64Tracks.right - rcCheckBoxAlignD64Tracks.left), (rcCheckBoxAlignD64Tracks.bottom - rcCheckBoxAlignD64Tracks.top), TRUE);

				UpdateWindow(hDlg);
			}
		}
	}

}
void CPRGBrowse::DrawC64String(HDC hdc, int x, int y, BYTE str[], int length, bool bShifted)
{
int i;
int row,col;
unsigned char c, gdata,petascii;
COLORREF bg,fg;
RGBQUAD rgb;

	memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicBLUE], 4);
	bg = PALETTERGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);

	memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicLIGHT_BLUE], 4);
	fg = PALETTERGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
	
	for (i=0; i<length; i++, x+=8)
	{

		petascii = str[i];	
		c = C64File::ConvertPetAsciiToScreenCode(petascii);

		for (row=0; row<8; row++)
		{
			if (bShifted)
				gdata = charGen[c * 8 + row+ 2048];
			else
				gdata = charGen[c * 8 + row];
			for (col=0; col<8; col++, gdata <<= 1)
			{
				if (gdata & 0x80)
					SetPixel(hdc, x + col, y + row, fg);
				else
					SetPixel(hdc, x + col, y + row, bg);
			}
		}
	}
}

void CPRGBrowse::PopulateList(HWND hDlg)
{
LRESULT i,j,k;
int c;

	EnterCriticalSection(&mCrtStatus);
	miLoadedListItemCount = 0;
	SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
	if (mFileInspectorResult == S_OK && mFileInspectorStatus == COMPLETED)
	{
		j = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT("C64 Header"));
		if (j >= 0)
		{
			SendMessage(hListBox, LB_SETITEMDATA, 0, 0);

			c = c64file.GetFileCount();
			miLoadedListItemCount = min(c, MAXLISTITEMCOUNT);
			for (i=0,k=0 ; i < miLoadedListItemCount ; i++)
			{
				j = SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT("C64 File"));
				if (j < 0)
					break;
				SendMessage(hListBox, LB_SETITEMDATA, j, i);
				k++;
			}
			if (k>0)
				SendMessage(hListBox, LB_SETCURSEL, 0, 0);
		}
	}
	LeaveCriticalSection(&mCrtStatus);
}

UINT_PTR CALLBACK PRGBrowseDialogHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
OPENFILENAME *pOF=0;
CPRGBrowse *prgBrowseClass=0L;

	switch (msg) 
	{ 
	case WM_INITDIALOG:
		#pragma warning(disable:4244)
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		#pragma warning(default:4244)
		pOF = (OPENFILENAME *) lParam;
		break;
	default:
		pOF = (OPENFILENAME *) (LONG_PTR)GetWindowLongPtr(hDlg, DWLP_USER);
	}
	if (pOF)
		prgBrowseClass = (CPRGBrowse *)pOF->lCustData;
	if (prgBrowseClass)
		return prgBrowseClass->ChildDialogProc(hDlg, msg, wParam, lParam);
	else
		return FALSE;
}