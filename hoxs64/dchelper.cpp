#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>

#include "dchelper.h"

DcHelper::DcHelper()
{
	InitVars(NULL);
}

DcHelper::DcHelper(HDC hdc)
{
	InitVars(hdc);
}

void DcHelper::InitVars(HDC hdc)
{
	m_hdc = hdc;
	m_bChangedMapMode = false;
	m_bChangedFont = false;
	m_prevMapMode = 0;
	m_prevFont = NULL;
}

DcHelper::~DcHelper()
{
	Restore();
}

HFONT DcHelper::UseFont(HFONT hFont)
{
	if (m_hdc==NULL)
		return NULL;
	HFONT v = (HFONT)::SelectObject(m_hdc, hFont);
	if (v != NULL && !m_bChangedFont)
	{
		m_bChangedFont = true;
		m_prevFont = v;
	}
	return v;
}

int DcHelper::UseMapMode(int mode)
{
	if (m_hdc==NULL)
		return NULL;
	int v = SetMapMode(m_hdc, mode);
	if (v != 0 && !m_bChangedMapMode)
	{
		m_bChangedMapMode = true;
		m_prevMapMode = v;
	}
	return v;
}

void DcHelper::Restore()
{
	if (m_hdc==NULL)
		return;
	if (m_bChangedFont)
	{
		::SelectObject(m_hdc, m_prevFont);
	}
	if (m_bChangedMapMode)
	{
		::SetMapMode(m_hdc, m_prevMapMode);
	}
}

