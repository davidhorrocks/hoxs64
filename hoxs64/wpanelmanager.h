#ifndef __WPANELMANAGER_H__
#define __WPANELMANAGER_H__

typedef MListElement<WPanel *> WpElement;
typedef MList<WPanel *> WpList;

class WPanelManager : IWPanelManager
{
public:
	WPanelManager();
	~WPanelManager();
	HRESULT Init(HINSTANCE hInstance, CVirWindow *pParentWindow, HWND hWndRebar);
	HRESULT CreateNewPanel(WPanel::InsertionStyle::EInsertionStyle style);
	void SizePanels(HWND hWnd, int x, int y, int w, int h);

	bool Splitter_OnLButtonDown(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Splitter_OnLButtonUp(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Splitter_OnMouseMove(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Splitter_OnSetCursor(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	//IWPanelManager
	virtual CVirWindow *Get_ParentWindow();
	virtual void OnDestroyWPanel(WPanel *pwp);
	virtual int Get_SizerGap();
	virtual void Get_RootRect(RECT *prc);
	//

	int GetMinWindowHeight();
	WPanel *GetPanelSizerFromClientPos(int x, int y, LPRECT prcSizerBar);
	void DrawXorBar(HDC hdc, int x1, int y1, int width, int height);
	void ClipPointToRect(const RECT &rc, POINT *pt);
	void ClipPointToValidPanelSizer(POINT *pt);
	WPanel *m_pPanelToSize;
	bool m_bIsRootRectValid;
	RECT m_rcRoot;
	RECT  m_rcSizer;
	int  m_y_offset;
	int  m_oldy;
	BOOL m_fMoved;
	BOOL m_fDragMode;
	int m_iSplitterPos;

	CVirWindow *m_pParentWindow;
	HWND m_hWndRebar;

	HINSTANCE m_hInstance;
	WpList m_WpList;
	CDPI m_dpi;
	int m_iSizerGap;
};

#endif