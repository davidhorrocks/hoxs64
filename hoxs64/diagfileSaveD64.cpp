#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
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
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "mlist.h"
#include "carray.h"

#include "resource.h"
#include "diagfilesaved64.h"

UINT_PTR CALLBACK D64FileSaveBrowseDialogHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

CDiagFileSaveD64::CDiagFileSaveD64()
{
	hInstance = 0;
	SelectedNumberOfTracks = 35;

	hParent = 0;
	idStaticTracks = 0;
	mgapStaticTracksBottom = 0;
	mbGapsDone = false;
}

CDiagFileSaveD64::~CDiagFileSaveD64()
{
	CleanUp();
}

HRESULT CDiagFileSaveD64::Init(int selectedNumberOfTracks)
{
	SelectedNumberOfTracks = selectedNumberOfTracks;
	return S_OK;
}

BOOL CDiagFileSaveD64::Open(HINSTANCE hInstance, OPENFILENAME *pOF)
{
BOOL r;
	CDiagFileSaveD64::hInstance = hInstance;
	pOF->hInstance = hInstance;
	pOF->Flags = pOF->Flags | OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_ENABLESIZING;
	pOF->lpTemplateName = MAKEINTRESOURCE(IDD_SAVED64);
	pOF->lCustData = (LPARAM) this;
	pOF->lpfnHook = D64FileSaveBrowseDialogHookProc;
	r = GetSaveFileName(pOF);
	return r;
}

UINT CDiagFileSaveD64::ChildDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
NMHDR *mh;
int len=0;
LPOFNOTIFY lpOfNotify;
WORD w,h;
RECT rcDlg;
RECT rcStaticTracks;
RECT rcCheckTracks35;
RECT rcCheckTracks40;

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
				if (hDlg !=0 && hStaticTracks!=0 && hCheckTracks35!=0 && hCheckTracks40!=0)
				{
					if (GetWindowRect(hStaticTracks, &rcStaticTracks) != 0 && GetWindowRect(hCheckTracks35, &rcCheckTracks35) != 0 && GetWindowRect(hCheckTracks40, &rcCheckTracks40) != 0)
					{
						ScreenToClient(hDlg, (POINT*)&rcStaticTracks.left);
						ScreenToClient(hDlg, (POINT*)&rcStaticTracks.right);
						ScreenToClient(hDlg, (POINT*)&rcCheckTracks35.left);
						ScreenToClient(hDlg, (POINT*)&rcCheckTracks35.right);
						ScreenToClient(hDlg, (POINT*)&rcCheckTracks40.left);
						ScreenToClient(hDlg, (POINT*)&rcCheckTracks40.right);

						GetClientRect(hDlg, &rcDlg);
						mgapStaticTracksBottom = rcDlg.bottom - rcStaticTracks.bottom;
						mgapCheckTracks35Bottom = rcDlg.bottom - rcCheckTracks35.bottom;
						mgapCheckTracks40Bottom = rcDlg.bottom - rcCheckTracks40.bottom;
						mbGapsDone = true;							
						ReSize(hDlg, rcDlg.right- rcDlg.left, rcDlg.bottom - rcDlg.top);
					}
				}
				break;
			case CDN_SELCHANGE:
				break;
			case CDN_FILEOK:
				SelectedNumberOfTracks = (IsDlgButtonChecked(hDlg, IDC_RAD_TRACKS40) != BST_UNCHECKED) ? 40 : 35;
				lpOfNotify = (LPOFNOTIFY) lParam;
				return FALSE;// FALSE indicates to accept the file
			}
		}
		break;
	case WM_SIZE:
		w = LOWORD(lParam);
		h = HIWORD(lParam);
		ReSize(hDlg, w, h);
		return TRUE;
	default:
		return FALSE;

	}
	return FALSE;
}

