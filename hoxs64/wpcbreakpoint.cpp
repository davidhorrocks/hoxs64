#include <windows.h>
#include "dx_version.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdarg.h>
#include <tchar.h>
#include <vector>
#include <list>
#include <assert.h>
#include "CDPI.h"
#include "utils.h"
#include "hexconv.h"
#include "IC64.h"

#include "wpanel.h"
#include "wpanelmanager.h"
#include "wpcbreakpoint.h"
#include "resource.h"

const TCHAR WpcBreakpoint::ClassName[] = TEXT("Hoxs64WpcBreakpoint");

WpcBreakpoint::WpcBreakpoint(IC64 *c64, IAppCommand *pAppCommand)
{
HRESULT hr;
	m_bSuppressThisBreakpointEvent = false;
	m_hLvBreak = NULL;
	m_hMenuBreakPoint = NULL;
	this->c64 = c64;
	this->m_pAppCommand = pAppCommand;
	hr = Init();
	if (FAILED(hr))
		throw std::runtime_error("WpcBreakpoint::Init() failed");
	m_bHexDisplay = true;
}

WpcBreakpoint::~WpcBreakpoint()
{
	if (m_hMenuBreakPoint)
	{
		DestroyMenu(m_hMenuBreakPoint);
		m_hMenuBreakPoint = NULL;
	}
}

HRESULT WpcBreakpoint::Init()
{
HSink hs;

	m_hMenuBreakPoint = LoadMenu(this->GetHinstance(), TEXT("MENU_BREAKPOINT"));
	if (!m_hMenuBreakPoint)
		return E_FAIL;

	hs = m_pAppCommand->EsBreakpointChanged.Advise(this);
	if (!hs)
		return E_FAIL;
	return S_OK;
}

HRESULT WpcBreakpoint::RegisterClass(HINSTANCE hInstance)
{
WNDCLASSEX  wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = CS_OWNDC;//; CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC)::WindowProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = sizeof(WPanel *);
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON_SMALL));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = NULL;
    wc.lpszClassName = ClassName;
	wc.hIconSm       = NULL;
	if (RegisterClassEx(&wc)==0)
		return E_FAIL;
	return S_OK;	
}

HWND WpcBreakpoint::Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu)
{
	return CVirWindow::CreateVirWindow(0L, ClassName, NULL, WS_CHILD | WS_VISIBLE, x, y, w, h, hWndParent, hMenu, hInstance);
}

LRESULT WpcBreakpoint::OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
HRESULT hr;
	CREATESTRUCT *pcs = (CREATESTRUCT *)lParam;
	if (pcs == NULL)
		return -1;
	m_hLvBreak = CreateListView(pcs, hWnd);
	if (!m_hLvBreak)
		return -1;
	hr = FillListView(m_hLvBreak);
	if (FAILED(hr))
		return -1;
	return 0;
}

HWND WpcBreakpoint::CreateListView(CREATESTRUCT *pcs, HWND hWndParent)
{
HRESULT hr;
RECT rcWin;
HWND hWnd = NULL;
HIMAGELIST hLarge = NULL;
HIMAGELIST hSmall = NULL;
HIMAGELIST hState = NULL;
bool ok = false;
	const ImageInfo lvImageList[] = 
	{
		{IDB_BREAK, 0},
		{IDB_BREAKDISABLE, 0, 0},
		{0, 0, IDI_CHIP1}
	};
	
	do
	{
		GetClientRect(hWndParent, &rcWin);
		hWnd = CreateWindowEx(0, WC_LISTVIEW, TEXT(""), WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_OWNERDATA, rcWin.left, rcWin.top, rcWin.right - rcWin.left, rcWin.bottom - rcWin.top, hWndParent, (HMENU)IDC_LVBREAKPOINT, pcs->hInstance, NULL);
		if (!hWnd)
			break;

		ListView_SetExtendedListViewStyleEx(hWnd, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);
		ListView_SetCallbackMask(hWnd, LVIS_STATEIMAGEMASK);
		hSmall = G::CreateImageListNormal(pcs->hInstance, hWndParent, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), lvImageList, _countof(lvImageList));
		if (!hSmall)
			break;
		hLarge = G::CreateImageListNormal(pcs->hInstance, hWndParent, GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), lvImageList, _countof(lvImageList));
		if (!hLarge)
			break;
		hState = G::CreateImageListNormal(pcs->hInstance, hWndParent, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), lvImageList, _countof(lvImageList));
		if (!hState)
			break;

		ListView_SetImageList(hWnd, hSmall, LVSIL_SMALL);
		ListView_SetImageList(hWnd, hLarge, LVSIL_NORMAL);
		ListView_SetImageList(hWnd, hState, LVSIL_STATE);
		hSmall = NULL;
		hLarge = NULL;
		hState = NULL;

		hr = InitListViewColumns(hWnd);
		if (FAILED(hr))
			break;
	
		ok = true;
	}  while (false);

	if (!ok)
	{
		hWnd = NULL;
	}
	if (hSmall)
	{
		ImageList_Destroy(hSmall);
		hSmall = NULL;
	}
	if (hLarge)
	{
		ImageList_Destroy(hLarge);
		hLarge = NULL;
	}
	if (hState)
	{
		ImageList_Destroy(hState);
		hState = NULL;
	}
	return hWnd;
}

