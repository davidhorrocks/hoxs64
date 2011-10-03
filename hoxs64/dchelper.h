#ifndef __DCHELPER_H__
#define __DCHELPER_H__

class DcHelper
{
public:
	DcHelper();
	DcHelper(HDC hdc);
	virtual ~DcHelper();

	HFONT UseFont(HFONT hFont);
	int UseMapMode(int mode);
	void Restore();
	HDC m_hdc;
private:
	void InitVars(HDC hdc);

	bool m_bChangedMapMode;
	int m_prevMapMode;

	bool m_bChangedFont;
	HFONT m_prevFont;
};

#endif
