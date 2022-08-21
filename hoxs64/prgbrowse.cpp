#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <winuser.h>
#include "dx_version.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <algorithm>
#include "boost2005.h"
#include "defines.h"
#include "mlist.h"
#include "carray.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "StringConverter.h"
#include "ErrorLogger.h"
#include "errormsg.h"
#include "hconfig.h"
#include "appstatus.h"
#include "dxstuff9.h"
#include "register.h"
#include "savestate.h"
#include "cart.h"
#include "c6502.h"
#include "ram64.h"
#include "cpu6510.h"
#include "vic6569.h"
#include "t64.h"
#include "p64.h"
#include "d64.h"
#include "sidfile.h"
#include "c64file.h"
#include "resource.h"
#include "prgbrowse.h"

#define MAXLENLVITEM (24)
#define HEIGHT_LVITEM_96 (16)
#define HEIGHT_LVFONT_96 (8)
#define MARGIN_TOP_LVITEM_96 (4)

UINT_PTR CALLBACK PRGBrowseDialogHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#define WM_FILEINSPECTOR WM_USER
CPRGBrowse::CPRGBrowse() noexcept
{
	mhEvtQuit = 0;
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = S_OK;
	mhWndInspector = 0;
	mhEvtComplete = 0;
	mbSectionOK = false;
	m_hbrush = 0;
	m_pCharGen = 0;
	m_hParent=0;
	m_hBrowse=0;
	m_hListBox= 0;
	m_hInstance = 0;
	SelectedListItem=-1;
	SelectedDirectoryIndex=-1;
	SelectedQuickLoadDiskFile = false;
	SelectedAlignD64Tracks = false;
	SelectedWantReu = false;
	mbGapsDone = false;
	miLoadedListItemCount = 0;
	DisableQuickLoad = false;
}

CPRGBrowse::~CPRGBrowse()
{
	CleanUp();
}

void CPRGBrowse::CleanUp() noexcept
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

HRESULT CPRGBrowse::Init(bit8 *pCharGen, IC64* c64)
{
	RGBQUAD rgb;
	m_pC64 = c64;
	mhEvtComplete = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (!mhEvtComplete)
	{
		CleanUp();
		return E_FAIL;
	}

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
	this->m_pCharGen = pCharGen;
	return S_OK;
}

BOOL CPRGBrowse::Open(HINSTANCE hInstance, OPENFILENAME *pOF, enum FileTypeFlag::filetype filetypes)
{
BOOL r = 0;
	CPRGBrowse::m_hInstance = hInstance;
	pOF->hInstance = hInstance;
	pOF->Flags = pOF->Flags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_ENABLESIZING;
	pOF->lpTemplateName = MAKEINTRESOURCE(IDD_BROWSEPRG);
	pOF->lCustData = (LPARAM)this;
	pOF->lpfnHook = PRGBrowseDialogHookProc;
	mAllowTypes = filetypes;
	r = GetOpenFileName(pOF);
	return r;
}