int WpcBreakpoint::GetTextWidth(HWND hWnd, LPCTSTR szText, int fallbackWidthOnError)
{
SIZE size;
	HRESULT hr = G::GetTextSize(hWnd, szText, size);
	if (SUCCEEDED(hr))
		return size.cx;
	else
		return fallbackWidthOnError;
}

HRESULT WpcBreakpoint::InitListViewColumns(HWND hWndListView)
{
	int WidthPaddingItem = 7;
	int WidthPaddingSubItem = 14;
	int r;
	LVCOLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = (int)LvBreakColumnIndex::Cpu;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = TEXT("CPU");
	lvc.cx = G::CalcListViewMinWidth(hWndListView, lvc.pszText, sDisk, sC64, NULL) + WidthPaddingItem + m_dpi.ScaleX((16+8) *2);
	r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Cpu, &lvc);
	if (r == -1)
		return E_FAIL;

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = (int)LvBreakColumnIndex::Type;
	lvc.fmt = LVCFMT_LEFT;
	lvc.pszText = TEXT("Type");
	lvc.cx = G::CalcListViewMinWidth(hWndListView, lvc.pszText, sRasterCompare, NULL) + WidthPaddingSubItem;
	r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Type, &lvc);
	if (r == -1)
		return E_FAIL;

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = (int)LvBreakColumnIndex::Address;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.pszText = TEXT("Address");
	lvc.cx = G::CalcListViewMinWidth(hWndListView, lvc.pszText, TEXT("$CCCCC"), NULL) + WidthPaddingSubItem;
	r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Address, &lvc);
	if (r == -1)
		return E_FAIL;

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = (int)LvBreakColumnIndex::Line;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.pszText = TEXT("Line");
	lvc.cx = G::CalcListViewMinWidth(hWndListView, lvc.pszText, TEXT("$CCCC"), NULL) + WidthPaddingSubItem;
	r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Line, &lvc);
	if (r == -1)
		return E_FAIL;

	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.iSubItem = (int)LvBreakColumnIndex::Cycle;
	lvc.fmt = LVCFMT_RIGHT;
	lvc.pszText = TEXT("Cycle");
	lvc.cx = G::CalcListViewMinWidth(hWndListView, lvc.pszText, TEXT("$CCCCC"), NULL) + WidthPaddingSubItem;
	r = ListView_InsertColumn(hWndListView, (int)LvBreakColumnIndex::Cycle, &lvc);
	if (r == -1)
		return E_FAIL;
	return S_OK;
}

HRESULT WpcBreakpoint::FillListView(HWND hWndListView)
{
HRESULT hr = E_FAIL;
bool ok = false;
	try
	{
		m_lstBreak.clear();
	
		Sp_BreakpointItem v;
	
		IEnumBreakpointItem *pEnumBpMain = c64->GetMon()->BM_CreateEnumBreakpointItem();
		if (pEnumBpMain)
		{
			while (pEnumBpMain->GetNext(v))
			{
				m_lstBreak.push_back(v);
			}
			delete pEnumBpMain;
		}
		ListView_SetItemCount(hWndListView, m_lstBreak.size());
		ok = true;
	}
	catch(...)
	{
		ok = false;
	}
	return ok ? S_OK : E_FAIL;
}

