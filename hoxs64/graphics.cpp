#include <windows.h>
#include <assert.h>
#include "wfs.h"
#include <tchar.h>
#include <versionhelpers.h>
#include "graphics.h"
#include "ErrorLogger.h"
#include <sstream>
#include "directoryviewer.h"

Graphics::Graphics() noexcept
{
	for (int i = 0; i < _countof(cbmCharRom); i++)
	{
		if (cbmCharRom[i] != nullptr)
		{
			cbmCharRom[i] = nullptr;
		}
	}
}

HRESULT Graphics::Initialize(IC64 *c64, IAppCommand* appCommand, CAppStatus* appStatus)
{
	HRESULT hr;
	isInitOK = false;
	isStartedImGuiDx11 = false;
	currentDrawFrameCounter = 0;
	isValidlastMouse = false;
	mouseSnapShotFrameCounter = 0;
	lastMouseX = 0;
	lastMouseY = 0;
	bWindowedMode = false;
	isWantingFullscreen = false;
	this->c64 = c64;
	this->appCommand = appCommand;
	this->appStatus = appStatus;
	Cleanup();
	isTearingSupported = false;
	useVsync = true;
	this->otherFlags = DXGI_SWAP_CHAIN_FLAG::DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	ZeroMemory(&assignedSwapChainDesc, sizeof(assignedSwapChainDesc));
	this->assignedWidth = 0;
	this->assignedHeight = 0;
	SetBgColor(0, 0, 0);
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)dxgifactory2.ReleaseAndGetAddressOf());
	if (SUCCEEDED(hr))
	{
		dxgifactory1 = dxgifactory2;
	}
	else
	{
		dxgifactory2.Reset();
		hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)dxgifactory1.ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "CreateDXGIFactory1 failed to create DXGI factory version 1.1.");
			return hr;
		}
	}

	dxgifactory1->QueryInterface(__uuidof(IDXGIFactory5), (void**)dxgifactory5.ReleaseAndGetAddressOf());
	return hr;
}

bool Graphics::IsFullscreen()
{
	if (swapChain == nullptr)
	{
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIOutput> outputExistingFullscreen;
	BOOL isCurrentlyFullscreen = FALSE;
	HRESULT hr = this->swapChain->GetFullscreenState(&isCurrentlyFullscreen, outputExistingFullscreen.GetAddressOf());
	if (FAILED(hr))
	{
		return this->isWentFullscreen;
	}

	bool isFullScreen = isCurrentlyFullscreen != FALSE;
	appStatus->m_bWindowed = !isFullScreen;
	return isFullScreen;
}

bool Graphics::IsWantingFullscreen()
{
	return isWantingFullscreen;
}

bool Graphics::IsWentFullscreen()
{
	return isWentFullscreen;
}

HRESULT Graphics::GoFullscreen()
{
	HRESULT hr;
	if (!this->isInitOK)
	{
		return E_FAIL;
	}

	if (swapChain == nullptr)
	{
		return E_FAIL;
	}

	Microsoft::WRL::ComPtr<IDXGIOutput> outputExistingFullscreen;
	BOOL isCurrentlyFullscreen = FALSE;
	hr = this->swapChain->GetFullscreenState(&isCurrentlyFullscreen, outputExistingFullscreen.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	isWentFullscreen = isCurrentlyFullscreen != FALSE;
	appStatus->m_bWindowed = !isWentFullscreen;
	if (!isCurrentlyFullscreen)
	{
		// Going fullscreen.
		Microsoft::WRL::ComPtr<IDXGIOutput> currentOutput;
		if (useDefaultAdapter)
		{
			if (this->outputNumber >= vOutputs.size())
			{
				return E_FAIL;
			}

			currentOutput = vOutputs[outputNumber];
		}
		else
		{
			hr = swapChain->GetContainingOutput(currentOutput.GetAddressOf());
			if (FAILED(hr))
			{
				return hr;
			}
		}

		DXGI_MODE_DESC selectedFullScreenModeDesc = {};
		hr = GraphicsHelper::GetPreferredMode(device.Get(), currentOutput.Get(), &requestedMode, &selectedFullScreenModeDesc);
		if (FAILED(hr))
		{
			return hr;
		}

		isWantingFullscreen = true;
		DXGI_MODE_DESC mode = {};
		mode.Width = selectedFullScreenModeDesc.Width;
		mode.Height = selectedFullScreenModeDesc.Height;
		mode.RefreshRate = selectedFullScreenModeDesc.RefreshRate;
		mode.Format = selectedFullScreenModeDesc.Format;
		mode.ScanlineOrdering = selectedFullScreenModeDesc.ScanlineOrdering;
		mode.Scaling = selectedFullScreenModeDesc.Scaling;
		hr = swapChain->ResizeTarget(&mode);
		if (FAILED(hr))
		{
			isWantingFullscreen = false;
			return hr;
		}

		hr = swapChain->SetFullscreenState(TRUE, currentOutput.Get());
		if (SUCCEEDED(hr))
		{
			isWentFullscreen = true;
			appStatus->m_bWindowed = false;
		}
		else
		{
			isWantingFullscreen = false;
		}

	}
	else
	{
		hr = S_OK;
	}

	SetVsyncMode(appStatus->m_bWindowed ? appStatus->m_syncModeWindowed : appStatus->m_syncModeFullscreen);
	return hr;
}

HRESULT Graphics::GoWindowed()
{
	HRESULT hr;
	if (!this->isInitOK)
	{
		return E_FAIL;
	}

	Microsoft::WRL::ComPtr<IDXGIOutput> outputExistingFullscreen;
	BOOL isCurrentlyFullscreen = FALSE;
	hr = this->swapChain->GetFullscreenState(&isCurrentlyFullscreen, outputExistingFullscreen.GetAddressOf());
	if (FAILED(hr))
	{
		return hr;
	}

	isWantingFullscreen = false;
	isWentFullscreen = isCurrentlyFullscreen != FALSE;
	appStatus->m_bWindowed = !isWentFullscreen;
	if (isCurrentlyFullscreen)
	{
		hr = swapChain->SetFullscreenState(FALSE, NULL);
		if (SUCCEEDED(hr))
		{
			isWantingFullscreen = false;
			isWentFullscreen = false;
			appStatus->m_bWindowed = true;
		}
	}

	SetVsyncMode(appStatus->m_bWindowed ? appStatus->m_syncModeWindowed : appStatus->m_syncModeFullscreen);
	return hr;
}

HRESULT Graphics::SetMode(bool useDefaultAdapter, UINT adapterNumber, UINT outputNumber, HWND hWnd, unsigned int width, unsigned int height, unsigned int refreshNumerator, unsigned int refreshDenominator, DXGI_MODE_SCANLINE_ORDER dxgiScanlineOrder, DXGI_MODE_SCALING dxgiScaling, bool bWindowedMode, HCFG::EMUBORDERSIZE borderSize, bool bShowFloppyLed, HCFG::EMUWINDOWSTRETCH stretch, bool bUsePointFilter, HCFG::FULLSCREENSYNCMODE syncMode)
{
	HRESULT hr;
	isWantingFullscreen = !bWindowedMode;
	bool wantCleanDevice = true;
	if (this->isInitOK && useDefaultAdapter == this->useDefaultAdapter && adapterNumber == this->adapterNumber && outputNumber == this->outputNumber)
	{
		// Changes to the same adapterNumber and outputNumber should work with any existing device and device context;
		wantCleanDevice = false;
	}

	CleanForModeChange(wantCleanDevice);
	requestedMode = {};
	if (useDefaultAdapter)
	{
		requestedMode.Width = 0;
		requestedMode.Height = 0;
		requestedMode.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
		requestedMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		requestedMode.RefreshRate.Numerator = 0;
		requestedMode.RefreshRate.Denominator = 0;
	}
	else
	{
		requestedMode.Width = width;
		requestedMode.Height = height;
		requestedMode.Scaling = dxgiScaling;
		requestedMode.ScanlineOrdering = dxgiScanlineOrder;
		requestedMode.RefreshRate.Numerator = refreshNumerator;
		requestedMode.RefreshRate.Denominator = refreshDenominator;
	}

	requestedMode.Format = GraphicsHelper::DefaultPixelFormat;
	this->useDefaultAdapter = useDefaultAdapter;
	this->adapterNumber = adapterNumber;
	this->outputNumber = outputNumber;
	this->hWnd = hWnd;
	this->bWindowedMode = bWindowedMode;
	this->syncMode = syncMode;
	this->bUsePointFilter = bUsePointFilter;
	this->borderSize = borderSize;
	this->bShowFloppyLed = bShowFloppyLed;
	this->stretch = stretch;
	SetVsyncMode(syncMode);
	do
	{
		hr = InitializeDirectX();
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed InitializeDirectX");
			break;
		}

		hr = InitializeShaders();
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed InitializeShaders");
			break;
		}

		hr = InitializeScene();
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed InitializeShaders");
			break;
		}
	} while (false);

	if (SUCCEEDED(hr))
	{
		ImGuiStart();
		isInitOK = true;
		hr = S_OK; 
	}

	return hr;
}

