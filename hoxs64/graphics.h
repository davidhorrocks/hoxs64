#pragma once
#include "dx_version.h"
#include <DirectXMath.h>
#include <vector>
#include <wrl.h>
#include "graphicshelper.h"
#include "Shaders.h"
#include "ConstantBuffer.h"
#include "ConstantBufferTypes.h"
#include "Camera2D.h"
#include "C64Display.h"
#include "util.h"
#include "appstatus.h"
#include "CDPI.h"
#include "directoryviewer.h"

class Graphics
{
public:
	static const unsigned int ASSUMED_DPI_DEFAULT = 96;
	HRESULT Initialize(IC64* c64, IAppCommand* appCommand, CAppStatus* appStatus);
	void Cleanup() noexcept;
	void CleanShaders();
	void CleanScene();
	void CleanDevice();
	void CleanForModeChange(bool wantCleanDevice);
	HRESULT SetMode(bool useDefaultAdapter, UINT adapterNumber, UINT outputNumber, HWND hWnd, unsigned int fullScreenWidth, unsigned int fullScreenHeight, unsigned int refreshNumerator, unsigned int refreshDenominator, DXGI_MODE_SCANLINE_ORDER dxgiScanlineOrder, DXGI_MODE_SCALING dxgiScaling, bool bWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter, HCFG::FULLSCREENSYNCMODE syncMode);
	bool IsFullscreen();
	bool IsWantingFullscreen();
	bool IsWentFullscreen();
	HRESULT GoFullscreen();
	HRESULT GoWindowed();
	void FillAdapters(std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> &vAdapters);
	void FillOutputs(IDXGIAdapter1* adapter, std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>> &vOutputs);
	HRESULT GetCurrentBufferFormat(DXGI_FORMAT* pFormat);
	HRESULT RenderFrame();
	HRESULT PresentFrame();
	HRESULT TestPresent();
	void DrawGui();
	void DrawDemoGui();
	void SetBgColor(unsigned char r, unsigned char b, unsigned char g);
	void ClearFrame();
	HRESULT ResizeBuffers(unsigned int width, unsigned int height);
	void SetPixelFilter(bool usePointFilter);
	void SetVsync(bool useVsync);
	void SetVsyncMode(HCFG::FULLSCREENSYNCMODE syncMode);
	HRESULT LoadCbmCharTexture();
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapChain;
	C64Display c64display;
	bool isInitOK = false;
	bool isTearingSupported = false;
	IC64* c64;
	IAppCommand* appCommand;
	CAppStatus* appStatus;
private:
	HRESULT InitializeDirectX();
	HRESULT CreateRenderTargetDepthAndViewport(unsigned int width, unsigned int height);
	HRESULT InitializeShaders();
	HRESULT InitializeScene();
	void SetDefaultOverride();
	void CleanSwapChain();
	void CleanRenderTarget();
	void CleanCharRomTextureSet();
	void ImGuiStart();
	void ImGuiShutdown();
	void SetMenuKeepAliveHeightFromScreenHeight(int screenHeight);
	void GetTextureCoordFromScreenCode(unsigned int screencode, ImVec2& uv0, ImVec2& uv1) const;

	Microsoft::WRL::ComPtr<ID3D11Debug> dxdebug;
	Microsoft::WRL::ComPtr<IDXGIFactory1> dxgifactory1;
	Microsoft::WRL::ComPtr<IDXGIFactory2> dxgifactory2;
	Microsoft::WRL::ComPtr<IDXGIFactory5> dxgifactory5;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> blendState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSamplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> pointSamplerState;
	std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>> vOutputs;	
	Texture* cbmCharRom[2] = {};
	UINT otherFlags = 0;
	bool wantFullScreenBorderlessWindow = false;
	bool enableFullScreenBorderlessTearing = false;
	VertexShader vertex_shader_2d;
	PixelShader pixel_shader_2d;
	ConstantBuffer<CB_VS_vertexshader_2d> cb_vs_vertexshader_2d;
	ConstantBuffer<CB_VS_vertexshader> cb_vs_vertexshader;	
	Camera2D camera2D;
	bool useDefaultAdapter = false;
	UINT adapterNumber = 0;
	UINT outputNumber = 0;
	HWND hWnd = nullptr;
	bool useVsync = false;
	DXGI_MODE_DESC requestedMode;
	bool bWindowedMode = true;
	bool isWantingFullscreen = false;
	bool isWentFullscreen = false;
	bool isStartedImGuiDx11 = false;
	HCFG::EMUBORDERSIZE borderSize = HCFG::EMUBORDERSIZE::EMUBORDER_TV;
	bool bShowFloppyLed = false;
	HCFG::EMUWINDOWSTRETCH stretch;
	bool bUsePointFilter = false;
	HCFG::FULLSCREENSYNCMODE syncMode = HCFG::FULLSCREENSYNCMODE::FSSM_VBL;
	DXGI_SWAP_CHAIN_DESC assignedSwapChainDesc = {};
	UINT assignedWidth = 0;
	UINT assignedHeight = 0;
	UINT assignedUpperMouseKeepAlive = 0;
	FLOAT bgcolor[4];
	unsigned int currentDrawFrameCounter = 0;
	bool isValidlastMouse = false;
	unsigned int mouseSnapShotFrameCounter = 0;
	int lastMouseX = 0;
	int lastMouseY = 0;
	CDPI dpi;
	static const int KeepAliveTopPercentage = 10;
	static const int MenuShowTopPercentage = 50;
	int menukeepAliveHeight = 5;
	int menuShowHeight = 5;
	int mouseActiveDistance = 20;
	FileSys::DirectoryViewer directoryViewer;
};