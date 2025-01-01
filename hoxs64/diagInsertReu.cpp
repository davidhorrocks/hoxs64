#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winuser.h>
#include <string.h>
#include "dx_version.h"
#include "defines.h"
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
#include "diagInsertReu.h"
#include "cart.h"
#include "wfs.h"
#include "resource.h"

void CDiagInsertReu::Init(bool reu_use_image_file, std::wstring reu_image_filename, unsigned int reu_extraAddressBits)
{
	ClearError();
	if (reu_extraAddressBits > CartReu1750::MaxExtraBits)
	{
		reu_extraAddressBits = CartReu1750::MaxExtraBits;
	}

	this->m_reu_use_image_file = reu_use_image_file;
	this->m_reu_image_filename = reu_image_filename;
	this->m_reu_extraAddressBits = reu_extraAddressBits;
}

BOOL CDiagInsertReu::DialogProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	int wmId, wmEvent;
	wmId = LOWORD(wParam); // Remember, these are...
	wmEvent = HIWORD(wParam); // ...different for Win32!

	switch (message)
	{
	case WM_INITDIALOG:
		G::ArrangeOKCancel(hwndDlg);
		SetDlgItemText(hwndDlg, IDC_TXT_REU_FILENAME, m_reu_image_filename.c_str());
		if (m_reu_use_image_file)
		{
			CheckDlgButton(hwndDlg, IDC_REU_USE_CUSTOM_FILE, TRUE);
		}
		else
		{
			CheckDlgButton(hwndDlg, IDC_REU_USE_CUSTOM_FILE, FALSE);
		}

		UpdateReuSize(hwndDlg, m_reu_extraAddressBits);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_BROWSE_REU_FILE:
			BrowseReuFile(hwndDlg);
			break;
		case IDOK:
			ReadFileName(hwndDlg);
			ReadReuSize(hwndDlg);
			ReadReuUseCustomFile(hwndDlg);
			m_ok = true;
			EndDialog(hwndDlg, wParam);
			return TRUE;
		case IDCANCEL:
			EndDialog(hwndDlg, wParam);
			return TRUE;
		default:
			break;
		}

		return FALSE;
	default:
		break;
	}

	return FALSE;
}

void CDiagInsertReu::ReadReuSize(HWND hWnd)
{
	if (IsDlgButtonChecked(hWnd, IDC_RAD_REU_512K))
	{
		m_reu_extraAddressBits = 0;
	}
	else if (IsDlgButtonChecked(hWnd, IDC_RAD_REU_16M))
	{
		m_reu_extraAddressBits = CartReu1750::MaxExtraBits;
	}
	else
	{
		m_reu_extraAddressBits = 0;
	}
}

void CDiagInsertReu::ReadFileName(HWND hWnd)
{
	m_ok = false;
	try
	{
		std::shared_ptr<TCHAR[]> buffer;
		std::wstring wsfilename;
		wsfilename.clear();
		LRESULT lr = SendDlgItemMessageW(hWnd, IDC_TXT_REU_FILENAME, WM_GETTEXTLENGTH, 0, 0);
		if (lr >= 0 && lr <= G::MaxApplicationPathLength)
		{
			int size = (int)lr + 1;
			if (lr != 0)
			{
				buffer = std::shared_ptr<WCHAR[]>(new WCHAR[size]);
				lr = GetDlgItemTextW(hWnd, IDC_TXT_REU_FILENAME, buffer.get(), size);
				if (lr < 0 || lr >= (LRESULT)size)
				{
					buffer.get()[0] = TEXT('\0');
				}

				wsfilename.append(buffer.get());
			}
		}

		this->m_reu_image_filename.assign(wsfilename);
	}
	catch (std::exception e)
	{
		ErrorLogger::Log(hWnd, e.what());
	}
}

void CDiagInsertReu::UpdateReuSize(HWND hWnd, unsigned int extraAddressBits)
{
	if (extraAddressBits == 0)
	{
		CheckRadioButton(hWnd, IDC_RAD_REU_512K, IDC_RAD_REU_16M, IDC_RAD_REU_512K);
	}
	else
	{
		CheckRadioButton(hWnd, IDC_RAD_REU_512K, IDC_RAD_REU_16M, IDC_RAD_REU_16M);
	}
}

void CDiagInsertReu::UpdateReuUseCustomFile(HWND hWnd, bool use_image_file)
{
	if (use_image_file)
	{
		CheckDlgButton(hWnd, IDC_REU_USE_CUSTOM_FILE, BST_CHECKED);
	}
	else
	{
		CheckDlgButton(hWnd, IDC_REU_USE_CUSTOM_FILE, BST_UNCHECKED);
	}
}

void CDiagInsertReu::ReadReuUseCustomFile(HWND hWnd)
{
	if (IsDlgButtonChecked(hWnd, IDC_REU_USE_CUSTOM_FILE))
	{
		this->m_reu_use_image_file = true;
	}
	else
	{
		this->m_reu_use_image_file = false;
	}	
}

void CDiagInsertReu::BrowseReuFile(HWND hWnd)
{
	OPENFILENAME of;
	std::shared_ptr<TCHAR[]> spFilename(new TCHAR[MAX_SIZE_FILE_PATH + 1]);
	BOOL b;

	spFilename[0] = 0;
	G::InitOfn(of,
		hWnd,
		TEXT("Choose a REU image file"),
		spFilename.get(),
		MAX_SIZE_FILE_PATH,
		TEXT("REU image file (*.reu)\0*.reu\0All files (*.*)\0*.*\0\0"),
		NULL,
		0);

	of.nFilterIndex = 2;
	b = GetOpenFileName((LPOPENFILENAME)&of);
	if (b)
	{
		m_reu_image_filename.assign(&spFilename[0]);
		G::Trim(m_reu_image_filename);
		SetDlgItemText(hWnd, IDC_TXT_REU_FILENAME, m_reu_image_filename.c_str());
		if (!G::IsStringBlank(m_reu_image_filename))
		{
			HANDLE hFile = nullptr;
			try
			{
				hFile = CreateFile(Wfs::EnsureLongNamePrefix(m_reu_image_filename).c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					unsigned __int64 filesize;
					if (G::TryGetFileSize(hFile, filesize))
					{
						if (filesize >= CartReu1750::MaxRAMSize)
						{
							UpdateReuSize(hWnd, CartReu1750::MaxExtraBits);
						}
						else if (filesize > CartReu1750::DefaultRAMSize)
						{
							UpdateReuSize(hWnd, CartReu1750::MaxExtraBits);
						}
					}
				}
			}
			catch (...)
			{

			}

			if (hFile != NULL && hFile != INVALID_HANDLE_VALUE)
			{
				CloseHandle(hFile);
			}
		}
	}
}