HRESULT CPRGBrowse::CreateControls(HWND hDlg)
{
	m_hBrowse = GetDlgItem(hDlg, IDD_BROWSE_FILE);
	if (!m_hBrowse)
	{
		return E_FAIL;
	}

	m_hListBox = GetDlgItem(hDlg, IDC_CUSTOMPRGLIST);
	if (!m_hListBox)
	{
		return E_FAIL;
	}

	m_hCheckQuickLoad = GetDlgItem(hDlg, IDC_CHKQUICKLOAD);
	if (!m_hCheckQuickLoad)
	{
		return E_FAIL;
	}

	m_hCheckAlignD64Tracks = GetDlgItem(hDlg, IDC_CHKALIGND64TRACKS);
	if (!m_hCheckAlignD64Tracks)
	{
		return E_FAIL;
	}

	m_hCheckReu = GetDlgItem(hDlg, IDC_CHKREU);
	if (!m_hCheckReu)
	{
		return E_FAIL;
	}

	SendMessage(m_hListBox, LB_SETITEMHEIGHT, 0, m_dpi.ScaleY(HEIGHT_LVITEM_96));
	SendMessage(m_hListBox, LB_SETHORIZONTALEXTENT, m_dpi.ScaleX(MAXLENLVITEM * HEIGHT_LVFONT_96), 0);

	if (m_pC64->IsReuAttached())
	{
		CheckDlgButton(hDlg, IDC_CHKREU, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hDlg, IDC_CHKREU, BST_UNCHECKED);
	}

	SelectedListItem=-1;
	SelectedDirectoryIndex=-1;
	return S_OK;
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

	if (mAllowTypes != FileTypeFlag::ALL)
	{
		hr = m_c64file.GetC64FileType(mptsFileName, ftype);
		if (FAILED(hr))
		{
			InspectorCompleteFail();
			return 0;
		}

		switch(ftype)
		{
		case C64File::ef_FDI:
			if (mAllowTypes & FileTypeFlag::FDI)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_P64:
			if (mAllowTypes & FileTypeFlag::P64)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_G64:
			if (mAllowTypes & FileTypeFlag::G64)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_D64:
			if (mAllowTypes & FileTypeFlag::D64)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_PRG:
			if (mAllowTypes & FileTypeFlag::PRG)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_P00:
			if (mAllowTypes & FileTypeFlag::P00)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_T64:
			if (mAllowTypes & FileTypeFlag::T64)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_TAP:
			if (mAllowTypes & FileTypeFlag::TAP)
			{
				bTypeOK=true;
			}
			break;
		case C64File::ef_SID:
			if (mAllowTypes & FileTypeFlag::SID)
			{
				bTypeOK=true;
			}
			break;
		}
	}

	if (bTypeOK || mAllowTypes == FileTypeFlag::ALL)
	{
		int i;
		hr = m_c64file.LoadDirectory(mptsFileName, MAXT64LIST, i, false, mhEvtQuit);
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
	m_c64file.ClearDirectory();
	mFileInspectorStatus = WORKING;
	mFileInspectorResult = E_FAIL; 
	ResetEvent(mhEvtComplete);
	if (mhWndInspector!=0)
		PostMessage(mhWndInspector, WM_FILEINSPECTOR, 0, 0);
	LeaveCriticalSection(&mCrtStatus);
}

void CPRGBrowse::InspectorCompleteFail()
{
	EnterCriticalSection(&mCrtStatus);
	m_c64file.ClearDirectory();
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = E_FAIL; 
	SetEvent(mhEvtComplete);
	if (mhWndInspector!=0)
		PostMessage(mhWndInspector, WM_FILEINSPECTOR, 0, 0);
	LeaveCriticalSection(&mCrtStatus);
}

void CPRGBrowse::InspectorCompleteOK()
{
	EnterCriticalSection(&mCrtStatus);
	mFileInspectorStatus = COMPLETED;
	mFileInspectorResult = S_OK; 
	SetEvent(mhEvtComplete);
	if (mhWndInspector!=0)
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

HRESULT hr;
int i;

LPOFNOTIFY lpOfNotify;
CPRGBrowse::FIS fis;
WORD w,h;
RECT rcBrowse;
RECT rcListBox;
RECT rcCheckBoxQuickLoad;
RECT rcCheckBoxAlignD64Tracks;
RECT rcCheckReu;
RECT rcDlg;
LRESULT lr;
	switch (msg) 
	{ 
	case WM_INITDIALOG:
		m_hParent = GetParent(hDlg);
		mbGapsDone = false;
		hr = CreateControls(hDlg);
		return (SUCCEEDED(hr)) ? TRUE : FALSE;
	case WM_SHOWWINDOW:
		return FALSE;
	case WM_NOTIFY:
		mh = (NMHDR *)lParam;
		if (mh)
		{
			switch(mh->code)
			{
			case CDN_INITDONE:
				if (hDlg !=0 && m_hListBox!=0 && m_hCheckQuickLoad!=0 && m_hCheckAlignD64Tracks!=0 && m_hCheckReu!=0)
				{
					GetClientRect(hDlg, &rcDlg);
					GetWindowRect(m_hBrowse, &rcBrowse);
					GetWindowRect(m_hListBox, &rcListBox);
					GetWindowRect(m_hCheckQuickLoad, &rcCheckBoxQuickLoad);
					GetWindowRect(m_hCheckAlignD64Tracks, &rcCheckBoxAlignD64Tracks);
					GetWindowRect(m_hCheckReu, &rcCheckReu);
					MapWindowPoints(NULL, hDlg, (POINT *)&rcBrowse, 2);
					MapWindowPoints(NULL, hDlg, (POINT *)&rcListBox, 2);
					MapWindowPoints(NULL, hDlg, (POINT *)&rcCheckBoxQuickLoad, 2);
					MapWindowPoints(NULL, hDlg, (POINT *)&rcCheckBoxAlignD64Tracks, 2);
					MapWindowPoints(NULL, hDlg, (POINT*)&rcCheckReu, 2);
					m_width_custom = abs(rcDlg.right - rcDlg.left) - rcBrowse.right;

					mgapTop = abs(rcBrowse.top - rcDlg.top);
					mgapBottom = abs(rcBrowse.bottom - rcDlg.bottom);
					mbGapsDone = true;							
					ReSize(hDlg, rcDlg.right- rcDlg.left, rcDlg.bottom - rcDlg.top);
				}

				if (DisableQuickLoad)
				{
					if (m_hCheckQuickLoad)
					{
						ShowWindow(m_hCheckQuickLoad, SW_HIDE);
					}
				}

				break;
			case CDN_SELCHANGE:
				CancelFileInspector();
				EnterCriticalSection(&mCrtStatus);
				m_c64file.ClearDirectory();
				SendMessage(m_hListBox, LB_RESETCONTENT, 0, 0);
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
					G::DebugMessageBox(GetParent(hDlg), TEXT("Path too long."), APPNAME, MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				EnableWindow(m_hCheckQuickLoad, TRUE);
				EnableWindow(m_hCheckAlignD64Tracks, TRUE);
				hr = BeginFileInspector(hDlg, fileName);
				break;
			case CDN_FILEOK:
				CancelFileInspector();
				SelectedListItem = -1;
				SelectedDirectoryIndex = -1;
				SelectedC64FileNameLength = 0;
				SelectedQuickLoadDiskFile = (IsDlgButtonChecked(hDlg, IDC_CHKQUICKLOAD) != BST_UNCHECKED) ? true : false;
				SelectedAlignD64Tracks = (IsDlgButtonChecked(hDlg, IDC_CHKALIGND64TRACKS) != BST_UNCHECKED) ? true : false;
				SelectedWantReu = (IsDlgButtonChecked(hDlg, IDC_CHKREU) != BST_UNCHECKED) ? true : false;
				memset(SelectedC64FileName, 0xA0, sizeof(SelectedC64FileName));
				lpOfNotify = (LPOFNOTIFY) lParam;
				lr = SendMessage(m_hListBox, LB_GETCOUNT, 0, 0);
				if (lr == LB_ERR || lr <= 0 || lr > MAXLONG)
				{
					//FALSE indicates to accept the file
					return FALSE;
				}

				lr = SendMessage(m_hListBox, LB_GETCURSEL, 0, 0);
				if (lr == LB_ERR || lr <= 0 || lr > MAXLONG)
				{
					return FALSE;
				}

				i = (LONG)lr;				
				SelectedListItem = i;
				if (SelectedListItem >= HEADERLINES)
				{
					lr = SendMessage(m_hListBox, LB_GETITEMDATA, SelectedListItem, 0);
					if (lr != LB_ERR && lr >= 0 && lr < MAXLONG)
					{
						i = (LONG)lr;
						SelectedDirectoryIndex = m_c64file.GetOriginalDirectoryIndex(i);
						SelectedC64FileNameLength = m_c64file.GetDirectoryItemName(i, SelectedC64FileName, sizeof(SelectedC64FileName));
						if (SelectedC64FileNameLength > sizeof(SelectedC64FileName))
						{
							SelectedC64FileNameLength = sizeof(SelectedC64FileName);
						}
					}
				}

				return FALSE;
			}
		}
		break;
	case WM_CTLCOLORLISTBOX:
		return (LRESULT)m_hbrush;
	case WM_MEASUREITEM:
		lpmis =0;
		if (wParam == IDC_CUSTOMPRGLIST)
		{
			lpmis = (LPMEASUREITEMSTRUCT) lParam;
			OnMeasureListViewItem(lpmis);
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		if (wParam == IDC_CUSTOMPRGLIST)
		{
			lpdis = (LPDRAWITEMSTRUCT) lParam;
			OnDrawListViewItem(lpdis);
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
				lr = SendMessage(m_hListBox, LB_GETCURSEL, 0, 0);
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
		EnterCriticalSection(&mCrtStatus);
		if (mFileInspectorStatus == CPRGBrowse::COMPLETED)
		{
			PopulateList(hDlg);

			switch (m_c64file.GetFileType())
			{
			case C64File::ef_P64:
				EnableWindow(m_hCheckQuickLoad, TRUE);
				EnableWindow(m_hCheckAlignD64Tracks, FALSE);
				break;
			case C64File::ef_FDI:
				EnableWindow(m_hCheckQuickLoad, TRUE);
				EnableWindow(m_hCheckAlignD64Tracks, FALSE);
				break;
			case C64File::ef_G64:
				EnableWindow(m_hCheckQuickLoad, TRUE);
				EnableWindow(m_hCheckAlignD64Tracks, FALSE);
				break;
			case C64File::ef_D64:
				EnableWindow(m_hCheckQuickLoad, TRUE);
				EnableWindow(m_hCheckAlignD64Tracks, TRUE);
				break;
			default:
				EnableWindow(m_hCheckQuickLoad, FALSE);
				EnableWindow(m_hCheckAlignD64Tracks, FALSE);
				break;
			}
		}
		else if (mFileInspectorStatus == CPRGBrowse::WORKING)
		{
			SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT(""));
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
		mhWndInspector = NULL;
		LeaveCriticalSection(&mCrtStatus);
		CancelFileInspector();
		return TRUE;
	default:
		return FALSE;

	}
	return FALSE;
}

void CPRGBrowse::OnMeasureListViewItem(LPMEASUREITEMSTRUCT lpmis)
{
	lpmis->itemHeight = m_dpi.ScaleY(HEIGHT_LVITEM_96);
}

void CPRGBrowse::OnDrawListViewItem(LPDRAWITEMSTRUCT lpdis)
{
	const char S_WORKING[] = "WORKING...";
	BYTE tempC64String[MAXLENLVITEM];
	RGBQUAD rgb;
	HBRUSH hBrushListBox;
	HBRUSH hbrushOld;
	int charLen=0;

	if (lpdis->itemID == -1)
	{
		return;
	}

	switch (lpdis->itemAction)
	{
	case ODA_SELECT:
	case ODA_DRAWENTIRE:
		EnterCriticalSection(&mCrtStatus);
		bool bUsedShifedCharROMSet = false;
		if (mFileInspectorStatus == WORKING)
		{
			//Worker thread is still looking for items.
			charLen = sizeof(tempC64String);
			memset(tempC64String, 0x20, sizeof(tempC64String));
			if (lpdis->itemID == 0)
			{
				memcpy_s(tempC64String, sizeof(tempC64String), S_WORKING, sizeof(S_WORKING) - 1);
			}
		}
		else if (lpdis->itemID == 0)
		{
			//Put the title in list index position 0.
			memset(tempC64String, 0x20, sizeof(tempC64String));
			charLen = m_c64file.GetC64Diskname(tempC64String, sizeof(tempC64String));
			if (charLen > sizeof(tempC64String))
			{
				charLen = sizeof(tempC64String);
			}
		}
		else if (lpdis->itemData >= (DWORD)miLoadedListItemCount)
		{
			//Blank out list index positions that are out of bounds.
			charLen = sizeof(tempC64String);
			memset(tempC64String, 0x20, sizeof(tempC64String));
		}
		else
		{
			//Get character data for this list item.
			memset(tempC64String, 0x20, sizeof(tempC64String));
			charLen = m_c64file.GetDirectoryItemName((int)lpdis->itemData, tempC64String, sizeof(tempC64String));
			if (charLen > sizeof(tempC64String))
			{
				charLen = sizeof(tempC64String);
			}

			const bit8 *ftname = m_c64file.GetDirectoryItemTypeName((int)lpdis->itemData);
			if (ftname!=NULL)
			{
				if (sizeof(tempC64String) >= (charLen + sizeof(C64File::FTN_CLR) + 1))
				{
					memcpy_s(&tempC64String[sizeof(tempC64String) - sizeof(C64File::FTN_CLR)], sizeof(C64File::FTN_CLR), ftname, sizeof(C64File::FTN_CLR));
				}
			}
		}

		if (m_c64file.GetFileType()==C64File::ef_SID)
		{
			bUsedShifedCharROMSet = true;
		}

		//Create background brush.
		memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicBLUE], 4);
		hBrushListBox = CreateSolidBrush(RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));
		if (hBrushListBox)
		{
			hbrushOld = (HBRUSH)SelectObject(lpdis->hDC, hBrushListBox);
			if (hbrushOld)
			{
				FillRect(lpdis->hDC, &lpdis->rcItem, hBrushListBox);
				DrawC64String(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top + m_dpi.ScaleY(MARGIN_TOP_LVITEM_96), tempC64String, sizeof(tempC64String), bUsedShifedCharROMSet, m_dpi.ScaleY(HEIGHT_LVFONT_96));
				if (lpdis->itemID == 0 && miLoadedListItemCount!=0)
				{
					//Draw unline for the title position
					int lineThickness = m_dpi.ScaleY(2);
					memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicGREEN], 4);
					HPEN hPen = CreatePen(PS_SOLID, lineThickness, RGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue));
					if (hPen)
					{
						int y = lpdis->rcItem.bottom - 1 - lineThickness;
						HPEN hOldPen = (HPEN)SelectObject(lpdis->hDC, hPen);
						MoveToEx(lpdis->hDC, 0, y, NULL);
						LineTo(lpdis->hDC, lpdis->rcItem.right-1, y);
						SelectObject(lpdis->hDC, hOldPen);
						DeleteObject(hPen);
						hPen = NULL;
					}
				}

				if ((lpdis->itemState & ODS_SELECTED))
				{ 
					DrawFocusRect(lpdis->hDC, &lpdis->rcItem);
				} 

				SelectObject(lpdis->hDC, hbrushOld);
			}

			DeleteObject(hBrushListBox);
		}				

		LeaveCriticalSection(&mCrtStatus);
	}
}