void Graphics::CleanForModeChange(bool wantCleanDevice)
{
	if (wantCleanDevice)
	{
		CleanDevice();
	}
	else
	{
		CleanSwapChain();
	}
}

void Graphics::Cleanup() noexcept
{
	try
	{
		CleanDevice();
		dxgifactory5.Reset();
		dxgifactory2.Reset();
		dxgifactory1.Reset();
	}
	catch(...)
	{

	}
}

void Graphics::FillAdapters(std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>>& vAdapters)
{
	IDXGIFactory1 *pDxgifac1;
	if (dxgifactory1 == nullptr)
	{
		return;
	}

	if (dxgifactory2 != nullptr)
	{
		pDxgifac1 = dxgifactory2.Get();
	}
	else
	{
		pDxgifac1 = dxgifactory1.Get();
	}

	UINT i = 0;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;	
	while (pDxgifac1->EnumAdapters1(i, adapter.GetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vAdapters.push_back(adapter);
		adapter.Detach();
		++i;
		if (i > 1000)
		{
			break;
		}
	}
}

void Graphics::FillOutputs(IDXGIAdapter1 *adapter, std::vector<Microsoft::WRL::ComPtr<IDXGIOutput>> &vOutputs)
{
	UINT i = 0;
	Microsoft::WRL::ComPtr<IDXGIOutput> output;
	while (adapter->EnumOutputs(i, output.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
	{
		vOutputs.push_back(output);
		++i;
		if (i > 1000)
		{
			break;
		}
	}
}

void Graphics::SetDefaultOverride()
{
	adapterNumber = 0;
	outputNumber = 0;
	requestedMode.Width = 0;
	requestedMode.Height = 0;
	requestedMode.RefreshRate.Numerator= 0;
	requestedMode.RefreshRate.Denominator = 1;
	requestedMode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER::DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	requestedMode.Scaling = DXGI_MODE_SCALING::DXGI_MODE_SCALING_UNSPECIFIED;
	bWindowedMode = true;
}

HRESULT Graphics::InitializeDirectX()
{
	HRESULT hr = S_OK;
	char *pMsgBuffer = nullptr;
	std::string message;
	try
	{
		HMONITOR hmonitor = 0;
		if (useDefaultAdapter)
		{
			hmonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
			if (hmonitor == nullptr)
			{
				hr = E_FAIL;
				COM_ERROR_IF_FAILED(hr, "MonitorFromWindow returned NULL.");
			}

			MONITORINFOEXW monitorInfo = {};
			monitorInfo.cbSize = sizeof(monitorInfo);
			if (GetMonitorInfoW(hmonitor, &monitorInfo))
			{
				bool done = false;
				Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
				for (int adapterIndex = 0; !done; adapterIndex++)
				{
					hr = dxgifactory1->EnumAdapters1(adapterIndex, adapter.ReleaseAndGetAddressOf());
					if (FAILED(hr))
					{
						break;
					}

					DXGI_ADAPTER_DESC1 adapterDesc;
					hr = adapter->GetDesc1(&adapterDesc);
					if (SUCCEEDED(hr))
					{
						for (int outputIndex = 0; !done; outputIndex++)
						{
							Microsoft::WRL::ComPtr<IDXGIOutput> output;
							hr = adapter->EnumOutputs(outputIndex, output.ReleaseAndGetAddressOf());
							if (FAILED(hr))
							{
								break;
							}

							DXGI_OUTPUT_DESC outputDesc;
							hr = output->GetDesc(&outputDesc);
							if (SUCCEEDED(hr))
							{
								if (lstrcmpW(outputDesc.DeviceName, monitorInfo.szDevice) == 0)
								{
									adapterNumber = adapterIndex;
									outputNumber = outputIndex;
									done = true;
									break;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			if (!bWindowedMode)
			{
				Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
				Microsoft::WRL::ComPtr<IDXGIOutput> output;
				hr = dxgifactory1->EnumAdapters1(adapterNumber, adapter.ReleaseAndGetAddressOf());
				if (SUCCEEDED(hr))
				{
					hr = adapter->EnumOutputs(outputNumber, output.ReleaseAndGetAddressOf());
					if (SUCCEEDED(hr))
					{
						DXGI_OUTPUT_DESC outputDesc;
						hr = output->GetDesc(&outputDesc);
						if (SUCCEEDED(hr))
						{
							SetWindowPos(hWnd, HWND_TOP, outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.top, outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left, outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top, SWP_NOCOPYBITS | SWP_NOSENDCHANGING | SWP_DEFERERASE);
						}
					}
				}
			}
		}

		Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
		hr = dxgifactory1->EnumAdapters1(adapterNumber, adapter.GetAddressOf());
		if (FAILED(hr))
		{
			message.clear();
			pMsgBuffer = StringConverter::MallocFormattedStringA("EnumAdapters1 failed to get the adapter %u", (unsigned int)adapterNumber);
			if (pMsgBuffer != nullptr)
			{
				message.append(pMsgBuffer);
				free(pMsgBuffer);
				pMsgBuffer = nullptr;
				COM_ERROR_IF_FAILED(hr, message);
			}
			else
			{
				COM_ERROR_IF_FAILED(hr, "EnumAdapters1 failed to get the adapter.");
			}
		}

		wantFullScreenBorderlessWindow = false;
		enableFullScreenBorderlessTearing = false;
		BOOL allowTearing;
		if (SUCCEEDED(dxgifactory5->CheckFeatureSupport(DXGI_FEATURE::DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing))))
		{
			this->isTearingSupported = allowTearing != FALSE;
		}
		else
		{
			this->isTearingSupported = false;
		}

		vOutputs.clear();
		FillOutputs(adapter.Get(), this->vOutputs);
		if (this->outputNumber >= vOutputs.size())
		{
			COM_ERROR_IF_FAILED(hr, "Failed to select a monitor output.");			
		}

		Microsoft::WRL::ComPtr<IDXGIOutput> pOutput = vOutputs[outputNumber];

		// There feature levels require non power of 2 texture dimensions.
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_1
			, D3D_FEATURE_LEVEL_11_0
			, D3D_FEATURE_LEVEL_10_1
			, D3D_FEATURE_LEVEL_10_0
		};

		UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		if (device == nullptr || deviceContext == nullptr)
		{
			// TODO D3D11_CREATE_DEVICE_SINGLETHREADED
			D3D_FEATURE_LEVEL featureLevel;
			hr = D3D11CreateDevice(
				adapter.Get()
				, D3D_DRIVER_TYPE_UNKNOWN
				, nullptr
				, createDeviceFlags
				, featureLevels
				, ARRAYSIZE(featureLevels)
				, D3D11_SDK_VERSION
				, device.ReleaseAndGetAddressOf()
				, &featureLevel
				, deviceContext.ReleaseAndGetAddressOf()
			);

			if (hr == E_INVALIDARG)
			{
				hr = D3D11CreateDevice(
					adapter.Get()
					, D3D_DRIVER_TYPE_UNKNOWN
					, nullptr
					, createDeviceFlags
					, &featureLevels[1]
					, ARRAYSIZE(featureLevels) - 1
					, D3D11_SDK_VERSION
					, device.ReleaseAndGetAddressOf()
					, &featureLevel
					, deviceContext.ReleaseAndGetAddressOf()
				);
			}

			COM_ERROR_IF_FAILED(hr, "D3D11CreateDevice failed.");
		}

		DXGI_MODE_DESC selectedFullScreenModeDesc = {};
		if (!bWindowedMode)
		{
			hr = GraphicsHelper::GetPreferredMode(device.Get(), pOutput.Get(), &requestedMode, &selectedFullScreenModeDesc);
			COM_ERROR_IF_FAILED(hr, "Unable to find a fullscreen matching mode.");
		}

		if (dxgifactory2)
		{
			// DirectX 11.1 systems
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC* pSwapFullDesc = nullptr;
			DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapFullDesc;
			ZeroMemory(&swapFullDesc, sizeof(swapFullDesc));
			if (!bWindowedMode)
			{
				// Fullscreen description
				swapFullDesc.Windowed = bWindowedMode ? TRUE : FALSE;
				swapFullDesc.RefreshRate = selectedFullScreenModeDesc.RefreshRate;
				swapFullDesc.Scaling = selectedFullScreenModeDesc.Scaling;
				swapFullDesc.ScanlineOrdering = selectedFullScreenModeDesc.ScanlineOrdering;
				pSwapFullDesc = &swapFullDesc;
			}

			DXGI_SWAP_CHAIN_DESC1 swapWindowedDesc = {};
			this->otherFlags &= ~DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
			if (!bWindowedMode && this->wantFullScreenBorderlessWindow)
			{
				if (syncMode != HCFG::FULLSCREENSYNCMODE::FSSM_VBL)
				{
					if (this->isTearingSupported)
					{
						this->enableFullScreenBorderlessTearing = true;
						this->otherFlags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
					}
				}
			}

			swapWindowedDesc.Format = GraphicsHelper::DefaultPixelFormat;
			swapWindowedDesc.Stereo = false;
			swapWindowedDesc.SampleDesc.Count = 1;
			swapWindowedDesc.SampleDesc.Quality = 0;
			swapWindowedDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapWindowedDesc.BufferCount = bWindowedMode ? 1 : 2;
			swapWindowedDesc.Scaling = DXGI_SCALING_STRETCH;
			swapWindowedDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
			swapWindowedDesc.Flags = this->otherFlags;
			if (bWindowedMode)
			{
				swapWindowedDesc.Width = 0;
				swapWindowedDesc.Height = 0;
				swapWindowedDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			}
			else
			{
				swapWindowedDesc.Width = selectedFullScreenModeDesc.Width;
				swapWindowedDesc.Height = selectedFullScreenModeDesc.Height;
				swapWindowedDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			}

			hr = dxgifactory2->CreateSwapChainForHwnd(
				device.Get(),
				hWnd,
				&swapWindowedDesc,
				pSwapFullDesc,
				pOutput.Get(),
				swapChain1.ReleaseAndGetAddressOf()
			);

			COM_ERROR_IF_FAILED(hr, "CreateSwapChainForHwnd failed.");
			swapChain = swapChain1;
		}
		else
		{
			// DirectX 11.0 systems
			DXGI_SWAP_CHAIN_DESC sd = {};
			sd.BufferCount = bWindowedMode ? 1 : 2;
			if (bWindowedMode)
			{
				sd.BufferDesc.Width = 0;
				sd.BufferDesc.Height = 0;
				sd.BufferDesc.RefreshRate.Numerator = 0;
				sd.BufferDesc.RefreshRate.Denominator = 0;
				sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
				sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			}
			else
			{
				sd.BufferDesc.Width = selectedFullScreenModeDesc.Width;
				sd.BufferDesc.Height = selectedFullScreenModeDesc.Height;
				sd.BufferDesc.RefreshRate = selectedFullScreenModeDesc.RefreshRate;
				sd.BufferDesc.Scaling = selectedFullScreenModeDesc.Scaling;
				sd.BufferDesc.ScanlineOrdering = selectedFullScreenModeDesc.ScanlineOrdering;
			}

			sd.BufferDesc.Format = GraphicsHelper::DefaultPixelFormat;
			sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.OutputWindow = hWnd;
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
			sd.Windowed = bWindowedMode ? TRUE : FALSE;
			hr = dxgifactory1->CreateSwapChain(device.Get(), &sd, swapChain.ReleaseAndGetAddressOf());
			COM_ERROR_IF_FAILED(hr, "CreateSwapChain failed.");
		}


		dxgifactory1->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER | DXGI_MWA_NO_WINDOW_CHANGES);
		assignedWidth = 0;
		assignedHeight = 0;
		assignedSwapChainDesc = {};
		hr = swapChain->GetDesc(&assignedSwapChainDesc);
		COM_ERROR_IF_FAILED(hr, "GetDesc failed.");				
		if (bWindowedMode)
		{
			assignedWidth = assignedSwapChainDesc.BufferDesc.Width;
			assignedHeight = assignedSwapChainDesc.BufferDesc.Height;
		}
		else
		{
			assignedWidth = assignedSwapChainDesc.BufferDesc.Width;
			assignedHeight = assignedSwapChainDesc.BufferDesc.Height;
		}

		this->appStatus->m_bWindowed = bWindowedMode;
		isWentFullscreen = !bWindowedMode;

		hr = CreateRenderTargetDepthAndViewport(assignedWidth, assignedHeight);
		COM_ERROR_IF_FAILED(hr, "Failed to create render target depth buffer and viewport.");

		//Create Rasterizer State
		CD3D11_RASTERIZER_DESC rasterizerDesc(D3D11_DEFAULT);
		hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerState.ReleaseAndGetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create rasterizer state.");

		//Create Blend State
		D3D11_RENDER_TARGET_BLEND_DESC rtbd = { 0 };
		rtbd.BlendEnable = true;
		rtbd.SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
		rtbd.DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
		rtbd.DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
		rtbd.BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
		D3D11_BLEND_DESC blendDesc = { 0 };
		blendDesc.RenderTarget[0] = rtbd;
		hr = this->device->CreateBlendState(&blendDesc, this->blendState.ReleaseAndGetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create blend state.");

		//Create sampler description for sampler state
		CD3D11_SAMPLER_DESC linearSampDesc(D3D11_DEFAULT);
		linearSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		linearSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		linearSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		linearSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = this->device->CreateSamplerState(&linearSampDesc, this->linearSamplerState.ReleaseAndGetAddressOf()); //Create sampler state
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");

		CD3D11_SAMPLER_DESC pointSampDesc(D3D11_DEFAULT);
		pointSampDesc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
		pointSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		pointSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		pointSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		hr = this->device->CreateSamplerState(&pointSampDesc, this->pointSamplerState.ReleaseAndGetAddressOf()); //Create sampler state
		COM_ERROR_IF_FAILED(hr, "Failed to create sampler state.");
		hr = S_OK;
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		hr = exception.HResult();
	}

	if (pMsgBuffer != nullptr)
	{
		free(pMsgBuffer);
		pMsgBuffer = nullptr;
	}

	return hr;
}

HRESULT Graphics::CreateRenderTargetDepthAndViewport(unsigned int width, unsigned int height)
{
	HRESULT hr;
	try
	{
		SetMenuKeepAliveHeightFromScreenHeight(height);

		// Create a render target view
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		COM_ERROR_IF_FAILED(hr, "IDXGISwapChain::GetBuffer failed.");

		hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.ReleaseAndGetAddressOf());
		backBuffer.Reset();
		COM_ERROR_IF_FAILED(hr, "ID3D11Device::CreateRenderTargetView failed.");

		//Describe our Depth/Stencil Buffer
		CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height);
		depthStencilTextureDesc.MipLevels = 1;
		depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		hr = this->device->CreateTexture2D(&depthStencilTextureDesc, NULL, this->depthStencilBuffer.ReleaseAndGetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil buffer.");

		hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.ReleaseAndGetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil view.");

		deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), this->depthStencilView.Get());

		//Create depth stencil state
		CD3D11_DEPTH_STENCIL_DESC depthstencildesc(D3D11_DEFAULT);
		depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

		hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.ReleaseAndGetAddressOf());
		COM_ERROR_IF_FAILED(hr, "Failed to create depth stencil state.");

		//Create & set the Viewport
		CD3D11_VIEWPORT viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
		this->deviceContext->RSSetViewports(1, &viewport);
	}
	catch (COMException & exception)
	{
		ErrorLogger::Log(exception);
		return exception.HResult();
	}

	return S_OK;
}

void Graphics::CleanRenderTarget()
{
	if (deviceContext != nullptr)
	{
		deviceContext->OMSetRenderTargets(0, nullptr, nullptr);
	}

	c64display.Cleanup();
	CleanCharRomTextureSet();
	CleanScene();
	CleanShaders();
	linearSamplerState.Reset();
	pointSamplerState.Reset();
	blendState.Reset();
	rasterizerState.Reset();
	depthStencilState.Reset();
	depthStencilView.Reset();
	depthStencilBuffer.Reset();
	renderTargetView.Reset();
}

void Graphics::CleanSwapChain()
{
	if (swapChain != nullptr)
	{
		swapChain->SetFullscreenState(FALSE, nullptr);
	}

	CleanRenderTarget();
	swapChain1.Reset();
	swapChain.Reset();
}

void Graphics::CleanDevice()
{
	ImGuiShutdown();
	CleanSwapChain();
	if (deviceContext != nullptr)
	{
		deviceContext->ClearState();
		deviceContext->Flush();
	}

	deviceContext.Reset();
	device.Reset();
	isInitOK = false;
}

void Graphics::ImGuiStart()
{
	if (!isStartedImGuiDx11)
	{
		isStartedImGuiDx11 = true;
		ImGui_ImplWin32_Init(this->hWnd);
		ImGui_ImplDX11_Init(device.Get(), deviceContext.Get());
		ImGuiIO& io = ImGui::GetIO();
		//io.DisplaySize.x = this->assignedWidth;
		//io.DisplaySize.y = this->assignedHeight;
	}
}

void Graphics::ImGuiShutdown()
{
	if (isStartedImGuiDx11)
	{
		isStartedImGuiDx11 = false;
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
	}
}


HRESULT Graphics::InitializeShaders()
{
	std::wstring vertexShaderFilename;
	if (!GraphicsHelper::GetAppFileFullPath(L"vertexshader_2d.cso", vertexShaderFilename))
	{
		ErrorLogger::Log("Failed GetAppFileFullPath.");
		return E_FAIL;
	}

	std::wstring pixelShaderFilename;
	if (!GraphicsHelper::GetAppFileFullPath(L"pixelshader_2d.cso", pixelShaderFilename))
	{
		ErrorLogger::Log("Failed GetAppFileFullPath.");
		return E_FAIL;
	}

	D3D11_INPUT_ELEMENT_DESC layout2D[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
	};

	UINT numElements2D = ARRAYSIZE(layout2D);
	if (!this->vertex_shader_2d.Initialize(this->device.Get(), vertexShaderFilename.c_str(), layout2D, numElements2D))
	{
		return E_FAIL;
	}

	if (!this->pixel_shader_2d.Initialize(this->device.Get(), pixelShaderFilename.c_str()))
	{
		return E_FAIL;
	}

	return S_OK;
}

void Graphics::CleanShaders()
{
	pixel_shader_2d.Cleanup();
	vertex_shader_2d.Cleanup();
}

HRESULT Graphics::InitializeScene()
{
	HRESULT hr = E_FAIL;
	try
	{
		//Initialize Constant Buffer(s)
		hr = this->cb_vs_vertexshader_2d.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize 2d constant buffer.");

		hr = this->cb_vs_vertexshader.Initialize(this->device.Get(), this->deviceContext.Get());
		COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

		hr = c64display.Initialize(this->device.Get(), this->deviceContext.Get(), this->cb_vs_vertexshader_2d, this->pointSamplerState.Get(), this->linearSamplerState.Get(), hWnd, assignedWidth, assignedHeight, bWindowedMode, borderSize, bShowFloppyLed, stretch, bUsePointFilter, c64, appStatus);
		COM_ERROR_IF_FAILED(hr, "Failed to initialize c64 display.");

		hr = LoadCbmCharTexture();
		COM_ERROR_IF_FAILED(hr, "Failed to initialize CBM ROM textures.");

		float windowWidth = (float)assignedWidth;
		float windowHeight = (float)assignedHeight;
		camera2D.SetProjectionValues(windowWidth, windowHeight, 0.0f, 1.0f);
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return exception.HResult();
	}

	return hr;
}

void Graphics::CleanScene()
{
	cb_vs_vertexshader.Cleanup();
	cb_vs_vertexshader_2d.Cleanup();
}

void Graphics::CleanCharRomTextureSet()
{
	for (int i = 0; i < _countof(cbmCharRom); i++)
	{
		if (cbmCharRom[i] != nullptr)
		{
			delete cbmCharRom[i];
			cbmCharRom[i] = nullptr;
		}
	}
}

HRESULT Graphics::GetCurrentBufferFormat(DXGI_FORMAT* pFormat)
{
	if (pFormat == nullptr)
	{
		return E_INVALIDARG;
	}

	*pFormat = assignedSwapChainDesc.BufferDesc.Format;
	return S_OK;
}

HRESULT Graphics::RenderFrame()
{
	if (!this->isInitOK)
	{
		return E_FAIL;
	}

	deviceContext->ClearRenderTargetView(renderTargetView.Get(), bgcolor);
	deviceContext->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->RSSetState(rasterizerState.Get());
	deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);	
	deviceContext->PSSetSamplers(0, 1, linearSamplerState.GetAddressOf());
	deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	deviceContext->IASetInputLayout(vertex_shader_2d.GetInputLayout());
	deviceContext->PSSetShader(pixel_shader_2d.GetShader(), NULL, 0);
	deviceContext->VSSetShader(vertex_shader_2d.GetShader(), NULL, 0);
	c64display.Draw(camera2D.GetWorldMatrix() * camera2D.GetOrthoMatrix());
	DrawGui();
	return S_OK;
}

HRESULT Graphics::PresentFrame()
{
	if (!this->isInitOK)
	{
		return E_FAIL;
	}

	if (useVsync)
	{
		return this->swapChain->Present(1, 0);
	}
	else
	{
		return this->swapChain->Present(0, 0);
	}
}

HRESULT Graphics::TestPresent()
{
	if (!this->isInitOK)
	{
		return E_FAIL;
	}

	return this->swapChain->Present(0, DXGI_PRESENT_TEST);

}

void Graphics::DrawGui()
{
	//DrawDemoGui();
	//return;

	static bool show_file_open = false;
	static bool show_main_menu = false;
	static bool wasEnableC64Input = false;
	static int keep_alive_show_main_menu = 0;
	bool enableC64Input = true;

	for (;;)
	{
		if (!this->isStartedImGuiDx11)
		{
			break;
		}

		if (this->bWindowedMode && !this->appStatus->m_bEnableImGuiWindowed)
		{
			break;
		}
		
		int mouseX;
		int mouseY;
		currentDrawFrameCounter++;
		appCommand->GetLastMousePosition(&mouseX, &mouseY);
		constexpr int MouseCapturePeriod = 10;
		constexpr int KeepAliveCount = 10;
		if (currentDrawFrameCounter - mouseSnapShotFrameCounter > MouseCapturePeriod)
		{
			mouseSnapShotFrameCounter = currentDrawFrameCounter;
			if (abs(mouseX - lastMouseX) > mouseActiveDistance || abs(mouseY - lastMouseY) > mouseActiveDistance || mouseY < menukeepAliveHeight)
			{
				keep_alive_show_main_menu = KeepAliveCount;
			}

			if (keep_alive_show_main_menu > 0)
			{
				show_main_menu = true;
				keep_alive_show_main_menu--;
			}
			else
			{
				show_main_menu = false;
			}

			lastMouseX = mouseX;
			lastMouseY = mouseY;
		}

		if (show_file_open)
		{
			show_main_menu = false;
			keep_alive_show_main_menu = 0;
		}

		if (!show_main_menu && !show_file_open)
		{
			if (keep_alive_show_main_menu == 0)
			{
				keep_alive_show_main_menu = -1;
				if (!appStatus->m_bDebug)
				{
					ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_None);
					SetCursor(NULL);
				}
			}

			break;
		}

		ImGuiIO& io = ImGui::GetIO();
		enableC64Input = !(io.WantCaptureKeyboard || io.WantTextInput);

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		if (show_main_menu)
		{
			// Start the Dear ImGui frame
			if (ImGui::BeginMainMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					if (ImGui::MenuItem("Open - Autoload"))
					{
						show_file_open = true;
					}

					ImGui::Separator();
					if (ImGui::MenuItem("Exit"))
					{
						this->appCommand->PostCloseMainWindow();
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Settings"))
				{
					if (ImGui::MenuItem("Toggle Fullscreen"))
					{
						this->appCommand->PostToggleFullscreen();
					}

					ImGui::EndMenu();
				}

				ImGui::EndMainMenuBar();
			}
		}

		if (show_file_open)
		{
			static bool was_sid_file = false;
			static int char_set_number = 0;
			static int currentFileSystemItem = -1;
			static const char* title = "Choose a C64 program file";
			if (!ImGui::IsPopupOpen(title))
			{
				currentFileSystemItem = -1;
				directoryViewer.Set_CbmDirectoryLoaded(false);
				directoryViewer.Set_IsCbmDiskTitleSelected(false);
				directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
				directoryViewer.Set_IsQuickloadEnabled(false);
				directoryViewer.UseC64FilesFilter();
				directoryViewer.Fill();
				ImGui::OpenPopup(title);
			}

			ImGui::SetNextWindowSizeConstraints(ImVec2(500, 100), ImVec2(FLT_MAX, FLT_MAX));
			ImGui::SetNextWindowSize(ImVec2((float)(this->assignedWidth / 1.2f), (float)(this->assignedHeight / 2)), ImGuiCond_FirstUseEver);
			if (ImGui::BeginPopupModal(title))
			{
				bool wantOpen = false;
				bool wantInsert = false;
				bool wantClose = false;
				bool wantChangeDirectory = false;
				bool wantChangeParentDirectory = false;
				bool wantQuickload = false;
				int wantChangeDirectoryIndex = -1;
				int wantChangeParentDirectoryIndex = -1;
				int wantOpenFileIndex = -1;
				int wantCbmDirectoryItem = -1;
				ImGuiStyle style = ImGui::GetStyle();
				ImGui::BeginChild("header pane", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetFontSize() + style.ItemSpacing.y + style.ScrollbarSize + 2.0f * (style.ChildBorderSize + style.WindowPadding.y + style.FramePadding.y)), true, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
				{
					std::string name;

					ImGui::PushID("drives");
					if (ImGui::Button("Drives"))
					{
						currentFileSystemItem = -1;
						directoryViewer.ClearCbmDirectory();
						directoryViewer.Set_CbmDirectoryLoaded(false);
						directoryViewer.Set_IsCbmDiskTitleSelected(false);
						directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
						wantChangeParentDirectory = true;
						wantChangeParentDirectoryIndex = -1;
					}

					ImGui::PopID();

					for (size_t i = 0; i < directoryViewer.ParentFullPath.size(); i++)
					{
						FileSys::DirectoryItem& di = directoryViewer.ParentFullPath[i];
						name.clear();
						name.append(di.GetNameA());
						ImGui::PushID("pathitem");
						ImGui::PushID((int)i);

						ImGui::SameLine();
						if (ImGui::Button(name.c_str()))
						{
							currentFileSystemItem = -1;
							directoryViewer.ClearCbmDirectory();
							directoryViewer.Set_CbmDirectoryLoaded(false);
							directoryViewer.Set_IsCbmDiskTitleSelected(false);
							directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
							wantChangeParentDirectory = true;
							wantChangeParentDirectoryIndex = (int)i;
						}

						ImGui::PopID();
						ImGui::PopID();
					}
				}

				ImGui::EndChild();
				ImGui::BeginChild("left pane", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
				{
					std::string name;
					for (size_t i = 0; i < directoryViewer.CurrentDirectoryItems.size(); i++)
					{
						FileSys::DirectoryItem& di = directoryViewer.CurrentDirectoryItems[i];
						name.clear();
						if (di.IsDirectory())
						{
							name.append("<dir> ");
						}

						name.append(di.GetNameA());
						ImGui::PushID("filesystemitem");
						ImGui::PushID((int)i);
						if (ImGui::Selectable(name.c_str(), currentFileSystemItem == (int)i, ImGuiSelectableFlags_AllowDoubleClick))
						{
							if (di.IsDirectory())
							{
								currentFileSystemItem = -1;
								directoryViewer.ClearCbmDirectory();
								directoryViewer.Set_CbmDirectoryLoaded(false);
								directoryViewer.Set_IsCbmDiskTitleSelected(false);
								directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
								wantChangeDirectory = true;
								wantChangeDirectoryIndex = (int)i;
							}
							else if (di.IsCommodore64File())
							{
								if (ImGui::IsMouseDoubleClicked(0))
								{
									wantOpen = true;
									wantOpenFileIndex = (int)i;
									wantCbmDirectoryItem = -1;
								}

								if (currentFileSystemItem != i)
								{
									directoryViewer.ClearCbmDirectory();
									directoryViewer.Set_CbmDirectoryLoaded(false);
									directoryViewer.Set_IsCbmDiskTitleSelected(false);
									directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
								}

								currentFileSystemItem = (int)i;
							}
							else
							{
								directoryViewer.ClearCbmDirectory();
								directoryViewer.Set_CbmDirectoryLoaded(false);
								directoryViewer.Set_IsCbmDiskTitleSelected(false);
								directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
							}
						}

						ImGui::PopID();
						ImGui::PopID();
					}
				}

				ImGui::EndChild();
				ImGui::SameLine();
				ImGui::BeginChild("right pane", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_HorizontalScrollbar);
				{
					if (ImGui::RadioButton("1st Char Set", char_set_number == 0))
					{
						char_set_number = 0;
					}

					ImGui::SameLine();
					if (ImGui::RadioButton("2nd Char Set", char_set_number == 1))
					{
						char_set_number = 1;
					}

					ImGui::SameLine();
					wantQuickload = directoryViewer.Get_IsQuickloadEnabled();
					if (ImGui::Checkbox("Quickload", &wantQuickload))
					{
						directoryViewer.Set_IsQuickloadEnabled(wantQuickload);
					}

					if (!directoryViewer.Get_IsCbmDirectoryLoaded())
					{
						directoryViewer.Set_CbmDirectoryLoaded(true);
						directoryViewer.Set_IsCbmDiskTitleSelected(false);
						directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
						directoryViewer.ClearCbmDirectory();
						if (currentFileSystemItem >= 0 && (size_t)currentFileSystemItem < directoryViewer.CurrentDirectoryItems.size())
						{
							FileSys::DirectoryItem& di = directoryViewer.CurrentDirectoryItems[currentFileSystemItem];
							if (di.IsCommodore64File())
							{
								int count;
								appCommand->SetBusyApp(true);
								HRESULT hr = directoryViewer.LoadCbmDirectory(directoryViewer.GetItemName(currentFileSystemItem).c_str(), C64Directory::D64MAXDIRECTORYITEMCOUNT, count, false, nullptr);
								if (FAILED(hr))
								{
									directoryViewer.ClearCbmDirectory();
								}

								if (di.IsCommodore64Sid())
								{
									if (!was_sid_file)
									{
										char_set_number = 1;
									}

									was_sid_file = true;
								}
								else
								{
									if (was_sid_file)
									{
										char_set_number = 0;
									}

									was_sid_file = false;
								}								

								appCommand->SetBusyApp(false);
							}
						}
					}

					if (directoryViewer.Get_IsCbmDirectorySuccessfullyLoaded())
					{
						float cbmfontsize = (float)dpi.ScaleY(16);
						constexpr int WidthLeftSelector = 1;
						constexpr int WidthRightSelector = 1;
						constexpr int WidthFileTypeGap = 2;
						constexpr int WidthFileType = 3;
						constexpr int IndexTypeName = WidthLeftSelector + C64Directory::D64FILENAMELENGTH + WidthFileTypeGap;
						constexpr int IndexLeftSelector = 0;
						constexpr int IndexRightSelector = WidthLeftSelector + C64Directory::D64FILENAMELENGTH + WidthFileTypeGap + WidthFileType;
						bit8 tempC64String[WidthLeftSelector + C64Directory::D64FILENAMELENGTH + WidthFileTypeGap + WidthFileType + WidthRightSelector];
						ImVec2 szbutton = ImVec2(sizeof(tempC64String) * cbmfontsize, cbmfontsize - 4);
						ImVec2 pos = ImGui::GetCursorPos();
						C64File& c64file = directoryViewer.c64file;
						//deviceContext->PSSetSamplers(0, 1, pointSamplerState.GetAddressOf());

						// Draw disk title.
						memset(tempC64String, 0xA0, sizeof(tempC64String));
						if (directoryViewer.Get_IsCbmDiskTitleSelected())
						{
							tempC64String[0] = 0x3E; // screen code for '>'
							tempC64String[IndexRightSelector] = 0x3C; // screen code for '<'
						}

						c64file.GetC64Diskname(&tempC64String[WidthLeftSelector], C64Directory::D64FILENAMELENGTH);
						for (size_t j = 0; j < (size_t)sizeof(tempC64String); j++)
						{
							if (j > 0)
							{
								ImGui::SameLine(0, 0);
							}

							unsigned int screencode = (unsigned int)C64File::ConvertPetAsciiToScreenCode(tempC64String[j]);
							ImVec2 uv0;
							ImVec2 uv1;
							GetTextureCoordFromScreenCode(screencode, uv0, uv1);
							ImGui::Image(this->cbmCharRom[char_set_number]->GetTextureResourceView(), ImVec2(cbmfontsize, cbmfontsize), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
						}

						ImGui::SetCursorPos(pos);
						ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.50f);
						ImGui::PushID("disktitle");
						if (ImGui::Selectable("", directoryViewer.Get_IsCbmDiskTitleSelected(), ImGuiSelectableFlags_AllowDoubleClick, szbutton))
						{
							directoryViewer.Set_IsCbmDiskTitleSelected(true);
							directoryViewer.Set_IndexCurrentCbmDiskItem(-1);
							if (ImGui::IsMouseDoubleClicked(0))
							{
								wantOpen = true;
								wantOpenFileIndex = currentFileSystemItem;
								wantCbmDirectoryItem = -1;
							}
						}

						ImGui::PopID();
						ImGui::PopStyleVar();

						for (int k = 0; k < c64file.GetFileCount(); k++)
						{
							pos.y += cbmfontsize;
							ImGui::SetCursorPosY(pos.y);
							memset(tempC64String, 0xA0, sizeof(tempC64String));
							bool isSelected = directoryViewer.Get_IndexCurrentCbmDiskItem() == k && !directoryViewer.Get_IsCbmDiskTitleSelected();
							if (isSelected)
							{
								tempC64String[0] = 0x3E; // screen code for '>'
								tempC64String[IndexRightSelector] = 0x3C; // screen code for '<'
							}

							c64file.GetDirectoryItemName(k, &tempC64String[WidthLeftSelector], C64Directory::D64FILENAMELENGTH);
							const bit8* ftname = c64file.GetDirectoryItemTypeName(k);
							if (ftname != nullptr)
							{
								for (size_t j = 0; j < sizeof(C64File::FTN_CLR); j++)
								{
									tempC64String[IndexTypeName + j] = ftname[j];
								}
							}

							// Draw disk directory item.
							for (size_t j = 0; j < (size_t)sizeof(tempC64String); j++)
							{
								if (j > 0)
								{
									ImGui::SameLine(0, 0);
								}

								unsigned int screencode = (unsigned int)C64File::ConvertPetAsciiToScreenCode(tempC64String[j]);
								ImVec2 uv0;
								ImVec2 uv1;
								GetTextureCoordFromScreenCode(screencode, uv0, uv1);
								ImGui::Image(this->cbmCharRom[char_set_number]->GetTextureResourceView(), ImVec2(cbmfontsize, cbmfontsize), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
							}

							ImGui::SetCursorPos(pos);
							ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.50f);
							ImGui::PushID("cbmdiritem");
							ImGui::PushID(k);
							if (ImGui::Selectable("", isSelected, ImGuiSelectableFlags_AllowDoubleClick, szbutton))
							{
								directoryViewer.Set_IndexCurrentCbmDiskItem(k);
								directoryViewer.Set_IsCbmDiskTitleSelected(false);
								if (ImGui::IsMouseDoubleClicked(0))
								{
									wantOpen = true;
									wantOpenFileIndex = currentFileSystemItem;
									wantCbmDirectoryItem = k;
								}
							}

							ImGui::PopID();
							ImGui::PopID();
							ImGui::PopStyleVar();
						}
					}
				}

				ImGui::EndChild();

				// Open file.
				if (ImGui::Button("Open / Restart (Enter)", ImVec2(0, 0)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Enter)))
				{
					wantOpen = true;
					wantOpenFileIndex = currentFileSystemItem;
					wantCbmDirectoryItem = directoryViewer.Get_IsCbmDiskTitleSelected() ? -1 : directoryViewer.Get_IndexCurrentCbmDiskItem();
				}

				ImGui::SameLine();

				// Insert disk.
				if (ImGui::Button("Insert Disk (Alt I)", ImVec2(0, 0)) || (ImGui::IsKeyReleased('I') && io.KeyAlt))
				{
					wantInsert = true;
					wantOpenFileIndex = currentFileSystemItem;
					wantCbmDirectoryItem = -1;
				}

				ImGui::SameLine();

				// Close pop up.
				if (ImGui::Button("Close", ImVec2(0, 0)) || ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					wantClose = true;
				}
				
				if (wantOpen)
				{
					wantOpen = false;
					appCommand->SetBusyApp(true);
					if (wantOpenFileIndex >= 0 && (size_t)wantOpenFileIndex < directoryViewer.CurrentDirectoryItems.size())
					{
						FileSys::DirectoryItem& di = directoryViewer.CurrentDirectoryItems[wantOpenFileIndex];
						if (di.IsDirectory())
						{
							wantChangeDirectory = true;
							wantChangeDirectoryIndex = wantOpenFileIndex;
						}
						else if (di.IsCommodore64File())
						{
							if (!directoryViewer.Get_IsCbmDiskTitleSelected() && directoryViewer.Get_IndexCurrentCbmDiskItem() >= 0)
							{
								wantCbmDirectoryItem = directoryViewer.Get_IndexCurrentCbmDiskItem();
							}
							else
							{
								wantCbmDirectoryItem = -1;
							}

							if (this->appCommand->PostAutoLoadFile(Wfs::Path_Combine(directoryViewer.GetCurrentDir(), di.GetNameW()).c_str(), wantCbmDirectoryItem, wantQuickload))
							{
								wantClose = true;
							}
						}
					}

					appCommand->SetBusyApp(false);
				}
				else if (wantInsert)
				{
					wantInsert = false;
					appCommand->SetBusyApp(true);
					if (wantOpenFileIndex >= 0 && (size_t)wantOpenFileIndex < directoryViewer.CurrentDirectoryItems.size())
					{
						FileSys::DirectoryItem& di = directoryViewer.CurrentDirectoryItems[wantOpenFileIndex];
						if (!di.IsDirectory())
						{
							if (di.IsCommodore64Disk())
							{
								if (this->appCommand->InsertDiskImageFromFile(Wfs::Path_Combine(directoryViewer.GetCurrentDir(), di.GetNameW()).c_str()))
								{
									wantClose = true;
								}
							}
							else if (di.IsCommodore64Tape())
							{
								if (this->appCommand->InsertTapeImageFromFile(Wfs::Path_Combine(directoryViewer.GetCurrentDir(), di.GetNameW()).c_str()))
								{
									wantClose = true;
								}
							}
							
							if (!directoryViewer.Get_IsCbmDiskTitleSelected() && directoryViewer.Get_IndexCurrentCbmDiskItem() >= 0)
							{
								wantCbmDirectoryItem = directoryViewer.Get_IndexCurrentCbmDiskItem();
							}
							else
							{
								wantCbmDirectoryItem = -1;
							}
						}
					}

					appCommand->SetBusyApp(false);
				}

				if (wantChangeDirectory)
				{
					wantChangeDirectory = false;
					directoryViewer.ChangeDirectory(wantChangeDirectoryIndex);
					currentFileSystemItem = -1;
				}
				else if (wantChangeParentDirectory)
				{
					wantChangeParentDirectory = false;
					if (wantChangeParentDirectoryIndex >= 0)
					{
						directoryViewer.ChangeParentDirectory(wantChangeParentDirectoryIndex);
					}
					else
					{
						directoryViewer.ChangeToRoot();
					}

					currentFileSystemItem = -1;
				}

				if (wantClose)
				{
					ImGui::CloseCurrentPopup();
					show_file_open = false;
					enableC64Input = true;
				}
				
				ImGui::EndPopup();
			}
			else
			{
				show_file_open = false;
				enableC64Input = true;
			}
		}

		ImGui::Render();
		//g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
		//g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		break;
	}


	if (enableC64Input != wasEnableC64Input)
	{
		this->appCommand->EnableC64Input(enableC64Input);
		wasEnableC64Input = enableC64Input;
	}
}

void::Graphics::GetTextureCoordFromScreenCode(unsigned int screencode, ImVec2& uv0, ImVec2& uv1) const
{
	float size = 264.0f;
	unsigned int chx = screencode % 16;
	chx = chx * 2 + 1;
	chx = chx * 8;

	unsigned int chy = screencode / 16;
	chy = chy * 2 + 1;
	chy = chy * 8;

	uv0 = ImVec2((float)chx / size, (float)chy / size);
	uv1 = ImVec2((float)(chx + 8) / size, (float)(chy + 8) / size);
}

void Graphics::DrawDemoGui()
{
	if (!this->isStartedImGuiDx11)
	{
		return;
	}

	static bool show_demo_window = false;
	static bool show_another_window = false;
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}

	// Rendering
	ImGui::Render();
	//g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
	//g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Graphics::SetBgColor(unsigned char r, unsigned char b, unsigned char g)
{
	bgcolor[0] = r / 255.0f;
	bgcolor[1] = g / 255.0f;
	bgcolor[2] = b / 255.0f;
	bgcolor[3] = 1.0f;
}

void Graphics::ClearFrame()
{
	return;
	if (!this->isInitOK)
	{
		return;
	}

	if (deviceContext != nullptr)
	{
		deviceContext->ClearRenderTargetView(renderTargetView.Get(), bgcolor);
	}
}

HRESULT Graphics::ResizeBuffers(unsigned int width, unsigned int height)
{
	if (!this->isInitOK || swapChain == nullptr)
	{
		return E_FAIL;
	}

	isInitOK = false;
	if (deviceContext != nullptr)
	{
		deviceContext->OMSetRenderTargets(0, 0, 0);
		deviceContext->OMSetDepthStencilState(NULL, 0);
	}

	depthStencilState.Reset();
	depthStencilView.Reset();
	depthStencilBuffer.Reset();
	renderTargetView.Reset();
	UINT buffers = 0;
	BOOL isFull = FALSE;
	Microsoft::WRL::ComPtr<IDXGIOutput> output;
	HRESULT hr = swapChain->GetFullscreenState(&isFull, output.GetAddressOf());
	if (SUCCEEDED(hr))
	{
		buffers = isFull != FALSE ? 2 : 1;
	}
	else
	{
		return hr;
	}

	hr = swapChain->ResizeBuffers(buffers, (UINT)width, (UINT)height, DXGI_FORMAT::DXGI_FORMAT_UNKNOWN, this->otherFlags);
	if (FAILED(hr))
	{
		return hr;
	}

	assignedWidth = width;
	assignedHeight = height;
	hr = this->CreateRenderTargetDepthAndViewport(width, height);
	if (FAILED(hr))
	{
		return hr;
	}

	camera2D.SetProjectionValues((float)width, (float)height, 0.0f, 1.0f);
	this->c64display.SetDisplaySize(!this->isWentFullscreen, width, height);
	isInitOK = true;
	return hr;
}
void Graphics::SetMenuKeepAliveHeightFromScreenHeight(int screenHeight)
{
	menukeepAliveHeight = (int)(screenHeight * KeepAliveTopPercentage / 100);
	mouseActiveDistance = dpi.ScaleY(20);
}

void Graphics::SetPixelFilter(bool usePointFilter)
{
	this->bUsePointFilter = usePointFilter;
	this->c64display.SetPixelFilter(usePointFilter);
}

void Graphics::SetVsyncMode(HCFG::FULLSCREENSYNCMODE syncMode)
{
	this->syncMode = syncMode;
	switch (syncMode)
	{
	case HCFG::FULLSCREENSYNCMODE::FSSM_VBL:
		useVsync = true;
		break;
	case HCFG::FULLSCREENSYNCMODE::FSSM_LINE:
		useVsync = false;
		break;
	case HCFG::FULLSCREENSYNCMODE::FSSM_FRAME_DOUBLER:
		useVsync = false;
		break;
	default:
		useVsync = true;
	}

}

void Graphics::SetVsync(bool useVsync)
{
	this->useVsync = useVsync;
}


HRESULT Graphics::LoadCbmCharTexture()
{
	HRESULT hr = E_FAIL;
	constexpr unsigned int bytesperpixel = 4;
	constexpr unsigned int bitmapsize = 8 * 8 * (16 * 2 + 1) * (16 * 2 + 1) * bytesperpixel;
	constexpr unsigned int pitchpixel = 8 * (16 * 2 + 1);
	constexpr unsigned int pitchbyte = pitchpixel * bytesperpixel;
	constexpr unsigned int bitmapheight = 8 * (16 * 2 + 1);
	constexpr unsigned int bitmapwidth = 8 * (16 * 2 + 1);
	bit8* pSet1 = nullptr;
	bit8* pSet2 = nullptr;
	try
	{
		CleanCharRomTextureSet();
		bit32 fg = VicIIPalette::Pepto[14];
		bit32 bg = VicIIPalette::Pepto[6];
		Color colorRgbOn = Color(fg);
		Color colorRgbOff = Color(bg);		
		colorRgbOn.SetA(0xff);
		colorRgbOff.SetA(0xff);
		DWORD colorDataOn = colorRgbOn.GetColorRef();
		DWORD colorDataOff = colorRgbOff.GetColorRef();
		for (;;)
		{
			pSet1 = (bit8*)GlobalAlloc(GPTR, bitmapsize);
			if (pSet1 == nullptr)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			{
				bit32* p = (bit32*)pSet1;
				for (int i = 0; i < bitmapsize / sizeof(bit32); i++, p++)
				{
					*p = colorDataOff;
				}
			}

			pSet2 = (bit8*)GlobalAlloc(GPTR, bitmapsize);
			if (pSet2 == nullptr)
			{
				hr = E_OUTOFMEMORY;
				break;
			}

			{
				bit32* p = (bit32*)pSet2;
				for (int i = 0; i < bitmapsize / sizeof(bit32); i++, p++)
				{
					*p = colorDataOff;
				}
			}

			bit8* prom = this->c64->GetRomCharPointer();
			if (prom == nullptr)
			{
				return E_FAIL;
			}

			for (int setnumber = 0; setnumber < 2; setnumber++)
			{
				bit8* psetsource;
				bit8* ptarget;
				if (setnumber == 0)
				{
					psetsource = prom;
					ptarget = pSet1;
				}
				else
				{
					psetsource = prom + 0x800;
					ptarget = pSet2;
				}

				bit8* psource = psetsource;
				for (unsigned int ch = 0; ch < 256; ch++)
				{
					unsigned int chx = ch % 16;
					chx = chx * 2 + 1;
					chx = chx * 8;

					unsigned int chy = ch / 16;
					chy = chy * 2 + 1;
					chy = chy * 8;
					for (unsigned int row = 0; row < 8; row++, psource++)
					{
						assert(psource == psetsource + (ch * 8 + row));
						bit8 gdata = psetsource[ch * 8 + row];
						bit32* p = (bit32*)&ptarget[pitchbyte * (chy + row) + bytesperpixel * chx];
						for (unsigned int col = 0; col < 8; col++, gdata <<= 1, p += 1)
						{
							if (gdata & 0x80)
							{
								*p = colorDataOn;
							}
							else
							{
								*p = colorDataOff;
							}
						}
					}
				}

				cbmCharRom[setnumber] = new Texture(this->device.Get(), (Color*)ptarget, bitmapwidth, bitmapheight, aiTextureType::aiTextureType_EMISSIVE);
			}

			hr = S_OK;
			break;
		}
	}
	catch (std::bad_alloc)
	{
		hr = E_OUTOFMEMORY;
	}
	catch (std::exception ex)
	{
		if (pSet1)
		{
			GlobalFree(pSet1);
			pSet1 = nullptr;
		}

		if (pSet2)
		{
			GlobalFree(pSet2);
			pSet1 = nullptr;
		}

		COM_ERROR_IF_FAILED(hr, ex.what());
		hr = E_FAIL;
	}

	if (pSet1)
	{
		GlobalFree(pSet1);
		pSet1 = nullptr;
	}

	if (pSet2)
	{
		GlobalFree(pSet2);
		pSet1 = nullptr;
	}

	return hr;
}