void CDiagFileSaveD64::CreateControls(HWND hDlg)
{
	hStaticTracks = GetDlgItem(hDlg, IDC_STA_TRACKS);
	hCheckTracks35 = GetDlgItem(hDlg, IDC_RAD_TRACKS35);
	hCheckTracks40 = GetDlgItem(hDlg, IDC_RAD_TRACKS40);
	
	if (SelectedNumberOfTracks == 35)
	{
		CheckDlgButton(hDlg, IDC_RAD_TRACKS35, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hDlg, IDC_RAD_TRACKS40, BST_CHECKED);
	}
}


void CDiagFileSaveD64::ReSize(HWND hDlg, LONG w, LONG h)
{
/*
RECT rcStaticTracks;
RECT rcCheckTracks35;
RECT rcCheckTracks40;
RECT rcDlg;
	if (hDlg !=0 && hStaticTracks!=0 && hCheckTracks35!=0 && hCheckTracks40!=0 && mbGapsDone)
	{
		if (GetWindowRect(hStaticTracks, &rcStaticTracks)!=0 && GetWindowRect(hCheckTracks35, &rcCheckTracks35)!=0 && GetWindowRect(hCheckTracks40, &rcCheckTracks40)!=0)
		{
			ScreenToClient(hDlg, (POINT*)&rcStaticTracks.left);
			ScreenToClient(hDlg, (POINT*)&rcStaticTracks.right);
			ScreenToClient(hDlg, (POINT*)&rcCheckTracks35.left);
			ScreenToClient(hDlg, (POINT*)&rcCheckTracks35.right);
			ScreenToClient(hDlg, (POINT*)&rcCheckTracks40.left);
			ScreenToClient(hDlg, (POINT*)&rcCheckTracks40.right);
			GetClientRect(hDlg, &rcDlg);
			//w = LOWORD(lParam);
			//h = HIWORD(lParam);
			w = rcDlg.right - rcDlg.left;
			h = rcDlg.bottom - rcDlg.top;
			if (w>0 && h >0)
			{
				InvalidateRect(hDlg, &rcStaticTracks, TRUE);
				InvalidateRect(hDlg, &rcCheckTracks35, TRUE);
				InvalidateRect(hDlg, &rcCheckTracks40, TRUE);
				
				MoveWindow(hListBox, rcListBox.left, rcListBox.top, (rcListBox.right - rcListBox.left), h - mgapListBoxBottom - rcListBox.top, TRUE);
				MoveWindow(hCheckQuickLoad, rcListBox.left, h - mgapCheckBoxQuickLoadBottom - (rcCheckBoxQuickLoad.bottom-rcCheckBoxQuickLoad.top), (rcCheckBoxQuickLoad.right - rcCheckBoxQuickLoad.left), (rcCheckBoxQuickLoad.bottom - rcCheckBoxQuickLoad.top), TRUE);
				MoveWindow(hCheckAlignD64Tracks, rcListBox.left, h - mgapCheckBoxAlignD64TracksBottom - (rcCheckBoxAlignD64Tracks.bottom-rcCheckBoxAlignD64Tracks.top), (rcCheckBoxAlignD64Tracks.right - rcCheckBoxAlignD64Tracks.left), (rcCheckBoxAlignD64Tracks.bottom - rcCheckBoxAlignD64Tracks.top), TRUE);

				UpdateWindow(hDlg);
			}
		}
	}
	*/
}

void CDiagFileSaveD64::CleanUp()
{
}

UINT_PTR CALLBACK D64FileSaveBrowseDialogHookProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
#pragma warning(disable:4244)
OPENFILENAME *pOF=0;
CDiagFileSaveD64 *childDialog=0L;

	switch (msg) 
	{ 
	case WM_INITDIALOG:
		SetWindowLongPtr(hDlg, DWLP_USER, lParam);
		pOF = (OPENFILENAME *) lParam;
		break;
	default:
		pOF = (OPENFILENAME *) (LONG_PTR)GetWindowLongPtr(hDlg, DWLP_USER);
	}
	if (pOF)
		childDialog = (CDiagFileSaveD64 *)pOF->lCustData;
	if (childDialog)
		return childDialog->ChildDialogProc(hDlg, msg, wParam, lParam);
	else
		return FALSE;
#pragma warning(default:4244)
}
