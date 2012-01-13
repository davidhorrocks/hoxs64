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
	void SizePanels(HWND hWnd, int w, int h);

	LRESULT Splitter_OnLButtonDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT Splitter_OnLButtonUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT Splitter_OnMouseMove(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	bool Splitter_OnSetCursor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	//IWPanelManager
	virtual CVirWindow *Get_ParentWindow();
	virtual void OnDestroyWPanel(WPanel *pwp);
	virtual int Get_SizerGap();

	void DrawXorBar(HDC hdc, int x1, int y1, int width, int height);


	int  oldy;
	BOOL fMoved;
	BOOL fDragMode;
	int nSplitterPos;

	CVirWindow *m_pParentWindow;
	HWND m_hWndRebar;

	HINSTANCE m_hInstance;
	WpList m_WpList;
	CDPI m_dpi;
	int m_iSizerGap;
};

#endif