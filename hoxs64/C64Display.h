#pragma once
#include <vector>
#include <wrl.h>
#include <directxmath.h>
#include "GameObject2D.h"
#include "Texture.h"
#include "ConstantBuffer.h"
#include <string>
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "Vertex.h"
#include "C64ScreenTexture.h"
#include "Sprite.h"
#include "PositionOnlyGameObject2D.h"
#include "util.h"
#include "IC64.h"

struct VicCursorPosition
{
	VicCursorPosition(int VicCycle, int VicLine) noexcept;
	int VicCycle = 0;
	int VicLine = 0;
};

class C64Display : public GameObject2D
{
public:
	HRESULT Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexshader_2d>& cb_vs_vertexshader_2d, ID3D11SamplerState* pointSamplerState, ID3D11SamplerState* linearSamplerState, HWND hWnd, unsigned int width, unsigned int height, bool bWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter, IC64* c64, CAppStatus *appStatus);
	void Draw(const XMMATRIX& orthoMatrix); //2d camera orthogonal matrix
	void SetDisplaySize(bool windowedMode, unsigned int width, unsigned int height);
	HRESULT SetRenderStyle(unsigned int width, unsigned int height, bool isWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter);
	float GetWidth();
	float GetHeight();
	bool CanMode1X(UINT Width, UINT Height, const C64WindowDimensions& dims, BOOL bShowFloppyLed);
	bool CanMode2X(UINT Width, UINT Height, const C64WindowDimensions& dims, BOOL bShowFloppyLed);
	int GetToolBarHeight(bool bShowFloppyLed);
	int GetToolBarHeight(bool bShowFloppyLed, bool bWindowedMode, int displayHeight, int* pLedHeight, int* pLedSpacing);
	void Cleanup();

	bool isInitOK = false;
	C64ScreenTexture screenTexture;
	D3D11_RECT drcScreen = {};
	D3D11_RECT drcStatusBar = {};

	//The first C64 raster line to be displayed [0-311] at the top most edge of the display window.
	unsigned int displayFirstVicRaster = 0;

	//The last C64 raster line to be displayed [0-311] at the bottom most edge of the display window.
	unsigned int displayLastVicRaster = 0;

	//The width of the display in C64 pixels.
	unsigned int displayWidth = 0;

	//The height of the display in C64 pixels.
	unsigned int displayHeight = 0;

	//The first C64 X pixel position (zero based) that is at the left most edge of the display window.
	unsigned int displayStart = 0;

	void SetSamplerState(ID3D11SamplerState *pointSamplerState, ID3D11SamplerState* linearSamplerState);
	void ReleaseSamplerState();
	void SetPixelFilter(bool usePointFilter);
	void GetDisplayRectFromVicRect(const RECT& rcVicCycle, float& left, float& top, float& width, float& height);
	void DrawCursorAtVicPosition(const XMMATRIX& orthoMatrix, int cycle, int line);
	void EnableVicCursorSprites(bool enabled);
	void ClearVicCursorSprites();
	void InsertCursorSprite(VicCursorPosition&& viccursor);
private:
	void UpdateMatrix() override;
	void SetClearingRects(D3D11_RECT rects[], unsigned int count);
	void CalcClearingRects(int modeWidth, int modeHeight, const C64WindowDimensions& dims, const DWORD scale, bool bShowFloppyLed, RECT& rcTargetRect, std::vector<D3D11_RECT>& drcEraseRects, D3D11_RECT& drcStatusBar);
	void CalcStretchToFitClearingRects(int modeWidth, int modeHeight, const C64WindowDimensions& dims, bool bShowFloppyLed, RECT& rcTargetRect, std::vector<D3D11_RECT>& drcEraseRects, D3D11_RECT& drcStatusBar);
	bool IsValidRect(const D3D11_RECT& rc);

	HWND hWnd = 0;
	ConstantBuffer<CB_VS_vertexshader_2d>* cb_vs_vertexshader_2d = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	XMMATRIX worldMatrixTotalArea = XMMatrixIdentity();
	IndexBuffer indices;
	VertexBuffer<Vertex2D> vertices;
	PositionOnlyGameObject2D screenPosition;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pointSamplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSamplerState;
	Sprite spriteToolbarBackground;
	Sprite spriteLedMotorOn;
	Sprite spriteLedMotorOff;
	Sprite spriteLedDriveOn;
	Sprite spriteLedDriveOff;
	Sprite spriteLedWriteOn;
	Sprite spriteLedWriteOff;
	Sprite spriteVicCursorBar;
	vector<VicCursorPosition> listVicCursorPosition;

	IC64* c64 = nullptr;
	CAppStatus* appStatus = nullptr;
	std::vector<D3D11_RECT> drcEraseRects;
	bool bScreenOk = false;
	bool bStatusBarOk = false;
	unsigned int width = 0;
	unsigned int height = 0;
	HCFG::FULLSCREENSYNCMODE syncMode = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	bool isWindowedMode = true;
	HCFG::EMUBORDERSIZE borderSize = HCFG::EMUBORDERSIZE::EMUBORDER_TV;
	bool bShowFloppyLed = false;
	bool bUsePointFilter = false;
	bool enableVicCursorSprites = false;
	HCFG::EMUWINDOWSTRETCH stretch = HCFG::EMUWINDOWSTRETCH::EMUWINSTR_ASPECTSTRETCH;
	PALETTEENTRY paletteEntry[256] = {};
	static const int defaultToolbarHeight = 10;
	static const int defaultToolbarPadding = 1;
	static const int defaultLedSpacing = 8;
	static const int defaultLedHeight = 8;
	static const int screenDefaultHeight = 1080;
};