LRESULT WpcBreakpoint::OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int w = (short)LOWORD(lParam);  // horizontal position of cursor 
	int h = (short)HIWORD(lParam);
	if (wParam == SIZE_MAXHIDE || wParam == SIZE_MAXSHOW)
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	MoveWindow(m_hLvBreak, 0, 0, w, h, TRUE);
	return 0;
}

HRESULT WpcBreakpoint::LvBreakPoint_RowCol_GetData(int iRow, Sp_BreakpointItem& bp)
{
	if ((iRow < 0) || ((unsigned int)iRow >= this->m_lstBreak.size()))
	{
		return E_FAIL;
	}
	bp = m_lstBreak[iRow];
	return S_OK;
}

LPCTSTR WpcBreakpoint::sVic = TEXT("VIC");
LPCTSTR WpcBreakpoint::sDisk = TEXT("Disk");
LPCTSTR WpcBreakpoint::sC64 = TEXT("C64");
LPCTSTR WpcBreakpoint::sExecute = TEXT("Execute");
LPCTSTR WpcBreakpoint::sRasterCompare = TEXT("Raster Compare");
LPCTSTR WpcBreakpoint::sLine = TEXT("Line");
LPCTSTR WpcBreakpoint::sCycle = TEXT("Cycle");
LPCTSTR WpcBreakpoint::sAddress = TEXT("Address");
HRESULT WpcBreakpoint::LvBreakPoint_RowCol_GetText(int iRow, int iCol, LPTSTR pText, int cch)
{
#define ADDRESS_DIGITS 4
HRESULT hr;

TCHAR sAddressHexBuf[ADDRESS_DIGITS + 1];
TCHAR sIntBuffer[20];

	if (cch <= 0)
		return E_FAIL;
	*pText = _T('\0');
	Sp_BreakpointItem bp;
	hr = LvBreakPoint_RowCol_GetData(iRow, bp);
	if (FAILED(hr))
		return hr;
	switch(iCol)
	{
	case LvBreakColumnIndex::Cpu:
		if (bp->machineident == DBGSYM::MachineIdent::MainCpu)
			_tcsncpy_s(pText, cch, sC64, _TRUNCATE);
		else if (bp->machineident == DBGSYM::MachineIdent::DiskCpu)
			_tcsncpy_s(pText, cch, sDisk, _TRUNCATE);
		else if (bp->machineident == DBGSYM::MachineIdent::Vic)
			_tcsncpy_s(pText, cch, sVic, _TRUNCATE);
		break;
	case LvBreakColumnIndex::Type:
		if (bp->bptype == DBGSYM::BreakpointType::Execute)
			_tcsncpy_s(pText, cch, sExecute, _TRUNCATE);
		else if (bp->bptype == DBGSYM::BreakpointType::VicRasterCompare)
			_tcsncpy_s(pText, cch, sRasterCompare, _TRUNCATE);
		break;
	case LvBreakColumnIndex::Address:
		if (bp->bptype == DBGSYM::BreakpointType::Execute)
		{
			if (m_bHexDisplay)
			{
				HexConv::long_to_hex(bp->address, sAddressHexBuf, ADDRESS_DIGITS); 
				_tcsncpy_s(pText, cch, TEXT("$"), _TRUNCATE);
				_tcsncat_s(pText, cch, sAddressHexBuf, _TRUNCATE);
			}
			else
			{
				_stprintf_s(sIntBuffer, _countof(sIntBuffer), TEXT("%d"), bp->address);
				_tcsncpy_s(pText, cch, sIntBuffer, _TRUNCATE);
			}
		}
		break;
	case LvBreakColumnIndex::Line:
		if (bp->bptype == DBGSYM::BreakpointType::VicRasterCompare)
		{
			if (m_bHexDisplay)
			{
				_stprintf_s(sIntBuffer, _countof(sIntBuffer), TEXT("$%03X"), bp->vic_line);
				_tcsncpy_s(pText, cch, sIntBuffer, _TRUNCATE);
			}
			else
			{
				_stprintf_s(sIntBuffer, _countof(sIntBuffer), TEXT("%d"), bp->vic_line);
				_tcsncpy_s(pText, cch, sIntBuffer, _TRUNCATE);
			}
		}
		break;
	case LvBreakColumnIndex::Cycle:
		if (bp->bptype == DBGSYM::BreakpointType::VicRasterCompare)
		{
			if (m_bHexDisplay)
			{
				_stprintf_s(sIntBuffer, _countof(sIntBuffer), TEXT("$%02X"), bp->vic_cycle);
				_tcsncpy_s(pText, cch, sIntBuffer, _TRUNCATE);
			}
			else
			{
				_stprintf_s(sIntBuffer, _countof(sIntBuffer), TEXT("%d"), bp->vic_cycle);
				_tcsncpy_s(pText, cch, sIntBuffer, _TRUNCATE);
			}
		}
		break;
	}
	return S_OK;
}

