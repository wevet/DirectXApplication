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
	m_ClientWidth  = WINDOW_WIDTH;
	m_ClientHeight = WINDOW_HEIGHT;
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
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)) 
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
	return static_cast<int>(msg.wParam);
} 

bool DXApp::Initialize()
{
	if (!InitWindow()) 
	{
		return false;
	}
	if (!InitDirect3D())
	{
		return false;
	}
	return true;
}

bool DXApp::InitWindow()
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
		return false;
	}

	RECT rect = { 0,0, m_ClientWidth, m_ClientHeight };
	AdjustWindowRect(&rect, m_WndStyle, false);
	UINT width  = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;
	UINT x = GetSystemMetrics(SM_CXSCREEN) / 2 - width / 2;
	UINT y = GetSystemMetrics(SM_CXSCREEN) / 2 - height / 2;

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
		return false;
	}

	ShowWindow(m_hAppWnd, SW_SHOW);
	return true;
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

bool DXApp::InitDirect3D()
{
	UINT createDeviceFlags = 0;
#if defined(DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif //defined(DEBUG)
	
	D3D_DRIVER_TYPE driverTypes[] = 
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] = 
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_10_1,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// swap chainの構成設定
	DXGI_SWAP_CHAIN_DESC swapDesc;
	ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	swapDesc.BufferCount = m_SwapChainCount;
	swapDesc.BufferDesc.Width  = m_ClientWidth;
	swapDesc.BufferDesc.Height = m_ClientHeight;
	swapDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapDesc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
	swapDesc.OutputWindow = m_hAppWnd;
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapDesc.Windowed = true;
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;
	swapDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //alt enter full screen

	HRESULT result;
	for (int i = 0; i < numDriverTypes; ++i)
	{
		/*
		D3D11CreateDeviceAndSwapChain(
			nullptr,	// どのビデオアダプタを使用するか？既定ならばnullptrで、IDXGIAdapterのアドレスを渡す.
			D3D_DRIVER_TYPE_HARDWARE,	// ドライバのタイプを渡す。これ以外は基本的にソフトウェア実装で、どうしてもという時やデバグ用に用いるべし.
			nullptr,	// 上記をD3D_DRIVER_TYPE_SOFTWAREに設定した際に、その処理を行うDLLのハンドルを渡す。それ以外を指定している際には必ずnullptrを渡す.
			0,			// 何らかのフラグを指定する。詳しくはD3D11_CREATE_DEVICE列挙型で検索検索ぅ.
			nullptr,	// 実はここでD3D_FEATURE_LEVEL列挙型の配列を与える。nullptrにすることで上記featureと同等の内容の配列が使用される.
			0,			// 上記引数で、自分で定義した配列を与えていた場合、その配列の要素数をここに記述する.
			D3D11_SDK_VERSION,	// SDKのバージョン。必ずこの値.
			&desc,		// DXGI_SWAP_CHAIN_DESC構造体のアドレスを設定する。ここで設定した構造愛に設定されているパラメータでSwapChainが作成される.
			&swap_chain,	// 作成が成功した場合に、そのSwapChainのアドレスを格納するポインタ変数へのアドレス。ここで指定したポインタ変数経由でSwapChainを操作する.
			&device,	// 上記とほぼ同様で、こちらにはDeviceのポインタ変数のアドレスを設定する.
			&level,		// 実際に作成に成功したD3D_FEATURE_LEVELを格納するためのD3D_FEATURE_LEVEL列挙型変数のアドレスを設定する.
			&context	// SwapChainやDeviceと同様に、こちらにはContextのポインタ変数のアドレスを設定する.
		)		
		*/

		result = D3D11CreateDeviceAndSwapChain(
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

		if (SUCCEEDED(result))
		{
			m_DriverType = driverTypes[i];
			break;
		}
	}

	if (FAILED(result))
	{
		OutputDebugString("\n Failed to create device and swap chain \n");
		return false;
	}

	// バックバッファを取得.
	ID3D11Texture2D* m_pBackBufferTexture = 0;
	HR(m_pSwapChain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pBackBufferTexture)));
	HR(m_pDevice->CreateRenderTargetView(m_pBackBufferTexture, nullptr, &m_pRenderTargetView));
	Memory::SafeRelease(m_pBackBufferTexture);

	// bind render target view
	m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, nullptr);

	// viewport
	m_Viewport.Width  = static_cast<float>(m_ClientWidth);
	m_Viewport.Height = static_cast<float>(m_ClientHeight);
	m_Viewport.TopLeftX = 0;
	m_Viewport.TopLeftY = 0;
	m_Viewport.MinDepth = 0.0f;
	m_Viewport.MaxDepth = 1.0f;

	m_pImmediateContext->RSSetViewports(1, &m_Viewport);
	return true;
}

void DXApp::InitDirect3DInternal()
{
	//
}

