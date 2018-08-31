#include <tchar.h>
#include <Windows.h>
#include <chrono>
#include <thread>
#include "DXApp.h"

namespace
{
	// used forward msgs to user defined proc function
	DXApp* g_pApp = nullptr;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (g_pApp)
	{
		return g_pApp->MsgProc(hwnd, msg, wParam, lParam);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

DXApp::DXApp(HINSTANCE hInstance)
{
	m_hAppInstance = hInstance;
	m_hAppWnd      = NULL;
	m_AppTitle = "DirectX11 Application";
	m_WndStyle = WS_OVERLAPPEDWINDOW;
	m_SwapChainCount = 2;
	g_pApp = this;
	m_pDevice = nullptr;
	m_pImmediateContext = nullptr;
	m_pRenderTargetView = nullptr;
	m_pSwapChain = nullptr;
}

DXApp::~DXApp(void)
{
	if (m_pImmediateContext)
	{
		m_pImmediateContext->ClearState();
	}
	Memory::SafeRelease(m_pRenderTargetView);
	Memory::SafeRelease(m_pSwapChain);
	Memory::SafeRelease(m_pImmediateContext);
	Memory::SafeRelease(m_pDevice);
}

int DXApp::Run()
{
	MSG msg = { 0 };
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			Update(0.0f);
			Render(0.0f);
		}
	}
	return (int)msg.wParam;
} 

bool DXApp::Initialize()
{
	if (FAILED(InitWindow())) 
	{
		return false;
	}

	if (FAILED(InitDevice()))
	{
		OutputDebugString("\n Failed InitDevice \n");
		return false;
	}
	return true;
}

HRESULT DXApp::InitWindow()
{
	//WNDCLASSX
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = m_hAppInstance;
	wcex.lpfnWndProc = MainWndProc;
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName  = _T("DirectX11");
	wcex.lpszClassName = _T("DirectX11");
	wcex.hIcon   = LoadIcon(m_hAppInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hIconSm = LoadIcon(m_hAppInstance, IDI_APPLICATION);

	// window classの登録
	if (!RegisterClassEx(&wcex))
	{
		OutputDebugString("\n Failed to create window class \n");
		return E_FAIL;
	}

	UINT width = WINDOW_WIDTH;
	UINT height = WINDOW_HEIGHT;
	RECT rect = { 0,0, width, height, };
	AdjustWindowRect(&rect, m_WndStyle, false);

	//ウィンドウ生成
	m_hAppWnd = CreateWindow(
		wcex.lpszClassName,
		_T(m_AppTitle.c_str()),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		(rect.right - rect.left),
		(rect.bottom - rect.top),
		nullptr,
		nullptr,
		m_hAppInstance,
		nullptr);

	if (!m_hAppWnd)
	{
		OutputDebugString("\n Failed to create window \n");
		return E_FAIL;
	}

	ShowWindow(m_hAppWnd, SW_SHOW);
	return S_OK;
}

HRESULT DXApp::InitDevice()
{
	HRESULT hr = S_OK;
	RECT rect;
	GetClientRect(m_hAppWnd, &rect);
	UINT width  = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	UINT createDeviceFlags = 0;
#if defined(DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// swap chainの構成設定
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = m_SwapChainCount;
	swapDesc.BufferDesc.Width  = width;
	swapDesc.BufferDesc.Height = height;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapDesc.OutputWindow = m_hAppWnd;
	swapDesc.SampleDesc.Count   = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Windowed = true;

	for (int i = 0; i < numDriverTypes; ++i)
	{
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverTypes[i],
			nullptr,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&swapDesc,
			&m_pSwapChain,
			&m_pDevice,
			&m_FeatureLevel,
			&m_pImmediateContext);

		if (SUCCEEDED(hr))
		{
			m_DriverType = driverTypes[i];
			break;
		}
	}

	if (FAILED(hr))
	{
		OutputDebugString("\n Failed to create device and swap chain \n");
		return hr;
	}
	// user annotation
	m_pImmediateContext->QueryInterface(__uuidof(ID3DUserDefinedAnnotation), (void**)&m_pUserAnnotation);

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed GetBuffer \n");
		return hr;
	}

	hr = m_pDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
	Memory::SafeRelease(pBackBuffer);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed CreateRenderTargetView \n");
		return hr;
	}

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(D3D11_TEXTURE2D_DESC));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_pDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed D3D11_TEXTURE2D_DESC \n");
		return hr;
	}

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_pDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed D3D11_DEPTH_STENCIL_VIEW_DESC \n");
		return hr;
	}

	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	// Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC descDSS;
	ZeroMemory(&descDSS, sizeof(descDSS));
	descDSS.DepthEnable = TRUE;
	descDSS.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	descDSS.DepthFunc = D3D11_COMPARISON_LESS;
	descDSS.StencilEnable = FALSE;
	hr = m_pDevice->CreateDepthStencilState(&descDSS, &m_pDepthStencilState);
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed D3D11_DEPTH_STENCIL_DESC \n");
		return hr;
	}

	// viewport
	m_Viewport.Width  = (float)width;
	m_Viewport.Height = (float)height;
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;
	m_pImmediateContext->RSSetViewports(1, &m_Viewport);

	m_pWorld = XMMatrixIdentity();

	hr = InitApp();
	if (FAILED(hr))
	{
		OutputDebugString("\n Failed InitApp \n");
		return hr;
	}

	return S_OK;
}

HRESULT DXApp::InitApp()
{
	return E_NOTIMPL;
}

LRESULT DXApp::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
}


void DXApp::InitDirect3DInternal()
{
	//
}

HRESULT DXApp::CompileShaderFromFile(WCHAR * szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob ** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#if defined(DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
		{
			OutputDebugString("\n Failed pErrorBlob \n");
		}
		if (pErrorBlob) 
		{
			pErrorBlob->Release();
		}
		return hr;
	}
	if (pErrorBlob) 
	{
		pErrorBlob->Release();
	}

	return S_OK;
}

