#ifndef __WPANEL_H__
#define __WPANEL_H__

class WPanel : public CVirWindow
{
public:
	static const TCHAR ClassName[];
	static const TCHAR MenuName[];

	WPanel();
	virtual ~WPanel();

	static HRESULT RegisterClass(HINSTANCE hInstance);
	HRESULT Init();
	HWND Create(HINSTANCE hInstance, CVirWindow *pParentWindow, const TCHAR title[], int x,int y, int w, int h, HMENU ctrlID);
	HWND Show();
};

#endif