int WpcBreakpoint::LvBreakPoint_RowCol_State(int iRow, int iCol)
{
	Sp_BreakpointItem bp;
	HRESULT hr = LvBreakPoint_RowCol_GetData(iRow, bp);
	if (FAILED(hr))
		return 0;
	if (bp->enabled)
		return 1;
	else
		return 2;
}

bool WpcBreakpoint::LvBreakPoint_OnDispInfo(NMLVDISPINFO *pnmh, LRESULT &lresult)
{
	lresult = 0;
	LVITEM& item = pnmh->item;
	if (item.mask & LVIF_TEXT && item.pszText != LPSTR_TEXTCALLBACK)
	{
		LvBreakPoint_RowCol_GetText(item.iItem, item.iSubItem, item.pszText, item.cchTextMax);
	}
	if (item.mask & LVIF_IMAGE)
	{
		item.iImage = 2;
	}
	if (item.mask & LVIF_STATE)
	{
		int k = LvBreakPoint_RowCol_State(item.iItem, item.iSubItem);
		item.state = (item.mask & ~(LVIS_STATEIMAGEMASK)) | (INDEXTOSTATEIMAGEMASK(k & 0xf));
		item.stateMask |= LVIS_STATEIMAGEMASK;
	}
	return true;
}

bool WpcBreakpoint::LvBreakPoint_OnLClick(NMITEMACTIVATE *pnmh, LRESULT &lresult)
{
HRESULT hr;
lresult = 0;

	LVHITTESTINFO lvHitTestInfo;
	lvHitTestInfo.pt = pnmh->ptAction;
	int iRow = ListView_HitTest(this->m_hLvBreak, &lvHitTestInfo);
	if (iRow >= 0)
	{
		if ((lvHitTestInfo.flags & LVHT_ONITEMSTATEICON) != 0 && (lvHitTestInfo.flags & LVHT_ONITEMLABEL) == 0)
		{
			m_bSuppressThisBreakpointEvent = true;
			Sp_BreakpointItem bp;
			hr = LvBreakPoint_RowCol_GetData(pnmh->iItem, bp);
			if (SUCCEEDED(hr))
			{
				if (bp->enabled)
					this->DisableBreakpoint(bp);
				else
					this->EnableBreakpoint(bp);
			}
			m_bSuppressThisBreakpointEvent = false;
			RECT rcState;
			if (ListView_GetItemRect(m_hLvBreak, iRow, &rcState, LVIR_BOUNDS))
			{
				InvalidateRect(m_hLvBreak, &rcState, FALSE);
			}
			else
			{
				InvalidateRect(m_hLvBreak, NULL, FALSE);
			}

			UpdateWindow(m_hLvBreak);
		}
	}
	return true;
}

bool WpcBreakpoint::LvBreakPoint_OnRClick(NMITEMACTIVATE *pnmh, LRESULT &lresult)
{
RECT rcWin;
HMENU hMenu;

	GetWindowRect(m_hLvBreak, &rcWin);
	OffsetRect(&rcWin,  pnmh->ptAction.x, pnmh->ptAction.y);
	hMenu = GetSubMenu(m_hMenuBreakPoint, 0);
	if (hMenu)
	{
		if (m_bHexDisplay)
			CheckMenuItem (hMenu, IDM_BREAKPOINTOPTIONS_HEXADECIMAL, MF_BYCOMMAND | MF_CHECKED);
		else
			CheckMenuItem (hMenu, IDM_BREAKPOINTOPTIONS_HEXADECIMAL, MF_BYCOMMAND | MF_UNCHECKED);
		TrackPopupMenuEx(hMenu, TPM_LEFTALIGN, rcWin.left, rcWin.top, m_hWnd, NULL);
	}
	return false;
}