void CPRGBrowse::ReSize(HWND hDlg, LONG w, LONG h)
{
	RECT rcListBox;
	RECT rcCheckBoxQuickLoad;
	RECT rcCheckBoxAlignD64Tracks;
	RECT rcCheckReu;
	RECT rcDlg;
	RECT rcBrowse;

	if (hDlg != 0 && m_hListBox != 0 && m_hCheckQuickLoad != 0 && m_hCheckAlignD64Tracks != 0 && m_hCheckReu != 0 && mbGapsDone)
	{
		GetClientRect(hDlg, &rcDlg);
		GetWindowRect(m_hBrowse, &rcBrowse);
		GetWindowRect(m_hListBox, &rcListBox);
		GetWindowRect(m_hCheckQuickLoad, &rcCheckBoxQuickLoad);
		GetWindowRect(m_hCheckAlignD64Tracks, &rcCheckBoxAlignD64Tracks);
		GetWindowRect(m_hCheckReu, &rcCheckReu);
		MapWindowPoints(NULL, hDlg, (POINT*)&rcBrowse, 2);
		MapWindowPoints(NULL, hDlg, (POINT*)&rcListBox, 2);
		MapWindowPoints(NULL, hDlg, (POINT*)&rcCheckBoxQuickLoad, 2);
		MapWindowPoints(NULL, hDlg, (POINT*)&rcCheckBoxAlignD64Tracks, 2);
		MapWindowPoints(NULL, hDlg, (POINT*)&rcCheckReu, 2);
		int gap = m_dpi.ScaleY(4);
		int width_custom = m_width_custom;
		int height_custom = abs(rcDlg.bottom - rcDlg.top);
		int width_listbox = width_custom - 2 * gap;
		int height_listbox = height_custom - abs(rcCheckBoxQuickLoad.bottom - rcCheckBoxQuickLoad.top) - abs(rcCheckBoxAlignD64Tracks.bottom - rcCheckBoxAlignD64Tracks.top) - abs(rcCheckReu.bottom - rcCheckReu.top) - 4 * gap - mgapTop - mgapBottom;
		if (width_listbox < 0) width_listbox = 0;
		if (height_listbox < 0) height_listbox = 0;
		int cx = abs(rcDlg.right - rcDlg.left) - width_custom;

		if (width_listbox > 0 && height_listbox > 0)
		{
			if (!IsWindowVisible(m_hListBox))
			{
				ShowWindow(m_hListBox, SW_SHOW);
			}

			MoveWindow(m_hListBox, cx + gap, rcDlg.top + gap, width_listbox, height_listbox, TRUE);
		}
		else
		{
			if (IsWindowVisible(m_hListBox))
			{
				ShowWindow(m_hListBox, SW_HIDE);
			}

		}

		int nextYpos = rcDlg.top + gap + height_listbox + gap;
		MoveWindow(m_hCheckQuickLoad,      cx + gap, nextYpos, width_listbox, abs(rcCheckBoxQuickLoad.bottom - rcCheckBoxQuickLoad.top), TRUE);
		nextYpos += abs(rcCheckBoxQuickLoad.bottom - rcCheckBoxQuickLoad.top) + gap;
		MoveWindow(m_hCheckAlignD64Tracks, cx + gap, nextYpos, width_listbox, abs(rcCheckBoxAlignD64Tracks.bottom - rcCheckBoxAlignD64Tracks.top), TRUE);
		nextYpos += abs(rcCheckReu.bottom - rcCheckReu.top) + gap;
		MoveWindow(m_hCheckReu,            cx + gap, nextYpos, width_listbox, abs(rcCheckReu.bottom - rcCheckReu.top), TRUE);
		UpdateWindow(hDlg);
	}
}

