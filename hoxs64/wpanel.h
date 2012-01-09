#ifndef __WPANEL_H__
#define __WPANEL_H__

class WPanel;

class IWPanelManager
{
public:
	virtual CVirWindow *Get_ParentWindow() = 0;
	virtual void OnDestroyWPanel(WPanel *pwp) = 0;
};

class WPanel : public CVirWindow
{
public:
	WPanel();
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
	HRESULT Init(IWPanelManager *pIWPanelManager);
	HWND Create(HINSTANCE hInstance, const TCHAR title[], int x,int y, int w, int h, HMENU ctrlID);
	HWND Show();

	virtual LRESULT WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
protected:
	IWPanelManager *m_pIWPanelManager;
};

#endif