bool WpcBreakpoint::LvBreakPoint_OnKeyDown(NMLVKEYDOWN *pnmh, LRESULT &lresult)
{
SHORT nVirtKey;
lresult = 0;
const USHORT ISDOWN = 0x8000;
	switch (pnmh->wVKey)
	{
		case VK_DELETE:
			OnDeleteSelectedBreakpoint();
			return true;
		case 'P':
			nVirtKey = GetKeyState(VK_CONTROL); 
			if ((nVirtKey & ISDOWN))
			{
				OnEnableSelectedBreakpoint();
				return true;
			}
			break;
		case 'O':
			nVirtKey = GetKeyState(VK_CONTROL); 
			if ((nVirtKey & ISDOWN))
			{
				OnDisableSelectedBreakpoint();
				return true;
			}
			break;
	}
	return false;
}

bool WpcBreakpoint::OnNotify(HWND hWnd, int idCtrl, LPNMHDR pnmh, LRESULT &lresult)
{
	switch(pnmh->idFrom)
	{
	case IDC_LVBREAKPOINT:
		switch(pnmh->code)
		{
		case LVN_GETDISPINFO:
			if (pnmh->hwndFrom == this->m_hLvBreak)
			{
				return LvBreakPoint_OnDispInfo((NMLVDISPINFO *)pnmh, lresult);
			}
			break;
		case LVN_ODCACHEHINT:
			lresult = 0;
			return true;
		case LVN_ODFINDITEM:
			lresult = -1;
			return true;
		case NM_RCLICK:
			if (pnmh->hwndFrom == this->m_hLvBreak)
			{
				if (G::CachedCommonControlsVersion()  >= PACKVERSION(4,71))
				{
					return LvBreakPoint_OnRClick((NMITEMACTIVATE *)pnmh, lresult);
				}
			}
			break;
		case NM_CLICK: 
			if (pnmh->hwndFrom == this->m_hLvBreak)
			{
				if (G::CachedCommonControlsVersion()  >= PACKVERSION(4,71))
				{
					return LvBreakPoint_OnLClick((NMITEMACTIVATE *)pnmh, lresult);				
				}
			}
			break;
		case LVN_KEYDOWN:
			return LvBreakPoint_OnKeyDown((NMLVKEYDOWN *)pnmh, lresult);
		}
		break;
	}
	return false;
}

void WpcBreakpoint::OnBreakpointC64ExecuteChanged(void *sender, BreakpointC64ExecuteChangedEventArgs& e)
{
}

void WpcBreakpoint::OnBreakpointDiskExecuteChanged(void *sender, BreakpointDiskExecuteChangedEventArgs& e)
{
}

void WpcBreakpoint::OnBreakpointVicChanged(void *sender, BreakpointVicChangedEventArgs& e)
{
}

void WpcBreakpoint::OnBreakpointChanged(void *sender, BreakpointChangedEventArgs& e)
{
	if (m_bSuppressThisBreakpointEvent)
		return;
	FillListView(m_hLvBreak);
}

void WpcBreakpoint::OnShowAssembly()
{

	int i = ListView_GetNextItem(m_hLvBreak, -1, LVNI_SELECTED);
	if (i>=0)
	{
		Sp_BreakpointItem bp;
		if (SUCCEEDED(this->LvBreakPoint_RowCol_GetData(i, bp)))
		{
			switch(bp->machineident)
			{
			case DBGSYM::MachineIdent::MainCpu:
				this->m_pAppCommand->ShowCpuDisassembly(CPUID_MAIN, DBGSYM::SetDisassemblyAddress::EnsureAddressVisible, bp->address);
				break;
			case DBGSYM::MachineIdent::DiskCpu:
				this->m_pAppCommand->ShowCpuDisassembly(CPUID_DISK, DBGSYM::SetDisassemblyAddress::EnsureAddressVisible, bp->address);
				break;
			}
		}
	}
}

void WpcBreakpoint::DeleteBreakpoint(Sp_BreakpointKey bp)
{
	if(bp!=0)
	{
		this->c64->GetMon()->BM_DeleteBreakpoint(bp);
	}
}

void WpcBreakpoint::EnableBreakpoint(Sp_BreakpointKey bp)
{
	if(bp!=0)
	{
		this->c64->GetMon()->BM_EnableBreakpoint(bp);
	}
}

void WpcBreakpoint::DisableBreakpoint(Sp_BreakpointKey bp)
{
	if(bp!=0)
	{
		this->c64->GetMon()->BM_DisableBreakpoint(bp);
	}
}

