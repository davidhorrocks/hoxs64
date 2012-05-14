#ifndef __DCHELPER_H__
#define __DCHELPER_H__

class DcHelper
{
public:
	DcHelper(HDC hdc);
	virtual ~DcHelper();

	HFONT UseFont(HFONT hFont);
	int UseMapMode(int mode);
	void Restore();
	HDC m_hdc;
private:
	DcHelper();
	void InitVars(HDC hdc);

	int iSavedDC;
};

#endif

