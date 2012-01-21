#ifndef __WPANEL_H__
#define __WPANEL_H__

class WPanel;
class WPanelManager;

class IWPanelManager
{
public:
	virtual CVirWindow *Get_ParentWindow() = 0;
	virtual void OnDestroyWPanel(WPanel *pwp) = 0;
	virtual int Get_SizerGap() = 0;
	virtual void Get_RootRect(RECT *prc) = 0;
};

class WPanel : public CVirWindow
{
public:
	virtual ~WPanel();
	class InsertionStyle
	{
		public:
		enum EInsertionStyle
		{
			Top,Right,Bottom,Left
		};
	};
	static const TCHAR ClassName[];
	static const TCHAR MenuName[];


	static HRESULT RegisterClass(HINSTANCE hInstance);
	HWND Show();

	void GetPreferredSize(SIZE *psz);
	void SetPreferredSize(const SIZE *psz);
	void GetCurrentSize(SIZE *psz);
	void GetParentRect(RECT *rcParent);
protected:
	IWPanelManager *m_pIWPanelManager;
	CVirWindow *m_pChildWin;

	WPanel();
	HRESULT Init(IWPanelManager *pIWPanelManager, CVirWindow *pChildWin);
	virtual HWND Create(HINSTANCE hInstance, HWND hWndParent, const TCHAR title[], int x,int y, int w, int h, HMENU hMenu);
	LRESULT OnSysCommand(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnHitTestNCA(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	friend WPanelManager;
private:
	void UpdateSizerRegion(const RECT& rcWindow);
	CDPI m_dpi;
	SIZE m_szPreferredSize;
	HRGN m_hrgSizerTop;
};

#endif