void WpcBreakpoint::OnDeleteSelectedBreakpoint()
{
	try
	{
		int selcount = ListView_GetSelectedCount(m_hLvBreak);
		std::vector<Sp_BreakpointItem> vecDelete;
		vecDelete.reserve(selcount);
	
		for (int c = 0, i = -1 ; c==0 || (i>=0 && c<selcount) ; c++)
		{
			i = ListView_GetNextItem(m_hLvBreak, i, LVNI_SELECTED);
			if (i>=0)
			{
				Sp_BreakpointItem bp;
				if (SUCCEEDED(this->LvBreakPoint_RowCol_GetData(i, bp)))
				{
					vecDelete.push_back(bp);
				}
			}
		}
		for (std::vector<Sp_BreakpointItem>::iterator it = vecDelete.begin(); it != vecDelete.end(); it++)
		{
			c64->GetMon()->BM_DeleteBreakpoint(*it);
		}
	}
	catch(...)
	{
	}
}

void WpcBreakpoint::OnEnableSelectedBreakpoint()
{
	int selcount = ListView_GetSelectedCount(m_hLvBreak);
	for (int c = 0, i = -1 ; c==0 || (i>=0 && c<selcount) ; c++)
	{
		i = ListView_GetNextItem(m_hLvBreak, i, LVNI_SELECTED);
		if (i>=0)
		{
			Sp_BreakpointItem bp;
			if (SUCCEEDED(this->LvBreakPoint_RowCol_GetData(i, bp)))
			{
				c64->GetMon()->BM_EnableBreakpoint(bp);
			}
		}
	}
}

void WpcBreakpoint::OnDisableSelectedBreakpoint()
{
	int selcount = ListView_GetSelectedCount(m_hLvBreak);
	for (int c = 0, i = -1 ; c==0 || (i>=0 && c<selcount) ; c++)
	{
		i = ListView_GetNextItem(m_hLvBreak, i, LVNI_SELECTED);
		if (i>=0)
		{
			Sp_BreakpointItem bp;
			if (SUCCEEDED(this->LvBreakPoint_RowCol_GetData(i, bp)))
			{
				c64->GetMon()->BM_DisableBreakpoint(bp);
			}
		}
	}
}

void WpcBreakpoint::OnToggleHexadecimal()
{
	this->m_bHexDisplay = !this->m_bHexDisplay;
	int i = ListView_GetItemCount(m_hLvBreak);
	if (i > 0)
		ListView_RedrawItems (this->m_hLvBreak, 0, i - 1);
}

LRESULT WpcBreakpoint::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
LPNMHDR pnmh;
LRESULT lr;
bool bHandled;
int wmId, wmEvent;

	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(hWnd, uMsg, wParam, lParam);
	case WM_SIZE:
		return OnSize(hWnd, uMsg, wParam, lParam);
	case WM_NOTIFY:
		pnmh = (LPNMHDR)lParam;
		if (pnmh != NULL)
		{
			bHandled =  OnNotify(hWnd, (int)wParam, pnmh, lr);
			if (bHandled)
				return lr;
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam); // Remember, these are...
		wmEvent = HIWORD(wParam); // ...different for Win32!
		switch (wmId) 
		{
		case IDM_BREAKPOINTOPTIONS_SHOWASSEMBLY:
			OnShowAssembly();
			return 0;
		case IDM_BREAKPOINTOPTIONS_DELETEALLBREAKPOINTS:
			c64->GetMon()->BM_DeleteAllBreakpoints();
			return 0;
		case IDM_BREAKPOINTOPTIONS_DELETEBREAKPOINT:
			OnDeleteSelectedBreakpoint();
			return 0;
		case IDM_BREAKPOINTOPTIONS_ENABLEBREAKPOINT:
			OnEnableSelectedBreakpoint();
			return 0;
		case IDM_BREAKPOINTOPTIONS_DISABLEBREAKPOINT:
			OnDisableSelectedBreakpoint();
			return 0;
		case IDM_BREAKPOINTOPTIONS_HEXADECIMAL:
			OnToggleHexadecimal();
			return 0;
		}
		break;
	case WM_INITMENU:
		break;
	case WM_INITMENUPOPUP:
		break;
	}
	return DefWindowProc(m_hWnd, uMsg, wParam, lParam);;
}