void CPRGBrowse::DrawC64String(HDC hdc, int x, int y, BYTE str[], int length, bool bShifted, int fontheight)
{
const int MULT = 1;
int dx = 8 * length * MULT;
int dy = 8 * MULT;

	HDC hMemDC = CreateCompatibleDC(hdc);
	if (hMemDC)
	{
		HBITMAP hBmpMemory = CreateCompatibleBitmap(hdc, dx, dy);
		if (hBmpMemory)
		{
			HBITMAP hBmpOld = (HBITMAP)SelectObject(hMemDC, hBmpMemory);
			if (hBmpOld)
			{
				DrawC64String(hMemDC, 0, 0, str, length, bShifted, MULT, MULT);
				StretchBlt(hdc, x, y, fontheight * length, fontheight, hMemDC, 0, 0, dx, dy, SRCCOPY);
	
				SelectObject(hMemDC, hBmpOld);
			}
			DeleteObject(hBmpMemory);
		}
		DeleteDC(hMemDC);
	}
}

void CPRGBrowse::DrawC64String(HDC hdc, int x, int y, BYTE str[], int length, bool bShifted, int scalex, int scaley)
{
int i;
int row, col;
unsigned char c, gdata, petascii;
COLORREF bg, fg;
RGBQUAD rgb;

	memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicBLUE], 4);
	bg = PALETTERGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);

	memcpy(&rgb, &VIC6569::vic_color_array[VIC6569::vicLIGHT_BLUE], 4);
	fg = PALETTERGB(rgb.rgbRed, rgb.rgbGreen, rgb.rgbBlue);
	
	int x_advance = 8*scalex;
	for (i = 0; i < length; i++, x += x_advance)
	{
		petascii = str[i];	
		c = C64File::ConvertPetAsciiToScreenCode(petascii);

		for (row = 0; row < 8; row++)
		{
			if (bShifted)
				gdata = m_pCharGen[c * 8 + row + 2048];
			else
				gdata = m_pCharGen[c * 8 + row];
			for (col = 0; col < 8; col++, gdata <<= 1)
			{
				int py = y + row*scaley;
				for (int sy = 0 ; sy < scaley ; sy++, py++)
				{
					int px = x + col*scalex;
					for (int sx = 0 ; sx < scalex ; sx++, px++)
					{
						if (gdata & 0x80)
							SetPixel(hdc, px, py, fg);
						else
							SetPixel(hdc, px, py, bg);
					}
				}
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
	SendMessage(m_hListBox, LB_RESETCONTENT, 0, 0);
	if (mFileInspectorResult == S_OK && mFileInspectorStatus == COMPLETED)
	{
		j = SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT("C64 Header"));
		if (j >= 0)
		{
			SendMessage(m_hListBox, LB_SETITEMDATA, 0, 0);

			c = m_c64file.GetFileCount();
			miLoadedListItemCount = std::min(c, MAXLISTITEMCOUNT);
			for (i=0,k=0 ; i < miLoadedListItemCount ; i++)
			{
				j = SendMessage(m_hListBox, LB_ADDSTRING, 0, (LPARAM) TEXT("C64 File"));
				if (j < 0)
					break;
				SendMessage(m_hListBox, LB_SETITEMDATA, j, i);
				k++;
			}
			if (k>0)
				SendMessage(m_hListBox, LB_SETCURSEL, 0, 0);
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