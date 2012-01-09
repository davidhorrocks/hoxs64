#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include "defines.h"
#include "CDPI.h"
#include "bits.h"
#include "util.h"
#include "utils.h"
#include "assert.h"

#include "mlist.h"
#include "carray.h"
#include "wpanel.h"
#include "wpanelmanager.h"


WPanelManager::WPanelManager()
{
}

WPanelManager::~WPanelManager()
{
}

HRESULT WPanelManager::Init(HINSTANCE hInstance, CVirWindow *pParentWindow, HWND hWndRebar)
{
	//m_hWndMdiFrame = hWndMdiFrame;
	m_pParentWindow = pParentWindow;
	m_hWndRebar = hWndRebar;
	m_hInstance = hInstance;
	//m_hWndMDIClient = hWndMDIClient;
	return S_OK;
}

HRESULT WPanelManager::CreateNewPanel(WPanel::InsertionStyle::EInsertionStyle style)
{
	HWND hWndPanel = NULL;
	WPanel *pwp = new WPanel();
	HRESULT hr = pwp->Init(this);
	if (SUCCEEDED(hr))
	{
		HWND hWndPanel = pwp->Create(m_hInstance, TEXT("TOOL"), 0, 0, 200, 100, (HMENU) 1001);
		if (hWndPanel)
		{
			hr = this->m_WpList.Append(pwp);
			if (SUCCEEDED(hr))
			{
				return S_OK;
			}
		}
		else
		{
			hr = E_FAIL;
		}
	}

	if (hWndPanel)
	{
		DestroyWindow(hWndPanel);
	}
	else if (pwp)
	{
		delete pwp;
	}
	return hr;
}

void WPanelManager::SizePanels(HWND hWnd, int w, int h)
{
CDPI dpi;

BOOL br;
RECT rcAbsRebar;
int heightRebar = 0;
RECT rcWin;
RECT rcMdiClient;
	SetRect(&rcWin, 0, 0, w, h);
	if (m_hWndRebar != NULL)
	{
		br = GetWindowRect(m_hWndRebar, &rcAbsRebar);
		if (br == FALSE)
			return;
		heightRebar = rcAbsRebar.bottom - rcAbsRebar.top;

		SetWindowPos(m_hWndRebar, 0, 0, 0, w, heightRebar, SWP_NOREPOSITION | SWP_NOZORDER);

		SetRect(&rcWin, 0, heightRebar, w, h - heightRebar);
	}

	CopyRect(&rcMdiClient, &rcWin);

	int iMinHeightMdiClient = heightRebar;
	int iMinHeightPanel = heightRebar;
	RECT rcPanel;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = p->Next())
	{
		if (p->m_data != NULL)
		{
			WPanel *pwp = p->m_data;
			br = GetWindowRect(pwp->GetHwnd(), &rcPanel);
			if (br)
			{		
				int heightPanel = rcPanel.bottom - rcPanel.top;
				int widthPanel = rcPanel.right - rcPanel.left;
				int heightClientPart = rcWin.bottom - rcWin.top;
				int widthClientPart = rcWin.bottom - rcWin.top;
				if (heightPanel <= heightClientPart - iMinHeightMdiClient)
				{
					//Size mdi client window
					SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.bottom - heightPanel);
					SetRect(&rcPanel, rcWin.left, rcMdiClient.bottom, rcWin.right, rcWin.bottom);
				}
				else if (iMinHeightMdiClient + iMinHeightPanel <= heightClientPart)
				{
					//The panel's height is too tall so shrink it.
					heightPanel = heightClientPart - iMinHeightMdiClient;
					SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.top + iMinHeightMdiClient);
					SetRect(&rcPanel, rcWin.left, rcMdiClient.bottom, rcWin.right, rcWin.bottom);
				}
				else
				{
					heightPanel = iMinHeightPanel;
					SetRect(&rcMdiClient, rcWin.left, rcWin.top, rcWin.right, rcWin.top);
					SetRect(&rcPanel, rcWin.left, rcWin.top, rcWin.right, rcWin.bottom);
				}
				HWND hWndPanel = p->m_data->GetHwnd();
				if (hWndPanel)
				{
					if (rcPanel.right - rcPanel.left > 0 && rcPanel.bottom - rcPanel.top > 0)
						MoveWindow(hWndPanel, rcPanel.left, rcPanel.top, rcPanel.right - rcPanel.left, rcPanel.bottom - rcPanel.top, TRUE);
					else
						MoveWindow(hWndPanel, rcPanel.left, rcPanel.top, 0, 0, TRUE);
				}
			}
		}
	}

	if (m_pParentWindow)
	{
		HWND hWndMDIClient = m_pParentWindow->Get_MDIClientWindow();
		if (hWndMDIClient)
		{
			if (rcMdiClient.right - rcMdiClient.left > 0 && rcMdiClient.bottom - rcMdiClient.top > 0)
				MoveWindow(hWndMDIClient, rcMdiClient.left, rcMdiClient.top, rcMdiClient.right - rcMdiClient.left, rcMdiClient.bottom - rcMdiClient.top, TRUE);
			else
				MoveWindow(hWndMDIClient, rcMdiClient.left, rcMdiClient.top, 0, 0, TRUE);
		}
	}
	//if (m_pParentWindow)
	//{
	//	HWND hWndMDIClient = m_pParentWindow->Get_MDIClientWindow();
	//	if (hWndMDIClient)
	//	{
	//		if (rcWin.right - rcWin.left > 0 && rcWin.bottom - rcWin.top > 0)
	//			MoveWindow(hWndMDIClient, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, TRUE);
	//		else
	//			MoveWindow(hWndMDIClient, rcWin.left, rcWin.top, 0, 0, TRUE);
	//	}
	//}
}

CVirWindow *WPanelManager::Get_ParentWindow()
{
	return m_pParentWindow;
}

void WPanelManager::OnDestroyWPanel(WPanel *pwp)
{
	WpElement *n = NULL;
	for (WpElement *p = m_WpList.Head(); p!=NULL ; p = n)
	{
		n = p->Next();
		if (p->m_data!=NULL)
		{
			if (p->m_data == pwp)
			{
				m_WpList.Remove(p);
			}
		}
		
	}
}
