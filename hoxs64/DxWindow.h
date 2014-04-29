#pragma once

class DxWindow;
typedef shared_ptr<DxWindow> Sp_DxWindow;

typedef std::list<Sp_DxWindow> ListDxWindow;
typedef std::list<Sp_DxWindow>::iterator ListDxWindowIter;

class DxObject
{
public:
	DxObject();
	~DxObject();
	virtual HRESULT SetDevice(IDirect3DDevice9 *pd3dDevice);
protected:
	virtual HRESULT LoadDeviceObjects();
	void FreeDeviceObjects();
	IDirect3DDevice9 *m_pd3dDevice;
};

class DxWindow : public DxObject
{
public:
	DxWindow();
	LONG xpos;
	LONG ypos;
	LONG width;
	LONG height;
	virtual void Draw() = 0;
	ListDxWindow Controls;

	virtual HRESULT SetFont(LPCTSTR fontname, INT fontsize);
	virtual HRESULT SetFont(LPD3DXFONT font);
	virtual LPD3DXFONT GetFont();
	virtual HRESULT SetText(LPCTSTR psztext);
	virtual HRESULT GetText(LPTSTR *ppsztext);
	virtual HRESULT GetTextRect(RECT *prc);
	virtual void OnLostDevice();
	virtual void OnResetDevice();
protected:
	virtual ~DxWindow();
	virtual HRESULT LoadDeviceObjects();
	void FreeDeviceObjects();
	LPD3DXFONT m_dxfont;
	LPD3DXSPRITE m_sprMessageText;
	LPTSTR m_psztext;
	bool m_bIsLost;
};

class DxLabel : public DxWindow
{
public:
	DxLabel();
	virtual ~DxLabel();
	virtual void Draw();
protected:
};

class DxTexture : public DxObject
{
public:
	DxTexture();
	virtual ~DxTexture();
	virtual void OnLostDevice();
	virtual void OnResetDevice();
	virtual void SetResourceId();
	void FreeDeviceObjects();
	virtual HRESULT LoadDeviceObjects();
	LPDIRECT3DTEXTURE9 m_ptx;
protected:
	int m_resourceid;
};