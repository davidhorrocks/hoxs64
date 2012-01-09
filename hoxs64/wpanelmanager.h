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

private:
	CVirWindow *m_pParentWindow;
	//HWND m_hWndMdiFrame;
	HWND m_hWndRebar;
	//HWND m_hWndMDIClient;

	HINSTANCE m_hInstance;
	WpList m_WpList;

	//IWPanelManager
	virtual CVirWindow *Get_ParentWindow();
	virtual void OnDestroyWPanel(WPanel *pwp);
};

#endif