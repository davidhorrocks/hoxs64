#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

#include "dchelper.h"

DcHelper::DcHelper(HDC hdc)
{
	iSavedDC = 0;
	m_hdc = 0;
	InitVars(hdc);
}

void DcHelper::InitVars(HDC hdc)
{
	m_hdc = hdc;
	iSavedDC = ::SaveDC(hdc);
}

DcHelper::~DcHelper()
{
	Restore();
}

HFONT DcHelper::UseFont(HFONT hFont)
{
	if (m_hdc==NULL)
	{
		return NULL;
	}

	HFONT v = (HFONT)::SelectObject(m_hdc, hFont);
	return v;
}

int DcHelper::UseMapMode(int mode)
{
	if (m_hdc==NULL)
	{
		return NULL;
	}

	int v = SetMapMode(m_hdc, mode);
	return v;
}

void DcHelper::Restore()
{
	if (m_hdc != 0 && iSavedDC != 0)
	{
		::RestoreDC(m_hdc, iSavedDC);
		iSavedDC = 0;
	}
}

