#ifndef __BREAKPOINTFRAME_H__
#define __BREAKPOINTFRAME_H__

class WpcBreakpoint : public CVirWindow
{
public:
	WpcBreakpoint();
	virtual ~WpcBreakpoint();
	static const TCHAR ClassName[];
	static HRESULT RegisterClass(HINSTANCE hInstance);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);

	HRESULT Init(Monitor *pMonitor);
protected:
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LRESULT OnCreate(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	class LvBreakColumnIndex
	{
	public:
		enum tagEnumBreakColumnIndex
		{
			Cpu = 0,
			Address = 1,
			Type = 2
		};
	};
	HWND m_hLvBreak;
	HRESULT InitListViewColumns(HWND hWndListView);
	HRESULT FillListView();
	Monitor *m_pMonitor;
};

#endif